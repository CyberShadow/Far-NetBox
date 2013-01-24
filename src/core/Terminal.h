//------------------------------------------------------------------------------
#ifndef TerminalH
#define TerminalH

#include <CoreDefs.hpp>
#include <Classes.hpp>

#include "SessionInfo.h"
#include "Interface.h"
#include "FileOperationProgress.h"
#include "FileMasks.h"
#include "Exceptions.h"
//------------------------------------------------------------------------------
class TCopyParamType;
class TFileOperationProgressType;
class TRemoteDirectory;
class TRemoteFile;
class TCustomFileSystem;
class TTunnelThread;
class TSecureShell;
struct TCalculateSizeParams;
struct TOverwriteFileParams;
struct TSynchronizeData;
struct TSynchronizeOptions;
class TSynchronizeChecklist;
struct TCalculateSizeStats;
struct TFileSystemInfo;
struct TSpaceAvailable;
struct TFilesFindParams;
class TTunnelUI;
class TCallbackGuard;
//------------------------------------------------------------------------------
DEFINE_CALLBACK_TYPE8(TQueryUserEvent, void,
  TObject * /* Sender */, const UnicodeString & /* Query */, TStrings * /* MoreMessages */ , unsigned int /* Answers */,
  const TQueryParams * /* Params */, unsigned int & /* Answer */, TQueryType /* QueryType */, void * /* Arg */);
DEFINE_CALLBACK_TYPE8(TPromptUserEvent, void,
  TTerminalIntf * /* Terminal */, TPromptKind /* Kind */, const UnicodeString & /* Name */, const UnicodeString & /* Instructions */,
  TStrings * /* Prompts */, TStrings * /* Results */, bool & /* Result */, void * /* Arg */);
DEFINE_CALLBACK_TYPE5(TDisplayBannerEvent, void,
  TTerminalIntf * /* Terminal */, UnicodeString /* SessionName */, const UnicodeString & /* Banner */,
  bool & /* NeverShowAgain */, int /* Options */);
DEFINE_CALLBACK_TYPE3(TExtendedExceptionEvent, void,
  TTerminalIntf * /* Terminal */, Exception * /* E */, void * /* Arg */);
DEFINE_CALLBACK_TYPE2(TReadDirectoryEvent, void, TObject * /* Sender */, Boolean /* ReloadOnly */);
DEFINE_CALLBACK_TYPE3(TReadDirectoryProgressEvent, void,
  TObject * /* Sender */, int /* Progress */, bool & /* Cancel */);
DEFINE_CALLBACK_TYPE3(TProcessFileEvent, void,
  const UnicodeString & /* FileName */, const TRemoteFile * /* File */, void * /* Param */);
DEFINE_CALLBACK_TYPE4(TProcessFileEventEx, void,
  const UnicodeString & /* FileName */, const TRemoteFile * /* File */, void * /* Param */, int /* Index */);
DEFINE_CALLBACK_TYPE2(TFileOperationEvent, int,
  void * /* Param1 */, void * /* Param2 */);
DEFINE_CALLBACK_TYPE4(TSynchronizeDirectoryEvent, void,
  const UnicodeString & /* LocalDirectory */, const UnicodeString & /* RemoteDirectory */,
  bool & /* Continue */, bool /* Collect */);
DEFINE_CALLBACK_TYPE2(TDeleteLocalFileEvent, void,
  const UnicodeString & /* FileName */, bool /* Alternative */);
DEFINE_CALLBACK_TYPE3(TDirectoryModifiedEvent, int,
  TTerminalIntf * /* Terminal */, const UnicodeString & /* Directory */, bool /* SubDirs */);
DEFINE_CALLBACK_TYPE4(TInformationEvent, void,
  TTerminalIntf * /* Terminal */, const UnicodeString & /* Str */, bool /* Status */, int /* Phase */);
DEFINE_CALLBACK_TYPE5(TCreateLocalFileEvent, HANDLE,
  const UnicodeString & /* FileName */, DWORD /* DesiredAccess */,
  DWORD /* ShareMode */, DWORD /* CreationDisposition */, DWORD /* FlagsAndAttributes */);
DEFINE_CALLBACK_TYPE1(TGetLocalFileAttributesEvent, DWORD,
  const UnicodeString & /* FileName */);
DEFINE_CALLBACK_TYPE2(TSetLocalFileAttributesEvent, BOOL,
  const UnicodeString & /* FileName */, DWORD /* FileAttributes */);
DEFINE_CALLBACK_TYPE3(TMoveLocalFileEvent, BOOL,
  const UnicodeString & /* FileName */, const UnicodeString & /* NewFileName */, DWORD /* Flags */);
DEFINE_CALLBACK_TYPE1(TRemoveLocalDirectoryEvent, BOOL,
  const UnicodeString & /* LocalDirName */);
DEFINE_CALLBACK_TYPE2(TCreateLocalDirectoryEvent, BOOL,
  const UnicodeString & /* LocalDirName */, LPSECURITY_ATTRIBUTES /* SecurityAttributes */);
//------------------------------------------------------------------------------
#define SUSPEND_OPERATION(Command)                            \
  {                                                           \
    TSuspendFileOperationProgress Suspend(OperationProgress); \
    Command                                                   \
  }

#define THROW_SKIP_FILE(EXCEPTION, MESSAGE) \
  throw EScpSkipFile(EXCEPTION, MESSAGE)
#define THROW_SKIP_FILE_NULL THROW_SKIP_FILE(NULL, L"")

/* TODO : Better user interface (query to user) */
#define FILE_OPERATION_LOOP_CUSTOM(TERMINAL, ALLOW_SKIP, MESSAGE, OPERATION) { \
  bool DoRepeat;                                                            \
  do {                                                                      \
    DoRepeat = false;                                                       \
    try {                                                                   \
      OPERATION;                                                            \
    }                                                                       \
    catch (EAbort &)                                                        \
    {                                                                       \
      throw;                                                                \
    }                                                                       \
    catch (EScpSkipFile &)                                                  \
    {                                                                       \
      throw;                                                                \
    }                                                                       \
    catch (EFatal &)                                                        \
    {                                                                       \
      throw;                                                                \
    }                                                                       \
    catch (EFileNotFoundError &)                                            \
    {                                                                       \
      throw;                                                                \
    }                                                                       \
    catch (EOSError &)                                                      \
    {                                                                       \
      throw;                                                                \
    }                                                                       \
    catch (Exception & E)                                                   \
    {                                                                       \
      TERMINAL->FileOperationLoopQuery(E, OperationProgress, MESSAGE, ALLOW_SKIP); \
      DoRepeat = true;                                                      \
    } \
  } while (DoRepeat); }

#define FILE_OPERATION_LOOP(MESSAGE, OPERATION) \
  FILE_OPERATION_LOOP_EX(True, MESSAGE, OPERATION)
//------------------------------------------------------------------------------
enum TCurrentFSProtocol { cfsUnknown, cfsSCP, cfsSFTP, cfsFTP, cfsFTPS, cfsWebDAV };
//------------------------------------------------------------------------------
const int cpDelete = 0x01;
const int cpTemporary = 0x04;
const int cpNoConfirmation = 0x08;
const int cpNewerOnly = 0x10;
const int cpAppend = 0x20;
const int cpResume = 0x40;
//------------------------------------------------------------------------------
const int ccApplyToDirectories = 0x01;
const int ccRecursive = 0x02;
const int ccUser = 0x100;
//------------------------------------------------------------------------------
const int csIgnoreErrors = 0x01;
//------------------------------------------------------------------------------
const int ropNoReadDirectory = 0x02;
//------------------------------------------------------------------------------
const int boDisableNeverShowAgain = 0x01;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
enum TSynchronizeParam
{
  spDelete = 0x01, // cannot be combined with spTimestamp
  spNoConfirmation = 0x02, // has no effect for spTimestamp
  spExistingOnly = 0x04, // is implicit for spTimestamp
  spNoRecurse = 0x08,
  spUseCache = 0x10, // cannot be combined with spTimestamp
  spDelayProgress = 0x20, // cannot be combined with spTimestamp
  spPreviewChanges = 0x40, // not used by core
  spSubDirs = 0x80, // cannot be combined with spTimestamp
  spTimestamp = 0x100,
  spNotByTime = 0x200, // cannot be combined with spTimestamp and smBoth
  spBySize = 0x400, // cannot be combined with smBoth, has opposite meaning for spTimestamp
  // 0x800 is reserved for GUI (spSelectedOnly)
  spMirror = 0x1000,
};
//------------------------------------------------------------------------------
class TTerminalIntf : public TSessionUI
{
public:
  virtual ~TTerminalIntf() = 0 {}

  // TScript::SynchronizeProc relies on the order
  enum TSynchronizeMode { smRemote, smLocal, smBoth };

