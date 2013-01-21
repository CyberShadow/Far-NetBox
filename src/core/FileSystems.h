//---------------------------------------------------------------------------
#ifndef FileSystemsH
#define FileSystemsH

#include <SessionInfo.h>
#include "Exceptions.h"
//---------------------------------------------------------------------------
class TTerminal;
class TRights;
class TRemoteFile;
class TRemoteFileList;
class TCopyParamType;
struct TSpaceAvailable;
class TFileOperationProgressType;
class TRemoteProperties;
//---------------------------------------------------------------------------
enum TFSCommand { fsNull = 0, fsVarValue, fsLastLine, fsFirstLine,
  fsCurrentDirectory, fsChangeDirectory, fsListDirectory, fsListCurrentDirectory,
  fsListFile, fsLookupUsersGroups, fsCopyToRemote, fsCopyToLocal, fsDeleteFile,
  fsRenameFile, fsCreateDirectory, fsChangeMode, fsChangeGroup, fsChangeOwner,
  fsHomeDirectory, fsUnset, fsUnalias, fsCreateLink, fsCopyFile,
  fsAnyCommand, fsReadSymlink, fsChangeProperties, fsMoveFile };
//---------------------------------------------------------------------------
const int dfNoRecursive = 0x01;
const int dfAlternative = 0x02;
const int dfForceDelete = 0x04;
//---------------------------------------------------------------------------
enum TOverwriteMode { omOverwrite, omAppend, omResume };
//---------------------------------------------------------------------------
const int tfFirstLevel =   0x01;
const int tfAutoResume = 0x02;
const int tfNewDirectory = 0x04;
//---------------------------------------------------------------------------
struct TSinkFileParams
{
  UnicodeString TargetDir;
  const TCopyParamType *CopyParam;
  TFileOperationProgressType *OperationProgress;
  int Params;
  unsigned int Flags;
  bool Skipped;
};
//---------------------------------------------------------------------------
struct TFileTransferData
{
  TFileTransferData() :
    CopyParam(NULL),
    Params(0),
    OverwriteResult(-1),
    AutoResume(false)
  {
  }

  UnicodeString FileName;
  const TCopyParamType *CopyParam;
  int Params;
  int OverwriteResult;
  bool AutoResume;
};
//---------------------------------------------------------------------------
struct TClipboardHandler
{
  UnicodeString Text;

  void Copy(TObject * /*Sender*/)
  {
      CopyToClipboard(Text.c_str());
  }
};
//---------------------------------------------------------------------------
struct TOpenRemoteFileParams
{
  TOpenRemoteFileParams() :
    LocalFileAttrs(0),
    OperationProgress(NULL),
    CopyParam(NULL),
    Params(0),
    Resume(false),
    Resuming(false),
    OverwriteMode(omOverwrite),
    DestFileSize(0),
    FileParams(NULL),
    Confirmed(false)
  {}
  int LocalFileAttrs;
  UnicodeString RemoteFileName;
  TFileOperationProgressType * OperationProgress;
  const TCopyParamType * CopyParam;
  int Params;
  bool Resume;
  bool Resuming;
  TOverwriteMode OverwriteMode;
  __int64 DestFileSize; // output
  RawByteString RemoteFileHandle; // output
  TOverwriteFileParams *FileParams;
  bool Confirmed;
};
//---------------------------------------------------------------------------

/** @brief Interface for custom filesystems
  *
  */
class TFileSystemIntf
{
public:
  virtual ~TFileSystemIntf() = 0 {}

  virtual void Init(void *) = 0;
  virtual void * GetHandle() const = 0;
  virtual void FileTransferProgress(__int64 TransferSize, __int64 Bytes) = 0;
};

//---------------------------------------------------------------------------
class TCustomFileSystem : public TFileSystemIntf
{
public:
  virtual ~TCustomFileSystem();

