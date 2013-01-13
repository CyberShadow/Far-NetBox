//---------------------------------------------------------------------------
#ifndef InterfaceH
#define InterfaceH
//---------------------------------------------------------------------------
#include <CoreDefs.hpp>
#include <SubpluginDefs.hpp>

#include "Configuration.h"
#include "SessionData.h"

#define HELP_NONE ""
//---------------------------------------------------------------------------
TConfiguration * __fastcall CreateConfiguration();

void __fastcall ShowExtendedException(Exception * E);

UnicodeString __fastcall GetRegistryKey();
void __fastcall Busy(bool Start);
UnicodeString __fastcall AppNameString();
UnicodeString __fastcall SshVersionString();
void __fastcall CopyToClipboard(const UnicodeString & Text);
HANDLE __fastcall StartThread(void * SecurityAttributes, unsigned StackSize,
  /* TThreadFunc ThreadFunc, */ void * Parameter, unsigned CreationFlags,
  TThreadID & ThreadId);

const unsigned int qaYes =      0x00000001;
const unsigned int qaNo =       0x00000002;
const unsigned int qaOK =       0x00000004;
const unsigned int qaCancel =   0x00000008;
const unsigned int qaAbort =    0x00000010;
const unsigned int qaRetry =    0x00000020;
const unsigned int qaIgnore =   0x00000040;
const unsigned int qaAll =      0x00000080;
const unsigned int qaNoToAll =  0x00000100;
const unsigned int qaYesToAll = 0x00000200;
const unsigned int qaHelp =     0x00000400;
const unsigned int qaSkip =     0x00000800;

const unsigned int qaNeverAskAgain = 0x00010000;

const int qpFatalAbort =           0x01;
const int qpNeverAskAgainCheck =   0x02;
const int qpAllowContinueOnError = 0x04;
const int qpIgnoreAbort =          0x08;

struct TQueryButtonAlias
{
  TQueryButtonAlias();

  unsigned int Button;
  UnicodeString Alias;
  TNotifyEvent OnClick;
};

DEFINE_CALLBACK_TYPE1(TQueryParamsTimerEvent, void,
  unsigned int & /* Result */);

struct TQueryParams
{
  explicit TQueryParams(unsigned int AParams = 0, UnicodeString AHelpKeyword = HELP_NONE);

  const TQueryButtonAlias * Aliases;
  unsigned int AliasesCount;
  unsigned int Params;
  unsigned int Timer;
  TQueryParamsTimerEvent TimerEvent;
  UnicodeString TimerMessage;
  unsigned int TimerAnswers;
  unsigned int Timeout;
  unsigned int TimeoutAnswer;
  unsigned int NoBatchAnswers;
  UnicodeString HelpKeyword;
};

enum TQueryType { qtConfirmation, qtWarning, qtError, qtInformation };

enum TPromptKind
{
  pkPrompt,
  pkFileName,
  pkUserName,
  pkPassphrase,
  pkTIS,
  pkCryptoCard,
  pkKeybInteractive,
  pkPassword,
  pkNewPassword
};

bool __fastcall IsAuthenticationPrompt(TPromptKind Kind);
//---------------------------------------------------------------------------
DEFINE_CALLBACK_TYPE4(TFileFoundEvent, void,
  TTerminal * /* Terminal */, const UnicodeString & /* FileName */, const TRemoteFile * /* File */,
  bool & /* Cancel */);
