#include <vcl.h>
#pragma hdrstop

#include "FileSystemProxy.h"

#include "Terminal.h"
#include "Common.h"
// #include "Exceptions.h"
// #include "Interface.h"
// #include "TextsCore.h"
// #include "SecureShell.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------

//===========================================================================
TFileSystemProxy::TFileSystemProxy(TTerminal * ATerminal, TFSProtocol AFSProtocol) :
  TCustomFileSystem(ATerminal),
  FFSProtocol(AFSProtocol)
{
}
//---------------------------------------------------------------------------
TFileSystemProxy::~TFileSystemProxy()
{
}
//---------------------------------------------------------------------------
void TFileSystemProxy::Init(void * Data)
{
  FFileSystemInfo.ProtocolBaseName = SessionDataProvider->GetFSProtocolStrById(FFSProtocol);
  // DEBUG_PRINTF(L"FFileSystemInfo.ProtocolBaseName = %s", FFileSystemInfo.ProtocolBaseName.c_str());
  FFileSystemInfo.ProtocolName = FFileSystemInfo.ProtocolBaseName;
  for (intptr_t Index = 0; Index < fcCount; Index++)
  {
    FFileSystemInfo.IsCapable[Index] = IsCapable(Index);
  }
  FSessionInfo.ProtocolBaseName = FFileSystemInfo.ProtocolBaseName;
  FSessionInfo.ProtocolName = FSessionInfo.ProtocolBaseName;
}
//---------------------------------------------------------------------------
void TFileSystemProxy::Open()
{
}
//---------------------------------------------------------------------------
void TFileSystemProxy::Close()
{
}
//---------------------------------------------------------------------------
bool TFileSystemProxy::GetActive()
{
  bool Result = false;
  return Result;
}
//---------------------------------------------------------------------------
const TSessionInfo & TFileSystemProxy::GetSessionInfo()
{
  return FSessionInfo;
}
//---------------------------------------------------------------------------
const TFileSystemInfo & TFileSystemProxy::GetFileSystemInfo(bool Retrieve)
{
  if (FFileSystemInfo.AdditionalInfo.IsEmpty() && Retrieve)
  {
    UnicodeString UName;
    FFileSystemInfo.RemoteSystem = UName;
  }

  return FFileSystemInfo;
}
//---------------------------------------------------------------------------
bool TFileSystemProxy::TemporaryTransferFile(const UnicodeString & /*FileName*/)
{
  return false;
}
//---------------------------------------------------------------------------
bool TFileSystemProxy::GetStoredCredentialsTried()
{
  bool Result = false;
  return Result;
}
//---------------------------------------------------------------------------
UnicodeString TFileSystemProxy::GetUserName()
{
  UnicodeString Result;
  return Result;
}
//---------------------------------------------------------------------------
void TFileSystemProxy::Idle()
{
  TRACE_EXCEPT_BEGIN
  // Keep session alive
  TRACE_EXCEPT_END
}
//---------------------------------------------------------------------------
void TFileSystemProxy::AnyCommand(const UnicodeString & Command,
  TCaptureOutputEvent OutputEvent)
{
}
//---------------------------------------------------------------------------
UnicodeString TFileSystemProxy::AbsolutePath(const UnicodeString & Path, bool /*Local*/)
{
  return ::AbsolutePath(GetCurrentDirectory(), Path);
}
//---------------------------------------------------------------------------
bool TFileSystemProxy::IsCapable(int Capability) const
{
  return SessionDataProvider->IsCapable(FFSProtocol, static_cast<fs_capability_t>(Capability));
}
//---------------------------------------------------------------------------
UnicodeString TFileSystemProxy::GetCurrentDirectory()
{
  return FCurrentDirectory;
}
//---------------------------------------------------------------------------
void TFileSystemProxy::CustomCommandOnFile(const UnicodeString & FileName,
  const TRemoteFile * File, const UnicodeString & Command, int Params, TCaptureOutputEvent OutputEvent)
{
}
//---------------------------------------------------------------------------
void TFileSystemProxy::DoStartup()
{
  CALLSTACK;
  // SkipStartupMessage and DetectReturnVar must succeed,
  // otherwise session is to be closed.
  FTerminal->SetExceptionOnFail(true);
  TRACE("/");
}
void TFileSystemProxy::LookupUsersGroups()
{
}
//---------------------------------------------------------------------------
void TFileSystemProxy::ReadCurrentDirectory()
{
}
//---------------------------------------------------------------------------
void TFileSystemProxy::HomeDirectory()
{
}
//---------------------------------------------------------------------------
void TFileSystemProxy::AnnounceFileListOperation()
{
  // noop
}
//---------------------------------------------------------------------------
void TFileSystemProxy::ChangeDirectory(const UnicodeString & Directory)
{
}
//---------------------------------------------------------------------------
void TFileSystemProxy::CachedChangeDirectory(const UnicodeString & Directory)
{
}
//---------------------------------------------------------------------------
void TFileSystemProxy::ReadDirectory(TRemoteFileList * FileList)
{
  assert(FileList);
  // emtying file list moved before command execution

  bool Again;

  do
  {
    Again = false;
  }
  while (Again);
}
//---------------------------------------------------------------------------
void TFileSystemProxy::ReadSymlink(TRemoteFile * SymlinkFile,
  TRemoteFile *& File)
{
}
//---------------------------------------------------------------------------
void TFileSystemProxy::ReadFile(const UnicodeString & FileName,
  TRemoteFile *& File)
{
}
//---------------------------------------------------------------------------
void TFileSystemProxy::DeleteFile(const UnicodeString & FileName,
  const TRemoteFile * File, int Params, TRmSessionAction & Action)
{
  USEDPARAM(File);
  USEDPARAM(Params);
  Action.Recursive();
  assert(FLAGCLEAR(Params, dfNoRecursive) || (File && File->GetIsSymLink()));
}
//---------------------------------------------------------------------------
void TFileSystemProxy::RenameFile(const UnicodeString & FileName,
  const UnicodeString & NewName)
{
}
//---------------------------------------------------------------------------
void TFileSystemProxy::CopyFile(const UnicodeString & FileName,
  const UnicodeString & NewName)
{
}
//---------------------------------------------------------------------------
void TFileSystemProxy::CreateDirectory(const UnicodeString & DirName)
{
}
//---------------------------------------------------------------------------
void TFileSystemProxy::CreateLink(const UnicodeString & FileName,
  const UnicodeString & PointTo, bool Symbolic)
{
}
//---------------------------------------------------------------------------
void TFileSystemProxy::ChangeFileProperties(const UnicodeString & FileName,
  const TRemoteFile * File, const TRemoteProperties * Properties,
  TChmodSessionAction & Action)
{
}
//---------------------------------------------------------------------------
bool TFileSystemProxy::LoadFilesProperties(TStrings * /*FileList*/ )
{
  assert(false);
  return false;
}
//---------------------------------------------------------------------------
void TFileSystemProxy::CalculateFilesChecksum(const UnicodeString & /*Alg*/,
  TStrings * /*FileList*/, TStrings * /*Checksums*/,
  TCalculatedChecksumEvent /*OnCalculatedChecksum*/)
{
  assert(false);
}
//---------------------------------------------------------------------------
UnicodeString TFileSystemProxy::FileUrl(const UnicodeString & FileName)
{
  UnicodeString Result;
  return Result;
}
//---------------------------------------------------------------------------
TStrings * TFileSystemProxy::GetFixedPaths()
{
  return NULL;
}
//---------------------------------------------------------------------------
void TFileSystemProxy::SpaceAvailable(const UnicodeString & Path,
  TSpaceAvailable & /* ASpaceAvailable */)
{
  assert(false);
}
//---------------------------------------------------------------------------
void TFileSystemProxy::CopyToRemote(TStrings * FilesToCopy,
  const UnicodeString & TargetDir, const TCopyParamType * CopyParam,
  int Params, TFileOperationProgressType * OperationProgress,
  TOnceDoneOperation & OnceDoneOperation)
{
}
//---------------------------------------------------------------------------
void TFileSystemProxy::CopyToLocal(TStrings * FilesToCopy,
  const UnicodeString & TargetDir, const TCopyParamType * CopyParam,
  int Params, TFileOperationProgressType * OperationProgress,
  TOnceDoneOperation & OnceDoneOperation)
{
}
//---------------------------------------------------------------------------