  virtual void Open() = 0;
  virtual void Close() = 0;
  virtual void Reopen(int Params) = 0;
  virtual void DirectoryModified(const UnicodeString & Path, bool SubDirs) = 0;
  virtual void DirectoryLoaded(TRemoteFileList * FileList) = 0;
  virtual void ShowExtendedException(Exception * E) = 0;
  virtual void Idle() = 0;
  virtual void RecryptPasswords() = 0;
  virtual bool AllowedAnyCommand(const UnicodeString & Command) = 0;
  virtual void AnyCommand(const UnicodeString & Command, TCaptureOutputEvent OutputEvent) = 0;
  virtual void CloseOnCompletion(TOnceDoneOperation Operation = odoDisconnect, const UnicodeString & Message = L"") = 0;
  virtual UnicodeString & AbsolutePath(const UnicodeString & Path, bool Local) = 0;
  virtual void BeginTransaction() = 0;
  virtual void ReadCurrentDirectory() = 0;
  virtual void ReadDirectory(bool ReloadOnly, bool ForceCache = false) = 0;
  virtual TRemoteFileList * ReadDirectoryListing(const UnicodeString & Directory, const TFileMasks & Mask) = 0;
  virtual TRemoteFileList * CustomReadDirectoryListing(const UnicodeString & Directory, bool UseCache) = 0;
  virtual TRemoteFile * ReadFileListing(const UnicodeString & Path) = 0;
  virtual void ReadFile(const UnicodeString & FileName, TRemoteFile *& File) = 0;
  virtual bool FileExists(const UnicodeString & FileName, TRemoteFile ** File = NULL) = 0;
  virtual void ReadSymlink(TRemoteFile * SymlinkFile, TRemoteFile *& File) = 0;
  virtual bool CopyToLocal(TStrings * FilesToCopy,
    const UnicodeString & TargetDir, const TCopyParamType * CopyParam, int Params) = 0;
  virtual bool CopyToRemote(TStrings * FilesToCopy,
    const UnicodeString & TargetDir, const TCopyParamType * CopyParam, int Params) = 0;
  virtual void CreateDirectory(const UnicodeString & DirName,
    const TRemoteProperties * Properties = NULL) = 0;
  virtual void CreateLink(const UnicodeString & FileName, const UnicodeString & PointTo, bool Symbolic) = 0;
  virtual void DeleteFile(const UnicodeString & FileName,
    const TRemoteFile * File = NULL, void * Params = NULL) = 0;
  virtual bool DeleteFiles(TStrings * FilesToDelete, int Params = 0) = 0;
  virtual bool DeleteLocalFiles(TStrings * FileList, int Params = 0) = 0;
  virtual bool IsRecycledFile(const UnicodeString & FileName) = 0;
  virtual void CustomCommandOnFile(const UnicodeString & FileName,
    const TRemoteFile * File, void * AParams) = 0;
  virtual void CustomCommandOnFiles(UnicodeString Command, int Params,
    TStrings * Files, TCaptureOutputEvent OutputEvent) = 0;
  virtual void ChangeDirectory(const UnicodeString & Directory) = 0;
  virtual void EndTransaction() = 0;
  virtual void HomeDirectory() = 0;
  virtual void ChangeFileProperties(const UnicodeString & FileName,
    const TRemoteFile * File, /*const TRemoteProperties */ void * Properties) = 0;
  virtual void ChangeFilesProperties(TStrings * FileList,
    const TRemoteProperties * Properties) = 0;
  virtual bool LoadFilesProperties(TStrings * FileList) = 0;
  virtual void TerminalError(UnicodeString Msg) = 0;
  virtual void TerminalError(Exception * E, UnicodeString Msg) = 0;
  virtual void ReloadDirectory() = 0;
  virtual void RefreshDirectory() = 0;
  virtual void RenameFile(const UnicodeString & FileName, const UnicodeString & NewName) = 0;
  virtual void RenameFile(const TRemoteFile * File, const UnicodeString & NewName, bool CheckExistence) = 0;
  virtual void MoveFile(const UnicodeString & FileName, const TRemoteFile * File,
    /* const TMoveFileParams */ void * Param) = 0;
  virtual bool MoveFiles(TStrings * FileList, const UnicodeString & Target,
    const UnicodeString & FileMask) = 0;
  virtual void CopyFile(const UnicodeString & FileName, const TRemoteFile * File,
    /* const TMoveFileParams */ void * Param) = 0;
  virtual bool CopyFiles(TStrings * FileList, const UnicodeString & Target,
    const UnicodeString & FileMask) = 0;
  virtual void CalculateFilesSize(TStrings * FileList, __int64 & Size,
    int Params, const TCopyParamType * CopyParam = NULL, TCalculateSizeStats * Stats = NULL) = 0;
  virtual void CalculateFilesChecksum(const UnicodeString & Alg, TStrings * FileList,
    TStrings * Checksums, TCalculatedChecksumEvent OnCalculatedChecksum) = 0;
  virtual void ClearCaches() = 0;
  virtual TSynchronizeChecklist * SynchronizeCollect(const UnicodeString & LocalDirectory,
    const UnicodeString & RemoteDirectory, TSynchronizeMode Mode,
    const TCopyParamType * CopyParam, int Params,
    TSynchronizeDirectoryEvent OnSynchronizeDirectory, TSynchronizeOptions * Options) = 0;
  virtual void SynchronizeApply(TSynchronizeChecklist * Checklist,
    const UnicodeString & LocalDirectory, const UnicodeString & RemoteDirectory,
    const TCopyParamType * CopyParam, int Params,
    TSynchronizeDirectoryEvent OnSynchronizeDirectory) = 0;
  virtual void FilesFind(UnicodeString Directory, const TFileMasks & FileMask,
    TFileFoundEvent OnFileFound, TFindingFileEvent OnFindingFile) = 0;
  virtual void SpaceAvailable(const UnicodeString & Path, TSpaceAvailable & ASpaceAvailable) = 0;
  virtual bool DirectoryFileList(const UnicodeString & Path,
    TRemoteFileList *& FileList, bool CanLoad) = 0;
  virtual void MakeLocalFileList(const UnicodeString & FileName,
    const TSearchRec & Rec, void * Param) = 0;
  virtual bool FileOperationLoopQuery(Exception & E,
    TFileOperationProgressType * OperationProgress, const UnicodeString & Message,
    bool AllowSkip, const UnicodeString & SpecialRetry = UnicodeString()) = 0;
  virtual TUsableCopyParamAttrs UsableCopyParamAttrs(int Params) = 0;
  virtual bool QueryReopen(Exception * E, int Params,
    TFileOperationProgressType * OperationProgress) = 0;
  virtual UnicodeString & PeekCurrentDirectory() = 0;
  virtual void FatalAbort() = 0;

  virtual const TSessionInfo & GetSessionInfo() const = 0;
  virtual const TFileSystemInfo & GetFileSystemInfo(bool Retrieve = false) = 0;
  virtual void LogEvent(const UnicodeString & Str) = 0;

  virtual bool IsAbsolutePath(const UnicodeString & Path) = 0;
  virtual UnicodeString & ExpandFileName(const UnicodeString & Path,
    const UnicodeString & BasePath) = 0;

  virtual TSessionDataIntf * GetSessionData() = 0;
  virtual TSessionDataIntf * GetSessionData() const = 0;
  virtual TSessionLog * GetLog() = 0;
  virtual TActionLog * GetActionLog() = 0;
  virtual TConfiguration *GetConfiguration() = 0;
  virtual TSessionStatus GetStatus() = 0;
  virtual TRemoteDirectory * GetFiles() = 0;
  virtual TNotifyEvent & GetOnChangeDirectory() = 0;
  virtual void SetOnChangeDirectory(TNotifyEvent Value) = 0;
  virtual TReadDirectoryEvent & GetOnReadDirectory() = 0;
  virtual void SetOnReadDirectory(TReadDirectoryEvent Value) = 0;
  virtual TNotifyEvent & GetOnStartReadDirectory() = 0;
  virtual void SetOnStartReadDirectory(TNotifyEvent Value) = 0;
  virtual TReadDirectoryProgressEvent & GetOnReadDirectoryProgress() = 0;
  virtual void SetOnReadDirectoryProgress(TReadDirectoryProgressEvent Value) = 0;
  virtual TDeleteLocalFileEvent & GetOnDeleteLocalFile() = 0;
  virtual void SetOnDeleteLocalFile(TDeleteLocalFileEvent Value) = 0;
  virtual TCreateLocalFileEvent & GetOnCreateLocalFile() = 0;
  virtual void SetOnCreateLocalFile(TCreateLocalFileEvent Value) = 0;
  virtual TGetLocalFileAttributesEvent & GetOnGetLocalFileAttributes() = 0;
  virtual void SetOnGetLocalFileAttributes(TGetLocalFileAttributesEvent Value) = 0;
  virtual TSetLocalFileAttributesEvent & GetOnSetLocalFileAttributes() = 0;
  virtual void SetOnSetLocalFileAttributes(TSetLocalFileAttributesEvent Value) = 0;
  virtual TMoveLocalFileEvent & GetOnMoveLocalFile() = 0;
  virtual void SetOnMoveLocalFile(TMoveLocalFileEvent Value) = 0;
  virtual TRemoveLocalDirectoryEvent & GetOnRemoveLocalDirectory() = 0;
  virtual void SetOnRemoveLocalDirectory(TRemoveLocalDirectoryEvent Value) = 0;
  virtual TCreateLocalDirectoryEvent & GetOnCreateLocalDirectory() = 0;
  virtual void SetOnCreateLocalDirectory(TCreateLocalDirectoryEvent Value) = 0;
  virtual TFileOperationProgressEvent & GetOnProgress() = 0;
  virtual void SetOnProgress(TFileOperationProgressEvent Value) = 0;
  virtual TFileOperationFinishedEvent & GetOnFinished() = 0;
  virtual void SetOnFinished(TFileOperationFinishedEvent Value) = 0;
  virtual TCurrentFSProtocol GetFSProtocol() = 0;
  virtual bool GetUseBusyCursor() = 0;
  virtual void SetUseBusyCursor(bool Value) = 0;
  virtual bool GetAutoReadDirectory() = 0;
  virtual void SetAutoReadDirectory(bool Value) = 0;
  virtual TStrings * GetFixedPaths() = 0;
  virtual TQueryUserEvent & GetOnQueryUser() = 0;
  virtual void SetOnQueryUser(TQueryUserEvent Value) = 0;
  virtual TPromptUserEvent & GetOnPromptUser() = 0;
  virtual void SetOnPromptUser(TPromptUserEvent Value) = 0;
  virtual TDisplayBannerEvent & GetOnDisplayBanner() = 0;
  virtual void SetOnDisplayBanner(TDisplayBannerEvent Value) = 0;
  virtual TExtendedExceptionEvent & GetOnShowExtendedException() = 0;
  virtual void SetOnShowExtendedException(TExtendedExceptionEvent Value) = 0;
  virtual TInformationEvent & GetOnInformation() = 0;
  virtual void SetOnInformation(TInformationEvent Value) = 0;
  virtual TNotifyEvent & GetOnClose() = 0;
  virtual void SetOnClose(TNotifyEvent Value) = 0;
  virtual int GetTunnelLocalPortNumber() = 0;

  virtual bool PromptUser(TSessionDataIntf * Data, TPromptKind Kind,
    const UnicodeString & Name, const UnicodeString & Instructions,
    const UnicodeString & Prompt, bool Echo,
    int MaxLen, UnicodeString & Result) = 0;