DEFINE_CALLBACK_TYPE3(TFindingFileEvent, void,
  TTerminal * /* Terminal */, const UnicodeString & /* Directory */, bool & /* Cancel */);
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class ISubpluginsManagerIntf
{
public:
  virtual ~ISubpluginsManagerIntf() = 0 {}

  virtual intptr_t GetFSProtocolsCount() = 0;
  virtual intptr_t GetFSProtocolId(intptr_t Index) = 0;
  virtual UnicodeString GetFSProtocolStr(intptr_t Index) = 0;
  virtual UnicodeString GetFSProtocolStrById(intptr_t ProtocolId) = 0;

  // TCustomFileSystem functions
  virtual fs_handle_t Init(intptr_t ProtocolId, void * Data) = 0;
  virtual void Open(fs_handle_t Handle) = 0;
  virtual void Close(fs_handle_t Handle) = 0;
  virtual bool GetActive(fs_handle_t Handle) = 0;
  // virtual void DoStartup(intptr_t ProtocolId) = 0;
  // virtual const TSessionInfo & GetSessionInfo(intptr_t ProtocolId) = 0;
  // virtual const TFileSystemInfo & GetFileSystemInfo(intptr_t ProtocolId, bool Retrieve) = 0;
  // virtual bool TemporaryTransferFile(intptr_t ProtocolId, const UnicodeString & /*FileName*/) = 0;
  // virtual bool GetStoredCredentialsTried(intptr_t ProtocolId) = 0;
  // virtual UnicodeString GetUserName(intptr_t ProtocolId) = 0;
  // virtual void Idle(intptr_t ProtocolId) = 0;
  // void AnyCommand(intptr_t ProtocolId, const UnicodeString & Command,
    // TCaptureOutputEvent OutputEvent) = 0;
  // virtual UnicodeString AbsolutePath(intptr_t ProtocolId, const UnicodeString & Path, bool /*Local*/) = 0;
  virtual bool IsCapable(fs_handle_t Handle, fs_capability_enum_t Capability) = 0;
  virtual UnicodeString GetCurrentDirectory(fs_handle_t Handle) = 0;
  // void CustomCommandOnFile(intptr_t ProtocolId, const UnicodeString & FileName,
    // const TRemoteFile * File, const UnicodeString & Command, int Params, TCaptureOutputEvent OutputEvent) = 0;
  // virtual void LookupUsersGroups(intptr_t ProtocolId) = 0;
  // virtual void ReadCurrentDirectory(intptr_t ProtocolId) = 0;
  // virtual void HomeDirectory(intptr_t ProtocolId) = 0;
  // virtual void AnnounceFileListOperation(intptr_t ProtocolId) = 0;
  // virtual void ChangeDirectory(intptr_t ProtocolId, const UnicodeString & Directory) = 0;
  // virtual void CachedChangeDirectory(intptr_t ProtocolId, const UnicodeString & Directory) = 0;
  // virtual void ReadDirectory(intptr_t ProtocolId, TRemoteFileList * FileList) = 0;
  // virtual void ReadSymlink(intptr_t ProtocolId, TRemoteFile * SymlinkFile,
    // TRemoteFile *& File) = 0;
  // virtual void ReadFile(intptr_t ProtocolId, const UnicodeString & FileName,
    // TRemoteFile *& File) = 0;
  // virtual void DeleteFile(intptr_t ProtocolId, const UnicodeString & FileName,
    // const TRemoteFile * File, int Params, TRmSessionAction & Action) = 0;
  // virtual void RenameFile(const UnicodeString & FileName,
    // const UnicodeString & NewName) = 0;
  // virtual void CopyFile(intptr_t ProtocolId, const UnicodeString & FileName,
    // const UnicodeString & NewName) = 0;
  // virtual void CreateDirectory(intptr_t ProtocolId, const UnicodeString & DirName) = 0;
  // virtual void CreateLink(intptr_t ProtocolId, const UnicodeString & FileName,
    // const UnicodeString & PointTo, bool Symbolic) = 0;
  // void ChangeFileProperties(intptr_t ProtocolId, const UnicodeString & FileName,
    // const TRemoteFile * File, const TRemoteProperties * Properties,
    // TChmodSessionAction & Action) = 0;
  // virtual bool LoadFilesProperties(intptr_t ProtocolId, TStrings * /*FileList*/ ) = 0;
  // virtual void CalculateFilesChecksum(intptr_t ProtocolId, const UnicodeString & /*Alg*/,
    // TStrings * /*FileList*/, TStrings * /*Checksums*/,
    // TCalculatedChecksumEvent /*OnCalculatedChecksum*/) = 0;
  // virtual UnicodeString FileUrl(intptr_t ProtocolId, const UnicodeString & FileName) = 0;
  // virtual TStrings * GetFixedPaths(intptr_t ProtocolId) = 0;
  // virtual void SpaceAvailable(intptr_t ProtocolId, const UnicodeString & Path,
    // TSpaceAvailable & /* ASpaceAvailable */) = 0;
  // virtual void CopyToRemote(intptr_t ProtocolId, TStrings * FilesToCopy,
    // const UnicodeString & TargetDir, const TCopyParamType * CopyParam,
    // int Params, TFileOperationProgressType * OperationProgress,
    // TOnceDoneOperation & OnceDoneOperation) = 0;
  // virtual void CopyToLocal(intptr_t ProtocolId, TStrings * FilesToCopy,
    // const UnicodeString & TargetDir, const TCopyParamType * CopyParam,
    // int Params, TFileOperationProgressType * OperationProgress,
    // TOnceDoneOperation & OnceDoneOperation) = 0;

    virtual UnicodeString GetSessionUrl(fs_handle_t Handle) = 0;
};

//---------------------------------------------------------------------------
extern ISubpluginsManagerIntf * SubpluginsManager;
//---------------------------------------------------------------------------

#endif
