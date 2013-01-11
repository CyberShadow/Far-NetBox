//---------------------------------------------------------------------------
#ifndef FileSystemProxyH
#define FileSystemProxyH

#include <FileSystems.h>
//---------------------------------------------------------------------------
class TCommandSet;
class TSecureShell;
//---------------------------------------------------------------------------
class TFileSystemProxy : public TCustomFileSystem
{
public:
  explicit TFileSystemProxy(TTerminal * ATerminal);
  virtual ~TFileSystemProxy();

  virtual void Init(void *); // TSecureShell *
  virtual void FileTransferProgress(__int64 TransferSize, __int64 Bytes) {}

  virtual void __fastcall Open();
  virtual void __fastcall Close();
  virtual bool __fastcall GetActive();
  virtual void __fastcall Idle();
  virtual UnicodeString __fastcall AbsolutePath(const UnicodeString & Path, bool Local);
  virtual void __fastcall AnyCommand(const UnicodeString & Command,
    TCaptureOutputEvent OutputEvent);
  virtual void __fastcall ChangeDirectory(const UnicodeString & Directory);
  virtual void __fastcall CachedChangeDirectory(const UnicodeString & Directory);
  virtual void __fastcall AnnounceFileListOperation();
  virtual void __fastcall ChangeFileProperties(const UnicodeString & FileName,
    const TRemoteFile * File, const TRemoteProperties * Properties,
    TChmodSessionAction & Action);
  virtual bool __fastcall LoadFilesProperties(TStrings * FileList);
  virtual void __fastcall CalculateFilesChecksum(const UnicodeString & Alg,
    TStrings * FileList, TStrings * Checksums,
    TCalculatedChecksumEvent OnCalculatedChecksum);
  virtual void __fastcall CopyToLocal(TStrings * FilesToCopy,
    const UnicodeString & TargetDir, const TCopyParamType * CopyParam,
    int Params, TFileOperationProgressType * OperationProgress,
    TOnceDoneOperation & OnceDoneOperation);
  virtual void __fastcall CopyToRemote(TStrings * FilesToCopy,
    const UnicodeString & TargetDir, const TCopyParamType * CopyParam,
    int Params, TFileOperationProgressType * OperationProgress,
    TOnceDoneOperation & OnceDoneOperation);
  virtual void __fastcall CreateDirectory(const UnicodeString & DirName);
  virtual void __fastcall CreateLink(const UnicodeString & FileName, const UnicodeString & PointTo, bool Symbolic);
  virtual void __fastcall DeleteFile(const UnicodeString & FileName,
    const TRemoteFile * File, int Params, TRmSessionAction & Action);
  virtual void CustomCommandOnFile(const UnicodeString & FileName,
    const TRemoteFile * File, const UnicodeString & Command, int Params, TCaptureOutputEvent OutputEvent);
  virtual void __fastcall DoStartup();
  virtual void __fastcall HomeDirectory();
  virtual bool __fastcall IsCapable(int Capability) const;
  virtual void __fastcall LookupUsersGroups();
  virtual void __fastcall ReadCurrentDirectory();
  virtual void __fastcall ReadDirectory(TRemoteFileList * FileList);
  virtual void __fastcall ReadFile(const UnicodeString & FileName,
    TRemoteFile *& File);
  virtual void __fastcall ReadSymlink(TRemoteFile * SymlinkFile,
    TRemoteFile *& File);
  virtual void __fastcall RenameFile(const UnicodeString & FileName,
    const UnicodeString & NewName);
  virtual void __fastcall CopyFile(const UnicodeString & FileName,
    const UnicodeString & NewName);
  virtual UnicodeString __fastcall FileUrl(const UnicodeString & FileName);
  virtual TStrings * __fastcall GetFixedPaths();
  virtual void __fastcall SpaceAvailable(const UnicodeString & Path,
    TSpaceAvailable & ASpaceAvailable);
  virtual const TSessionInfo & __fastcall GetSessionInfo();
  virtual const TFileSystemInfo & __fastcall GetFileSystemInfo(bool Retrieve);
  virtual bool __fastcall TemporaryTransferFile(const UnicodeString & FileName);
  virtual bool __fastcall GetStoredCredentialsTried();
  virtual UnicodeString __fastcall GetUserName();

  virtual UnicodeString __fastcall GetCurrentDirectory();

protected:

private:
  TSecureShell * FSecureShell;

  TFileSystemInfo FFileSystemInfo;
  UnicodeString FCurrentDirectory;

private:
  TFileSystemProxy(const TFileSystemProxy &);
  TFileSystemProxy & operator=(const TFileSystemProxy &);
};
//---------------------------------------------------------------------------
#endif // ScpFileSystemH