  // virtual void SetMasks(const UnicodeString & Value) = 0;
  virtual UnicodeString & GetCurrentDirectory() = 0;
  virtual bool GetExceptionOnFail() const = 0;
  virtual TRemoteTokenList * GetGroups() = 0;
  virtual TRemoteTokenList * GetUsers() = 0;
  virtual TRemoteTokenList * GetMembership() = 0;
  virtual void SetCurrentDirectory(const UnicodeString & Value) = 0;
  virtual void SetExceptionOnFail(bool Value) = 0;
  virtual const UnicodeString & GetUserName() const = 0;
  virtual bool GetAreCachesEmpty() const = 0;
  virtual bool GetIsCapable(TFSCapability Capability) const = 0;
  virtual void ClearCachedFileList(const UnicodeString & Path, bool SubDirs) = 0;
  virtual void AddCachedFileList(TRemoteFileList * FileList) = 0;
  virtual bool GetCommandSessionOpened() = 0;
  virtual TTerminalIntf * GetCommandSession() = 0;
  virtual bool GetResolvingSymlinks() = 0;
  virtual bool GetActive() = 0;
  virtual UnicodeString & GetPassword() = 0;
  virtual UnicodeString & GetTunnelPassword() = 0;
  virtual bool GetStoredCredentialsTried() = 0;

  virtual void AnnounceFileListOperation() = 0;
  virtual UnicodeString & TranslateLockedPath(const UnicodeString & Path, bool Lock) = 0;
  virtual void CommandError(Exception * E, const UnicodeString & Msg) = 0;
  virtual unsigned int CommandError(Exception * E, const UnicodeString & Msg, unsigned int Answers) = 0;
  virtual void ReactOnCommand(int /*TFSCommand*/ Cmd) = 0;

  virtual void DoReadDirectory(bool ReloadOnly) = 0;
  virtual void DoCreateDirectory(const UnicodeString & DirName) = 0;
  virtual void DoDeleteFile(const UnicodeString & FileName, const TRemoteFile * File,
    int Params) = 0;
  virtual void DoCustomCommandOnFile(UnicodeString FileName,
    const TRemoteFile * File, UnicodeString Command, int Params, TCaptureOutputEvent OutputEvent) = 0;
  virtual void DoRenameFile(const UnicodeString & FileName,
    const UnicodeString & NewName, bool Move) = 0;
  virtual void DoCopyFile(const UnicodeString & FileName, const UnicodeString & NewName) = 0;
  virtual void DoChangeFileProperties(const UnicodeString & vFileName,
    const TRemoteFile * File, const TRemoteProperties * Properties) = 0;
  virtual void DoChangeDirectory() = 0;
  virtual void EnsureNonExistence(const UnicodeString & FileName) = 0;
  virtual void LookupUsersGroups() = 0;
  virtual void FileModified(const TRemoteFile * File,
    const UnicodeString & FileName, bool ClearDirectoryChange = false) = 0;
  virtual int FileOperationLoop(TFileOperationEvent CallBackFunc,
    TFileOperationProgressType * OperationProgress, bool AllowSkip,
    const UnicodeString & Message, void * Param1 = NULL, void * Param2 = NULL) = 0;
  virtual bool ProcessFiles(TStrings * FileList, TFileOperation Operation,
    TProcessFileEvent ProcessFile, void * Param = NULL, TOperationSide Side = osRemote,
    bool Ex = false) = 0;
  virtual bool ProcessFilesEx(TStrings * FileList, TFileOperation Operation,
    TProcessFileEventEx ProcessFile, void * Param = NULL, TOperationSide Side = osRemote) = 0;
  virtual void ProcessDirectory(const UnicodeString & DirName,
    TProcessFileEvent CallBackFunc, void * Param = NULL, bool UseCache = false,
    bool IgnoreErrors = false) = 0;
  virtual void CalculateFileSize(const UnicodeString & FileName,
    const TRemoteFile * File, /*TCalculateSizeParams*/ void * Size) = 0;
  virtual void DoCalculateDirectorySize(const UnicodeString & FileName,
    const TRemoteFile * File, TCalculateSizeParams * Params) = 0;
  virtual void CalculateLocalFileSize(const UnicodeString & FileName,
    const TSearchRec & Rec, /*__int64*/ void * Params) = 0;
  virtual void CalculateLocalFilesSize(TStrings * FileList, __int64 & Size,
    const TCopyParamType * CopyParam = NULL) = 0;
  virtual TBatchOverwrite EffectiveBatchOverwrite(
    int Params, TFileOperationProgressType * OperationProgress, bool Special) = 0;
  virtual bool CheckRemoteFile(int Params, TFileOperationProgressType * OperationProgress) = 0;
  virtual unsigned int ConfirmFileOverwrite(const UnicodeString & FileName,
    const TOverwriteFileParams * FileParams, unsigned int Answers, const TQueryParams * QueryParams,
    TOperationSide Side, int Params, TFileOperationProgressType * OperationProgress,
    UnicodeString Message = L"") = 0;
  virtual void DoSynchronizeCollectDirectory(const UnicodeString & LocalDirectory,
    const UnicodeString & RemoteDirectory, TSynchronizeMode Mode,
    const TCopyParamType * CopyParam, int Params,
    TSynchronizeDirectoryEvent OnSynchronizeDirectory,
    TSynchronizeOptions * Options, int Level, TSynchronizeChecklist * Checklist) = 0;
  virtual void SynchronizeCollectFile(const UnicodeString & FileName,
    const TRemoteFile * File, /*TSynchronizeData*/ void * Param) = 0;
  virtual void SynchronizeRemoteTimestamp(const UnicodeString & FileName,
    const TRemoteFile * File, void * Param) = 0;
  virtual void SynchronizeLocalTimestamp(const UnicodeString & FileName,
    const TRemoteFile * File, void * Param) = 0;
  virtual void DoSynchronizeProgress(const TSynchronizeData & Data, bool Collect) = 0;
  virtual void DeleteLocalFile(const UnicodeString & FileName,
    const TRemoteFile * File, void * Param) = 0;
  virtual void RecycleFile(const UnicodeString & FileName, const TRemoteFile * File) = 0;
  virtual void DoStartup() = 0;
  virtual bool DoQueryReopen(Exception * E) = 0;
  virtual void FatalError(Exception * E, const UnicodeString & Msg) = 0;
  virtual void ResetConnection() = 0;
  virtual bool DoPromptUser(TSessionDataIntf * Data, TPromptKind Kind,
    const UnicodeString & Name, const UnicodeString & Instructions, TStrings * Prompts,
    TStrings * Response) = 0;
  virtual void OpenTunnel() = 0;
  virtual void CloseTunnel() = 0;
  virtual void DoInformation(const UnicodeString & Str, bool Status, int Phase = -1) = 0;
  virtual UnicodeString & FileUrl(const UnicodeString & FileName) = 0;
  virtual UnicodeString & FileUrl(const UnicodeString & Protocol, const UnicodeString & FileName) = 0;
  virtual void FileFind(const UnicodeString & FileName, const TRemoteFile * File, void * Param) = 0;
  virtual void DoFilesFind(UnicodeString Directory, TFilesFindParams & Params) = 0;
  virtual bool DoCreateLocalFile(const UnicodeString & FileName,
    TFileOperationProgressType * OperationProgress, HANDLE * AHandle,
    bool NoConfirmation) = 0;

  virtual void Information(const UnicodeString & Str, bool Status) = 0;
  virtual unsigned int QueryUser(const UnicodeString & Query,
    TStrings * MoreMessages, unsigned int Answers, const TQueryParams * Params,
    TQueryType QueryType = qtConfirmation) = 0;
  virtual unsigned int QueryUserException(const UnicodeString & Query,
    Exception * E, unsigned int Answers, const TQueryParams * Params,
    TQueryType QueryType = qtConfirmation) = 0;
  virtual void DisplayBanner(const UnicodeString & Banner) = 0;
  virtual void Closed() = 0;
  virtual void HandleExtendedException(Exception * E) = 0;
  virtual bool IsListenerFree(unsigned int PortNumber) = 0;

  virtual void DoReadDirectoryProgress(int Progress, bool & Cancel) = 0;
  virtual void SetOperationProgress(TFileOperationProgressType * AOperationProgress) = 0;
  virtual TFileOperationProgressType * GetOperationProgress() = 0;
  virtual void ReadDirectory(TRemoteFileList * FileList) = 0;
  virtual void CustomReadDirectory(TRemoteFileList * FileList) = 0;
  virtual void DoCreateLink(const UnicodeString & FileName,
    const UnicodeString & PointTo, bool Symbolic) = 0;
  virtual bool CreateLocalFile(const UnicodeString & FileName,
    TFileOperationProgressType * OperationProgress, HANDLE * AHandle,
    bool NoConfirmation) = 0;
  virtual void OpenLocalFile(const UnicodeString & FileName, unsigned int Access,
    int * Attrs, HANDLE * Handle, __int64 * ACTime, __int64 * MTime,
    __int64 * ATime, __int64 * Size, bool TryWriteReadOnly = true) = 0;
  virtual bool AllowLocalFileTransfer(const UnicodeString & FileName,
    const TCopyParamType *CopyParam) = 0;
  virtual bool HandleException(Exception * E) = 0;

  virtual void DoProgress(TFileOperationProgressType & ProgressData, TCancelStatus & Cancel) = 0;
  virtual void DoFinished(TFileOperation Operation, TOperationSide Side, bool Temp,
    const UnicodeString & FileName, bool Success, TOnceDoneOperation & OnceDoneOperation) = 0;
  virtual void RollbackAction(TSessionAction & Action,
    TFileOperationProgressType * OperationProgress, Exception * E = NULL) = 0;
  virtual void DoAnyCommand(const UnicodeString & Command, TCaptureOutputEvent OutputEvent,
    TCallSessionAction * Action) = 0;
  virtual TRemoteFileList * DoReadDirectoryListing(UnicodeString Directory, bool UseCache) = 0;
  virtual RawByteString & EncryptPassword(const UnicodeString & Password) = 0;
  virtual UnicodeString & DecryptPassword(const RawByteString & Password) = 0;

  virtual void SetLocalFileTime(const UnicodeString & LocalFileName,
    const TDateTime & Modification) = 0;
  virtual void SetLocalFileTime(const UnicodeString & LocalFileName,
    FILETIME * AcTime, FILETIME * WrTime) = 0;
  virtual HANDLE CreateLocalFile(const UnicodeString & LocalFileName, DWORD DesiredAccess,
    DWORD ShareMode, DWORD CreationDisposition, DWORD FlagsAndAttributes) = 0;
  virtual DWORD GetLocalFileAttributes(const UnicodeString & LocalFileName) = 0;
  virtual BOOL SetLocalFileAttributes(const UnicodeString & LocalFileName, DWORD FileAttributes) = 0;
  virtual BOOL MoveLocalFile(const UnicodeString & LocalFileName, const UnicodeString & NewLocalFileName, DWORD Flags) = 0;
  virtual BOOL RemoveLocalDirectory(const UnicodeString & LocalDirName) = 0;
  virtual BOOL CreateLocalDirectory(const UnicodeString & LocalDirName, LPSECURITY_ATTRIBUTES SecurityAttributes) = 0;

