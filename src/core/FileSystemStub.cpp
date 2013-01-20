#include <vcl.h>
#pragma hdrstop

#include "FileSystemStub.h"

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
TFileSystemStub::TFileSystemStub(TTerminalIntf * ATerminal, TFSProtocol AFSProtocol) :
  TCustomFileSystem(ATerminal),
  FFSProtocol(AFSProtocol)
{
  assert(SubpluginsManager);
  FImpl = SubpluginsManager->Create(FFSProtocol, ATerminal);
  assert(FImpl);
  if (!FImpl)
  {
    throw Exception(FMTLOAD(CANNOT_CREATE_SUBPLUGIN, L""));
  }
  if (FImpl->api_version < NBAPI_CORE_VER)
  {
    throw Exception(FMTLOAD(CANNOT_CREATE_SUBPLUGIN2, L""));
  }
}
//---------------------------------------------------------------------------
TFileSystemStub::~TFileSystemStub()
{
  if (FImpl->destroy)
    FImpl->destroy(FImpl, NULL);
}
//---------------------------------------------------------------------------
void TFileSystemStub::Init(void * Data)
{
  FFileSystemInfo.ProtocolBaseName = SubpluginsManager->GetFSProtocolStrById(FFSProtocol);
  // DEBUG_PRINTF(L"FFileSystemInfo.ProtocolBaseName = %s", FFileSystemInfo.ProtocolBaseName.c_str());
  FFileSystemInfo.ProtocolName = FFileSystemInfo.ProtocolBaseName;
  FSessionInfo.ProtocolBaseName = FFileSystemInfo.ProtocolBaseName;
  FSessionInfo.ProtocolName = FSessionInfo.ProtocolBaseName;
  if (FImpl->init)
  {
    FImpl->init(FImpl, Data, NULL);
  }
  /*for (intptr_t Index = 0; Index < fcCount; Index++)
  {
    FFileSystemInfo.IsCapable[Index] = IsCapable(Index);
  }*/
}
//---------------------------------------------------------------------------
UnicodeString TFileSystemStub::GetUrlPrefix()
{
  UnicodeString Result;
  if (FImpl->get_session_url_prefix)
  {
    const wchar_t * Prefix = FImpl->get_session_url_prefix(FImpl, NULL);
    Result = Prefix ? Prefix : L"";
  }
  return Result;
}
//---------------------------------------------------------------------------
void TFileSystemStub::Open()
{
}
//---------------------------------------------------------------------------
void TFileSystemStub::Close()
{
}
//---------------------------------------------------------------------------
bool TFileSystemStub::GetActive()
{
  bool Result = false;
  return Result;
}
//---------------------------------------------------------------------------
const TSessionInfo & TFileSystemStub::GetSessionInfo()
{
  return FSessionInfo;
}
//---------------------------------------------------------------------------
const TFileSystemInfo & TFileSystemStub::GetFileSystemInfo(bool Retrieve)
{
  if (FFileSystemInfo.AdditionalInfo.IsEmpty() && Retrieve)
  {
    UnicodeString UName;
    FFileSystemInfo.RemoteSystem = UName;
  }

  return FFileSystemInfo;
}
//---------------------------------------------------------------------------
bool TFileSystemStub::TemporaryTransferFile(const UnicodeString & /*FileName*/)
{
  return false;
}
//---------------------------------------------------------------------------
bool TFileSystemStub::GetStoredCredentialsTried()
{
  bool Result = false;
  return Result;
}
//---------------------------------------------------------------------------
UnicodeString TFileSystemStub::GetUserName()
{
  UnicodeString Result;
  return Result;
}
//---------------------------------------------------------------------------
void TFileSystemStub::Idle()
{
  TRACE_EXCEPT_BEGIN
  // Keep session alive
  TRACE_EXCEPT_END
}
//---------------------------------------------------------------------------
void TFileSystemStub::AnyCommand(const UnicodeString & Command,
  TCaptureOutputEvent OutputEvent)
{
}
//---------------------------------------------------------------------------
UnicodeString TFileSystemStub::AbsolutePath(const UnicodeString & Path, bool /*Local*/)
{
  return ::AbsolutePath(GetCurrentDirectory(), Path);
}
//---------------------------------------------------------------------------
bool TFileSystemStub::IsCapable(int Capability) const
{
  if (FImpl->is_capable)
  {
    return FImpl->is_capable(
      FImpl,
      static_cast<fs_capability_enum_t>(Capability),
      &TFileSystemStub::error_handler) == nb_true;
  }
  return false;
}
//---------------------------------------------------------------------------
UnicodeString TFileSystemStub::GetCurrentDirectory()
{
  UnicodeString Result;
  // return SubpluginsManager->GetCurrentDirectory(GetHandle());
  // if (FImpl->get_current_directory)
  //   return FImpl->get_current_directory();
  return Result;
}
//---------------------------------------------------------------------------
void TFileSystemStub::CustomCommandOnFile(const UnicodeString & FileName,
  const TRemoteFile * File, const UnicodeString & Command, int Params, TCaptureOutputEvent OutputEvent)
{
}
//---------------------------------------------------------------------------
void TFileSystemStub::DoStartup()
{
  // SkipStartupMessage and DetectReturnVar must succeed,
  // otherwise session is to be closed.
  FTerminal->SetExceptionOnFail(true);
  FTerminal->SetExceptionOnFail(false);
}
void TFileSystemStub::LookupUsersGroups()
{
}
//---------------------------------------------------------------------------
void TFileSystemStub::ReadCurrentDirectory()
{
}
//---------------------------------------------------------------------------
void TFileSystemStub::HomeDirectory()
{
}
//---------------------------------------------------------------------------
void TFileSystemStub::AnnounceFileListOperation()
{
  // noop
}
//---------------------------------------------------------------------------
void TFileSystemStub::ChangeDirectory(const UnicodeString & Directory)
{
}
//---------------------------------------------------------------------------
void TFileSystemStub::CachedChangeDirectory(const UnicodeString & Directory)
{
}
//---------------------------------------------------------------------------
void TFileSystemStub::ReadDirectory(TRemoteFileList * FileList)
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
void TFileSystemStub::ReadSymlink(TRemoteFile * SymlinkFile,
  TRemoteFile *& File)
{
}
//---------------------------------------------------------------------------
void TFileSystemStub::ReadFile(const UnicodeString & FileName,
  TRemoteFile *& File)
{
}
//---------------------------------------------------------------------------
void TFileSystemStub::DeleteFile(const UnicodeString & FileName,
  const TRemoteFile * File, int Params, TRmSessionAction & Action)
{
  USEDPARAM(File);
  USEDPARAM(Params);
  Action.Recursive();
  assert(FLAGCLEAR(Params, dfNoRecursive) || (File && File->GetIsSymLink()));
}
//---------------------------------------------------------------------------
void TFileSystemStub::RenameFile(const UnicodeString & FileName,
  const UnicodeString & NewName)
{
}
//---------------------------------------------------------------------------
void TFileSystemStub::CopyFile(const UnicodeString & FileName,
  const UnicodeString & NewName)
{
}
//---------------------------------------------------------------------------
void TFileSystemStub::CreateDirectory(const UnicodeString & DirName)
{
}
//---------------------------------------------------------------------------
void TFileSystemStub::CreateLink(const UnicodeString & FileName,
  const UnicodeString & PointTo, bool Symbolic)
{
}
//---------------------------------------------------------------------------
void TFileSystemStub::ChangeFileProperties(const UnicodeString & FileName,
  const TRemoteFile * File, const TRemoteProperties * Properties,
  TChmodSessionAction & Action)
{
}
//---------------------------------------------------------------------------
bool TFileSystemStub::LoadFilesProperties(TStrings * /*FileList*/ )
{
  assert(false);
  return false;
}
//---------------------------------------------------------------------------
void TFileSystemStub::CalculateFilesChecksum(const UnicodeString & /*Alg*/,
  TStrings * /*FileList*/, TStrings * /*Checksums*/,
  TCalculatedChecksumEvent /*OnCalculatedChecksum*/)
{
  assert(false);
}
//---------------------------------------------------------------------------
UnicodeString TFileSystemStub::FileUrl(const UnicodeString & FileName)
{
  UnicodeString Result;
  return Result;
}
//---------------------------------------------------------------------------
TStrings * TFileSystemStub::GetFixedPaths()
{
  return NULL;
}
//---------------------------------------------------------------------------
void TFileSystemStub::SpaceAvailable(const UnicodeString & Path,
  TSpaceAvailable & /* ASpaceAvailable */)
{
  assert(false);
}
//---------------------------------------------------------------------------
void TFileSystemStub::CopyToRemote(TStrings * FilesToCopy,
  const UnicodeString & TargetDir, const TCopyParamType * CopyParam,
  int Params, TFileOperationProgressType * OperationProgress,
  TOnceDoneOperation & OnceDoneOperation)
{
}
//---------------------------------------------------------------------------
void TFileSystemStub::CopyToLocal(TStrings * FilesToCopy,
  const UnicodeString & TargetDir, const TCopyParamType * CopyParam,
  int Params, TFileOperationProgressType * OperationProgress,
  TOnceDoneOperation & OnceDoneOperation)
{
}
//---------------------------------------------------------------------------
void NBAPI
TFileSystemStub::error_handler(
  nbptr_t data,
  subplugin_error_t code,
  const wchar_t * msg)
{
  DEBUG_PRINTF(L"begin");
  DEBUG_PRINTF(L"end");
}
//---------------------------------------------------------------------------
