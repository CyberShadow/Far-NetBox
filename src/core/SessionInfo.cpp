//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#define TRACE_LOG_ADD TRACING
#define TRACE_LOG_ADD2 NOTRACING

#include <stdio.h>
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
UnicodeString DoXmlEscape(UnicodeString Str, bool NewLine)
{
  for (int i = 1; i <= Str.Length(); i++)
  {
    const wchar_t * Repl = NULL;
    switch (Str[i])
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
        Str.Delete(i, 1);
        i--;
        break;
    }

    if (Repl != NULL)
    {
      Str[i] = L'&';
      Str.Insert(Repl, i + 1);
      i += static_cast<int>(wcslen(Repl));
    }
  }
  return Str;
}
//---------------------------------------------------------------------------
UnicodeString XmlEscape(UnicodeString Str)
{
  return DoXmlEscape(Str, false);
}
//---------------------------------------------------------------------------
UnicodeString XmlAttributeEscape(UnicodeString Str)
{
  return DoXmlEscape(Str, true);
}
//---------------------------------------------------------------------------
TStrings * ExceptionToMessages(Exception * E)
{
  TStrings * Result = NULL;
  UnicodeString Message;
  if (ExceptionMessage(E, Message))
  {
    Result = new TStringList();
    Result->Add(Message);
    ExtException * EE = dynamic_cast<ExtException *>(E);
    if ((EE != NULL) && (EE->GetMoreMessages() != NULL))
    {
      Result->AddStrings(EE->GetMoreMessages());
    }
  }
  return Result;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#pragma warn -inl
class TSessionActionRecord
{
public:
  explicit TSessionActionRecord(TActionLog * Log, TLogAction Action) :
    FLog(Log),
    FAction(Action),
    FState(Opened),
    FRecursive(false),
    FErrorMessages(NULL),
    FNames(new TStringList()),
    FValues(new TStringList()),
    FFileList(NULL),
    FFile(NULL)
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
    FErrorMessages = NULL;
    delete FFileList;
    FFileList = NULL;
    delete FFile;
    FFile = NULL;
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
          UnicodeString Value = FValues->Strings[Index];
          if (Value.IsEmpty())
          {
            FLog->AddIndented(FORMAT(L"  <%s />", FNames->Strings[Index].c_str()));
          }
          else
          {
            FLog->AddIndented(FORMAT(L"  <%s value=\"%s\" />",
              FNames->Strings[Index].c_str(), XmlAttributeEscape(Value).c_str()));
          }
        }
        if (FFileList != NULL)
        {
          FLog->AddIndented(L"  <files>");
          for (intptr_t Index = 0; Index < FFileList->GetCount(); ++Index)
          {
            TRemoteFile * File = FFileList->GetFiles(Index);

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
        if (FFile != NULL)
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
          if (FErrorMessages != NULL)
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
    assert(FErrorMessages == NULL);
    FErrorMessages = ExceptionToMessages(E);
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

  void AddOutput(UnicodeString Output, bool StdError)
  {
    const wchar_t * Name = (StdError ? L"erroroutput" : L"output");
    intptr_t Index = FNames->IndexOf(Name);
    if (Index >= 0)
    {
      FValues->Strings[Index] = FValues->Strings[Index] + L"\r\n" + Output;
    }
    else
    {
      Parameter(Name, Output);
    }
  }

  void FileList(TRemoteFileList * FileList)
  {
    if (FFileList == NULL)
    {
      FFileList = new TRemoteFileList();
    }
    FileList->DuplicateTo(FFileList);
  }

  void File(TRemoteFile * File)
  {
    if (FFile != NULL)
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
    FRecord = NULL;
  }
}
//---------------------------------------------------------------------------
TSessionAction::~TSessionAction()
{
  if (FRecord != NULL)
  {
    Commit();
  }
}
//---------------------------------------------------------------------------
void TSessionAction::Restart()
{
  if (FRecord != NULL)
  {
    FRecord->Restart();
  }
}
//---------------------------------------------------------------------------
void TSessionAction::Commit()
{
  if (FRecord != NULL)
  {
    TSessionActionRecord * Record = FRecord;
    FRecord = NULL;
    Record->Commit();
  }
}
//---------------------------------------------------------------------------
void TSessionAction::Rollback(Exception * E)
{
  if (FRecord != NULL)
  {
    TSessionActionRecord * Record = FRecord;
    FRecord = NULL;
    Record->Rollback(E);
  }
}
//---------------------------------------------------------------------------
void TSessionAction::Cancel()
{
  if (FRecord != NULL)
  {
    TSessionActionRecord * Record = FRecord;
    FRecord = NULL;
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
  if (FRecord != NULL)
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
  if (FRecord != NULL)
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
  if (FRecord != NULL)
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
  if (FRecord != NULL)
  {
    FRecord->Rights(Rights);
  }
}
//---------------------------------------------------------------------------
TTouchSessionAction::TTouchSessionAction(
    TActionLog * Log, const UnicodeString & FileName, const TDateTime & Modification) :
  TFileSessionAction(Log, laTouch, FileName)
{
  if (FRecord != NULL)
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
  if (FRecord != NULL)
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
  if (FRecord != NULL)
  {
    FRecord->Command(Command);
    FRecord->Destination(Destination);
  }
}
//---------------------------------------------------------------------------
void TCallSessionAction::AddOutput(const UnicodeString & Output, bool StdError)
{
  if (FRecord != NULL)
  {
    FRecord->AddOutput(Output, StdError);
  }
}
//---------------------------------------------------------------------------
TLsSessionAction::TLsSessionAction(TActionLog * Log,
    const UnicodeString & Destination) :
  TSessionAction(Log, laLs)
{
  if (FRecord != NULL)
  {
    FRecord->Destination(Destination);
  }
}
//---------------------------------------------------------------------------
void TLsSessionAction::FileList(TRemoteFileList * FileList)
{
  if (FRecord != NULL)
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
  if (FRecord != NULL)
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
  memset(&IsCapable, (int)false, sizeof(IsCapable));
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static FILE *
OpenFile(UnicodeString LogFileName, TSessionDataIntf * SessionData, bool Append, UnicodeString & NewFileName)
{
  CALLSTACK;
  FILE * Result;
  TRACEFMT("1 [%s]", LogFileName.c_str());
  UnicodeString ANewFileName = StripPathQuotes(GetExpandedLogFileName(LogFileName, SessionData));
  TRACEFMT("2 [%s]", ANewFileName.c_str());
  // Result = _wfopen(ANewFileName.c_str(), (Append ? L"a" : L"w"));
  Result = _fsopen(W2MB(ANewFileName.c_str()).c_str(),
    Append ? "a" : "w", SH_DENYWR); // _SH_DENYNO); // 
  if (Result != NULL)
  {
    TRACEFMT("3 [%d]", int(FileExists(ANewFileName)));
    setvbuf(Result, NULL, _IONBF, BUFSIZ);
    NewFileName = ANewFileName;
  }
  else
  {
    TRACE("4");
    throw Exception(FMTLOAD(LOG_OPENERROR, ANewFileName.c_str()));
  }
  TRACE("/");
  return Result;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
const wchar_t *LogLineMarks = L"<>!.*";
TSessionLog::TSessionLog(TSessionUI* UI, TSessionData * SessionData,
  TConfiguration * Configuration):
  TStringList()
{
  CALLSTACK;
  FCriticalSection = new TCriticalSection();
  FLogging = false;
  FConfiguration = Configuration;
  FParent = NULL;
  FUI = UI;
  FSessionData = SessionData;
  FFile = NULL;
  FLoggedLines = 0;
  FTopIndex = -1;
  FCurrentLogFileName = L"";
  FCurrentFileName = L"";
  FClosed = false;
  TRACE("/");
}
//---------------------------------------------------------------------------
TSessionLog::~TSessionLog()
{
  CALLSTACK;
  FClosed = true;
  ReflectSettings();
  assert(FFile == NULL);
  delete FCriticalSection;
  TRACE("/");
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
UnicodeString TSessionLog::GetSessionName()
{
  assert(FSessionData != NULL);
  return FSessionData->GetSessionName();
}
//---------------------------------------------------------------------------
UnicodeString TSessionLog::GetLine(Integer Index)
{
  return Strings[Index - FTopIndex];
}
//---------------------------------------------------------------------------
TLogLineType TSessionLog::GetType(int Index)
{
  return static_cast<TLogLineType>(reinterpret_cast<size_t>(Objects[Index - FTopIndex]));
}
//---------------------------------------------------------------------------
void TSessionLog::DoAddToParent(TLogLineType Type, const UnicodeString & Line)
{
  CCALLSTACK(TRACE_LOG_ADD);
  assert(FParent != NULL);
  FParent->Add(Type, Line);
}
//---------------------------------------------------------------------------
void TSessionLog::DoAddToSelf(TLogLineType Type, const UnicodeString & Line)
{
  CCALLSTACK(TRACE_LOG_ADD);
  if (static_cast<int>(FTopIndex) < 0)
  {
    FTopIndex = 0;
  }

  TStringList::AddObject(Line, static_cast<TObject *>(reinterpret_cast<void *>(static_cast<size_t>(Type))));

  FLoggedLines++;

  if (LogToFile())
  {
    if (FFile == NULL)
    {
      TRACE("1");
      OpenLogFile();
    }

    if (FFile != NULL)
    {
#ifndef _MSC_VER
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
      UTF8String UtfLine = UTF8String(UnicodeString(LogLineMarks[Type]) + Timestamp + Line + "\n");
      fprintf_s(static_cast<FILE *>(FFile), "%s", const_cast<char *>(AnsiString(UtfLine).c_str()));
#endif
    }
  }
}
//---------------------------------------------------------------------------
void TSessionLog::DoAdd(TLogLineType Type, UnicodeString Line,
  TDoAddLogEvent Event)
{
  CCALLSTACK(TRACE_LOG_ADD);
  UnicodeString Prefix;

  if (!GetName().IsEmpty())
  {
    Prefix = L"[" + GetName() + L"] ";
  }

  while (!Line.IsEmpty())
  {
    Event(Type, Prefix + CutToChar(Line, L'\n', false));
  }
}
//---------------------------------------------------------------------------
void TSessionLog::Add(TLogLineType Type, const UnicodeString & Line)
{
  assert(FConfiguration);
  CTRACEFMT(TRACE_LOG_ADD, "[%s]", Line.c_str());
  if (GetLogging())
  {
    try
    {
      if (FParent != NULL)
      {
        CTRACE(TRACE_LOG_ADD, "Parent");
        DoAdd(Type, Line, MAKE_CALLBACK(TSessionLog::DoAddToParent, this));
      }
      else
      {
        CTRACE(TRACE_LOG_ADD, "Pre Guard");
        TGuard Guard(FCriticalSection);

        CTRACE(TRACE_LOG_ADD, "Post Guard");
        BeginUpdate();

        TRY_FINALLY (
        {
          CTRACE(TRACE_LOG_ADD, "DoAdd");
          DoAdd(Type, Line, MAKE_CALLBACK(TSessionLog::DoAddToSelf, this));
        }
        ,
        {
          CTRACE(TRACE_LOG_ADD, "Finally");
          DeleteUnnecessary();

          EndUpdate();
        }
        );
      }
    }
    catch (Exception &E)
    {
      CTRACE(TRACE_LOG_ADD, "E1");
      // We failed logging, turn it off and notify user.
      FConfiguration->SetLogging(false);
      try
      {
        throw ExtException(&E, LOG_GEN_ERROR);
      }
      catch (Exception &E)
      {
        CTRACEFMT(TRACE_LOG_ADD, "E2 [%s]", E.Message.c_str());
        AddException(&E);
        FUI->HandleExtendedException(&E);
      }
    }
  }
  CTRACE(TRACE_LOG_ADD, "/");
}
//---------------------------------------------------------------------------
void TSessionLog::AddException(Exception * E)
{
  if (E != NULL)
  {
    Add(llException, ExceptionLogString(E));
  }
}
//---------------------------------------------------------------------------
void TSessionLog::ReflectSettings()
{
  CALLSTACK;
  TGuard Guard(FCriticalSection);

  bool ALogging =
    !FClosed &&
    ((FParent != NULL) || FConfiguration->GetLogging());

  if (FLogging != ALogging)
  {
    TRACE("3");
    FLogging = ALogging;
    StateChange();
  }

  // if logging to file was turned off or log file was changed -> close current log file
  if ((FFile != NULL) &&
      (!LogToFile() || (FCurrentLogFileName != FConfiguration->GetLogFileName())))
  {
    TRACE("4");
    CloseLogFile();
  }

  DeleteUnnecessary();
  TRACE("/");
}
//---------------------------------------------------------------------------
bool TSessionLog::LogToFile()
{
  return GetLogging() && FConfiguration->GetLogToFile() && (FParent == NULL);
}
//---------------------------------------------------------------------------
void TSessionLog::CloseLogFile()
{
  CALLSTACK;
  if (FFile != NULL)
  {
    TRACE("1");
    fclose(static_cast<FILE *>(FFile));
    FFile = NULL;
  }
  FCurrentLogFileName = L"";
  FCurrentFileName = L"";
  StateChange();
  TRACE("/");
}
//---------------------------------------------------------------------------
void TSessionLog::OpenLogFile()
{
  CALLSTACK;
  try
  {
    assert(FFile == NULL);
    assert(FConfiguration != NULL);
    FCurrentLogFileName = FConfiguration->GetLogFileName();
    TRACEFMT("1 [%s]", FCurrentLogFileName.c_str());
    FFile = OpenFile(FCurrentLogFileName, FSessionData, FConfiguration->GetLogFileAppend(), FCurrentFileName);
    TRACEFMT("2 [%s]", FCurrentFileName.c_str());
  }
  catch (Exception & E)
  {
    // TRACEE;
    // We failed logging to file, turn it off and notify user.
    FCurrentLogFileName = L"";
    FCurrentFileName = L"";
    FConfiguration->SetLogFileName(UnicodeString());
    try
    {
      throw ExtException(&E, LOG_GEN_ERROR);
    }
    catch (Exception & E)
    {
      TRACEFMT("E2 [%s]", E.Message.c_str());
      AddException(&E);
      FUI->HandleExtendedException(&E);
    }
  }
  StateChange();
  TRACE("/");
}
//---------------------------------------------------------------------------
void TSessionLog::StateChange()
{
  if (FOnStateChange != NULL)
  {
    FOnStateChange(this);
  }
}
//---------------------------------------------------------------------------
void TSessionLog::DeleteUnnecessary()
{
  CCALLSTACK(TRACE_LOG_ADD);
  BeginUpdate();
  TRY_FINALLY (
  {
    if (!GetLogging() || (FParent != NULL))
    {
      CTRACE(TRACE_LOG_ADD2, "1");
      Clear();
    }
    else
    {
      CTRACE(TRACE_LOG_ADD2, "2");
      while (!FConfiguration->GetLogWindowComplete() && (GetCount() > FConfiguration->GetLogWindowLines()))
      {
        Delete(0);
        ++FTopIndex;
      }
    }
  }
  ,
  {
    CTRACE(TRACE_LOG_ADD2, "3");
    EndUpdate();
  }
  );
}
//---------------------------------------------------------------------------
void TSessionLog::AddStartupInfo()
{
  CALLSTACK;
  if (GetLogging())
  {
    if (FParent != NULL)
    {
      // do not add session info for secondary session
      // (this should better be handled in the TSecondaryTerminal)
    }
    else
    {
      DoAddStartupInfo(FSessionData);
    }
  }
//!CLEANBEGIN
  #ifdef _DEBUG
  else
  {
    if (FParent == NULL)
    {
      DoAddStartupInfo(FSessionData);
    }
  }
  #endif
//!CLEANEND
}
//---------------------------------------------------------------------------
void TSessionLog::DoAddStartupInfo(TSessionData * Data)
{
  CALLSTACK;
  TGuard Guard(FCriticalSection);

  BeginUpdate();
  try
  {
    #define ADF(S, ...) DoAdd(llMessage, FORMAT(S, __VA_ARGS__), MAKE_CALLBACK(TSessionLog::DoAddToSelf, this));
//!CLEANBEGIN
    #ifdef _DEBUG
    #undef ADF
    // #define ADF(S, F) DoAdd(llMessage, FORMAT(S, F), DoAddToSelf);
    #define ADF(S, ...) DoAdd(llMessage, FORMAT(S, __VA_ARGS__), MAKE_CALLBACK(TSessionLog::DoAddToSelf, this));
    #endif
//!CLEANEND
    AddSeparator();
    ADF(L"NetBox %s (OS %s)", FConfiguration->GetVersionStr().c_str(), FConfiguration->GetOSVersionStr().c_str());
    THierarchicalStorage * Storage = FConfiguration->CreateScpStorage(false);
    assert(Storage);
    TRY_FINALLY (
    {
      ADF(L"Configuration: %s", Storage->GetSource().c_str());
    }
    ,
    {
      delete Storage;
    }
    );

    if (0)
    {
      typedef BOOL (WINAPI * TGetUserNameEx)(EXTENDED_NAME_FORMAT NameFormat, LPWSTR lpNameBuffer, PULONG nSize);
      HINSTANCE Secur32 = LoadLibrary(L"secur32.dll");
      TGetUserNameEx GetUserNameEx =
        (Secur32 != NULL) ? reinterpret_cast<TGetUserNameEx>(GetProcAddress(Secur32, "GetUserNameExW")) : NULL;
      wchar_t UserName[UNLEN + 1];
      unsigned long UserNameSize = LENOF(UserName);
      if ((GetUserNameEx == NULL) || !GetUserNameEx(NameSamCompatible, (LPWSTR)UserName, &UserNameSize))
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
    ADF(L"Login time: %s", dt.c_str());
    AddSeparator();
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
    int PingInterval;
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
        FORMAT(L" (%s)", ProxyMethodList[Data->GetActualProxyMethod()]).c_str() :
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
      Bugs = L"";
      for (intptr_t Index = 0; Index < SFTP_BUG_COUNT; ++Index)
      {
        Bugs += UnicodeString(BugFlags[Data->GetSFTPBug(static_cast<TSftpBug>(Index))])+(Index<SFTP_BUG_COUNT-1 ? L"," : L"");
      }
      ADF(L"SFTP Bugs: %s", Bugs.c_str());
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
    if (Data->GetFSProtocol() == fsFTP)
    {
      UnicodeString Ftps;
      switch (Data->GetFtps())
      {
        case ftpsImplicit:
          Ftps = L"Implicit SSL/TLS";
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
      ADF(L"FTP: FTPS: %s; Passive: %s [Force IP: %c]; MLSD: %c",
         Ftps.c_str(), BooleanToEngStr(Data->GetFtpPasvMode()).c_str(),
         BugFlags[Data->GetFtpForcePasvIp()],
         BugFlags[Data->GetFtpUseMlsd()]);
    }
    ADF(L"Local directory: %s, Remote directory: %s, Update: %s, Cache: %s",
      (Data->GetLocalDirectory().IsEmpty() ? UnicodeString(L"default").c_str() : Data->GetLocalDirectory().c_str()),
       (Data->GetRemoteDirectory().IsEmpty() ? UnicodeString(L"home").c_str() : Data->GetRemoteDirectory().c_str()),
       BooleanToEngStr(Data->GetUpdateDirectories()).c_str(),
       BooleanToEngStr(Data->GetCacheDirectories()).c_str());
    ADF(L"Cache directory changes: %s, Permanent: %s",
       BooleanToEngStr(Data->GetCacheDirectoryChanges()).c_str(),
       BooleanToEngStr(Data->GetPreserveDirectoryChanges()).c_str());
    ADF(L"DST mode: %d", static_cast<int>(Data->GetDSTMode()));

    if (Data->GetFSProtocol() == fsWebDAV)
    {
      ADF(L"Compression: %s",
        BooleanToEngStr(Data->GetCompression()).c_str());
    }

    AddSeparator();

    #undef ADF
  }
  catch (...)
  {
    DeleteUnnecessary();
    EndUpdate();
    throw;
  }
  DeleteUnnecessary();
  EndUpdate();
}
//---------------------------------------------------------------------------
void TSessionLog::AddSeparator()
{
  Add(llMessage, L"--------------------------------------------------------------------------");
}
//---------------------------------------------------------------------------
intptr_t TSessionLog::GetBottomIndex()
{
  return (GetCount() > 0 ? (GetTopIndex() + GetCount() - 1) : -1);
}
//---------------------------------------------------------------------------
bool TSessionLog::GetLoggingToFile()
{
  assert((FFile == NULL) || LogToFile());
  return (FFile != NULL);
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
void TSessionLog::SetParent(TSessionLog *value)
{
  FParent = value;
}
//---------------------------------------------------------------------------
bool TSessionLog::GetLogging()
{
  return FLogging;
}
//---------------------------------------------------------------------------
TNotifyEvent & TSessionLog::GetOnChange()
{
  return TStringList::GetOnChange();
}
//---------------------------------------------------------------------------
void TSessionLog::SetOnChange(TNotifyEvent value)
{
  TStringList::SetOnChange(value);
}
//---------------------------------------------------------------------------
TNotifyEvent & TSessionLog::GetOnStateChange()
{
  return FOnStateChange;
}
//---------------------------------------------------------------------------
void TSessionLog::SetOnStateChange(TNotifyEvent value)
{
  FOnStateChange = value;
}
//---------------------------------------------------------------------------
UnicodeString TSessionLog::GetCurrentFileName()
{
  return FCurrentFileName;
}
//---------------------------------------------------------------------------
intptr_t TSessionLog::GetTopIndex()
{
  return FTopIndex;
}
//---------------------------------------------------------------------------
UnicodeString TSessionLog::GetName()
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
TActionLog::TActionLog(TSessionUI* UI, TSessionData * SessionData,
  TConfiguration * Configuration)
{
  CALLSTACK;
  FCriticalSection = new TCriticalSection;
  FConfiguration = Configuration;
  FUI = UI;
  FSessionData = SessionData;
  FFile = NULL;
  FCurrentLogFileName = L"";
  FCurrentFileName = L"";
  FLogging = false;
  FClosed = false;
  FPendingActions = new TList();
  FIndent = L"  ";
  FInGroup = false;
  FEnabled = true;
  TRACE("/");
}
//---------------------------------------------------------------------------
TActionLog::~TActionLog()
{
  CALLSTACK;
  assert(FPendingActions->GetCount() == 0);
  delete FPendingActions;
  FClosed = true;
  ReflectSettings();
  assert(FFile == NULL);
  delete FCriticalSection;
  TRACE("/");
}
//---------------------------------------------------------------------------
void TActionLog::Add(const UnicodeString & Line)
{
  assert(FConfiguration);
  CTRACEFMT(TRACE_LOG_ADD, "[%s]", Line.c_str());
  if (FLogging)
  {
    try
    {
      TGuard Guard(FCriticalSection);
      if (FFile == NULL)
      {
        TRACE("1");
        OpenLogFile();
      }

      if (FFile != NULL)
      {
        UTF8String UtfLine = UTF8String(Line);
//!CLEANBEGIN
        #ifdef _DEBUG
        size_t Written =
        #endif
//!CLEANEND
        fwrite(UtfLine.c_str(), 1, UtfLine.Length(), (FILE *)FFile);
//!CLEANBEGIN
        #ifdef _DEBUG
        Written +=
        #endif
//!CLEANEND
        fwrite("\n", 1, 1, (FILE *)FFile);
//!CLEANBEGIN
        #ifdef _DEBUG
        TRACEFMT("2 [%d]", int(Written));
        #endif
//!CLEANEND
      }
    }
    catch (Exception &E)
    {
      CTRACE(TRACE_LOG_ADD, "E");
      // We failed logging, turn it off and notify user.
      FConfiguration->SetLogActions(false);
      try
      {
        throw ExtException(&E, LOG_GEN_ERROR);
      }
      catch (Exception &E)
      {
        CTRACEFMT(TRACE_LOG_ADD, "[%s]", E.Message.c_str());
        FUI->HandleExtendedException(&E);
      }
    }
  }
  CTRACE(TRACE_LOG_ADD, "/");
}
//---------------------------------------------------------------------------
void TActionLog::AddIndented(const UnicodeString & Line)
{
  Add(FIndent + Line);
}
//---------------------------------------------------------------------------
void TActionLog::AddFailure(TStrings * Messages)
{
  CALLSTACK;
  AddIndented(L"<failure>");
  AddMessages(L"  ", Messages);
  AddIndented(L"</failure>");
}
//---------------------------------------------------------------------------
void TActionLog::AddFailure(Exception * E)
{
  CALLSTACK;
  TStrings * Messages = ExceptionToMessages(E);
  if (Messages != NULL)
  {
    TRY_FINALLY (
    {
      AddFailure(Messages);
    }
    ,
    {
      delete Messages;
    }
    );
  }
}
//---------------------------------------------------------------------------
void TActionLog::AddMessages(UnicodeString Indent, TStrings * Messages)
{
  CALLSTACK;
  for (intptr_t Index = 0; Index < Messages->GetCount(); ++Index)
  {
    AddIndented(
      FORMAT((Indent + L"<message>%s</message>").c_str(), XmlEscape(Messages->Strings[Index]).c_str()));
  }
}
//---------------------------------------------------------------------------
void TActionLog::ReflectSettings()
{
  CALLSTACK;
  TGuard Guard(FCriticalSection);

  bool ALogging =
    !FClosed && FConfiguration->GetLogActions() && GetEnabled();

  if (ALogging && !FLogging)
  {
    TRACE("1");
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
    TRACE("2");
    Add(L"</session>");
    CloseLogFile();
    FLogging = false;
  }

  TRACE("/");
}
//---------------------------------------------------------------------------
void TActionLog::CloseLogFile()
{
  CALLSTACK;
  if (FFile != NULL)
  {
    TRACE("1");
    fclose((FILE *)FFile);
    FFile = NULL;
  }
  FCurrentLogFileName = L"";
  FCurrentFileName = L"";
  TRACE("/");
}
//---------------------------------------------------------------------------
void TActionLog::OpenLogFile()
{
  CALLSTACK;
  try
  {
    assert(FFile == NULL);
    assert(FConfiguration != NULL);
    FCurrentLogFileName = FConfiguration->GetActionsLogFileName();
    TRACEFMT("1 [%s]", FCurrentLogFileName.c_str());
    FFile = OpenFile(FCurrentLogFileName, FSessionData, false, FCurrentFileName);
    TRACEFMT("2 [%s]", FCurrentFileName.c_str());
  }
  catch (Exception & E)
  {
    // TRACEE;
    // We failed logging to file, turn it off and notify user.
    FCurrentLogFileName = L"";
    FCurrentFileName = L"";
    FConfiguration->SetLogActions(false);
    try
    {
      throw ExtException(&E, LOG_GEN_ERROR);
    }
    catch (Exception & E)
    {
      TRACEFMT("E2 [%s]", E.Message.c_str());
      FUI->HandleExtendedException(&E);
    }
  }
  TRACE("/");
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
         static_cast<TSessionActionRecord *>(FPendingActions->Items[0])->Record())
  {
    FPendingActions->Delete(0);
  }
}
//---------------------------------------------------------------------------
void TActionLog::BeginGroup(UnicodeString Name)
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
  AddIndented(L"</group>");
}
//---------------------------------------------------------------------------
void TActionLog::SetEnabled(bool value)
{
  if (GetEnabled() != value)
  {
    FEnabled = value;
    ReflectSettings();
  }
}