  virtual TCustomFileSystem * GetFileSystem() = 0;
  virtual UnicodeString & GetSessionUrl() = 0;

};
//------------------------------------------------------------------------------
class TTerminal : public TTerminalIntf
{
public:
// friend class TSCPFileSystem;
// friend class TSFTPFileSystem;
// friend class TFTPFileSystem;
// friend class TWebDAVFileSystem;
friend class TTunnelUI;
friend class TCallbackGuard;

public:
  virtual ~TTerminal();
  // TTerminalIntf implementation
  virtual void Open();
  virtual void Close();
  virtual void Reopen(int Params);
  virtual void DirectoryModified(const UnicodeString & Path, bool SubDirs);
  virtual void DirectoryLoaded(TRemoteFileList * FileList);
  virtual void ShowExtendedException(Exception * E);
  virtual void Idle();
  virtual void RecryptPasswords();
  virtual bool AllowedAnyCommand(const UnicodeString & Command);
  virtual void AnyCommand(const UnicodeString & Command, TCaptureOutputEvent OutputEvent);
  virtual void CloseOnCompletion(TOnceDoneOperation Operation = odoDisconnect, const UnicodeString & Message = L"");
  virtual UnicodeString & AbsolutePath(const UnicodeString & Path, bool Local);
  virtual void BeginTransaction();
  virtual void ReadCurrentDirectory();
  virtual void ReadDirectory(bool ReloadOnly, bool ForceCache = false);
  virtual TRemoteFileList * ReadDirectoryListing(const UnicodeString & Directory, const TFileMasks & Mask);
  virtual TRemoteFileList * CustomReadDirectoryListing(const UnicodeString & Directory, bool UseCache);
  virtual TRemoteFile * ReadFileListing(const UnicodeString & Path);
  virtual void ReadFile(const UnicodeString & FileName, TRemoteFile *& File);
  virtual bool FileExists(const UnicodeString & FileName, TRemoteFile ** File = NULL);
  virtual void ReadSymlink(TRemoteFile * SymlinkFile, TRemoteFile *& File);
  virtual bool CopyToLocal(TStrings * FilesToCopy,
    const UnicodeString & TargetDir, const TCopyParamType * CopyParam, int Params);
  virtual bool CopyToRemote(TStrings * FilesToCopy,
    const UnicodeString & TargetDir, const TCopyParamType * CopyParam, int Params);
  virtual void CreateDirectory(const UnicodeString & DirName,
    const TRemoteProperties * Properties = NULL);
  virtual void CreateLink(const UnicodeString & FileName, const UnicodeString & PointTo, bool Symbolic);
  virtual void DeleteFile(const UnicodeString & FileName,
    const TRemoteFile * File = NULL, void * Params = NULL);
  virtual bool DeleteFiles(TStrings * FilesToDelete, int Params = 0);
  virtual bool DeleteLocalFiles(TStrings * FileList, int Params = 0);
  virtual bool IsRecycledFile(const UnicodeString & FileName);
  virtual void CustomCommandOnFile(const UnicodeString & FileName,
    const TRemoteFile * File, void * AParams);
  virtual void CustomCommandOnFiles(UnicodeString Command, int Params,
    TStrings * Files, TCaptureOutputEvent OutputEvent);
  virtual void ChangeDirectory(const UnicodeString & Directory);
  virtual void EndTransaction();
  virtual void HomeDirectory();
  virtual void ChangeFileProperties(const UnicodeString & FileName,
    const TRemoteFile * File, /*const TRemoteProperties */ void * Properties);
  virtual void ChangeFilesProperties(TStrings * FileList,
    const TRemoteProperties * Properties);
  virtual bool LoadFilesProperties(TStrings * FileList);
  virtual void TerminalError(UnicodeString Msg);
  virtual void TerminalError(Exception * E, UnicodeString Msg);
  virtual void ReloadDirectory();
  virtual void RefreshDirectory();
  virtual void RenameFile(const UnicodeString & FileName, const UnicodeString & NewName);
  virtual void RenameFile(const TRemoteFile * File, const UnicodeString & NewName, bool CheckExistence);
  virtual void MoveFile(const UnicodeString & FileName, const TRemoteFile * File,
    /* const TMoveFileParams */ void * Param);
  virtual bool MoveFiles(TStrings * FileList, const UnicodeString & Target,
    const UnicodeString & FileMask);
  virtual void CopyFile(const UnicodeString & FileName, const TRemoteFile * File,
    /* const TMoveFileParams */ void * Param);
  virtual bool CopyFiles(TStrings * FileList, const UnicodeString & Target,
    const UnicodeString & FileMask);
  virtual void CalculateFilesSize(TStrings * FileList, __int64 & Size,
    int Params, const TCopyParamType * CopyParam = NULL, TCalculateSizeStats * Stats = NULL);
  virtual void CalculateFilesChecksum(const UnicodeString & Alg, TStrings * FileList,
    TStrings * Checksums, TCalculatedChecksumEvent OnCalculatedChecksum);
  virtual void ClearCaches();
  virtual TSynchronizeChecklist * SynchronizeCollect(const UnicodeString & LocalDirectory,
    const UnicodeString & RemoteDirectory, TSynchronizeMode Mode,
    const TCopyParamType * CopyParam, int Params,
    TSynchronizeDirectoryEvent OnSynchronizeDirectory, TSynchronizeOptions * Options);
  virtual void SynchronizeApply(TSynchronizeChecklist * Checklist,
    const UnicodeString & LocalDirectory, const UnicodeString & RemoteDirectory,
    const TCopyParamType * CopyParam, int Params,
    TSynchronizeDirectoryEvent OnSynchronizeDirectory);
  virtual void FilesFind(UnicodeString Directory, const TFileMasks & FileMask,
    TFileFoundEvent OnFileFound, TFindingFileEvent OnFindingFile);
  virtual void SpaceAvailable(const UnicodeString & Path, TSpaceAvailable & ASpaceAvailable);
  virtual bool DirectoryFileList(const UnicodeString & Path,
    TRemoteFileList *& FileList, bool CanLoad);
  virtual void MakeLocalFileList(const UnicodeString & FileName,
    const TSearchRec & Rec, void * Param);
  virtual bool FileOperationLoopQuery(Exception & E,
    TFileOperationProgressType * OperationProgress, const UnicodeString & Message,
    bool AllowSkip, const UnicodeString & SpecialRetry = UnicodeString());
  virtual TUsableCopyParamAttrs UsableCopyParamAttrs(int Params);
  virtual bool QueryReopen(Exception * E, int Params,
    TFileOperationProgressType * OperationProgress);
  virtual UnicodeString & PeekCurrentDirectory();
  virtual void FatalAbort();

  virtual const TSessionInfo & GetSessionInfo() const;
  virtual const TFileSystemInfo & GetFileSystemInfo(bool Retrieve = false);
  virtual void LogEvent(const UnicodeString & Str);

  virtual bool IsAbsolutePath(const UnicodeString & Path);
  virtual UnicodeString & ExpandFileName(const UnicodeString & Path,
    const UnicodeString & BasePath);

  virtual TSessionDataIntf * GetSessionData() { return FSessionData; }
  virtual TSessionData * GetSessionData() const { return FSessionData; }
  virtual TSessionLog * GetLog() { return FLog; }
  virtual TActionLog * GetActionLog() { return FActionLog; };
  virtual TConfiguration *GetConfiguration() { return FConfiguration; }
  virtual TSessionStatus GetStatus() { return FStatus; }
  virtual TRemoteDirectory * GetFiles() { return FFiles; }
  virtual TNotifyEvent & GetOnChangeDirectory() { return FOnChangeDirectory; }
  virtual void SetOnChangeDirectory(TNotifyEvent Value) { FOnChangeDirectory = Value; }
  virtual TReadDirectoryEvent & GetOnReadDirectory() { return FOnReadDirectory; }
  virtual void SetOnReadDirectory(TReadDirectoryEvent Value) { FOnReadDirectory = Value; }
  virtual TNotifyEvent & GetOnStartReadDirectory() { return FOnStartReadDirectory; }
  virtual void SetOnStartReadDirectory(TNotifyEvent Value) { FOnStartReadDirectory = Value; }
  virtual TReadDirectoryProgressEvent & GetOnReadDirectoryProgress() { return FOnReadDirectoryProgress; }
  virtual void SetOnReadDirectoryProgress(TReadDirectoryProgressEvent Value) { FOnReadDirectoryProgress = Value; }
  virtual TDeleteLocalFileEvent & GetOnDeleteLocalFile() { return FOnDeleteLocalFile; }
  virtual void SetOnDeleteLocalFile(TDeleteLocalFileEvent Value) { FOnDeleteLocalFile = Value; }
  virtual TCreateLocalFileEvent & GetOnCreateLocalFile() { return FOnCreateLocalFile; }
  virtual void SetOnCreateLocalFile(TCreateLocalFileEvent Value) { FOnCreateLocalFile = Value; }
  virtual TGetLocalFileAttributesEvent & GetOnGetLocalFileAttributes() { return FOnGetLocalFileAttributes; }
  virtual void SetOnGetLocalFileAttributes(TGetLocalFileAttributesEvent Value) { FOnGetLocalFileAttributes = Value; }
  virtual TSetLocalFileAttributesEvent & GetOnSetLocalFileAttributes() { return FOnSetLocalFileAttributes; }
  virtual void SetOnSetLocalFileAttributes(TSetLocalFileAttributesEvent Value) { FOnSetLocalFileAttributes = Value; }
  virtual TMoveLocalFileEvent & GetOnMoveLocalFile() { return FOnMoveLocalFile; }
  virtual void SetOnMoveLocalFile(TMoveLocalFileEvent Value) { FOnMoveLocalFile = Value; }
  virtual TRemoveLocalDirectoryEvent & GetOnRemoveLocalDirectory() { return FOnRemoveLocalDirectory; }
  virtual void SetOnRemoveLocalDirectory(TRemoveLocalDirectoryEvent Value) { FOnRemoveLocalDirectory = Value; }
  virtual TCreateLocalDirectoryEvent & GetOnCreateLocalDirectory() { return FOnCreateLocalDirectory; }
  virtual void SetOnCreateLocalDirectory(TCreateLocalDirectoryEvent Value) { FOnCreateLocalDirectory = Value; }
  virtual TFileOperationProgressEvent & GetOnProgress() { return FOnProgress; }
  virtual void SetOnProgress(TFileOperationProgressEvent Value) { FOnProgress = Value; }
  virtual TFileOperationFinishedEvent & GetOnFinished() { return FOnFinished; }
  virtual void SetOnFinished(TFileOperationFinishedEvent Value) { FOnFinished = Value; }
  virtual TCurrentFSProtocol GetFSProtocol() { return FFSProtocol; }
  virtual bool GetUseBusyCursor() { return FUseBusyCursor; }
  virtual void SetUseBusyCursor(bool Value) { FUseBusyCursor = Value; }
  virtual bool GetAutoReadDirectory() { return FAutoReadDirectory; }
  virtual void SetAutoReadDirectory(bool Value) { FAutoReadDirectory = Value; }
  virtual TStrings * GetFixedPaths();
  virtual TQueryUserEvent & GetOnQueryUser() { return FOnQueryUser; }
  virtual void SetOnQueryUser(TQueryUserEvent Value) { FOnQueryUser = Value; }
  virtual TPromptUserEvent & GetOnPromptUser() { return FOnPromptUser; }
  virtual void SetOnPromptUser(TPromptUserEvent Value) { FOnPromptUser = Value; }
  virtual TDisplayBannerEvent & GetOnDisplayBanner() { return FOnDisplayBanner; }
  virtual void SetOnDisplayBanner(TDisplayBannerEvent Value) { FOnDisplayBanner = Value; }
  virtual TExtendedExceptionEvent & GetOnShowExtendedException() { return FOnShowExtendedException; }
  virtual void SetOnShowExtendedException(TExtendedExceptionEvent Value) { FOnShowExtendedException = Value; }
  virtual TInformationEvent & GetOnInformation() { return FOnInformation; }
  virtual void SetOnInformation(TInformationEvent Value) { FOnInformation = Value; }
  virtual TNotifyEvent & GetOnClose() { return FOnClose; }
  virtual void SetOnClose(TNotifyEvent Value) { FOnClose = Value; }
  virtual int GetTunnelLocalPortNumber() { return FTunnelLocalPortNumber; }

