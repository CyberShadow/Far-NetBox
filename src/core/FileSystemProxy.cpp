//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include <stdio.h>

#include "FileSystemProxy.h"

#include "Terminal.h"
#include "Common.h"
#include "Exceptions.h"
#include "Interface.h"
#include "TextsCore.h"
#include "SecureShell.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
#undef FILE_OPERATION_LOOP_EX
#define FILE_OPERATION_LOOP_EX(ALLOW_SKIP, MESSAGE, OPERATION) \
  FILE_OPERATION_LOOP_CUSTOM(FTerminal, ALLOW_SKIP, MESSAGE, OPERATION)
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#define THROW_FILE_SKIPPED(EXCEPTION, MESSAGE) \
  throw EScpFileSkipped(EXCEPTION, MESSAGE)

#define THROW_SCP_ERROR(EXCEPTION, MESSAGE) \
  throw EScp(EXCEPTION, MESSAGE)
//===========================================================================
//---------------------------------------------------------------------------
//===========================================================================
TFileSystemProxy::TFileSystemProxy(TTerminal * ATerminal) :
  TCustomFileSystem(ATerminal)
{
}

void TFileSystemProxy::Init(void * Data)
{
  FSecureShell = reinterpret_cast<TSecureShell *>(Data);
  assert(FSecureShell);
  // FFileSystemInfo.ProtocolBaseName = L"SCP";
  // FFileSystemInfo.ProtocolName = FFileSystemInfo.ProtocolBaseName;
  // for (int Index = 0; Index < fcCount; Index++)
  // {
    // FFileSystemInfo.IsCapable[Index] = IsCapable(static_cast<TFSCapability>(Index));
  // }
}
//---------------------------------------------------------------------------
/* __fastcall */ TFileSystemProxy::~TFileSystemProxy()
{
  CALLSTACK;
  TRACE("/");
}
//---------------------------------------------------------------------------
void __fastcall TFileSystemProxy::Open()
{
  CALLSTACK;
  FSecureShell->Open();
  TRACE("/");
}
//---------------------------------------------------------------------------
void __fastcall TFileSystemProxy::Close()
{
  FSecureShell->Close();
}
//---------------------------------------------------------------------------
bool __fastcall TFileSystemProxy::GetActive()
{
  return FSecureShell->GetActive();
}
//---------------------------------------------------------------------------
const TSessionInfo & __fastcall TFileSystemProxy::GetSessionInfo()
{
  return FSecureShell->GetSessionInfo();
}
//---------------------------------------------------------------------------
const TFileSystemInfo & __fastcall TFileSystemProxy::GetFileSystemInfo(bool Retrieve)
{
  if (FFileSystemInfo.AdditionalInfo.IsEmpty() && Retrieve)
  {
    UnicodeString UName;
    FFileSystemInfo.RemoteSystem = UName;
  }

  return FFileSystemInfo;
}
//---------------------------------------------------------------------------
bool __fastcall TFileSystemProxy::TemporaryTransferFile(const UnicodeString & /*FileName*/)
{
  return false;
}
//---------------------------------------------------------------------------
bool __fastcall TFileSystemProxy::GetStoredCredentialsTried()
{
  return FSecureShell->GetStoredCredentialsTried();
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TFileSystemProxy::GetUserName()
{
  return FSecureShell->GetUserName();
}
//---------------------------------------------------------------------------
void __fastcall TFileSystemProxy::Idle()
{
  CALLSTACK;
  TRACE_EXCEPT_BEGIN
  TRACE("1");
  // Keep session alive
  if ((FTerminal->GetSessionData()->GetPingType() != ptOff) &&
      (Now() - FSecureShell->GetLastDataSent() > FTerminal->GetSessionData()->GetPingIntervalDT()))
  {
    if ((FTerminal->GetSessionData()->GetPingType() == ptDummyCommand) &&
        FSecureShell->GetReady())
    {
    }
    else
    {
      TRACE("4");
      FSecureShell->KeepAlive();
    }
  }

  TRACE("5");
  FSecureShell->Idle();
  TRACE_EXCEPT_END
}
//---------------------------------------------------------------------------
void __fastcall TFileSystemProxy::AnyCommand(const UnicodeString & Command,
  TCaptureOutputEvent OutputEvent)
{
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TFileSystemProxy::AbsolutePath(const UnicodeString & Path, bool /*Local*/)
{
  return ::AbsolutePath(GetCurrentDirectory(), Path);
}
//---------------------------------------------------------------------------
bool __fastcall TFileSystemProxy::IsCapable(int Capability) const
{
  assert(FTerminal);
  switch (Capability) {
    case fcUserGroupListing:
    case fcModeChanging:
    case fcModeChangingUpload:
    case fcPreservingTimestampUpload:
    case fcGroupChanging:
    case fcOwnerChanging:
    case fcAnyCommand:
    case fcShellAnyCommand:
    case fcHardLink:
    case fcSymbolicLink:
    case fcResolveSymlink:
    case fcRename:
    case fcRemoteMove:
    case fcRemoteCopy:
      return true;

    case fcTextMode:
      return FTerminal->GetSessionData()->GetEOLType() != FTerminal->GetConfiguration()->GetLocalEOLType();

    case fcNativeTextMode:
    case fcNewerOnlyUpload:
    case fcTimestampChanging:
    case fcLoadingAdditionalProperties:
    case fcCheckingSpaceAvailable:
    case fcIgnorePermErrors:
    case fcCalculatingChecksum:
    case fcSecondaryShell: // has fcShellAnyCommand
    case fcGroupOwnerChangingByID: // by name
      return false;

    default:
      assert(false);
      return false;
  }
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TFileSystemProxy::GetCurrentDirectory()
{
  return FCurrentDirectory;
}
//---------------------------------------------------------------------------
void TFileSystemProxy::CustomCommandOnFile(const UnicodeString & FileName,
  const TRemoteFile * File, const UnicodeString & Command, int Params, TCaptureOutputEvent OutputEvent)
{
}
//---------------------------------------------------------------------------
void __fastcall TFileSystemProxy::DoStartup()
{
  CALLSTACK;
  // SkipStartupMessage and DetectReturnVar must succeed,
  // otherwise session is to be closed.
  FTerminal->SetExceptionOnFail(true);
  TRACE("/");
}
void __fastcall TFileSystemProxy::LookupUsersGroups()
{
}
//---------------------------------------------------------------------------
void __fastcall TFileSystemProxy::ReadCurrentDirectory()
{
}
//---------------------------------------------------------------------------
void __fastcall TFileSystemProxy::HomeDirectory()
{
}
//---------------------------------------------------------------------------
void __fastcall TFileSystemProxy::AnnounceFileListOperation()
{
  // noop
}
//---------------------------------------------------------------------------
void __fastcall TFileSystemProxy::ChangeDirectory(const UnicodeString & Directory)
{
}
//---------------------------------------------------------------------------
void __fastcall TFileSystemProxy::CachedChangeDirectory(const UnicodeString & Directory)
{
}
//---------------------------------------------------------------------------
void __fastcall TFileSystemProxy::ReadDirectory(TRemoteFileList * FileList)
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
void __fastcall TFileSystemProxy::ReadSymlink(TRemoteFile * SymlinkFile,
  TRemoteFile *& File)
{
  CALLSTACK;
}
//---------------------------------------------------------------------------
void __fastcall TFileSystemProxy::ReadFile(const UnicodeString & FileName,
  TRemoteFile *& File)
{
  CALLSTACK;
}
//---------------------------------------------------------------------------
void __fastcall TFileSystemProxy::DeleteFile(const UnicodeString & FileName,
  const TRemoteFile * File, int Params, TRmSessionAction & Action)
{
  USEDPARAM(File);
  USEDPARAM(Params);
  Action.Recursive();
  assert(FLAGCLEAR(Params, dfNoRecursive) || (File && File->GetIsSymLink()));
}
//---------------------------------------------------------------------------
void __fastcall TFileSystemProxy::RenameFile(const UnicodeString & FileName,
  const UnicodeString & NewName)
{
}
//---------------------------------------------------------------------------
void __fastcall TFileSystemProxy::CopyFile(const UnicodeString & FileName,
  const UnicodeString & NewName)
{
}
//---------------------------------------------------------------------------
void __fastcall TFileSystemProxy::CreateDirectory(const UnicodeString & DirName)
{
}
//---------------------------------------------------------------------------
void __fastcall TFileSystemProxy::CreateLink(const UnicodeString & FileName,
  const UnicodeString & PointTo, bool Symbolic)
{
}
//---------------------------------------------------------------------------
void __fastcall TFileSystemProxy::ChangeFileProperties(const UnicodeString & FileName,
  const TRemoteFile * File, const TRemoteProperties * Properties,
  TChmodSessionAction & Action)
{
}
//---------------------------------------------------------------------------
bool __fastcall TFileSystemProxy::LoadFilesProperties(TStrings * /*FileList*/ )
{
  assert(false);
  return false;
}
//---------------------------------------------------------------------------
void __fastcall TFileSystemProxy::CalculateFilesChecksum(const UnicodeString & /*Alg*/,
  TStrings * /*FileList*/, TStrings * /*Checksums*/,
  TCalculatedChecksumEvent /*OnCalculatedChecksum*/)
{
  assert(false);
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TFileSystemProxy::FileUrl(const UnicodeString & FileName)
{
  UnicodeString Result;
  return Result;
}
//---------------------------------------------------------------------------
TStrings * __fastcall TFileSystemProxy::GetFixedPaths()
{
  return NULL;
}
//---------------------------------------------------------------------------
void __fastcall TFileSystemProxy::SpaceAvailable(const UnicodeString & Path,
  TSpaceAvailable & /*ASpaceAvailable*/)
{
  assert(false);
}
//---------------------------------------------------------------------------
void __fastcall TFileSystemProxy::CopyToRemote(TStrings * FilesToCopy,
  const UnicodeString & TargetDir, const TCopyParamType * CopyParam,
  int Params, TFileOperationProgressType * OperationProgress,
  TOnceDoneOperation & OnceDoneOperation)
{
  CALLSTACK;
}
//---------------------------------------------------------------------------
void __fastcall TFileSystemProxy::CopyToLocal(TStrings * FilesToCopy,
  const UnicodeString & TargetDir, const TCopyParamType * CopyParam,
  int Params, TFileOperationProgressType * OperationProgress,
  TOnceDoneOperation & OnceDoneOperation)
{
}