  virtual void * GetHandle() const { return NULL; }
  virtual void Open() = 0;
  virtual void Close() = 0;
  virtual bool GetActive() = 0;
  virtual void Idle() = 0;
  virtual UnicodeString AbsolutePath(const UnicodeString & Path, bool Local) = 0;
  virtual void AnyCommand(const UnicodeString & Command,
    TCaptureOutputEvent OutputEvent) = 0;
  virtual void ChangeDirectory(const UnicodeString & Directory) = 0;
  virtual void CachedChangeDirectory(const UnicodeString & Directory) = 0;
  virtual void AnnounceFileListOperation() = 0;
  virtual void ChangeFileProperties(const UnicodeString & FileName,
    const TRemoteFile * File, const TRemoteProperties * Properties,
    TChmodSessionAction & Action) = 0;
  virtual bool LoadFilesProperties(TStrings * FileList) = 0;
  virtual void CalculateFilesChecksum(const UnicodeString & Alg,
    TStrings * FileList, TStrings * Checksums,
    TCalculatedChecksumEvent OnCalculatedChecksum) = 0;
  virtual void CopyToLocal(TStrings * FilesToCopy,
    const UnicodeString & TargetDir, const TCopyParamType * CopyParam,
    int Params, TFileOperationProgressType * OperationProgress,
    TOnceDoneOperation & OnceDoneOperation) = 0;
  virtual void CopyToRemote(TStrings * FilesToCopy,
    const UnicodeString & TargetDir, const TCopyParamType * CopyParam,
    int Params, TFileOperationProgressType * OperationProgress,
    TOnceDoneOperation & OnceDoneOperation) = 0;
  virtual void CreateDirectory(const UnicodeString & DirName) = 0;
  virtual void CreateLink(const UnicodeString & FileName, const UnicodeString & PointTo, bool Symbolic) = 0;
  virtual void DeleteFile(const UnicodeString & FileName,
    const TRemoteFile * File, int Params,
    TRmSessionAction & Action) = 0;
  virtual void CustomCommandOnFile(const UnicodeString & FileName,
    const TRemoteFile * File, const UnicodeString & Command, int Params, TCaptureOutputEvent OutputEvent) = 0;
  virtual void DoStartup() = 0;
  virtual void HomeDirectory() = 0;
  virtual bool IsCapable(int Capability) const = 0;
  virtual void LookupUsersGroups() = 0;
  virtual void ReadCurrentDirectory() = 0;
  virtual void ReadDirectory(TRemoteFileList * FileList) = 0;
  virtual void ReadFile(const UnicodeString & ileName,
    TRemoteFile *& File) = 0;
  virtual void ReadSymlink(TRemoteFile * SymLinkFile,
    TRemoteFile *& File) = 0;
  virtual void RenameFile(const UnicodeString & ileName,
    const UnicodeString & NewName) = 0;
  virtual void CopyFile(const UnicodeString & ileName,
    const UnicodeString & NewName) = 0;
  virtual UnicodeString FileUrl(const UnicodeString & ileName) = 0;
  virtual TStrings * GetFixedPaths() = 0;
  virtual void SpaceAvailable(const UnicodeString & ath,
    TSpaceAvailable & ASpaceAvailable) = 0;
  virtual const TSessionInfo & GetSessionInfo() = 0;
  virtual const TFileSystemInfo & GetFileSystemInfo(bool Retrieve) = 0;
  virtual bool TemporaryTransferFile(const UnicodeString & FileName) = 0;
  virtual bool GetStoredCredentialsTried() = 0;
  virtual UnicodeString GetUserName() = 0;

  virtual UnicodeString GetCurrentDirectory() = 0;

protected:
  TTerminal * FTerminal;

  explicit TCustomFileSystem(TTerminal * ATerminal);

  static void FindCustomCommandPattern(
    const UnicodeString & Command, int Index, int & Len, char & PatternCmd);
};
//---------------------------------------------------------------------------
#endif