  virtual bool PromptUser(TSessionDataIntf * Data, TPromptKind Kind,
    const UnicodeString & Name, const UnicodeString & Instructions,
    const UnicodeString & Prompt, bool Echo,
    int MaxLen, UnicodeString & Result);
  virtual bool PromptUser(TSessionDataIntf * Data, TPromptKind Kind,
    const UnicodeString & Name, const UnicodeString & Instructions,
    TStrings * Prompts, TStrings * Results);

  // virtual void SetMasks(const UnicodeString & Value);
  virtual UnicodeString & GetCurrentDirectory();
  virtual bool GetExceptionOnFail() const;
  virtual TRemoteTokenList * GetGroups();
  virtual TRemoteTokenList * GetUsers();
  virtual TRemoteTokenList * GetMembership();
  virtual void SetCurrentDirectory(const UnicodeString & Value);
  virtual void SetExceptionOnFail(bool Value);
  virtual UnicodeString & GetUserName() const;
  virtual bool GetAreCachesEmpty() const;
  virtual bool GetIsCapable(TFSCapability Capability) const;
  virtual void ClearCachedFileList(const UnicodeString & Path, bool SubDirs);
  virtual void AddCachedFileList(TRemoteFileList * FileList);
  virtual bool GetCommandSessionOpened();
  virtual TTerminalIntf * GetCommandSession();
  virtual bool GetResolvingSymlinks();
  virtual bool GetActive();
  virtual UnicodeString & GetPassword();
  virtual UnicodeString & GetTunnelPassword();
  virtual bool GetStoredCredentialsTried();

  virtual void AnnounceFileListOperation();
  virtual UnicodeString & TranslateLockedPath(const UnicodeString & Path, bool Lock);
  virtual void CommandError(Exception * E, const UnicodeString & Msg);
  virtual unsigned int CommandError(Exception * E, const UnicodeString & Msg, unsigned int Answers);
  virtual void ReactOnCommand(int /*TFSCommand*/ Cmd);

  virtual void DoReadDirectory(bool ReloadOnly);
  virtual void DoCreateDirectory(const UnicodeString & DirName);
  virtual void DoDeleteFile(const UnicodeString & FileName, const TRemoteFile * File,
    int Params);
  virtual void DoCustomCommandOnFile(UnicodeString FileName,
    const TRemoteFile * File, UnicodeString Command, int Params, TCaptureOutputEvent OutputEvent);
  virtual void DoRenameFile(const UnicodeString & FileName,
    const UnicodeString & NewName, bool Move);
  virtual void DoCopyFile(const UnicodeString & FileName, const UnicodeString & NewName);
  virtual void DoChangeFileProperties(const UnicodeString & vFileName,
    const TRemoteFile * File, const TRemoteProperties * Properties);
  virtual void DoChangeDirectory();
  virtual void EnsureNonExistence(const UnicodeString & FileName);
  virtual void LookupUsersGroups();
  virtual void FileModified(const TRemoteFile * File,
    const UnicodeString & FileName, bool ClearDirectoryChange = false);
  virtual int FileOperationLoop(TFileOperationEvent CallBackFunc,
    TFileOperationProgressType * OperationProgress, bool AllowSkip,
    const UnicodeString & Message, void * Param1 = NULL, void * Param2 = NULL);
  virtual bool ProcessFiles(TStrings * FileList, TFileOperation Operation,
    TProcessFileEvent ProcessFile, void * Param = NULL, TOperationSide Side = osRemote,
    bool Ex = false);
  virtual bool ProcessFilesEx(TStrings * FileList, TFileOperation Operation,
    TProcessFileEventEx ProcessFile, void * Param = NULL, TOperationSide Side = osRemote);
  virtual void ProcessDirectory(const UnicodeString & DirName,
    TProcessFileEvent CallBackFunc, void * Param = NULL, bool UseCache = false,
    bool IgnoreErrors = false);
  virtual void CalculateFileSize(const UnicodeString & FileName,
    const TRemoteFile * File, /*TCalculateSizeParams*/ void * Size);
  virtual void DoCalculateDirectorySize(const UnicodeString & FileName,
    const TRemoteFile * File, TCalculateSizeParams * Params);
  virtual void CalculateLocalFileSize(const UnicodeString & FileName,
    const TSearchRec & Rec, /*__int64*/ void * Params);
  virtual void CalculateLocalFilesSize(TStrings * FileList, __int64 & Size,
    const TCopyParamType * CopyParam = NULL);
  virtual TBatchOverwrite EffectiveBatchOverwrite(
    int Params, TFileOperationProgressType * OperationProgress, bool Special);
  virtual bool CheckRemoteFile(int Params, TFileOperationProgressType * OperationProgress);
  virtual unsigned int ConfirmFileOverwrite(const UnicodeString & FileName,
    const TOverwriteFileParams * FileParams, unsigned int Answers, const TQueryParams * QueryParams,
    TOperationSide Side, int Params, TFileOperationProgressType * OperationProgress,
    UnicodeString Message = L"");
  virtual void DoSynchronizeCollectDirectory(const UnicodeString & LocalDirectory,
    const UnicodeString & RemoteDirectory, TSynchronizeMode Mode,
    const TCopyParamType * CopyParam, int Params,
    TSynchronizeDirectoryEvent OnSynchronizeDirectory,
    TSynchronizeOptions * Options, int Level, TSynchronizeChecklist * Checklist);
  virtual void SynchronizeCollectFile(const UnicodeString & FileName,
    const TRemoteFile * File, /*TSynchronizeData*/ void * Param);
  virtual void SynchronizeRemoteTimestamp(const UnicodeString & FileName,
    const TRemoteFile * File, void * Param);
  virtual void SynchronizeLocalTimestamp(const UnicodeString & FileName,
    const TRemoteFile * File, void * Param);
  virtual void DoSynchronizeProgress(const TSynchronizeData & Data, bool Collect);
  virtual void DeleteLocalFile(const UnicodeString & FileName,
    const TRemoteFile * File, void * Param);
  virtual void RecycleFile(const UnicodeString & FileName, const TRemoteFile * File);
  virtual void DoStartup();
  virtual bool DoQueryReopen(Exception * E);
  virtual void FatalError(Exception * E, const UnicodeString & Msg);
  virtual void ResetConnection();
  virtual bool DoPromptUser(TSessionDataIntf * Data, TPromptKind Kind,
    const UnicodeString & Name, const UnicodeString & Instructions, TStrings * Prompts,
    TStrings * Response);
  virtual void OpenTunnel();
  virtual void CloseTunnel();
  virtual void DoInformation(const UnicodeString & Str, bool Status, int Phase = -1);
  virtual UnicodeString & FileUrl(const UnicodeString & FileName);
  virtual UnicodeString & FileUrl(const UnicodeString & Protocol, const UnicodeString & FileName);
  virtual void FileFind(const UnicodeString & FileName, const TRemoteFile * File, void * Param);
  virtual void DoFilesFind(UnicodeString Directory, TFilesFindParams & Params);
  virtual bool DoCreateLocalFile(const UnicodeString & FileName,
    TFileOperationProgressType * OperationProgress, HANDLE * AHandle,
    bool NoConfirmation);

  virtual void Information(const UnicodeString & Str, bool Status);
  virtual unsigned int QueryUser(const UnicodeString & Query,
    TStrings * MoreMessages, unsigned int Answers, const TQueryParams * Params,
    TQueryType QueryType = qtConfirmation);
  virtual unsigned int QueryUserException(const UnicodeString & Query,
    Exception * E, unsigned int Answers, const TQueryParams * Params,
    TQueryType QueryType = qtConfirmation);
  virtual void DisplayBanner(const UnicodeString & Banner);
  virtual void Closed();
  virtual void HandleExtendedException(Exception * E);
  virtual bool IsListenerFree(unsigned int PortNumber);

  virtual void DoReadDirectoryProgress(int Progress, bool & Cancel);
  virtual void SetOperationProgress(TFileOperationProgressType * AOperationProgress) { FOperationProgress = AOperationProgress; }
  virtual TFileOperationProgressType * GetOperationProgress() { return FOperationProgress; }
  virtual void ReadDirectory(TRemoteFileList * FileList);
  virtual void CustomReadDirectory(TRemoteFileList * FileList);
  virtual void DoCreateLink(const UnicodeString & FileName,
    const UnicodeString & PointTo, bool Symbolic);
  virtual bool CreateLocalFile(const UnicodeString & FileName,
    TFileOperationProgressType * OperationProgress, HANDLE * AHandle,
    bool NoConfirmation);
  virtual void OpenLocalFile(const UnicodeString & FileName, unsigned int Access,
    int * Attrs, HANDLE * Handle, __int64 * ACTime, __int64 * MTime,
    __int64 * ATime, __int64 * Size, bool TryWriteReadOnly = true);
  virtual bool AllowLocalFileTransfer(const UnicodeString & FileName,
    const TCopyParamType *CopyParam);
  virtual bool HandleException(Exception * E);

