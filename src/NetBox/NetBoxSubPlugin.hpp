#pragma once

#include <SubpluginDefs.hpp>
#include <Classes.hpp>

#include "Terminal.h"
#include "Configuration.h"

//------------------------------------------------------------------------------

class TBaseSubplugin : public TObject
{
public:
  explicit TBaseSubplugin();
  virtual ~TBaseSubplugin();

private:
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class TTerminalSkel
{
public:
  static void NBAPI
  fatal_error(
    nb_terminal_t * object,
    nb_exception_t * E,
    wchar_t * Msg);
};
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class TTerminalStub : public TTerminalIntf
{
public:
  explicit TTerminalStub(nb_terminal_t * terminal);

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
  virtual bool InTransaction() const;

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
  TSessionDataIntf * FSessionData;
};
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Utility functions

// wchar_t * GuidToStr(const GUID & Guid, wchar_t * Buffer, size_t sz);
