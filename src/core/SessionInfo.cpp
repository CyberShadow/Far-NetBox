//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include <stdio.h>
#include <share.h>
#include <lmcons.h>
#define SECURITY_WIN32
#include <sspi.h>
#include <secext.h>

#include "Common.h"
#include "SessionInfo.h"
#include "Exceptions.h"
#include "TextsCore.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
static UnicodeString DoXmlEscape(const UnicodeString & Str, bool NewLine)
{
  UnicodeString Result = Str;
  for (intptr_t I = 1; I <= Result.Length(); I++)
  {
    const wchar_t * Repl = nullptr;
    switch (Result[I])
    {
      case L'&':
        Repl = L"amp;";
        break;

      case L'>':
        Repl = L"gt;";
        break;

      case L'<':
        Repl = L"lt;";
        break;

      case L'"':
        Repl = L"quot;";
        break;

      case L'\n':
        if (NewLine)
        {
          Repl = L"#10;";
        }
        break;

      case L'\r':
        Result.Delete(I, 1);
        I--;
        break;
    }

    if (Repl != nullptr)
    {
      Result[I] = L'&';
      Result.Insert(Repl, I + 1);
      I += wcslen(Repl);
    }
  }
  return Result;
}
//---------------------------------------------------------------------------
static UnicodeString XmlEscape(const UnicodeString & Str)
{
  return DoXmlEscape(Str, false);
}
//---------------------------------------------------------------------------
static UnicodeString XmlAttributeEscape(const UnicodeString & Str)
{
  return DoXmlEscape(Str, true);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#pragma warn -inl
class TSessionActionRecord : public TObject
{
public:
  explicit TSessionActionRecord(TActionLog * Log, TLogAction Action) :
    FLog(Log),
    FAction(Action),
    FState(Opened),
    FRecursive(false),
    FErrorMessages(nullptr),
    FNames(new TStringList()),
    FValues(new TStringList()),
    FFileList(nullptr),
    FFile(nullptr)
  {
    FLog->AddPendingAction(this);
  }

  ~TSessionActionRecord()
  {
    delete FErrorMessages;
    delete FNames;
    delete FValues;
    delete FFileList;
    delete FFile;
  }

  void Restart()
  {
    FState = Opened;
    FRecursive = false;
    delete FErrorMessages;
    FErrorMessages = nullptr;
    delete FFileList;
    FFileList = nullptr;
    delete FFile;
    FFile = nullptr;
    FNames->Clear();
    FValues->Clear();
  }

  bool Record()
  {
    bool Result = (FState != Opened);
    if (Result)
    {
      if (FLog->FLogging && (FState != Cancelled))
      {
        const wchar_t * Name = ActionName();
        UnicodeString Attrs;
        if (FRecursive)
        {
          Attrs = L" recursive=\"true\"";
        }
        FLog->AddIndented(FORMAT(L"<%s%s>", Name,  Attrs.c_str()));
        for (intptr_t Index = 0; Index < FNames->GetCount(); ++Index)
        {
          UnicodeString Value = FValues->GetString(Index);
          if (Value.IsEmpty())
          {
            FLog->AddIndented(FORMAT(L"  <%s />", FNames->GetString(Index).c_str()));
          }
          else
          {
            FLog->AddIndented(FORMAT(L"  <%s value=\"%s\" />",
              FNames->GetString(Index).c_str(), XmlAttributeEscape(Value).c_str()));
          }
        }
        if (FFileList != nullptr)
        {
          FLog->AddIndented(L"  <files>");
          for (intptr_t Index = 0; Index < FFileList->GetCount(); ++Index)
          {
            TRemoteFile * File = FFileList->GetFile(Index);

            FLog->AddIndented(L"    <file>");
            FLog->AddIndented(FORMAT(L"      <filename value=\"%s\" />", XmlAttributeEscape(File->GetFileName()).c_str()));
            FLog->AddIndented(FORMAT(L"      <type value=\"%s\" />", XmlAttributeEscape(File->GetType()).c_str()));
            if (!File->GetIsDirectory())
            {
              FLog->AddIndented(FORMAT(L"      <size value=\"%s\" />", Int64ToStr(File->GetSize()).c_str()));
            }
            FLog->AddIndented(FORMAT(L"      <modification value=\"%s\" />", StandardTimestamp(File->GetModification()).c_str()));
            FLog->AddIndented(FORMAT(L"      <permissions value=\"%s\" />", XmlAttributeEscape(File->GetRights()->GetText()).c_str()));
            FLog->AddIndented(L"    </file>");
          }
          FLog->AddIndented(L"  </files>");
        }
        if (FFile != nullptr)
        {
          FLog->AddIndented(L"  <file>");
          FLog->AddIndented(FORMAT(L"    <type value=\"%s\" />", XmlAttributeEscape(FFile->GetType()).c_str()));
          if (!FFile->GetIsDirectory())
          {
            FLog->AddIndented(FORMAT(L"    <size value=\"%s\" />", Int64ToStr(FFile->GetSize()).c_str()));
          }
          FLog->AddIndented(FORMAT(L"    <modification value=\"%s\" />", StandardTimestamp(FFile->GetModification()).c_str()));
          FLog->AddIndented(FORMAT(L"    <permissions value=\"%s\" />", XmlAttributeEscape(FFile->GetRights()->GetText()).c_str()));
          FLog->AddIndented(L"  </file>");
        }
        if (FState == RolledBack)
        {
          if (FErrorMessages != nullptr)
          {
            FLog->AddIndented(L"  <result success=\"false\">");
            FLog->AddMessages(L"    ", FErrorMessages);
            FLog->AddIndented(L"  </result>");
          }
          else
          {
            FLog->AddIndented(L"  <result success=\"false\" />");
          }
        }
        else
        {
          FLog->AddIndented(L"  <result success=\"true\" />");
        }
        FLog->AddIndented(FORMAT(L"</%s>", Name));
      }
      delete this;
    }
    return Result;
  }

  void Commit()
  {
    Close(Committed);
  }

  void Rollback(Exception * E)
  {
    assert(FErrorMessages == nullptr);
    FErrorMessages = ExceptionToMoreMessages(E);
    Close(RolledBack);
  }

  void Cancel()
  {
    Close(Cancelled);
  }

  void FileName(const UnicodeString & FileName)
  {
    Parameter(L"filename", FileName);
  }

  void Destination(const UnicodeString & Destination)
  {
    Parameter(L"destination", Destination);
  }

  void Rights(const TRights & Rights)
  {
    Parameter(L"permissions", Rights.GetText());
  }

  void Modification(const TDateTime & DateTime)
  {
    Parameter(L"modification", StandardTimestamp(DateTime));
  }

  void Recursive()
  {
    FRecursive = true;
  }

  void Command(const UnicodeString & Command)
  {
    Parameter(L"command", Command);
  }

  void AddOutput(const UnicodeString & Output, bool StdError)
  {
    const wchar_t * Name = (StdError ? L"erroroutput" : L"output");
    intptr_t Index = FNames->IndexOf(Name);
    if (Index >= 0)
    {
      FValues->SetString(Index, FValues->GetString(Index) + L"\r\n" + Output);
    }
    else
    {
      Parameter(Name, Output);
    }
  }

  void FileList(TRemoteFileList * FileList)
  {
    if (FFileList == nullptr)
    {
      FFileList = new TRemoteFileList();
    }
    FileList->DuplicateTo(FFileList);
  }

  void File(TRemoteFile * File)
  {
    if (FFile != nullptr)
    {
      delete FFile;
    }
    FFile = File->Duplicate(true);
  }

protected:
  enum TState { Opened, Committed, RolledBack, Cancelled };

  inline void Close(TState State)
  {
    assert(FState == Opened);
    FState = State;
    FLog->RecordPendingActions();
  }

  const wchar_t * ActionName() const
  {
    switch (FAction)
    {
      case laUpload: return L"upload";
      case laDownload: return L"download";
      case laTouch: return L"touch";
      case laChmod: return L"chmod";
      case laMkdir: return L"mkdir";
      case laRm: return L"rm";
      case laMv: return L"mv";
      case laCall: return L"call";
      case laLs: return L"ls";
      case laStat: return L"stat";
      default: assert(false); return L"";
    }
  }

  void Parameter(const UnicodeString & Name, const UnicodeString & Value = L"")
  {
    FNames->Add(Name);
    FValues->Add(Value);
  }

private:
  TActionLog * FLog;
  TLogAction FAction;
  TState FState;
  bool FRecursive;
  TStrings * FErrorMessages;
  TStrings * FNames;
  TStrings * FValues;
  TRemoteFileList * FFileList;
  TRemoteFile * FFile;
};
#pragma warn .inl
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TSessionAction::TSessionAction(TActionLog *Log, TLogAction Action)
{
  if (Log->FLogging)
  {
    FRecord = new TSessionActionRecord(Log, Action);
  }
  else
  {
    FRecord = nullptr;
  }
}
//---------------------------------------------------------------------------
TSessionAction::~TSessionAction()
{
  if (FRecord != nullptr)
  {
    Commit();
  }
}
//---------------------------------------------------------------------------
void TSessionAction::Restart()
{
  if (FRecord != nullptr)
  {
    FRecord->Restart();
  }
}
//---------------------------------------------------------------------------
void TSessionAction::Commit()
{
  if (FRecord != nullptr)
  {
    TSessionActionRecord * Record = FRecord;
    FRecord = nullptr;
    Record->Commit();
  }
}
//---------------------------------------------------------------------------
void TSessionAction::Rollback(Exception * E)
{
  if (FRecord != nullptr)
  {
    TSessionActionRecord * Record = FRecord;
    FRecord = nullptr;
    Record->Rollback(E);
  }
}
//---------------------------------------------------------------------------
void TSessionAction::Cancel()
{
  if (FRecord != nullptr)
  {
    TSessionActionRecord * Record = FRecord;
    FRecord = nullptr;
    Record->Cancel();
  }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TFileSessionAction::TFileSessionAction(TActionLog * Log, TLogAction Action) :
  TSessionAction(Log, Action)
{
}
//---------------------------------------------------------------------------
TFileSessionAction::TFileSessionAction(
    TActionLog * Log, TLogAction Action, const UnicodeString & AFileName) :
  TSessionAction(Log, Action)
{
  FileName(AFileName);
}
//---------------------------------------------------------------------------
void TFileSessionAction::FileName(const UnicodeString & FileName)
{
  if (FRecord != nullptr)
  {
    FRecord->FileName(FileName);
  }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TFileLocationSessionAction::TFileLocationSessionAction(
    TActionLog * Log, TLogAction Action) :
  TFileSessionAction(Log, Action)
{
}
//---------------------------------------------------------------------------
TFileLocationSessionAction::TFileLocationSessionAction(
    TActionLog * Log, TLogAction Action, const UnicodeString & FileName) :
  TFileSessionAction(Log, Action, FileName)
{
}
//---------------------------------------------------------------------------
void TFileLocationSessionAction::Destination(const UnicodeString & Destination)
{
  if (FRecord != nullptr)
  {
    FRecord->Destination(Destination);
  }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TUploadSessionAction::TUploadSessionAction(TActionLog * Log) :
  TFileLocationSessionAction(Log, laUpload)
{
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TDownloadSessionAction::TDownloadSessionAction(TActionLog * Log) :
  TFileLocationSessionAction(Log, laDownload)
{
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TChmodSessionAction::TChmodSessionAction(
    TActionLog * Log, const UnicodeString & FileName) :
  TFileSessionAction(Log, laChmod, FileName)
{
}
//---------------------------------------------------------------------------
void TChmodSessionAction::Recursive()
{
  if (FRecord != nullptr)
  {
    FRecord->Recursive();
  }
}
//---------------------------------------------------------------------------
TChmodSessionAction::TChmodSessionAction(
    TActionLog * Log, const UnicodeString & FileName, const TRights & ARights) :
  TFileSessionAction(Log, laChmod, FileName)
{
  Rights(ARights);
}
//---------------------------------------------------------------------------
void TChmodSessionAction::Rights(const TRights & Rights)
{
  if (FRecord != nullptr)
  {
    FRecord->Rights(Rights);
  }
}
//---------------------------------------------------------------------------
TTouchSessionAction::TTouchSessionAction(
    TActionLog * Log, const UnicodeString & FileName, const TDateTime & Modification) :
  TFileSessionAction(Log, laTouch, FileName)
{
  if (FRecord != nullptr)
  {
    FRecord->Modification(Modification);
  }
}
//---------------------------------------------------------------------------
TMkdirSessionAction::TMkdirSessionAction(
    TActionLog * Log, const UnicodeString & FileName) :
  TFileSessionAction(Log, laMkdir, FileName)
{
}
//---------------------------------------------------------------------------
TRmSessionAction::TRmSessionAction(
    TActionLog * Log, const UnicodeString & FileName) :
  TFileSessionAction(Log, laRm, FileName)
{
}
//---------------------------------------------------------------------------
void TRmSessionAction::Recursive()
{
  if (FRecord != nullptr)
  {
    FRecord->Recursive();
  }
}
//---------------------------------------------------------------------------
TMvSessionAction::TMvSessionAction(TActionLog * Log,
    const UnicodeString & FileName, const UnicodeString & ADestination) :
  TFileLocationSessionAction(Log, laMv, FileName)
{
  Destination(ADestination);
}
//---------------------------------------------------------------------------
TCallSessionAction::TCallSessionAction(TActionLog * Log,
    const UnicodeString & Command, const UnicodeString & Destination) :
  TSessionAction(Log, laCall)
{
  if (FRecord != nullptr)
  {
    FRecord->Command(Command);
    FRecord->Destination(Destination);
  }
}
//---------------------------------------------------------------------------
void TCallSessionAction::AddOutput(const UnicodeString & Output, bool StdError)
{
  if (FRecord != nullptr)
  {
    FRecord->AddOutput(Output, StdError);
  }
}
//---------------------------------------------------------------------------
TLsSessionAction::TLsSessionAction(TActionLog * Log,
    const UnicodeString & Destination) :
  TSessionAction(Log, laLs)
{
  if (FRecord != nullptr)
  {
    FRecord->Destination(Destination);
  }
}
//---------------------------------------------------------------------------
void TLsSessionAction::FileList(TRemoteFileList * FileList)
{
  if (FRecord != nullptr)
  {
    FRecord->FileList(FileList);
  }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TStatSessionAction::TStatSessionAction(TActionLog * Log, const UnicodeString & FileName) :
  TFileSessionAction(Log, laStat, FileName)
{
}
//---------------------------------------------------------------------------
void TStatSessionAction::File(TRemoteFile * File)
{
  if (FRecord != nullptr)
  {
    FRecord->File(File);
  }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TSessionInfo::TSessionInfo()
{
  LoginTime = Now();
}
//---------------------------------------------------------------------------
TFileSystemInfo::TFileSystemInfo()
{
  memset(&IsCapable, 0, sizeof(IsCapable));
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
FILE * OpenFile(const UnicodeString & LogFileName, TSessionData * SessionData, bool Append, UnicodeString & NewFileName)
{
  FILE * Result;
  UnicodeString ANewFileName = StripPathQuotes(GetExpandedLogFileName(LogFileName, SessionData));
  // Result = _wfopen(ANewFileName.c_str(), (Append ? L"a" : L"w"));
  Result = _fsopen(W2MB(ANewFileName.c_str()).c_str(),
    Append ? "a" : "w", SH_DENYWR); // _SH_DENYNO); //
  if (Result != nullptr)
  {
    setvbuf(Result, nullptr, _IONBF, BUFSIZ);
    NewFileName = ANewFileName;
  }
  else
  {
    throw Exception(FMTLOAD(LOG_OPENERROR, ANewFileName.c_str()));
  }
  return Result;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
const wchar_t *LogLineMarks = L"<>!.*";
TSessionLog::TSessionLog(TSessionUI * UI, TSessionData * SessionData,
  TConfiguration * Configuration):
  TStringList()
{
  FCriticalSection = new TCriticalSection();
  FLogging = false;
  FConfiguration = Configuration;
  FParent = nullptr;
  FUI = UI;
  FSessionData = SessionData;
  FFile = nullptr;
  FLoggedLines = 0;
  FTopIndex = -1;
  FCurrentLogFileName = L"";
  FCurrentFileName = L"";
  FClosed = false;
}
//---------------------------------------------------------------------------
TSessionLog::~TSessionLog()
{
  FClosed = true;
  ReflectSettings();
  assert(FFile == nullptr);
  delete FCriticalSection;
}
//---------------------------------------------------------------------------
void TSessionLog::Lock()
{
  FCriticalSection->Enter();
}
//---------------------------------------------------------------------------
void TSessionLog::Unlock()
{
  FCriticalSection->Leave();
}
//---------------------------------------------------------------------------
UnicodeString TSessionLog::GetSessionName() const
{
  assert(FSessionData != nullptr);
  return FSessionData->GetSessionName();
}
//---------------------------------------------------------------------------
UnicodeString TSessionLog::GetLine(intptr_t Index) const
{
  return GetString(Index - FTopIndex);
}
//---------------------------------------------------------------------------
TLogLineType TSessionLog::GetType(intptr_t Index) const
{
  return static_cast<TLogLineType>(reinterpret_cast<size_t>(GetObject(Index - FTopIndex)));
}
//---------------------------------------------------------------------------
void TSessionLog::DoAddToParent(TLogLineType AType, const UnicodeString & ALine)
{
  assert(FParent != nullptr);
  FParent->Add(AType, ALine);
}
//---------------------------------------------------------------------------
void TSessionLog::DoAddToSelf(TLogLineType AType, const UnicodeString & ALine)
{
  if (FTopIndex < 0)
  {
    FTopIndex = 0;
  }

  TStringList::AddObject(ALine, static_cast<TObject *>(reinterpret_cast<void *>(static_cast<size_t>(AType))));

  FLoggedLines++;

  if (LogToFile())
  {
    if (FFile == nullptr)
    {
      OpenLogFile();
    }

    if (FFile != nullptr)
    {
#if defined(__BORLANDC__)
      UnicodeString Timestamp = FormatDateTime(L" yyyy-mm-dd hh:nn:ss.zzz ", Now());
      UTF8String UtfLine = UTF8String(UnicodeString(LogLineMarks[Type]) + Timestamp + Line + "\n");
      fwrite(UtfLine.c_str(), UtfLine.Length(), 1, (FILE *)FFile);
#else
      unsigned short Y, M, D, H, N, S, MS;
      TDateTime DateTime = Now();
      DateTime.DecodeDate(Y, M, D);
      DateTime.DecodeTime(H, N, S, MS);
      UnicodeString dt = FORMAT(L" %04d-%02d-%02d %02d:%02d:%02d.%03d ", Y, M, D, H, N, S, MS);
      UnicodeString Timestamp = dt;
      UTF8String UtfLine = UTF8String(UnicodeString(LogLineMarks[AType]) + Timestamp + ALine + "\n");
      fprintf_s(static_cast<FILE *>(FFile), "%s", const_cast<char *>(AnsiString(UtfLine).c_str()));
#endif
    }
  }
}
//---------------------------------------------------------------------------
void TSessionLog::DoAdd(TLogLineType AType, const UnicodeString & Line,
  TDoAddLogEvent Event)
{
  UnicodeString Prefix;

  if (!GetName().IsEmpty())
  {
    Prefix = L"[" + GetName() + L"] ";
  }

  UnicodeString Ln = Line;
  while (!Ln.IsEmpty())
  {
    // UnicodeString Param = ;
    Event(AType, Prefix + CutToChar(Ln, L'\n', false));
  }
}
//---------------------------------------------------------------------------
void TSessionLog::Add(TLogLineType Type, const UnicodeString & Line)
{
  assert(FConfiguration);
  if (GetLogging())
  {
    try
    {
      if (FParent != nullptr)
      {
        DoAdd(Type, Line, MAKE_CALLBACK(TSessionLog::DoAddToParent, this));
      }
      else
      {
        TGuard Guard(FCriticalSection);

        BeginUpdate();
        SCOPE_EXIT
        {
          DeleteUnnecessary();
          EndUpdate();
        };
        {
          DoAdd(Type, Line, MAKE_CALLBACK(TSessionLog::DoAddToSelf, this));
        }
      }
    }
    catch (Exception &E)
    {
      // We failed logging, turn it off and notify user.
      FConfiguration->SetLogging(false);
      try
      {
        throw ExtException(&E, LoadStr(LOG_GEN_ERROR));
      }
      catch (Exception &E)
      {
        AddException(&E);
        FUI->HandleExtendedException(&E);
      }
    }
  }
}
//---------------------------------------------------------------------------
void TSessionLog::AddException(Exception * E)
{
  if (E != nullptr)
  {
    Add(llException, ExceptionLogString(E));
  }
}
//---------------------------------------------------------------------------
void TSessionLog::ReflectSettings()
{
  TGuard Guard(FCriticalSection);

  bool ALogging =
    !FClosed &&
    ((FParent != nullptr) || FConfiguration->GetLogging());

  if (FLogging != ALogging)
  {
    FLogging = ALogging;
    StateChange();
  }

  // if logging to file was turned off or log file was changed -> close current log file
  if ((FFile != nullptr) &&
      (!LogToFile() || (FCurrentLogFileName != FConfiguration->GetLogFileName())))
  {
    CloseLogFile();
  }

  DeleteUnnecessary();
}
//---------------------------------------------------------------------------
bool TSessionLog::LogToFile() const
{
  return GetLogging() && FConfiguration->GetLogToFile() && (FParent == nullptr);
}
//---------------------------------------------------------------------------
void TSessionLog::CloseLogFile()
{
  if (FFile != nullptr)
  {
    fclose(static_cast<FILE *>(FFile));
    FFile = nullptr;
  }
  FCurrentLogFileName = L"";
  FCurrentFileName = L"";
  StateChange();
}
//---------------------------------------------------------------------------
void TSessionLog::OpenLogFile()
{
  try
  {
    assert(FFile == nullptr);
    assert(FConfiguration != nullptr);
    FCurrentLogFileName = FConfiguration->GetLogFileName();
    FFile = OpenFile(FCurrentLogFileName, FSessionData, FConfiguration->GetLogFileAppend(), FCurrentFileName);
  }
  catch (Exception & E)
  {
    // We failed logging to file, turn it off and notify user.
    FCurrentLogFileName = L"";
    FCurrentFileName = L"";
    FConfiguration->SetLogFileName(UnicodeString());
    try
    {
      throw ExtException(&E, LoadStr(LOG_GEN_ERROR));
    }
    catch (Exception & E)
    {
      AddException(&E);
      FUI->HandleExtendedException(&E);
    }
  }
  StateChange();
}
//---------------------------------------------------------------------------
void TSessionLog::StateChange()
{
  if (FOnStateChange != nullptr)
  {
    FOnStateChange(this);
  }
}
//---------------------------------------------------------------------------
void TSessionLog::DeleteUnnecessary()
{
  BeginUpdate();
  SCOPE_EXIT
  {
    EndUpdate();
  };
  {
    if (!GetLogging() || (FParent != nullptr))
    {
      Clear();
    }
    else
    {
      while (!FConfiguration->GetLogWindowComplete() && (GetCount() > FConfiguration->GetLogWindowLines()))
      {
        Delete(0);
        ++FTopIndex;
      }
    }
  }
}
//---------------------------------------------------------------------------
void TSessionLog::AddSystemInfo()
{
  AddStartupInfo(true);
}
//---------------------------------------------------------------------------
void TSessionLog::AddStartupInfo()
{
  AddStartupInfo(false);
}
//---------------------------------------------------------------------------
void TSessionLog::AddStartupInfo(bool System)
{
  TSessionData * Data = (System ? nullptr : FSessionData);
  if (GetLogging())
  {
    if (FParent != nullptr)
    {
      // do not add session info for secondary session
      // (this should better be handled in the TSecondaryTerminal)
    }
    else
    {
      DoAddStartupInfo(Data);
    }
  }
}
//---------------------------------------------------------------------------
UnicodeString TSessionLog::GetTlsVersionName(TTlsVersion TlsVersion)
{
  switch (TlsVersion)
  {
    default:
      FAIL;
    case ssl2:
      return "SSLv2";
    case ssl3:
      return "SSLv3";
    case tls10:
      return "TLSv1.0";
    case tls11:
      return "TLSv1.1";
    case tls12:
      return "TLSv1.2";
  }
}
//---------------------------------------------------------------------------
void TSessionLog::DoAddStartupInfo(TSessionData * Data)
{
  TGuard Guard(FCriticalSection);

  BeginUpdate();
  {
    SCOPE_EXIT
    {
      DeleteUnnecessary();
      EndUpdate();
    };
    #define ADSTR(S, ...) DoAdd(llMessage, FORMAT(S, ##__VA_ARGS__), MAKE_CALLBACK(TSessionLog::DoAddToSelf, this));
    #define ADF(S, ...) ADSTR(FORMAT(S, ##__VA_ARGS__));
    if (Data == nullptr)
    {
      AddSeparator();
      ADF(L"NetBox %s (OS %s)", FConfiguration->GetVersionStr().c_str(), FConfiguration->GetOSVersionStr().c_str());
      std::unique_ptr<THierarchicalStorage> Storage(FConfiguration->CreateStorage(false));
      assert(Storage.get());
      ADF(L"Configuration: %s", Storage->GetSource().c_str());

      if (0)
      {
        typedef BOOL (WINAPI * TGetUserNameEx)(EXTENDED_NAME_FORMAT NameFormat, LPWSTR lpNameBuffer, PULONG nSize);
        HINSTANCE Secur32 = LoadLibrary(L"secur32.dll");
        TGetUserNameEx GetUserNameEx =
          (Secur32 != nullptr) ? reinterpret_cast<TGetUserNameEx>(GetProcAddress(Secur32, "GetUserNameExW")) : nullptr;
        wchar_t UserName[UNLEN + 1];
        unsigned long UserNameSize = LENOF(UserName);
        if ((GetUserNameEx == nullptr) || !GetUserNameEx(NameSamCompatible, (LPWSTR)UserName, &UserNameSize))
        {
          wcscpy(UserName, L"<Failed to retrieve username>");
        }
        ADF(L"Local account: %s", UserName);
      }
      unsigned short Y, M, D, H, N, S, MS;
      TDateTime DateTime = Now();
      DateTime.DecodeDate(Y, M, D);
      DateTime.DecodeTime(H, N, S, MS);
      UnicodeString dt = FORMAT(L"%02d.%02d.%04d %02d:%02d:%02d", D, M, Y, H, N, S);
      // ADF(L"Login time: %s", FormatDateTime(L"dddddd tt", Now()).c_str());
      ADF(L"Working directory: %s", GetCurrentDir().c_str());
      // ADF(L"Command-line: %s", CmdLine.c_str());
      // ADF(L"Time zone: %s", GetTimeZoneLogString().c_str());
      if (!AdjustClockForDSTEnabled())
      {
        ADSTR(L"Warning: System option \"Automatically adjust clock for Daylight Saving Time\" is disabled, timestamps will not be represented correctly");
      }
      ADF(L"Login time: %s", dt.c_str());
      AddSeparator();
    }
    else
    {
      if (0)
      {
        ADF(L"Session name: %s (%s)", Data->GetSessionName().c_str(), Data->GetSource().c_str());
      }
      ADF(L"Host name: %s (Port: %d)", Data->GetHostNameExpanded().c_str(), Data->GetPortNumber());
      if (0)
      {
        ADF(L"User name: %s (Password: %s, Key file: %s)",
          Data->GetUserNameExpanded().c_str(), BooleanToEngStr(!Data->GetPassword().IsEmpty()).c_str(),
           BooleanToEngStr(!Data->GetPublicKeyFile().IsEmpty()).c_str())
      }
      ADF(L"Tunnel: %s", BooleanToEngStr(Data->GetTunnel()).c_str());
      if (Data->GetTunnel())
      {
        ADF(L"Tunnel: Host name: %s (Port: %d)", Data->GetTunnelHostName().c_str(), Data->GetTunnelPortNumber());
        if (0)
        {
          ADF(L"Tunnel: User name: %s (Password: %s, Key file: %s)",
            Data->GetTunnelUserName().c_str(), BooleanToEngStr(!Data->GetTunnelPassword().IsEmpty()).c_str(),
             BooleanToEngStr(!Data->GetTunnelPublicKeyFile().IsEmpty()).c_str());
            ADF(L"Tunnel: Local port number: %d", Data->GetTunnelLocalPortNumber());
        }
      }
      ADF(L"Transfer Protocol: %s", Data->GetFSProtocolStr().c_str());
      ADF(L"Code Page: %d", Data->GetCodePageAsNumber());
      wchar_t * PingTypes = L"-NC";
      TPingType PingType;
      intptr_t PingInterval;
      if (Data->GetFSProtocol() == fsFTP)
      {
        PingType = Data->GetFtpPingType();
        PingInterval = Data->GetFtpPingInterval();
      }
      else
      {
        PingType = Data->GetPingType();
        PingInterval = Data->GetPingInterval();
      }
      ADF(L"Ping type: %s, Ping interval: %d sec; Timeout: %d sec",
        UnicodeString(PingTypes[PingType]).c_str(), PingInterval, Data->GetTimeout());
      ADF(L"Proxy: %s%s", ProxyMethodList[Data->GetProxyMethod()],
        Data->GetProxyMethod() == pmSystem ?
          ::Format(L" (%s)", ProxyMethodList[Data->GetActualProxyMethod()]).c_str() :
          L"")
      if (Data->GetProxyMethod() != ::pmNone)
      {
        ADF(L"HostName: %s (Port: %d); Username: %s; Passwd: %s",
          Data->GetProxyHost().c_str(), Data->GetProxyPort(),
           Data->GetProxyUsername().c_str(), BooleanToEngStr(!Data->GetProxyPassword().IsEmpty()).c_str());
        if (Data->GetProxyMethod() == pmTelnet)
        {
          ADF(L"Telnet command: %s", Data->GetProxyTelnetCommand().c_str());
        }
        if (Data->GetProxyMethod() == pmCmd)
        {
          ADF(L"Local command: %s", Data->GetProxyLocalCommand().c_str());
        }
      }
      ADF(L"Send buffer: %d", Data->GetSendBuf());
      wchar_t const * BugFlags = L"+-A";
      if (Data->GetUsesSsh())
      {
        ADF(L"SSH protocol version: %s; Compression: %s",
          Data->GetSshProtStr().c_str(), BooleanToEngStr(Data->GetCompression()).c_str());
        ADF(L"Bypass authentication: %s",
         BooleanToEngStr(Data->GetSshNoUserAuth()).c_str());
        ADF(L"Try agent: %s; Agent forwarding: %s; TIS/CryptoCard: %s; KI: %s; GSSAPI: %s",
           BooleanToEngStr(Data->GetTryAgent()).c_str(), BooleanToEngStr(Data->GetAgentFwd()).c_str(), BooleanToEngStr(Data->GetAuthTIS()).c_str(),
           BooleanToEngStr(Data->GetAuthKI()).c_str(), BooleanToEngStr(Data->GetAuthGSSAPI()).c_str());
        if (Data->GetAuthGSSAPI())
        {
          ADF(L"GSSAPI: Forwarding: %s; Server realm: %s",
            BooleanToEngStr(Data->GetGSSAPIFwdTGT()).c_str(), Data->GetGSSAPIServerRealm().c_str());
        }
        ADF(L"Ciphers: %s; Ssh2DES: %s",
          Data->GetCipherList().c_str(), BooleanToEngStr(Data->GetSsh2DES()).c_str());
        UnicodeString Bugs;
        for (intptr_t Index = 0; Index < BUG_COUNT; ++Index)
        {
          Bugs += UnicodeString(BugFlags[Data->GetBug(static_cast<TSshBug>(Index))])+(Index<BUG_COUNT-1?L",":L"");
        }
        ADF(L"SSH Bugs: %s", Bugs.c_str());
        ADF(L"Simple channel: %s", BooleanToEngStr(Data->GetSshSimple()).c_str());
        ADF(L"Return code variable: %s; Lookup user groups: %c",
           Data->GetDetectReturnVar() ? UnicodeString(L"Autodetect").c_str() : Data->GetReturnVar().c_str(),
           BugFlags[Data->GetLookupUserGroups()]);
        ADF(L"Shell: %s", Data->GetShell().IsEmpty() ? UnicodeString(L"default").c_str() : Data->GetShell().c_str());
        ADF(L"EOL: %d, UTF: %d", Data->GetEOLType(), Data->GetNotUtf());
        ADF(L"Clear aliases: %s, Unset nat.vars: %s, Resolve symlinks: %s",
           BooleanToEngStr(Data->GetClearAliases()).c_str(), BooleanToEngStr(Data->GetUnsetNationalVars()).c_str(),
           BooleanToEngStr(Data->GetResolveSymlinks()).c_str());
        ADF(L"LS: %s, Ign LS warn: %s, Scp1 Comp: %s",
           Data->GetListingCommand().c_str(),
           BooleanToEngStr(Data->GetIgnoreLsWarnings()).c_str(),
           BooleanToEngStr(Data->GetScp1Compatibility()).c_str());
      }
      if (Data->GetFSProtocol() == fsSFTP)
      {
        UnicodeString Bugs;
        for (int Index = 0; Index < SFTP_BUG_COUNT; Index++)
        {
          Bugs += UnicodeString(BugFlags[Data->GetSFTPBug(static_cast<TSftpBug>(Index))])+(Index<SFTP_BUG_COUNT-1 ? L"," : L"");
        }
        ADF(L"SFTP Bugs: %s", Bugs.c_str());
        ADF(L"SFTP Server: %s", Data->GetSftpServer().IsEmpty()? UnicodeString(L"default").c_str() : Data->GetSftpServer().c_str());
      }
      if (Data->GetFSProtocol() == fsFTP)
      {
        UnicodeString Ftps;
        switch (Data->GetFtps())
        {
          case ftpsImplicit:
            Ftps = L"Implicit TLS/SSL";
            break;

          case ftpsExplicitSsl:
            Ftps = L"Explicit SSL";
            break;

          case ftpsExplicitTls:
            Ftps = L"Explicit TLS";
            break;

          default:
            assert(Data->GetFtps() == ftpsNone);
            Ftps = L"None";
            break;
        }
        ADF(L"FTP: FTPS: %s; Passive: %s [Force IP: %c]; MLSD: %c  [List all: %c]",
           Ftps.c_str(), BooleanToEngStr(Data->GetFtpPasvMode()).c_str(),
           BugFlags[Data->GetFtpForcePasvIp()],
           BugFlags[Data->GetFtpUseMlsd()],
           BugFlags[Data->GetFtpListAll()]);
        if (Data->GetFtps() != ftpsNone)
        {
          ADF(L"Session reuse: %s", BooleanToEngStr(Data->GetSslSessionReuse()).c_str());
          ADF(L"TLS/SSL versions: %s-%s", GetTlsVersionName(Data->GetMinTlsVersion()).c_str(), GetTlsVersionName(Data->GetMaxTlsVersion()).c_str());
        }
      }
      ADF(L"Local directory: %s, Remote directory: %s, Update: %s, Cache: %s",
        (Data->GetLocalDirectory().IsEmpty() ? UnicodeString(L"default").c_str() : Data->GetLocalDirectory().c_str()),
         (Data->GetRemoteDirectory().IsEmpty() ? UnicodeString(L"home").c_str() : Data->GetRemoteDirectory().c_str()),
         BooleanToEngStr(Data->GetUpdateDirectories()).c_str(),
         BooleanToEngStr(Data->GetCacheDirectories()).c_str());
      ADF(L"Cache directory changes: %s, Permanent: %s",
         BooleanToEngStr(Data->GetCacheDirectoryChanges()).c_str(),
         BooleanToEngStr(Data->GetPreserveDirectoryChanges()).c_str());
      intptr_t TimeDifferenceMin = TimeToMinutes(Data->GetTimeDifference());
      ADF(L"DST mode: %d; Timezone offset: %dh %dm", static_cast<int>(Data->GetDSTMode()), (TimeDifferenceMin / MinsPerHour), (TimeDifferenceMin % MinsPerHour));

      if (Data->GetFSProtocol() == fsWebDAV)
      {
        ADF(L"Compression: %s",
          BooleanToEngStr(Data->GetCompression()).c_str());
      }

      AddSeparator();
    }

    #undef ADF
    #undef ADSTR
  }
//  __finally
//  {
//    DeleteUnnecessary();

//    EndUpdate();
//  }
}
//---------------------------------------------------------------------------
void TSessionLog::AddSeparator()
{
  Add(llMessage, L"--------------------------------------------------------------------------");
}
//---------------------------------------------------------------------------
intptr_t TSessionLog::GetBottomIndex() const
{
  return (GetCount() > 0 ? (GetTopIndex() + GetCount() - 1) : -1);
}
//---------------------------------------------------------------------------
bool TSessionLog::GetLoggingToFile() const
{
  assert((FFile == nullptr) || LogToFile());
  return (FFile != nullptr);
}
//---------------------------------------------------------------------------
void TSessionLog::Clear()
{
  TGuard Guard(FCriticalSection);

  FTopIndex += GetCount();
  TStringList::Clear();
}
//---------------------------------------------------------------------------
TSessionLog * TSessionLog::GetParent()
{
  return FParent;
}
//---------------------------------------------------------------------------
void TSessionLog::SetParent(TSessionLog * Value)
{
  FParent = Value;
}
//---------------------------------------------------------------------------
bool TSessionLog::GetLogging() const
{
  return FLogging;
}
//---------------------------------------------------------------------------
TNotifyEvent & TSessionLog::GetOnChange()
{
  return TStringList::GetOnChange();
}
//---------------------------------------------------------------------------
void TSessionLog::SetOnChange(TNotifyEvent Value)
{
  TStringList::SetOnChange(Value);
}
//---------------------------------------------------------------------------
TNotifyEvent & TSessionLog::GetOnStateChange()
{
  return FOnStateChange;
}
//---------------------------------------------------------------------------
void TSessionLog::SetOnStateChange(TNotifyEvent Value)
{
  FOnStateChange = Value;
}
//---------------------------------------------------------------------------
UnicodeString TSessionLog::GetCurrentFileName() const
{
  return FCurrentFileName;
}
//---------------------------------------------------------------------------
intptr_t TSessionLog::GetTopIndex() const
{
  return FTopIndex;
}
//---------------------------------------------------------------------------
UnicodeString TSessionLog::GetName() const
{
  return FName;
}
//---------------------------------------------------------------------------
void TSessionLog::SetName(const UnicodeString & Value)
{
  FName = Value;
}
//---------------------------------------------------------------------------
intptr_t TSessionLog::GetCount() const
{
  return TStringList::GetCount();
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TActionLog::TActionLog(TSessionUI * UI, TSessionData * SessionData,
  TConfiguration * Configuration) :
  FConfiguration(Configuration),
  FCriticalSection(new TCriticalSection),
  FLogging(false),
  FFile(nullptr),
  FUI(UI),
  FSessionData(SessionData),
  FPendingActions(new TList()),
  FClosed(false),
  FInGroup(false),
  FEnabled(true),
  FIndent(L"  ")
{
}
//---------------------------------------------------------------------------
TActionLog::~TActionLog()
{
  assert(FPendingActions->GetCount() == 0);
  delete FPendingActions;
  FClosed = true;
  ReflectSettings();
  assert(FFile == nullptr);
  delete FCriticalSection;
}
//---------------------------------------------------------------------------
void TActionLog::Add(const UnicodeString & Line)
{
  assert(FConfiguration);
  if (FLogging)
  {
    try
    {
      TGuard Guard(FCriticalSection);
      if (FFile == nullptr)
      {
        OpenLogFile();
      }

      if (FFile != nullptr)
      {
        UTF8String UtfLine = UTF8String(Line);
        fwrite(UtfLine.c_str(), 1, UtfLine.Length(), (FILE *)FFile);
        fwrite("\n", 1, 1, (FILE *)FFile);
      }
    }
    catch (Exception &E)
    {
      // We failed logging, turn it off and notify user.
      FConfiguration->SetLogActions(false);
      try
      {
        throw ExtException(&E, LoadStr(LOG_GEN_ERROR));
      }
      catch (Exception &E)
      {
        FUI->HandleExtendedException(&E);
      }
    }
  }
}
//---------------------------------------------------------------------------
void TActionLog::AddIndented(const UnicodeString & Line)
{
  Add(FIndent + Line);
}
//---------------------------------------------------------------------------
void TActionLog::AddFailure(TStrings * Messages)
{
  AddIndented(L"<failure>");
  AddMessages(L"  ", Messages);
  AddIndented(L"</failure>");
}
//---------------------------------------------------------------------------
void TActionLog::AddFailure(Exception * E)
{
  std::unique_ptr<TStrings> Messages(ExceptionToMoreMessages(E));
  if (Messages.get() != nullptr)
  {
    AddFailure(Messages.get());
  }
}
//---------------------------------------------------------------------------
void TActionLog::AddMessages(const UnicodeString & Indent, TStrings * Messages)
{
  for (intptr_t Index = 0; Index < Messages->GetCount(); ++Index)
  {
    AddIndented(
      FORMAT((Indent + L"<message>%s</message>").c_str(), XmlEscape(Messages->GetString(Index)).c_str()));
  }
}
//---------------------------------------------------------------------------
void TActionLog::ReflectSettings()
{
  TGuard Guard(FCriticalSection);

  bool ALogging =
    !FClosed && FConfiguration->GetLogActions() && GetEnabled();

  if (ALogging && !FLogging)
  {
    FLogging = true;
    Add(L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    Add(FORMAT(L"<session xmlns=\"http://winscp.net/schema/session/1.0\" name=\"%s\" start=\"%s\">",
      XmlAttributeEscape(FSessionData->GetSessionName()).c_str(), StandardTimestamp().c_str()));
  }
  else if (!ALogging && FLogging)
  {
    if (FInGroup)
    {
      EndGroup();
    }
    // do not try to close the file, if it has not been opened, to avoid recursion
    if (FFile != nullptr)
    {
      Add(L"</session>");
    }
    CloseLogFile();
    FLogging = false;
  }

}
//---------------------------------------------------------------------------
void TActionLog::CloseLogFile()
{
  if (FFile != nullptr)
  {
    fclose((FILE *)FFile);
    FFile = nullptr;
  }
  FCurrentLogFileName = L"";
  FCurrentFileName = L"";
}
//---------------------------------------------------------------------------
void TActionLog::OpenLogFile()
{
  try
  {
    assert(FFile == nullptr);
    assert(FConfiguration != nullptr);
    FCurrentLogFileName = FConfiguration->GetActionsLogFileName();
    FFile = OpenFile(FCurrentLogFileName, FSessionData, false, FCurrentFileName);
  }
  catch (Exception & E)
  {
    // We failed logging to file, turn it off and notify user.
    FCurrentLogFileName = L"";
    FCurrentFileName = L"";
    FConfiguration->SetLogActions(false);
    try
    {
      throw ExtException(&E, LoadStr(LOG_GEN_ERROR));
    }
    catch (Exception & E)
    {
      FUI->HandleExtendedException(&E);
    }
  }
}
//---------------------------------------------------------------------------
void TActionLog::AddPendingAction(TSessionActionRecord * Action)
{
  FPendingActions->Add(Action);
}
//---------------------------------------------------------------------------
void TActionLog::RecordPendingActions()
{
  while ((FPendingActions->GetCount() > 0) &&
         static_cast<TSessionActionRecord *>(FPendingActions->GetItem(0))->Record())
  {
    FPendingActions->Delete(0);
  }
}
//---------------------------------------------------------------------------
void TActionLog::BeginGroup(const UnicodeString & Name)
{
  assert(!FInGroup);
  FInGroup = true;
  assert(FIndent == L"  ");
  AddIndented(FORMAT(L"<group name=\"%s\" start=\"%s\">",
    XmlAttributeEscape(Name).c_str(), StandardTimestamp().c_str()));
  FIndent = L"    ";
}
//---------------------------------------------------------------------------
void TActionLog::EndGroup()
{
  assert(FInGroup);
  FInGroup = false;
  assert(FIndent == L"    ");
  FIndent = L"  ";
  // this is called from ReflectSettings that in turn is called when logging fails,
  // so do not try to close the group, if it has not been opened, to avoid recursion
  if (FFile != nullptr)
  {
    AddIndented("</group>");
  }
}
//---------------------------------------------------------------------------
void TActionLog::SetEnabled(bool Value)
{
  if (GetEnabled() != Value)
  {
    FEnabled = Value;
    ReflectSettings();
  }
}