  virtual void DoProgress(TFileOperationProgressType & ProgressData, TCancelStatus & Cancel);
  virtual void DoFinished(TFileOperation Operation, TOperationSide Side, bool Temp,
    const UnicodeString & FileName, bool Success, TOnceDoneOperation & OnceDoneOperation);
  virtual void RollbackAction(TSessionAction & Action,
    TFileOperationProgressType * OperationProgress, Exception * E = NULL);
  virtual void DoAnyCommand(const UnicodeString & Command, TCaptureOutputEvent OutputEvent,
    TCallSessionAction * Action);
  virtual TRemoteFileList * DoReadDirectoryListing(UnicodeString Directory, bool UseCache);
  virtual RawByteString & EncryptPassword(const UnicodeString & Password);
  virtual UnicodeString & DecryptPassword(const RawByteString & Password);

  virtual void SetLocalFileTime(const UnicodeString & LocalFileName,
    const TDateTime & Modification);
  virtual void SetLocalFileTime(const UnicodeString & LocalFileName,
    FILETIME * AcTime, FILETIME * WrTime);
  virtual HANDLE CreateLocalFile(const UnicodeString & LocalFileName, DWORD DesiredAccess,
    DWORD ShareMode, DWORD CreationDisposition, DWORD FlagsAndAttributes);
  virtual DWORD GetLocalFileAttributes(const UnicodeString & LocalFileName);
  virtual BOOL SetLocalFileAttributes(const UnicodeString & LocalFileName, DWORD FileAttributes);
  virtual BOOL MoveLocalFile(const UnicodeString & LocalFileName, const UnicodeString & NewLocalFileName, DWORD Flags);
  virtual BOOL RemoveLocalDirectory(const UnicodeString & LocalDirName);
  virtual BOOL CreateLocalDirectory(const UnicodeString & LocalDirName, LPSECURITY_ATTRIBUTES SecurityAttributes);

  virtual TCustomFileSystem * GetFileSystem() { return FFileSystem; }
  virtual UnicodeString & GetSessionUrl();

private:
  TSessionData * FSessionData;
  TSessionLog * FLog;
  TActionLog * FActionLog;
  TConfiguration * FConfiguration;
  UnicodeString FCurrentDirectory;
  UnicodeString FLockDirectory;
  Integer FExceptionOnFail;
  TRemoteDirectory * FFiles;
  int FInTransaction;
  bool FSuspendTransaction;
  TNotifyEvent FOnChangeDirectory;
  TReadDirectoryEvent FOnReadDirectory;
  TNotifyEvent FOnStartReadDirectory;
  TReadDirectoryProgressEvent FOnReadDirectoryProgress;
  TDeleteLocalFileEvent FOnDeleteLocalFile;
  TCreateLocalFileEvent FOnCreateLocalFile;
  TGetLocalFileAttributesEvent FOnGetLocalFileAttributes;
  TSetLocalFileAttributesEvent FOnSetLocalFileAttributes;
  TMoveLocalFileEvent FOnMoveLocalFile;
  TRemoveLocalDirectoryEvent FOnRemoveLocalDirectory;
  TCreateLocalDirectoryEvent FOnCreateLocalDirectory;
  TRemoteTokenList FMembership;
  TRemoteTokenList FGroups;
  TRemoteTokenList FUsers;
  bool FUsersGroupsLookedup;
  TFileOperationProgressEvent FOnProgress;
  TFileOperationFinishedEvent FOnFinished;
  TFileOperationProgressType * FOperationProgress;
  bool FUseBusyCursor;
  TRemoteDirectoryCache * FDirectoryCache;
  TRemoteDirectoryChangesCache * FDirectoryChangesCache;
  TCustomFileSystem * FFileSystem;
  TSecureShell * FSecureShell;
  UnicodeString FLastDirectoryChange;
  TCurrentFSProtocol FFSProtocol;
  TTerminalIntf * FCommandSession;
  bool FAutoReadDirectory;
  bool FReadingCurrentDirectory;
  bool * FClosedOnCompletion;
  TSessionStatus FStatus;
  RawByteString FPassword;
  RawByteString FTunnelPassword;
  TTunnelThread * FTunnelThread;
  TSecureShell * FTunnel;
  TSessionData * FTunnelData;
  TSessionLog * FTunnelLog;
  TTunnelUI * FTunnelUI;
  int FTunnelLocalPortNumber;
  UnicodeString FTunnelError;
  TQueryUserEvent FOnQueryUser;
  TPromptUserEvent FOnPromptUser;
  TDisplayBannerEvent FOnDisplayBanner;
  TExtendedExceptionEvent FOnShowExtendedException;
  TInformationEvent FOnInformation;
  TNotifyEvent FOnClose;
  TCallbackGuard * FCallbackGuard;
  TFindingFileEvent FOnFindingFile;

  // void CommandError(Exception * E, const UnicodeString & Msg);
  // unsigned int CommandError(Exception * E, const UnicodeString & Msg, unsigned int Answers);
  // void ReactOnCommand(int /*TFSCommand*/ Cmd);
  inline bool InTransaction();

public:
  // void SetMasks(const UnicodeString & Value);
  // UnicodeString GetCurrentDirectory();
  // bool GetExceptionOnFail() const;
  // const TRemoteTokenList * GetGroups();
  // const TRemoteTokenList * GetUsers();
  // const TRemoteTokenList * GetMembership();
  // void SetCurrentDirectory(const UnicodeString & Value);
  // void SetExceptionOnFail(bool Value);
  // UnicodeString GetUserName() const;
  // bool GetAreCachesEmpty() const;
  // bool GetIsCapable(TFSCapability Capability) const;
  // void ClearCachedFileList(const UnicodeString & Path, bool SubDirs);
  // void AddCachedFileList(TRemoteFileList * FileList);
  // bool GetCommandSessionOpened();
  // TTerminal * GetCommandSession();
  // bool GetResolvingSymlinks();
  // bool GetActive();
  // UnicodeString GetPassword();
  // UnicodeString GetTunnelPassword();
  // bool GetStoredCredentialsTried();

protected:
  bool FReadCurrentDirectoryPending;
  bool FReadDirectoryPending;
  bool FTunnelOpening;

  void DoStartReadDirectory();
  // void DoReadDirectoryProgress(int Progress, bool & Cancel);
  // void DoReadDirectory(bool ReloadOnly);
  // void DoCreateDirectory(const UnicodeString & DirName);
  // void DoDeleteFile(const UnicodeString & FileName, const TRemoteFile * File,
    // int Params);
  // void DoCustomCommandOnFile(UnicodeString FileName,
    // const TRemoteFile * File, UnicodeString Command, int Params, TCaptureOutputEvent OutputEvent);
  // void DoRenameFile(const UnicodeString & FileName,
    // const UnicodeString & NewName, bool Move);
  // void DoCopyFile(const UnicodeString & FileName, const UnicodeString & NewName);
  // void DoChangeFileProperties(const UnicodeString & vFileName,
    // const TRemoteFile * File, const TRemoteProperties * Properties);
  // void DoChangeDirectory();
  // void EnsureNonExistence(const UnicodeString & FileName);
  // void LookupUsersGroups();
  // void FileModified(const TRemoteFile * File,
    // const UnicodeString & FileName, bool ClearDirectoryChange = false);
  // int FileOperationLoop(TFileOperationEvent CallBackFunc,
    // TFileOperationProgressType * OperationProgress, bool AllowSkip,
    // const UnicodeString & Message, void * Param1 = NULL, void * Param2 = NULL);
  // bool ProcessFiles(TStrings * FileList, TFileOperation Operation,
    // TProcessFileEvent ProcessFile, void * Param = NULL, TOperationSide Side = osRemote,
    // bool Ex = false);
  // bool ProcessFilesEx(TStrings * FileList, TFileOperation Operation,
    // TProcessFileEventEx ProcessFile, void * Param = NULL, TOperationSide Side = osRemote);
  // virtual void ProcessDirectory(const UnicodeString & DirName,
    // TProcessFileEvent CallBackFunc, void * Param = NULL, bool UseCache = false,
    // bool IgnoreErrors = false);
  // void AnnounceFileListOperation();
  // UnicodeString TranslateLockedPath(UnicodeString Path, bool Lock);
  // void ReadDirectory(TRemoteFileList * FileList);
  // void CustomReadDirectory(TRemoteFileList * FileList);
  // void DoCreateLink(const UnicodeString & FileName, const UnicodeString & PointTo, bool Symbolic);
  // bool CreateLocalFile(const UnicodeString & FileName,
    // TFileOperationProgressType * OperationProgress, HANDLE * AHandle,
    // bool NoConfirmation);
  // void OpenLocalFile(const UnicodeString & FileName, unsigned int Access,
    // int * Attrs, HANDLE * Handle, __int64 * ACTime, __int64 * MTime,
    // __int64 * ATime, __int64 * Size, bool TryWriteReadOnly = true);
  // bool AllowLocalFileTransfer(const UnicodeString & FileName, const TCopyParamType *CopyParam);
  // bool HandleException(Exception * E);
  // void CalculateFileSize(const UnicodeString & FileName,
    // const TRemoteFile * File, /*TCalculateSizeParams*/ void * Size);
  // void DoCalculateDirectorySize(const UnicodeString & FileName,
    // const TRemoteFile * File, TCalculateSizeParams * Params);
  // void CalculateLocalFileSize(const UnicodeString & FileName,
    // const TSearchRec & Rec, /*__int64*/ void * Params);
  // void CalculateLocalFilesSize(TStrings * FileList, __int64 & Size,
    // const TCopyParamType * CopyParam = NULL);
  // TBatchOverwrite EffectiveBatchOverwrite(
    // int Params, TFileOperationProgressType * OperationProgress, bool Special);
  // bool CheckRemoteFile(int Params, TFileOperationProgressType * OperationProgress);
  // unsigned int ConfirmFileOverwrite(const UnicodeString & FileName,
    // const TOverwriteFileParams * FileParams, unsigned int Answers, const TQueryParams * QueryParams,
    // TOperationSide Side, int Params, TFileOperationProgressType * OperationProgress,
    // UnicodeString Message = L"");
  // void DoSynchronizeCollectDirectory(const UnicodeString & LocalDirectory,
    // const UnicodeString & RemoteDirectory, TSynchronizeMode Mode,
    // const TCopyParamType * CopyParam, int Params,
    // TSynchronizeDirectoryEvent OnSynchronizeDirectory,
    // TSynchronizeOptions * Options, int Level, TSynchronizeChecklist * Checklist);
  // void SynchronizeCollectFile(const UnicodeString & FileName,
    // const TRemoteFile * File, /*TSynchronizeData*/ void * Param);
  // void SynchronizeRemoteTimestamp(const UnicodeString & FileName,
    // const TRemoteFile * File, void * Param);
  // void SynchronizeLocalTimestamp(const UnicodeString & FileName,
    // const TRemoteFile * File, void * Param);
  // void DoSynchronizeProgress(const TSynchronizeData & Data, bool Collect);
  // void DeleteLocalFile(const UnicodeString & FileName,
    // const TRemoteFile * File, void * Param);
  // void RecycleFile(const UnicodeString & FileName, const TRemoteFile * File);
  // void DoStartup();
  // virtual bool DoQueryReopen(Exception * E);
  // virtual void FatalError(Exception * E, const UnicodeString & Msg);
  // void ResetConnection();
  // virtual bool DoPromptUser(TSessionDataIntf * Data, TPromptKind Kind,
    // const UnicodeString & Name, const UnicodeString & Instructions, TStrings * Prompts,
    // TStrings * Response);
  // void OpenTunnel();
  // void CloseTunnel();
  // void DoInformation(const UnicodeString & Str, bool Status, int Phase = -1);
  // UnicodeString FileUrl(const UnicodeString & Protocol, const UnicodeString & FileName);
  // void FileFind(const UnicodeString & FileName, const TRemoteFile * File, void * Param);
  // void DoFilesFind(UnicodeString Directory, TFilesFindParams & Params);
  // bool DoCreateLocalFile(const UnicodeString & FileName,
    // TFileOperationProgressType * OperationProgress, HANDLE * AHandle,
    // bool NoConfirmation);

