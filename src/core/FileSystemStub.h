#pragma once

#include <FileSystems.h>
//---------------------------------------------------------------------------
// Host-side FileSystem representation
class TFileSystemStub : public TCustomFileSystem
{
public:
  explicit TFileSystemStub(TTerminalIntf * ATerminal, TFSProtocol AFSProtocol);
  virtual ~TFileSystemStub();

  virtual void Init(void *);
  virtual UnicodeString GetUrlPrefix();
  virtual void FileTransferProgress(__int64 TransferSize, __int64 Bytes) {}

  virtual void Open();
  virtual void Close();
  virtual bool GetActive();
  virtual void Idle();
  virtual UnicodeString AbsolutePath(const UnicodeString & Path, bool Local);
  virtual void AnyCommand(const UnicodeString & Command,
    TCaptureOutputEvent OutputEvent);
  virtual void ChangeDirectory(const UnicodeString & Directory);
  virtual void CachedChangeDirectory(const UnicodeString & Directory);
  virtual void AnnounceFileListOperation();
  virtual void ChangeFileProperties(const UnicodeString & FileName,
    const TRemoteFile * File, const TRemoteProperties * Properties,
    TChmodSessionAction & Action);
  virtual bool LoadFilesProperties(TStrings * FileList);
  virtual void CalculateFilesChecksum(const UnicodeString & Alg,
    TStrings * FileList, TStrings * Checksums,
    TCalculatedChecksumEvent OnCalculatedChecksum);
  virtual void CopyToLocal(TStrings * FilesToCopy,
    const UnicodeString & TargetDir, const TCopyParamType * CopyParam,
    int Params, TFileOperationProgressType * OperationProgress,
    TOnceDoneOperation & OnceDoneOperation);
  virtual void CopyToRemote(TStrings * FilesToCopy,
    const UnicodeString & TargetDir, const TCopyParamType * CopyParam,
    int Params, TFileOperationProgressType * OperationProgress,
    TOnceDoneOperation & OnceDoneOperation);
  virtual void CreateDirectory(const UnicodeString & DirName);
  virtual void CreateLink(const UnicodeString & FileName, const UnicodeString & PointTo, bool Symbolic);
  virtual void DeleteFile(const UnicodeString & FileName,
    const TRemoteFile * File, int Params, TRmSessionAction & Action);
  virtual void CustomCommandOnFile(const UnicodeString & FileName,
    const TRemoteFile * File, const UnicodeString & Command, int Params, TCaptureOutputEvent OutputEvent);
  virtual void DoStartup();
  virtual void HomeDirectory();
  virtual bool IsCapable(int Capability) const;
  virtual void LookupUsersGroups();
  virtual void ReadCurrentDirectory();
  virtual void ReadDirectory(TRemoteFileList * FileList);
  virtual void ReadFile(const UnicodeString & FileName,
    TRemoteFile *& File);
  virtual void ReadSymlink(TRemoteFile * SymlinkFile,
    TRemoteFile *& File);
  virtual void RenameFile(const UnicodeString & FileName,
    const UnicodeString & NewName);
  virtual void CopyFile(const UnicodeString & FileName,
    const UnicodeString & NewName);
  virtual UnicodeString FileUrl(const UnicodeString & FileName);
  virtual TStrings * GetFixedPaths();
  virtual void SpaceAvailable(const UnicodeString & Path,
    TSpaceAvailable & ASpaceAvailable);
  virtual const TSessionInfo & GetSessionInfo();
  virtual const TFileSystemInfo & GetFileSystemInfo(bool Retrieve);
  virtual bool TemporaryTransferFile(const UnicodeString & FileName);
  virtual bool GetStoredCredentialsTried();
  virtual UnicodeString GetUserName();

  virtual UnicodeString GetCurrentDirectory();

protected:

private:
  // error_handler_t implementation
  static void NBAPI
  error_handler(
    nbptr_t data,
    subplugin_error_t code,
    const wchar_t * msg);

private:
  TFSProtocol FFSProtocol;
  nb_filesystem_t * FImpl;

  TFileSystemInfo FFileSystemInfo;
  TSessionInfo FSessionInfo;

private:
  TFileSystemStub(const TFileSystemStub &);
  TFileSystemStub & operator=(const TFileSystemStub &);
};
//---------------------------------------------------------------------------