  // virtual void Information(const UnicodeString & Str, bool Status);
  // virtual unsigned int QueryUser(const UnicodeString & Query,
    // TStrings * MoreMessages, unsigned int Answers, const TQueryParams * Params,
    // TQueryType QueryType = qtConfirmation);
  // virtual unsigned int QueryUserException(const UnicodeString & Query,
    // Exception * E, unsigned int Answers, const TQueryParams * Params,
    // TQueryType QueryType = qtConfirmation);
  // virtual void DisplayBanner(const UnicodeString & Banner);
  // virtual void Closed();
  // virtual void HandleExtendedException(Exception * E);
  // bool IsListenerFree(unsigned int PortNumber);
  // void DoProgress(TFileOperationProgressType & ProgressData, TCancelStatus & Cancel);
  // void DoFinished(TFileOperation Operation, TOperationSide Side, bool Temp,
    // const UnicodeString & FileName, bool Success, TOnceDoneOperation & OnceDoneOperation);
  // void RollbackAction(TSessionAction & Action,
    // TFileOperationProgressType * OperationProgress, Exception * E = NULL);
  // void DoAnyCommand(const UnicodeString & Command, TCaptureOutputEvent OutputEvent,
    // TCallSessionAction * Action);
  // TRemoteFileList * DoReadDirectoryListing(UnicodeString Directory, bool UseCache);
  // RawByteString EncryptPassword(const UnicodeString & Password);
  // UnicodeString DecryptPassword(const RawByteString & Password);

  // TFileOperationProgressType * GetOperationProgress() { return FOperationProgress; }

  // void SetLocalFileTime(const UnicodeString & LocalFileName,
    // const TDateTime & Modification);
  // void SetLocalFileTime(const UnicodeString & LocalFileName,
    // FILETIME * AcTime, FILETIME * WrTime);
  // HANDLE CreateLocalFile(const UnicodeString & LocalFileName, DWORD DesiredAccess,
    // DWORD ShareMode, DWORD CreationDisposition, DWORD FlagsAndAttributes);
  // DWORD GetLocalFileAttributes(const UnicodeString & LocalFileName);
  // BOOL SetLocalFileAttributes(const UnicodeString & LocalFileName, DWORD FileAttributes);
  // BOOL MoveLocalFile(const UnicodeString & LocalFileName, const UnicodeString & NewLocalFileName, DWORD Flags);
  // BOOL RemoveLocalDirectory(const UnicodeString & LocalDirName);
  // BOOL CreateLocalDirectory(const UnicodeString & LocalDirName, LPSECURITY_ATTRIBUTES SecurityAttributes);

public:
  explicit TTerminal();
  void Init(TSessionDataIntf * SessionData, TConfiguration * Configuration);
  // virtual ~TTerminal();
  // void Open();
  // void Close();
  // void Reopen(int Params);
  // virtual void DirectoryModified(const UnicodeString & Path, bool SubDirs);
  // virtual void DirectoryLoaded(TRemoteFileList * FileList);
  // void ShowExtendedException(Exception * E);
  // void Idle();
  // void RecryptPasswords();
  // bool AllowedAnyCommand(const UnicodeString & Command);
  // void AnyCommand(const UnicodeString & Command, TCaptureOutputEvent OutputEvent);
  // void CloseOnCompletion(TOnceDoneOperation Operation = odoDisconnect, const UnicodeString & Message = L"");
  // UnicodeString AbsolutePath(const UnicodeString & Path, bool Local);
  // void BeginTransaction();
  // void ReadCurrentDirectory();
  // void ReadDirectory(bool ReloadOnly, bool ForceCache = false);
  // TRemoteFileList * ReadDirectoryListing(const UnicodeString & Directory, const TFileMasks & Mask);
  // TRemoteFileList * CustomReadDirectoryListing(const UnicodeString & Directory, bool UseCache);
  // TRemoteFile * ReadFileListing(const UnicodeString & Path);
  // void ReadFile(const UnicodeString & FileName, TRemoteFile *& File);
  // bool FileExists(const UnicodeString & FileName, TRemoteFile ** File = NULL);
  // void ReadSymlink(TRemoteFile * SymlinkFile, TRemoteFile *& File);
  // bool CopyToLocal(TStrings * FilesToCopy,
    // const UnicodeString & TargetDir, const TCopyParamType * CopyParam, int Params);
  // bool CopyToRemote(TStrings * FilesToCopy,
    // const UnicodeString & TargetDir, const TCopyParamType * CopyParam, int Params);
  // void CreateDirectory(const UnicodeString & DirName,
    // const TRemoteProperties * Properties = NULL);
  // void CreateLink(const UnicodeString & FileName, const UnicodeString & PointTo, bool Symbolic);
  // void DeleteFile(const UnicodeString & FileName,
    // const TRemoteFile * File = NULL, void * Params = NULL);
  // bool DeleteFiles(TStrings * FilesToDelete, int Params = 0);
  // bool DeleteLocalFiles(TStrings * FileList, int Params = 0);
  // bool IsRecycledFile(const UnicodeString & FileName);
  // void CustomCommandOnFile(const UnicodeString & FileName,
    // const TRemoteFile * File, void * AParams);
  // void CustomCommandOnFiles(UnicodeString Command, int Params,
    // TStrings * Files, TCaptureOutputEvent OutputEvent);
  // void ChangeDirectory(const UnicodeString & Directory);
  // void EndTransaction();
  // void HomeDirectory();
  // void ChangeFileProperties(const UnicodeString & FileName,
    // const TRemoteFile * File, /*const TRemoteProperties */ void * Properties);
  // void ChangeFilesProperties(TStrings * FileList,
    // const TRemoteProperties * Properties);
  // bool LoadFilesProperties(TStrings * FileList);
  // void TerminalError(UnicodeString Msg);
  // void TerminalError(Exception * E, UnicodeString Msg);
  // void ReloadDirectory();
  // void RefreshDirectory();
  // void RenameFile(const UnicodeString & FileName, const UnicodeString & NewName);
  // void RenameFile(const TRemoteFile * File, const UnicodeString & NewName, bool CheckExistence);
  // void MoveFile(const UnicodeString & FileName, const TRemoteFile * File,
    // /* const TMoveFileParams */ void * Param);
  // bool MoveFiles(TStrings * FileList, const UnicodeString & Target,
    // const UnicodeString & FileMask);
  // void CopyFile(const UnicodeString & FileName, const TRemoteFile * File,
    // /* const TMoveFileParams */ void * Param);
  // bool CopyFiles(TStrings * FileList, const UnicodeString & Target,
    // const UnicodeString & FileMask);
  // void CalculateFilesSize(TStrings * FileList, __int64 & Size,
    // int Params, const TCopyParamType * CopyParam = NULL, TCalculateSizeStats * Stats = NULL);
  // void CalculateFilesChecksum(const UnicodeString & Alg, TStrings * FileList,
    // TStrings * Checksums, TCalculatedChecksumEvent OnCalculatedChecksum);
  // void ClearCaches();
  // TSynchronizeChecklist * SynchronizeCollect(const UnicodeString & LocalDirectory,
    // const UnicodeString & RemoteDirectory, TSynchronizeMode Mode,
    // const TCopyParamType * CopyParam, int Params,
    // TSynchronizeDirectoryEvent OnSynchronizeDirectory, TSynchronizeOptions * Options);
  // void SynchronizeApply(TSynchronizeChecklist * Checklist,
    // const UnicodeString & LocalDirectory, const UnicodeString & RemoteDirectory,
    // const TCopyParamType * CopyParam, int Params,
    // TSynchronizeDirectoryEvent OnSynchronizeDirectory);
  // void FilesFind(UnicodeString Directory, const TFileMasks & FileMask,
    // TFileFoundEvent OnFileFound, TFindingFileEvent OnFindingFile);
  // void SpaceAvailable(const UnicodeString & Path, TSpaceAvailable & ASpaceAvailable);
  // bool DirectoryFileList(const UnicodeString & Path,
    // TRemoteFileList *& FileList, bool CanLoad);
  // void MakeLocalFileList(const UnicodeString & FileName,
    // const TSearchRec & Rec, void * Param);
  // UnicodeString FileUrl(const UnicodeString & FileName);
  // bool FileOperationLoopQuery(Exception & E,
    // TFileOperationProgressType * OperationProgress, const UnicodeString & Message,
    // bool AllowSkip, const UnicodeString & SpecialRetry = UnicodeString());
  // TUsableCopyParamAttrs UsableCopyParamAttrs(int Params);
  // bool QueryReopen(Exception * E, int Params,
    // TFileOperationProgressType * OperationProgress);
  // UnicodeString PeekCurrentDirectory();
  // void FatalAbort();

  // const TSessionInfo & GetSessionInfo() const;
  // const TFileSystemInfo & GetFileSystemInfo(bool Retrieve = false);
  // void LogEvent(const UnicodeString & Str);

  // static bool IsAbsolutePath(const UnicodeString & Path);
  // static UnicodeString ExpandFileName(const UnicodeString & Path,
    // const UnicodeString & BasePath);

  // TSessionData * GetSessionData() { return FSessionData; }
  // TSessionData * GetSessionData() const { return FSessionData; }
  // TSessionLog * GetLog() { return FLog; }
  // TActionLog * GetActionLog() { return FActionLog; };
  // TConfiguration *GetConfiguration() { return FConfiguration; }
  // TSessionStatus GetStatus() { return FStatus; }
  // TRemoteDirectory * GetFiles() { return FFiles; }
  // TNotifyEvent & GetOnChangeDirectory() { return FOnChangeDirectory; }
  // void SetOnChangeDirectory(TNotifyEvent Value) { FOnChangeDirectory = Value; }
  // TReadDirectoryEvent & GetOnReadDirectory() { return FOnReadDirectory; }
  // void SetOnReadDirectory(TReadDirectoryEvent Value) { FOnReadDirectory = Value; }
  // TNotifyEvent & GetOnStartReadDirectory() { return FOnStartReadDirectory; }
  // void SetOnStartReadDirectory(TNotifyEvent Value) { FOnStartReadDirectory = Value; }
  // TReadDirectoryProgressEvent & GetOnReadDirectoryProgress() { return FOnReadDirectoryProgress; }
  // void SetOnReadDirectoryProgress(TReadDirectoryProgressEvent Value) { FOnReadDirectoryProgress = Value; }
  // TDeleteLocalFileEvent & GetOnDeleteLocalFile() { return FOnDeleteLocalFile; }
  // void SetOnDeleteLocalFile(TDeleteLocalFileEvent Value) { FOnDeleteLocalFile = Value; }
  // TCreateLocalFileEvent & GetOnCreateLocalFile() { return FOnCreateLocalFile; }
  // void SetOnCreateLocalFile(TCreateLocalFileEvent Value) { FOnCreateLocalFile = Value; }
  // TGetLocalFileAttributesEvent & GetOnGetLocalFileAttributes() { return FOnGetLocalFileAttributes; }
  // void SetOnGetLocalFileAttributes(TGetLocalFileAttributesEvent Value) { FOnGetLocalFileAttributes = Value; }
  // TSetLocalFileAttributesEvent & GetOnSetLocalFileAttributes() { return FOnSetLocalFileAttributes; }
  // void SetOnSetLocalFileAttributes(TSetLocalFileAttributesEvent Value) { FOnSetLocalFileAttributes = Value; }
  // TMoveLocalFileEvent & GetOnMoveLocalFile() { return FOnMoveLocalFile; }
  // void SetOnMoveLocalFile(TMoveLocalFileEvent Value) { FOnMoveLocalFile = Value; }
  // TRemoveLocalDirectoryEvent & GetOnRemoveLocalDirectory() { return FOnRemoveLocalDirectory; }
  // void SetOnRemoveLocalDirectory(TRemoveLocalDirectoryEvent Value) { FOnRemoveLocalDirectory = Value; }
  // TCreateLocalDirectoryEvent & GetOnCreateLocalDirectory() { return FOnCreateLocalDirectory; }
  // void SetOnCreateLocalDirectory(TCreateLocalDirectoryEvent Value) { FOnCreateLocalDirectory = Value; }
  // TFileOperationProgressEvent & GetOnProgress() { return FOnProgress; }
  // void SetOnProgress(TFileOperationProgressEvent Value) { FOnProgress = Value; }
  // TFileOperationFinishedEvent & GetOnFinished() { return FOnFinished; }
  // void SetOnFinished(TFileOperationFinishedEvent Value) { FOnFinished = Value; }
  // TCurrentFSProtocol GetFSProtocol() { return FFSProtocol; }
  // bool GetUseBusyCursor() { return FUseBusyCursor; }
  // void SetUseBusyCursor(bool Value) { FUseBusyCursor = Value; }
  // bool GetAutoReadDirectory() { return FAutoReadDirectory; }
  // void SetAutoReadDirectory(bool Value) { FAutoReadDirectory = Value; }
  // TStrings * GetFixedPaths();
  // TQueryUserEvent & GetOnQueryUser() { return FOnQueryUser; }
  // void SetOnQueryUser(TQueryUserEvent Value) { FOnQueryUser = Value; }
  // TPromptUserEvent & GetOnPromptUser() { return FOnPromptUser; }
  // void SetOnPromptUser(TPromptUserEvent Value) { FOnPromptUser = Value; }
  // TDisplayBannerEvent & GetOnDisplayBanner() { return FOnDisplayBanner; }
  // void SetOnDisplayBanner(TDisplayBannerEvent Value) { FOnDisplayBanner = Value; }
  // TExtendedExceptionEvent & GetOnShowExtendedException() { return FOnShowExtendedException; }
  // void SetOnShowExtendedException(TExtendedExceptionEvent Value) { FOnShowExtendedException = Value; }
  // TInformationEvent & GetOnInformation() { return FOnInformation; }
  // void SetOnInformation(TInformationEvent Value) { FOnInformation = Value; }
  // TNotifyEvent & GetOnClose() { return FOnClose; }
  // void SetOnClose(TNotifyEvent Value) { FOnClose = Value; }
  // int GetTunnelLocalPortNumber() { return FTunnelLocalPortNumber; }

private:
  void TryOpen1();
  void TryOpen2();
  TCustomFileSystem * InitFileSystem();

private:
  mutable RawByteString FEncryptedPasswordStr;
  mutable UnicodeString FDecryptedPasswordStr;
  mutable UnicodeString FSessionUrlStr;
  mutable UnicodeString FAbsolutePathStr;
  mutable UnicodeString FTranslatedLockedPathStr;
  mutable UnicodeString FUserNameStr;
  mutable UnicodeString FFileUrlStr;
  mutable UnicodeString FPasswordStr;
  mutable UnicodeString FTunnelPasswordStr;
  mutable UnicodeString FExpandedFileNameStr;

private:
  TTerminal(const TTerminal &);
  TTerminal & operator = (const TTerminal &);
};
//------------------------------------------------------------------------------
class TSecondaryTerminal : public TTerminal
{
public:
  explicit TSecondaryTerminal(TTerminalIntf * MainTerminal);
  virtual ~TSecondaryTerminal() {}
  void Init(TSessionDataIntf * SessionData, TConfiguration * Configuration,
    const UnicodeString & Name);

  TTerminalIntf * GetMainTerminal() { return FMainTerminal; }

protected:
  virtual void DirectoryLoaded(TRemoteFileList * FileList);
  virtual void DirectoryModified(const UnicodeString & Path,
    bool SubDirs);
  virtual bool DoPromptUser(TSessionDataIntf * Data, TPromptKind Kind,
    const UnicodeString & Name, const UnicodeString & Instructions, TStrings * Prompts, TStrings * Results);

private:
  bool FMasterPasswordTried;
  bool FMasterTunnelPasswordTried;
  TTerminalIntf * FMainTerminal;
};
//------------------------------------------------------------------------------
class TTerminalList : public TObjectList
{
public:
  explicit TTerminalList(TConfiguration * AConfiguration);
  virtual ~TTerminalList();

  virtual TTerminalIntf * NewTerminal(TSessionDataIntf * Data);
  virtual void FreeTerminal(TTerminalIntf * Terminal);
  void FreeAndNullTerminal(TTerminalIntf * & Terminal);
  virtual void Idle();
  void RecryptPasswords();

  TTerminalIntf * GetTerminal(intptr_t Index);
  intptr_t GetActiveCount();

protected:
  virtual TTerminalIntf * CreateTerminal(TSessionDataIntf * Data);

private:
  TConfiguration * FConfiguration;

public:
  void SetMasks(const UnicodeString & Value);
};
//------------------------------------------------------------------------------
struct TSynchronizeOptions
{
  TSynchronizeOptions();
  ~TSynchronizeOptions();

  TStringList * Filter;

  bool FilterFind(const UnicodeString & FileName);
  bool MatchesFilter(const UnicodeString & FileName);
};
//------------------------------------------------------------------------------
class TSynchronizeChecklist
{
friend class TTerminal;

public:
  enum TAction { saNone, saUploadNew, saDownloadNew, saUploadUpdate,
    saDownloadUpdate, saDeleteRemote, saDeleteLocal };
  static const int ActionCount = saDeleteLocal;

  class TItem : public TObject
  {
  friend class TTerminal;

  public:
    struct TFileInfo
    {
      UnicodeString FileName;
      UnicodeString Directory;
      TDateTime Modification;
      TModificationFmt ModificationFmt;
      __int64 Size;
    };

    TAction Action;
    bool IsDirectory;
    TFileInfo Local;
    TFileInfo Remote;
    int ImageIndex;
    bool Checked;
    TRemoteFile * RemoteFile;

    const UnicodeString& GetFileName() const;

    ~TItem();

  private:
    FILETIME FLocalLastWriteTime;

    TItem();
  };

  ~TSynchronizeChecklist();

  intptr_t GetCount() const;
  const TItem * GetItem(intptr_t Index) const;

protected:
  TSynchronizeChecklist();

  void Sort();
  void Add(TItem * Item);

public:
  void SetMasks(const UnicodeString & Value);

private:
  TList * FList;

  static int Compare(const void * Item1, const void * Item2);
};
//------------------------------------------------------------------------------
struct TSpaceAvailable
{
  TSpaceAvailable();

  __int64 BytesOnDevice;
  __int64 UnusedBytesOnDevice;
  __int64 BytesAvailableToUser;
  __int64 UnusedBytesAvailableToUser;
  unsigned long BytesPerAllocationUnit;
};
//------------------------------------------------------------------------------
UnicodeString GetSessionUrl(const TTerminalIntf * Terminal);
//------------------------------------------------------------------------------
#endif
