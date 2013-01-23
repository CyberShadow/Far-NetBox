//------------------------------------------------------------------------------
#define NO_WIN32_LEAN_AND_MEAN
#include <vcl.h>
#pragma hdrstop

#define TRACE_TIMESTAMP TRACING

#include <assert.h>
#include <math.h>
#include <shlobj.h>

#include "Common.h"
#include "Exceptions.h"
#include "TextsCore.h"
#include "Interface.h"
#include <StrUtils.hpp>
#include <DateUtils.hpp>
//------------------------------------------------------------------------------
#pragma package(smart_init)
//------------------------------------------------------------------------------
//!CLEANBEGIN
#ifdef _DEBUG
#include <stdio.h>
static HANDLE TraceFile = NULL;
#ifdef NETBOX_DEBUG
bool IsTracing = true;
#else
bool IsTracing = false;
#endif
unsigned int CallstackTls = CallstackTlsOff;
TCriticalSection * TracingCriticalSection = NULL;
//------------------------------------------------------------------------------
void __callstack(const wchar_t*, const wchar_t*, unsigned int, const wchar_t*)
{
}
//------------------------------------------------------------------------------
void SetTraceFile(HANDLE ATraceFile)
{
  TraceFile = ATraceFile;
  IsTracing = (TraceFile != 0);
  if (TracingCriticalSection == NULL)
  {
    TracingCriticalSection = new TCriticalSection();
  }
}
//------------------------------------------------------------------------------
void CleanupTracing()
{
  if (TracingCriticalSection != NULL)
  {
    delete TracingCriticalSection;
    TracingCriticalSection = NULL;
  }
}
//------------------------------------------------------------------------------
#ifdef TRACE_IN_MEMORY
struct TTraceInMemory
{
  DWORD Ticks;
  DWORD Thread;
  const wchar_t * SourceFile;
  const wchar_t * Func;
  int Line;
  const wchar_t * Message;
};
typedef std::vector<TTraceInMemory> TTracesInMemory;
TTracesInMemory TracesInMemory;
//------------------------------------------------------------------------------
int TraceThreadProc(void *)
{
  TRACE(">");
  try
  {
    do
    {
      TRACE("2");
      TraceDumpToFile();
      TRACE("3");
      Sleep(60000);
      TRACE("4");
      // if resuming from sleep causes the previous Sleep to immediatelly break,
      // make sure we wait a little more before dumping
      Sleep(60000);
      TRACE("5");
    }
    while (true);
  }
  catch(...)
  {
    TRACE("E");
  }
  TRACE("/");
  return 0;
}
//------------------------------------------------------------------------------
void Trace(const wchar_t * SourceFile, const wchar_t * Func,
  int Line, const wchar_t * Message)
{
  if (TracingCriticalSection != NULL)
  {
    TTraceInMemory TraceInMemory;
    TraceInMemory.Ticks = GetTickCount();
    TraceInMemory.Thread = GetCurrentThreadId();
    TraceInMemory.SourceFile = SourceFile;
    TraceInMemory.Func = Func;
    TraceInMemory.Line = Line;
    TraceInMemory.Message = Message;

    TGuard Guard(TracingCriticalSection);

    if (TracesInMemory.capacity() == 0)
    {
      TracesInMemory.reserve(100000);
      TThreadID ThreadID;
      StartThread(NULL, 0, TraceThreadProc, NULL, 0, ThreadID);
    }

    TracesInMemory.push_back(TraceInMemory);
  }
}
//------------------------------------------------------------------------------
#ifndef _MSC_VER
void TraceFmt(const wchar_t * SourceFile, const wchar_t * Func,
  int Line, const wchar_t * AFormat, TVarRec * /*Args*/, const int /*Args_Size*/)
#else
void TraceFmt(const wchar_t * SourceFile, const wchar_t * Func,
  int Line, const wchar_t * AFormat, ...)
#endif
{
  Trace(SourceFile, Func, Line, AFormat);
}
//------------------------------------------------------------------------------
void TraceDumpToFile()
{
  if (TraceFile != NULL)
  {
    TGuard Guard(TracingCriticalSection);

    DWORD Written;

    TDateTime N = Now();
    DWORD Ticks = GetTickCount();

    const UnicodeString TimestampFormat = L"hh:mm:ss.zzz";
    UnicodeString TimeString = FormatDateTime(TimestampFormat, N);

    UTF8String Buffer = UTF8String(
      FORMAT("[%s] Dumping in-memory tracing =================================\n",
        TimeString.c_str()));
    WriteFile(TraceFile, Buffer.c_str(), Buffer.Length(), &Written, NULL);

    TTracesInMemory::const_iterator i = TracesInMemory.begin();
    while (i != TracesInMemory.end())
    {
      const wchar_t * SourceFile = i->SourceFile;
      const wchar_t * Slash = wcsrchr(SourceFile, L'\\');
      if (Slash != NULL)
      {
        SourceFile = Slash + 1;
      }

      TimeString =
        FormatDateTime(TimestampFormat,
          IncMilliSecond(N, -static_cast<int>(Ticks - i->Ticks)));
      Buffer = UTF8String(FORMAT(L"[%s] [%.4X] [%s:%d:%s] %s\n",
        TimeString.c_str(), int(i->Thread), SourceFile,
         i->Line, i->Func, i->Message));
      WriteFile(TraceFile, Buffer.c_str(), Buffer.Length(), &Written, NULL);
      i++;
    }
    TracesInMemory.clear();

    TimeString = FormatDateTime(TimestampFormat, Now());
    Buffer = UTF8String(
      FORMAT("[%s] Done in-memory tracing =================================\n",
        TimeString.c_str()));
    WriteFile(TraceFile, Buffer.c_str(), Buffer.Length(), &Written, NULL);
  }
}
//------------------------------------------------------------------------------
void TraceInMemoryCallback(System::UnicodeString Msg)
{
  Trace(L"PAS", L"unk", 0, Msg.c_str());
}
#else
void Trace(const wchar_t * SourceFile, const wchar_t * Func,
  int Line, const wchar_t * Message)
{
  assert(IsTracing);

  UnicodeString TimeString;
#ifndef _MSC_VER
  DateTimeToString(TimeString, L"hh:mm:ss.zzz", Now());
#else
  unsigned short H, N, S, MS;
  TDateTime DateTime = Now();
  DateTime.DecodeTime(H, N, S, MS);
  TimeString = FORMAT(L"%02d.%02d.%02d.%03d", H, N, S, MS);
#endif
  const wchar_t * Slash = wcsrchr(SourceFile, L'\\');
  if (Slash != NULL)
  {
    SourceFile = Slash + 1;
  }
  UTF8String Buffer = UTF8String(FORMAT(L"NetBox: [%s] [%.4X] [%s:%d:%s] %s\n",
    TimeString.c_str(), int(GetCurrentThreadId()), SourceFile,
     Line, Func, Message));
  // DWORD Written;
  // WriteFile(TraceFile, Buffer.c_str(), Buffer.Length(), &Written, NULL);
  // DEBUG_PRINTF(L"%s", Buffer.c_str());
  // OutputDebugStringW(Buffer.c_str());
}
//------------------------------------------------------------------------------
#ifndef _MSC_VER
void TraceFmt(const wchar_t * SourceFile, const wchar_t * Func,
  int Line, const wchar_t * AFormat, TVarRec * Args, const int Args_Size)
#else
void TraceFmt(const wchar_t * SourceFile, const wchar_t * Func,
  int Line, const wchar_t * AFormat, ...)
#endif
{
  assert(IsTracing);
#ifndef _MSC_VER
  UnicodeString Message = Format(AFormat, Args, Args_Size);
#else
  va_list args;
  va_start(args, AFormat);
  UnicodeString Message = Format(AFormat, args);
  va_end(args);
#endif
  Trace(SourceFile, Func, Line, Message.c_str());
}
#endif
//------------------------------------------------------------------------------
void DoAssert(wchar_t * Message, wchar_t * Filename, int LineNumber)
{
  if (IsTracing)
  {
    Trace(Filename, L"assert", LineNumber, Message);
  }
#ifndef _MSC_VER
  _assert(AnsiString(Message).c_str(), AnsiString(Filename).c_str(), LineNumber);
#else
  _wassert(Message, Filename, LineNumber);
#endif
}
#endif // ifdef _DEBUG
//!CLEANEND
//------------------------------------------------------------------------------
// TGuard
//------------------------------------------------------------------------------
TGuard::TGuard(TCriticalSection * ACriticalSection) :
  FCriticalSection(ACriticalSection)
{
  assert(ACriticalSection != NULL);
  FCriticalSection->Enter();
}
//------------------------------------------------------------------------------
TGuard::~TGuard()
{
  FCriticalSection->Leave();
}
//------------------------------------------------------------------------------
// TUnguard
//------------------------------------------------------------------------------
TUnguard::TUnguard(TCriticalSection * ACriticalSection) :
  FCriticalSection(ACriticalSection)
{
  assert(ACriticalSection != NULL);
  FCriticalSection->Leave();
}
//------------------------------------------------------------------------------
TUnguard::~TUnguard()
{
  FCriticalSection->Enter();
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
const wchar_t EngShortMonthNames[12][4] =
  {L"Jan", L"Feb", L"Mar", L"Apr", L"May", L"Jun",
   L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec"};
const std::string Bom = "\xEF\xBB\xBF";
const wchar_t TokenPrefix = L'%';
const wchar_t NoReplacement = wchar_t(false);
const wchar_t TokenReplacement = wchar_t(true);
const UnicodeString LocalInvalidChars = L"/\\:*?\"<>|";
//------------------------------------------------------------------------------
UnicodeString ReplaceChar(const UnicodeString & Str, wchar_t A, wchar_t B)
{
  UnicodeString Result = Str;
  for (intptr_t Index = 0; Index < Result.Length(); Index++)
    if (Result[Index+1] == A)
    {
      Result[Index+1] = B;
    }
  return Result;
}
//------------------------------------------------------------------------------
UnicodeString DeleteChar(const UnicodeString & Str, wchar_t C)
{
  UnicodeString Result = Str;
  intptr_t P;
  while ((P = Result.Pos(C)) > 0)
  {
    Result.Delete(P, 1);
  }
  return Result;
}
//------------------------------------------------------------------------------
void PackStr(UnicodeString & Str)
{
  // Following will free unnecessary bytes
  Str = Str.c_str();
}
//------------------------------------------------------------------------------
void PackStr(RawByteString &Str)
{
  // Following will free unnecessary bytes
  Str = Str.c_str();
}
//------------------------------------------------------------------------------
void Shred(UnicodeString & Str)
{
  if (!Str.IsEmpty())
  {
    Str.Unique();
    memset(const_cast<wchar_t *>(Str.c_str()), 0, Str.Length() * sizeof(*Str.c_str()));
    Str = L"";
  }
}
//------------------------------------------------------------------------------
UnicodeString MakeValidFileName(const UnicodeString & FileName)
{
  UnicodeString Result = FileName;
  static UnicodeString IllegalChars = L":;,=+<>|\"[] \\/?*";
  for (intptr_t Index = 0; Index < IllegalChars.Length(); Index++)
  {
    Result = ReplaceChar(Result, IllegalChars[Index+1], L'-');
  }
  return Result;
}
//------------------------------------------------------------------------------
UnicodeString RootKeyToStr(HKEY RootKey)
{
  if (RootKey == HKEY_USERS) return L"HKEY_USERS";
    else
  if (RootKey == HKEY_LOCAL_MACHINE) return L"HKEY_LOCAL_MACHINE";
    else
  if (RootKey == HKEY_CURRENT_USER) return L"HKEY_CURRENT_USER";
    else
  if (RootKey == HKEY_CLASSES_ROOT) return L"HKEY_CLASSES_ROOT";
    else
  if (RootKey == HKEY_CURRENT_CONFIG) return L"HKEY_CURRENT_CONFIG";
    else
  if (RootKey == HKEY_DYN_DATA) return L"HKEY_DYN_DATA";
    else
  {  Abort(); return L""; };
}
//------------------------------------------------------------------------------
UnicodeString BooleanToEngStr(bool B)
{
  if (B)
  {
    return L"Yes";
  }
  else
  {
    return L"No";
  }
}
//------------------------------------------------------------------------------
UnicodeString BooleanToStr(bool B)
{
  if (B)
  {
    return LoadStr(YES_STR);
  }
  else
  {
    return LoadStr(NO_STR);
  }
}
//------------------------------------------------------------------------------
UnicodeString DefaultStr(const UnicodeString & Str, const UnicodeString & Default)
{
  if (!Str.IsEmpty())
  {
    return Str;
  }
  else
  {
    return Default;
  }
}
//------------------------------------------------------------------------------
UnicodeString CutToChar(UnicodeString & Str, wchar_t Ch, bool Trim)
{
  intptr_t P = Str.Pos(Ch);
  UnicodeString Result;
  if (P)
  {
    Result = Str.SubString(1, P-1);
    Str.Delete(1, P);
  }
  else
  {
    Result = Str;
    Str = L"";
  }
  if (Trim)
  {
    Result = Result.TrimRight();
    Str = Str.TrimLeft();
  }
  return Result;
}
//------------------------------------------------------------------------------
UnicodeString CopyToChars(const UnicodeString & Str, intptr_t & From, UnicodeString Chs, bool Trim,
  wchar_t * Delimiter, bool DoubleDelimiterEscapes)
{
  UnicodeString Result;

  intptr_t P;
  for (P = From; P <= Str.Length(); P++)
  {
    if (IsDelimiter(Chs, Str, P))
    {
      if (DoubleDelimiterEscapes &&
          (P < Str.Length()) &&
          IsDelimiter(Chs, Str, P + 1))
      {
        Result += Str[P];
        P++;
      }
      else
      {
        break;
      }
    }
    else
    {
      Result += Str[P];
    }
  }

  if (P <= Str.Length())
  {
    if (Delimiter != NULL)
    {
      *Delimiter = Str[P];
    }
  }
  else
  {
    if (Delimiter != NULL)
    {
      *Delimiter = L'\0';
    }
  }
  // even if we reached the end, return index, as if there were the delimiter,
  // so caller can easily find index of the end of the piece by subtracting
  // 2 from From (as long as he did not asked for trimming)
  From = P+1;
  if (Trim)
  {
    Result = Result.TrimRight();
    while ((From <= Str.Length()) && (Str[From] == L' '))
    {
      From++;
    }
  }
  return Result;
}
//------------------------------------------------------------------------------
UnicodeString DelimitStr(const UnicodeString & Str, const UnicodeString & Chars)
{
  UnicodeString Result = Str;
  for (intptr_t I = 1; I <= Result.Length(); I++)
  {
    if (Result.IsDelimiter(Chars, I))
    {
      Result.Insert(L"\\", I);
      I++;
    }
  }
  return Result;
}
//------------------------------------------------------------------------------
UnicodeString ShellDelimitStr(const UnicodeString & Str, wchar_t Quote)
{
  static UnicodeString Chars = L"$\\";
  if (Quote == L'"')
  {
    Chars += L"`\"";
  }
  return DelimitStr(Str, Chars);
}
//------------------------------------------------------------------------------
UnicodeString ExceptionLogString(Exception *E)
{
  assert(E);
  if (dynamic_cast<Exception *>(E) != NULL)
  {
    UnicodeString Msg;
#ifndef _MSC_VER
    Msg = FORMAT(L"(%s) %s", (E->ClassName(), E->Message.c_str()));
#else
    Msg = FORMAT(L"%s", ::MB2W(E->what()).c_str());
#endif
    if (dynamic_cast<ExtException *>(E) != NULL)
    {
      TStrings * MoreMessages = dynamic_cast<ExtException *>(E)->GetMoreMessages();
      if (MoreMessages)
      {
        Msg += L"\n" +
          StringReplace(MoreMessages->Text, L"\r", L"", TReplaceFlags() << rfReplaceAll);
      }
    }
    return Msg;
  }
  else
  {
#ifndef _MSC_VER
    wchar_t Buffer[1024];
    ExceptionErrorMessage(ExceptObject(), ExceptAddr(), Buffer, LENOF(Buffer));
    return UnicodeString(Buffer);
#else
    return UnicodeString(E->what());
#endif
  }
}
//------------------------------------------------------------------------------
bool IsNumber(const UnicodeString & Str)
{
  int Value;
  return TryStrToInt(Str, Value);
}
//------------------------------------------------------------------------------
UnicodeString SystemTemporaryDirectory()
{
  UnicodeString TempDir;
  TempDir.SetLength(MAX_PATH);
  TempDir.SetLength(GetTempPath(MAX_PATH, const_cast<LPWSTR>(TempDir.c_str())));
  return TempDir;
}
//------------------------------------------------------------------------------
UnicodeString GetShellFolderPath(int CSIdl)
{
  UnicodeString Result;
  wchar_t Path[2 * MAX_PATH + 10] = L"\0";
  if (SUCCEEDED(SHGetFolderPath(NULL, CSIdl, NULL, SHGFP_TYPE_CURRENT, Path)))
  {
    Result = Path;
  }
  return Result;
}
//------------------------------------------------------------------------------
UnicodeString StripPathQuotes(const UnicodeString & Path)
{
  if ((Path.Length() >= 2) &&
      (Path[1] == L'\"') && (Path[Path.Length()] == L'\"'))
  {
    return Path.SubString(2, Path.Length() - 2);
  }
  else
  {
    return Path;
  }
}
//------------------------------------------------------------------------------
UnicodeString AddPathQuotes(const UnicodeString & Path)
{
  UnicodeString Result = StripPathQuotes(Path);
  if (Result.Pos(L" ") > 0)
  {
    Result = L"\"" + Result + L"\"";
  }
  return Result;
}
//------------------------------------------------------------------------------
static wchar_t * ReplaceChar(
  UnicodeString & FileName, wchar_t * InvalidChar, wchar_t InvalidCharsReplacement)
{
  CALLSTACK;
  intptr_t Index = InvalidChar - FileName.c_str() + 1;
  if (InvalidCharsReplacement == TokenReplacement)
  {
    TRACE("1");
    // currently we do not support unicode chars replacement
    if (FileName[Index] > 0xFF)
    {
      EXCEPTION;
    }

    FileName.Insert(ByteToHex(static_cast<unsigned char>(FileName[Index])), Index + 1);
    FileName[Index] = TokenPrefix;
    InvalidChar = const_cast<wchar_t *>(FileName.c_str() + Index + 2);
  }
  else
  {
    TRACE("2");
    FileName[Index] = InvalidCharsReplacement;
    InvalidChar = const_cast<wchar_t *>(FileName.c_str() + Index);
  }
  return InvalidChar;
}
//------------------------------------------------------------------------------
UnicodeString ValidLocalFileName(const UnicodeString & FileName)
{
  return ValidLocalFileName(FileName, L'_', L"", LocalInvalidChars);
}
//------------------------------------------------------------------------------
UnicodeString ValidLocalFileName(
  const UnicodeString & FileName, wchar_t InvalidCharsReplacement,
  const UnicodeString & TokenizibleChars, const UnicodeString & LocalInvalidChars)
{
  CALLSTACK;
  UnicodeString FileName2 = FileName;
  if (InvalidCharsReplacement != NoReplacement)
  {
    bool ATokenReplacement = (InvalidCharsReplacement == TokenReplacement);
    UnicodeString CharsStr = ATokenReplacement ? TokenizibleChars : LocalInvalidChars;
    const wchar_t * Chars = CharsStr.c_str();
    wchar_t * InvalidChar = const_cast<wchar_t *>(FileName2.c_str());
    TRACEFMT("1 [%d] [%s] [%s]", int(ATokenReplacement), Chars, InvalidChar);
    while ((InvalidChar = wcspbrk(InvalidChar, Chars)) != NULL)
    {
      TRACEFMT("2 [%s]", InvalidChar);
      intptr_t Pos = (InvalidChar - FileName2.c_str() + 1);
      wchar_t Char;
      if (ATokenReplacement &&
          (*InvalidChar == TokenPrefix) &&
          (((FileName2.Length() - Pos) <= 1) ||
           (((Char = static_cast<wchar_t>(HexToByte(FileName2.SubString(Pos + 1, 2)))) == L'\0') ||
            (TokenizibleChars.Pos(Char) == 0))))
      {
        InvalidChar++;
      }
      else
      {
        InvalidChar = ReplaceChar(FileName2, InvalidChar, InvalidCharsReplacement);
      }
      TRACEFMT("3 [%s]", InvalidChar);
    }

    // Windows trim trailing space or dot, hence we must encode it to preserve it
    if (!FileName2.IsEmpty() &&
        ((FileName2[FileName2.Length()] == L' ') ||
         (FileName2[FileName2.Length()] == L'.')))
    {
      TRACE("4");
      ReplaceChar(FileName2, const_cast<wchar_t *>(FileName2.c_str() + FileName2.Length() - 1), InvalidCharsReplacement);
    }

    if (IsReservedName(FileName2))
    {
      intptr_t P = FileName2.Pos(".");
      if (P == 0)
      {
        P = FileName2.Length() + 1;
      }
      FileName2.Insert(L"%00", P);
    }
  }
  return FileName2;
}
//------------------------------------------------------------------------------
void SplitCommand(const UnicodeString & Command, UnicodeString & Program,
  UnicodeString & Params, UnicodeString & Dir)
{
  UnicodeString Cmd = Command.Trim();
  Params = L"";
  Dir = L"";
  if (!Cmd.IsEmpty() && (Cmd[1] == L'\"'))
  {
    Cmd.Delete(1, 1);
    intptr_t P = Cmd.Pos(L'"');
    if (P)
    {
      Program = Cmd.SubString(1, P-1).Trim();
      Params = Cmd.SubString(P + 1, Cmd.Length() - P).Trim();
    }
    else
    {
      throw Exception(FMTLOAD(INVALID_SHELL_COMMAND, UnicodeString(L"\"" + Cmd).c_str()));
    }
  }
  else
  {
    intptr_t P = Cmd.Pos(L" ");
    if (P)
    {
      Program = Cmd.SubString(1, P).Trim();
      Params = Cmd.SubString(P + 1, Cmd.Length() - P).Trim();
    }
    else
    {
      Program = Cmd;
    }
  }
  intptr_t B = Program.LastDelimiter(L"\\/");
  if (B)
  {
    Dir = Program.SubString(1, B).Trim();
  }
}
//------------------------------------------------------------------------------
UnicodeString ExtractProgram(const UnicodeString & Command)
{
  UnicodeString Program;
  UnicodeString Params;
  UnicodeString Dir;

  SplitCommand(Command, Program, Params, Dir);

  return Program;
}
//------------------------------------------------------------------------------
UnicodeString FormatCommand(const UnicodeString & Program, const UnicodeString & Params)
{
  UnicodeString Result = Program.Trim();
  UnicodeString Params2 = Params.Trim();
  if (!Params2.IsEmpty()) Params2 = L" " + Params2;
  if (Result.Pos(L" ")) Result = L"\"" + Result + L"\"";
  return Result + Params2;
}
//------------------------------------------------------------------------------
const wchar_t ShellCommandFileNamePattern[] = L"!.!";
//------------------------------------------------------------------------------
void ReformatFileNameCommand(UnicodeString & Command)
{
  if (!Command.IsEmpty())
  {
    UnicodeString Program, Params, Dir;
    SplitCommand(Command, Program, Params, Dir);
    if (Params.Pos(ShellCommandFileNamePattern) == 0)
    {
      Params = Params + (Params.IsEmpty() ? L"" : L" ") + ShellCommandFileNamePattern;
    }
    Command = FormatCommand(Program, Params);
  }
}
//------------------------------------------------------------------------------
UnicodeString ExpandFileNameCommand(const UnicodeString & Command,
  const UnicodeString & FileName)
{
  return AnsiReplaceStr(Command, ShellCommandFileNamePattern,
    AddPathQuotes(FileName));
}
//------------------------------------------------------------------------------
UnicodeString EscapePuttyCommandParam(const UnicodeString & Param)
{
  UnicodeString Result = Param;
  bool Space = false;

  for (intptr_t I = 1; I <= Result.Length(); I++)
  {
    switch (Result[I])
    {
      case L'"':
        Result.Insert(L"\\", I);
        I++;
        break;

      case L' ':
        Space = true;
        break;

      case L'\\':
        intptr_t I2 = I;
        while ((I2 <= Result.Length()) && (Result[I2] == L'\\'))
        {
          I2++;
        }
        if ((I2 <= Result.Length()) && (Result[I2] == L'"'))
        {
          while (Result[I] == L'\\')
          {
            Result.Insert(L"\\", I);
            I += 2;
          }
          I--;
        }
        break;
    }
  }

  if (Space)
  {
    Result = L"\"" + Result + L'"';
  }

  return Result;
}
//------------------------------------------------------------------------------
UnicodeString ExpandEnvironmentVariables(const UnicodeString & Str)
{
  UnicodeString Buf;
  intptr_t Size = 1024;

  Buf.SetLength(Size);
  Buf.Unique();
  intptr_t Len = ExpandEnvironmentStrings(Str.c_str(), const_cast<LPWSTR>(Buf.c_str()), (DWORD)Size);

  if (Len > Size)
  {
    Buf.SetLength(Len);
    Buf.Unique();
    ExpandEnvironmentStrings(Str.c_str(), const_cast<LPWSTR>(Buf.c_str()), (DWORD)Len);
  }

  PackStr(Buf);

  return Buf;
}
//------------------------------------------------------------------------------
bool CompareFileName(const UnicodeString & Path1, const UnicodeString & Path2)
{
  UnicodeString ShortPath1 = ExtractShortPathName(Path1);
  UnicodeString ShortPath2 = ExtractShortPathName(Path2);

  bool Result;
  // ExtractShortPathName returns empty string if file does not exist
  if (ShortPath1.IsEmpty() || ShortPath2.IsEmpty())
  {
    Result = AnsiSameText(Path1, Path2);
  }
  else
  {
    Result = AnsiSameText(ShortPath1, ShortPath2);
  }
  return Result;
}
//------------------------------------------------------------------------------
bool ComparePaths(const UnicodeString & Path1, const UnicodeString & Path2)
{
  // TODO: ExpandUNCFileName
  return AnsiSameText(IncludeTrailingBackslash(Path1), IncludeTrailingBackslash(Path2));
}
//------------------------------------------------------------------------------
bool IsReservedName(const UnicodeString & FileName)
{
  UnicodeString fileName = FileName;
  intptr_t P = fileName.Pos(L".");
  intptr_t Len = (P > 0) ? P - 1 : fileName.Length();
  if ((Len == 3) || (Len == 4))
  {
    if (P > 0)
    {
      fileName.SetLength(P - 1);
    }
    static UnicodeString Reserved[] = {
      L"CON", L"PRN", L"AUX", L"NUL",
      L"COM1", L"COM2", L"COM3", L"COM4", L"COM5", L"COM6", L"COM7", L"COM8", L"COM9",
      L"LPT1", L"LPT2", L"LPT3", L"LPT4", L"LPT5", L"LPT6", L"LPT7", L"LPT8", L"LPT9" };
    for (unsigned int Index = 0; Index < LENOF(Reserved); Index++)
    {
      if (SameText(fileName, Reserved[Index]))
      {
        return true;
      }
    }
  }
  return false;
}
//------------------------------------------------------------------------------
UnicodeString DisplayableStr(const RawByteString & Str)
{
  bool Displayable = true;
  int Index = 1;
  while ((Index <= Str.Length()) && Displayable)
  {
    if (((Str[Index] < '\x20') || (static_cast<unsigned char>(Str[Index]) >= static_cast<unsigned char>('\x80'))) &&
        (Str[Index] != '\n') && (Str[Index] != '\r') && (Str[Index] != '\t') && (Str[Index] != '\b'))
    {
      Displayable = false;
    }
    Index++;
  }

  UnicodeString Result;
  if (Displayable)
  {
    Result = L"\"";
    for (int Index = 1; Index <= Str.Length(); Index++)
    {
      switch (Str[Index])
      {
        case '\n':
          Result += L"\\n";
          break;

        case '\r':
          Result += L"\\r";
          break;

        case '\t':
          Result += L"\\t";
          break;

        case '\b':
          Result += L"\\b";
          break;

        case '\\':
          Result += L"\\\\";
          break;

        case '"':
          Result += L"\\\"";
          break;

        default:
          Result += wchar_t(Str[Index]);
          break;
      }
    }
    Result += L"\"";
  }
  else
  {
    Result = L"0x" + BytesToHex(Str);
  }
  return Result;
}
//------------------------------------------------------------------------------
UnicodeString ByteToHex(unsigned char B, bool UpperCase)
{
  static wchar_t UpperDigits[] = L"0123456789ABCDEF";
  static wchar_t LowerDigits[] = L"0123456789abcdef";

  const wchar_t * Digits = (UpperCase ? UpperDigits : LowerDigits);
  UnicodeString Result;
  Result.SetLength(2);
  Result[1] = Digits[(B & 0xF0) >> 4];
  Result[2] = Digits[(B & 0x0F) >> 0];
  return Result;
}
//------------------------------------------------------------------------------
UnicodeString BytesToHex(const unsigned char * B, uintptr_t Length, bool UpperCase, wchar_t Separator)
{
  UnicodeString Result;
  for (uintptr_t i = 0; i < Length; i++)
  {
    Result += ByteToHex(B[i], UpperCase);
    if ((Separator != L'\0') && (i < Length - 1))
    {
      Result += Separator;
    }
  }
  return Result;
}
//------------------------------------------------------------------------------
UnicodeString BytesToHex(const RawByteString & Str, bool UpperCase, wchar_t Separator)
{
  return BytesToHex(reinterpret_cast<const unsigned char *>(Str.c_str()), Str.Length(), UpperCase, Separator);
}
//------------------------------------------------------------------------------
UnicodeString CharToHex(wchar_t Ch, bool UpperCase)
{
  return BytesToHex(reinterpret_cast<const unsigned char *>(&Ch), sizeof(Ch), UpperCase);
}
//------------------------------------------------------------------------------
RawByteString HexToBytes(const UnicodeString & Hex)
{
  static UnicodeString Digits = L"0123456789ABCDEF";
  RawByteString Result;
  intptr_t L = Hex.Length();
  if (L % 2 == 0)
  {
    for (intptr_t i = 1; i <= Hex.Length(); i += 2)
    {
      intptr_t P1 = Digits.Pos((wchar_t)toupper(Hex[i]));
      intptr_t P2 = Digits.Pos((wchar_t)toupper(Hex[i + 1]));
      if (P1 <= 0 || P2 <= 0)
      {
        Result = L"";
        break;
      }
      else
      {
        Result += static_cast<char>((P1 - 1) * 16 + P2 - 1);
      }
    }
  }
  return Result;
}
//------------------------------------------------------------------------------
unsigned char HexToByte(const UnicodeString & Hex)
{
  static UnicodeString Digits = L"0123456789ABCDEF";
  assert(Hex.Length() == 2);
  intptr_t P1 = Digits.Pos((wchar_t)toupper(Hex[1]));
  intptr_t P2 = Digits.Pos((wchar_t)toupper(Hex[2]));

  return
    static_cast<unsigned char>(((P1 <= 0) || (P2 <= 0)) ? 0 : (((P1 - 1) << 4) + (P2 - 1)));
}
//------------------------------------------------------------------------------
int FindCheck(int Result)
{
  if ((Result != ERROR_SUCCESS) &&
      (Result != ERROR_FILE_NOT_FOUND) &&
      (Result != ERROR_NO_MORE_FILES))
  {
    RaiseLastOSError();
  }
  return Result;
}
//------------------------------------------------------------------------------
int FindFirstChecked(const UnicodeString & Path, int Attr, TSearchRec & F)
{
  return FindCheck(FindFirst(Path, Attr, F));
}
//------------------------------------------------------------------------------
// It can make sense to use FindNextChecked, even if unchecked FindFirst is used.
// I.e. even if we do not care that FindFirst failed, if FindNext
// failes after successfull FindFirst, it mean some terrible problem
int FindNextChecked(TSearchRec & F)
{
  return FindCheck(FindNext(F));
}
//------------------------------------------------------------------------------
bool FileSearchRec(const UnicodeString & FileName, TSearchRec & Rec)
{
  int FindAttrs = faReadOnly | faHidden | faSysFile | faDirectory | faArchive;
  bool Result = (FindFirst(FileName, FindAttrs, Rec) == 0);
  if (Result)
  {
    FindClose(Rec);
  }
  return Result;
}
//------------------------------------------------------------------------------
void ProcessLocalDirectory(const UnicodeString & DirName,
  TProcessLocalFileEvent CallBackFunc, void * Param,
  int FindAttrs)
{
  assert(CallBackFunc);
  if (FindAttrs < 0)
  {
    FindAttrs = faReadOnly | faHidden | faSysFile | faDirectory | faArchive;
  }
  TSearchRec SearchRec = {0};

  UnicodeString DirName2 = IncludeTrailingBackslash(DirName);
  if (FindFirst(DirName2 + L"*.*", FindAttrs, SearchRec) == 0)
  {
    TRY_FINALLY (
    {
      do
      {
        if ((SearchRec.Name != L".") && (SearchRec.Name != L".."))
        {
          UnicodeString FileName = DirName2 + SearchRec.Name;
          CallBackFunc(FileName, SearchRec, Param);
        }

      } while (FindNext(SearchRec) == 0);
    }
    ,
    {
      FindClose(SearchRec);
    }
    );
  }
}
//------------------------------------------------------------------------------
TDateTime EncodeDateVerbose(Word Year, Word Month, Word Day)
{
  try
  {
    return EncodeDate(Year, Month, Day);
  }
  catch (EConvertError & E)
  {
    throw EConvertError(FORMAT(L"%s [%04u-%02u-%02u]", E.Message.c_str(), int(Year), int(Month), int(Day)));
  }
  return TDateTime();
}
//------------------------------------------------------------------------------
TDateTime EncodeTimeVerbose(Word Hour, Word Min, Word Sec, Word MSec)
{
  try
  {
    return EncodeTime(Hour, Min, Sec, MSec);
  }
  catch (EConvertError & E)
  {
    throw EConvertError(FORMAT(L"%s [%02u:%02u:%02u.%04u]", E.Message.c_str(), int(Hour), int(Min), int(Sec), int(MSec)));
  }
  return TDateTime();
}
//------------------------------------------------------------------------------
struct TDateTimeParams
{
  TDateTime UnixEpoch;
  double BaseDifference;
  long BaseDifferenceSec;
  // All Current* are actually global, not per-year
  double CurrentDaylightDifference;
  long CurrentDaylightDifferenceSec;
  double CurrentDifference;
  long CurrentDifferenceSec;
  double StandardDifference;
  long StandardDifferenceSec;
  double DaylightDifference;
  long DaylightDifferenceSec;
  SYSTEMTIME SystemStandardDate;
  SYSTEMTIME SystemDaylightDate;
  TDateTime StandardDate;
  TDateTime DaylightDate;
  bool SummerDST;
  // This is actually global, not per-year
  bool DaylightHack;
};
typedef std::map<int, TDateTimeParams> TYearlyDateTimeParams;
static TYearlyDateTimeParams YearlyDateTimeParams;
static std::auto_ptr<TCriticalSection> DateTimeParamsSection(new TCriticalSection());
static void EncodeDSTMargin(const SYSTEMTIME & Date, unsigned short Year,
  TDateTime & Result);
//------------------------------------------------------------------------------
static unsigned short DecodeYear(const TDateTime & DateTime)
{
  unsigned short Year, Month, Day;
  DecodeDate(DateTime, Year, Month, Day);
  return Year;
}
//------------------------------------------------------------------------------
static const TDateTimeParams * GetDateTimeParams(unsigned short Year)
{
  TGuard Guard(DateTimeParamsSection.get());

  TDateTimeParams * Result;

  TYearlyDateTimeParams::iterator i = YearlyDateTimeParams.find(Year);
  if (i != YearlyDateTimeParams.end())
  {
    Result = &(*i).second;
  }
  else
  {
    TRACE("1");
    // creates new entry as a side effect
    Result = &YearlyDateTimeParams[Year];
    TIME_ZONE_INFORMATION TZI;

    unsigned long GTZI;

    HINSTANCE Kernel32 = GetModuleHandle(kernel32);
    typedef BOOL (WINAPI * TGetTimeZoneInformationForYear)(USHORT wYear, PDYNAMIC_TIME_ZONE_INFORMATION pdtzi, LPTIME_ZONE_INFORMATION ptzi);
    TGetTimeZoneInformationForYear GetTimeZoneInformationForYear =
      (TGetTimeZoneInformationForYear)GetProcAddress(Kernel32, "GetTimeZoneInformationForYear");
    TRACEFMT("2 [%x]", int(GetTimeZoneInformationForYear));

    if ((Year == 0) || (GetTimeZoneInformationForYear == NULL))
    {
      GTZI = GetTimeZoneInformation(&TZI);
    }
    else
    {
      GetTimeZoneInformationForYear(Year, NULL, &TZI);
      GTZI = TIME_ZONE_ID_UNKNOWN;
    }

    switch (GTZI)
    {
      case TIME_ZONE_ID_UNKNOWN:
        Result->CurrentDaylightDifferenceSec = 0;
        break;

      case TIME_ZONE_ID_STANDARD:
        Result->CurrentDaylightDifferenceSec = TZI.StandardBias;
        break;

      case TIME_ZONE_ID_DAYLIGHT:
        Result->CurrentDaylightDifferenceSec = TZI.DaylightBias;
        break;

      case TIME_ZONE_ID_INVALID:
      default:
        throw Exception(FMTLOAD(TIMEZONE_ERROR));
    }

    Result->BaseDifferenceSec = TZI.Bias;
    Result->BaseDifference = double(TZI.Bias) / MinsPerDay;
    Result->BaseDifferenceSec *= SecsPerMin;
    TRACEFMT("BaseDifference [%g], BaseDifference [%d]", Result->BaseDifference, int(Result->BaseDifferenceSec));

    Result->CurrentDifferenceSec = TZI.Bias +
      Result->CurrentDaylightDifferenceSec;
    Result->CurrentDifference =
      double(Result->CurrentDifferenceSec) / MinsPerDay;
    Result->CurrentDifferenceSec *= SecsPerMin;
    TRACEFMT("CurrentDifference [%g], CurrentDifferenceSec [%d]", Result->CurrentDifference, int(Result->CurrentDifferenceSec));

    Result->CurrentDaylightDifference =
      double(Result->CurrentDaylightDifferenceSec) / MinsPerDay;
    Result->CurrentDaylightDifferenceSec *= SecsPerMin;
    TRACEFMT("CurrentDaylightDifference [%g], CurrentDaylightDifferenceSec [%d]", Result->CurrentDaylightDifference, int(Result->CurrentDaylightDifferenceSec));

    Result->DaylightDifferenceSec = TZI.DaylightBias * SecsPerMin;
    Result->DaylightDifference = double(TZI.DaylightBias) / MinsPerDay;
    TRACEFMT("DaylightDifference [%g], DaylightDifferenceSec [%d]", Result->DaylightDifference, int(Result->DaylightDifferenceSec));
    Result->StandardDifferenceSec = TZI.StandardBias * SecsPerMin;
    Result->StandardDifference = double(TZI.StandardBias) / MinsPerDay;
    TRACEFMT("StandardDifference [%g], StandardDifferenceSec [%d]", Result->StandardDifference, int(Result->StandardDifferenceSec));

    Result->SystemStandardDate = TZI.StandardDate;
    TRACEFMT("[%d/%d/%d] [%d] [%d:%d:%d.%d]", int(Result->SystemStandardDate.wYear), int(Result->SystemStandardDate.wMonth), int(Result->SystemStandardDate.wDay), int(Result->SystemStandardDate.wDayOfWeek), int(Result->SystemStandardDate.wHour), int(Result->SystemStandardDate.wMinute), int(Result->SystemStandardDate.wSecond), int(Result->SystemStandardDate.wMilliseconds));
    Result->SystemDaylightDate = TZI.DaylightDate;
    TRACEFMT("[%d/%d/%d] [%d] [%d:%d:%d.%d]", int(Result->SystemDaylightDate.wYear), int(Result->SystemDaylightDate.wMonth), int(Result->SystemDaylightDate.wDay), int(Result->SystemDaylightDate.wDayOfWeek), int(Result->SystemDaylightDate.wHour), int(Result->SystemDaylightDate.wMinute), int(Result->SystemDaylightDate.wSecond), int(Result->SystemDaylightDate.wMilliseconds));

    unsigned short AYear = (Year != 0) ? Year : DecodeYear(Now());
    if (Result->SystemStandardDate.wMonth != 0)
    {
      EncodeDSTMargin(Result->SystemStandardDate, AYear, Result->StandardDate);
    }
    if (Result->SystemDaylightDate.wMonth != 0)
    {
      EncodeDSTMargin(Result->SystemDaylightDate, AYear, Result->DaylightDate);
    }
    Result->SummerDST = (Result->DaylightDate < Result->StandardDate);
    TRACEFMT("Summer DST [%d]", int(Result->SummerDST));

    Result->DaylightHack = !IsWin7() || IsExactly2008R2();
    TRACEFMT("DaylightHack [%d]", int(Result->DaylightHack));
  }

  return Result;
}
//------------------------------------------------------------------------------
static void EncodeDSTMargin(const SYSTEMTIME & Date, unsigned short Year,
  TDateTime & Result)
{
  CCALLSTACK(TRACE_TIMESTAMP);
  CTRACEFMT(TRACE_TIMESTAMP, "Year [%d]; Month [%d]; DayOfWeek [%d]; Day [%d]", int(Year), int(Date.wMonth), int(Date.wDayOfWeek), int(Date.wDay));
  CTRACEFMT(TRACE_TIMESTAMP, "Hour [%d]; Minute [%d]; Second [%d]; Milliseconds [%d]", int(Date.wHour), int(Date.wMinute), int(Date.wSecond), int(Date.wMilliseconds));
  if (Date.wYear == 0)
  {
    TDateTime Temp = EncodeDateVerbose(Year, Date.wMonth, 1);
    Result = Temp + ((Date.wDayOfWeek - DayOfWeek(Temp) + 8) % 7) +
      (7 * (Date.wDay - 1));
    if (Date.wDay == 5)
    {
      unsigned short Month = static_cast<unsigned short>(Date.wMonth + 1);
      if (Month > 12)
      {
        Month = static_cast<unsigned short>(Month - 12);
        Year++;
      }

      if (Result >= EncodeDateVerbose(Year, Month, 1))
      {
        Result -= 7;
      }
    }
    Result += EncodeTimeVerbose(Date.wHour, Date.wMinute, Date.wSecond,
      Date.wMilliseconds);
  }
  else
  {
    Result = EncodeDateVerbose(Year, Date.wMonth, Date.wDay) +
      EncodeTimeVerbose(Date.wHour, Date.wMinute, Date.wSecond, Date.wMilliseconds);
  }
  CTRACEFMT(TRACE_TIMESTAMP, "1 [%s]", Result.FormatString(L"c").c_str());
}
//------------------------------------------------------------------------------
static bool IsDateInDST(const TDateTime & DateTime)
{
  CCALLSTACK(TRACE_TIMESTAMP);

  const TDateTimeParams * Params = GetDateTimeParams(DecodeYear(DateTime));

  bool Result;

  // On some systems it occurs that StandardDate is unset, while
  // DaylightDate is set. MSDN states that this is invalid and
  // should be treated as if there is no daylight saving.
  // So check both.
  if ((Params->SystemStandardDate.wMonth == 0) ||
      (Params->SystemDaylightDate.wMonth == 0))
  {
    CTRACE(TRACE_TIMESTAMP, "1");
    Result = false;
  }
  else
  {
    CTRACE(TRACE_TIMESTAMP, "2");

    if (Params->SummerDST)
    {
      Result =
        (DateTime >= Params->DaylightDate) &&
        (DateTime < Params->StandardDate);
    }
    else
    {
      Result =
        (DateTime < Params->StandardDate) ||
        (DateTime >= Params->DaylightDate);
    }
    CTRACEFMT(TRACE_TIMESTAMP, "5 [%d]", int(Result));
  }
  return Result;
}
//------------------------------------------------------------------------------
bool UsesDaylightHack()
{
  return GetDateTimeParams(0)->DaylightHack;
}
//------------------------------------------------------------------------------
TDateTime UnixToDateTime(__int64 TimeStamp, TDSTMode DSTMode)
{
  CCALLSTACK(TRACE_TIMESTAMP);
  assert(int(EncodeDateVerbose(1970, 1, 1)) == UnixDateDelta);

  CTRACEFMT(TRACE_TIMESTAMP, "0 [%s]", Int64ToStr(TimeStamp).c_str());
  TDateTime Result = TDateTime(UnixDateDelta + (double(TimeStamp) / SecsPerDay));
  CTRACEFMT(TRACE_TIMESTAMP, "1 [%s]", Result.FormatString(L"c").c_str());

  const TDateTimeParams * Params = GetDateTimeParams(DecodeYear(Result));

  if (Params->DaylightHack)
  {
    if ((DSTMode == dstmWin) || (DSTMode == dstmUnix))
    {
      const TDateTimeParams * CurrentParams = GetDateTimeParams(0);
      Result -= CurrentParams->CurrentDifference;
      CTRACEFMT(TRACE_TIMESTAMP, "2 [%s]", Result.FormatString(L"c").c_str());
    }
    else if (DSTMode == dstmKeep)
    {
      Result -= Params->BaseDifference;
      CTRACEFMT(TRACE_TIMESTAMP, "3 [%s]", Result.FormatString(L"c").c_str());
    }
  }
  else
  {
    Result -= Params->BaseDifference;
    CTRACEFMT(TRACE_TIMESTAMP, "4 [%s]", Result.FormatString(L"c").c_str());
  }

  if ((DSTMode == dstmUnix) || (DSTMode == dstmKeep))
  {
    Result -= (IsDateInDST(Result) ?
      Params->DaylightDifference : Params->StandardDifference);
    CTRACEFMT(TRACE_TIMESTAMP, "5 [%s]", Result.FormatString(L"c").c_str());
  }

  CTRACEFMT(TRACE_TIMESTAMP, "6 [%s]", Result.FormatString(L"c").c_str());
  CTRACE(TRACE_TIMESTAMP, "/");
  return Result;
}
//------------------------------------------------------------------------------
__int64 Round(double Number)
{
  double Floor = floor(Number);
  double Ceil = ceil(Number);
  return static_cast<__int64>(((Number - Floor) > (Ceil - Number)) ? Ceil : Floor);
}
//------------------------------------------------------------------------------
bool TryRelativeStrToDateTime(const UnicodeString & Str, TDateTime & DateTime)
{
  UnicodeString S = Str.Trim();
  intptr_t Index = 1;
  while ((Index <= S.Length()) && (S[Index] >= '0') && (S[Index] <= '9'))
  {
    Index++;
  }
  UnicodeString NumberStr = S.SubString(1, Index - 1);
  int Number;
  bool Result = TryStrToInt(NumberStr, Number);
  if (Result)
  {
    S.Delete(1, Index - 1);
    S = S.Trim().UpperCase();
    DateTime = Now();
    // These may not overlap with ParseSize (K, M and G)
    if (S == "S")
    {
      DateTime = IncSecond(DateTime, -Number);
    }
    else if (S == "N")
    {
      DateTime = IncMinute(DateTime, -Number);
    }
    else if (S == "H")
    {
      DateTime = IncHour(DateTime, -Number);
    }
    else if (S == "D")
    {
      DateTime = IncDay(DateTime, -Number);
    }
    else if (S == "Y")
    {
      DateTime = IncYear(DateTime, -Number);
    }
    else
    {
      Result = false;
    }
  }
  return Result;
}
//------------------------------------------------------------------------------
static __int64 DateTimeToUnix(const TDateTime DateTime)
{
  CCALLSTACK(TRACE_TIMESTAMP);
  const TDateTimeParams * CurrentParams = GetDateTimeParams(0);

  assert(int(EncodeDateVerbose(1970, 1, 1)) == UnixDateDelta);

  return Round(double(DateTime - UnixDateDelta) * SecsPerDay) +
    CurrentParams->CurrentDifferenceSec;
}
//------------------------------------------------------------------------------
FILETIME DateTimeToFileTime(const TDateTime DateTime,
  TDSTMode /*DSTMode*/)
{
  CCALLSTACK(TRACE_TIMESTAMP);
  CTRACEFMT(TRACE_TIMESTAMP, "DateTimeToFileTime 1 [%s] [%s]", DateTime.DateString().c_str(), DateTime.TimeString().c_str());
  __int64 UnixTimeStamp = ::DateTimeToUnix(DateTime);

  const TDateTimeParams * Params = GetDateTimeParams(DecodeYear(DateTime));
  if (!Params->DaylightHack)
  {
    UnixTimeStamp += (IsDateInDST(DateTime) ?
      Params->DaylightDifferenceSec : Params->StandardDifferenceSec);

    const TDateTimeParams * CurrentParams = GetDateTimeParams(0);
    UnixTimeStamp -= CurrentParams->CurrentDaylightDifferenceSec;
  }

  CTRACEFMT(TRACE_TIMESTAMP, "DateTimeToFileTime 2 [%s]", Int64ToStr(UnixTimeStamp).c_str());
  FILETIME Result;
  (*(__int64*)&(Result) = (__int64(UnixTimeStamp) + 11644473600LL) * 10000000LL);
  CTRACEFMT(TRACE_TIMESTAMP, "DateTimeToFileTime 3 [%s] [%s]", IntToStr(__int64(Result.dwLowDateTime)).c_str(), IntToStr(__int64(Result.dwHighDateTime)).c_str());

  return Result;
}
//------------------------------------------------------------------------------
TDateTime FileTimeToDateTime(const FILETIME & FileTime)
{
  // duplicated in DirView.pas
  CCALLSTACK(TRACE_TIMESTAMP);
  SYSTEMTIME SysTime;
  TRACEFMT("1 [%d] [%d]", int(FileTime.dwLowDateTime), int(FileTime.dwHighDateTime));
  if (!UsesDaylightHack())
  {
    SYSTEMTIME UniverzalSysTime;
    FileTimeToSystemTime(&FileTime, &UniverzalSysTime);
    SystemTimeToTzSpecificLocalTime(NULL, &UniverzalSysTime, &SysTime);
  }
  else
  {
    FILETIME LocalFileTime;
    FileTimeToLocalFileTime(&FileTime, &LocalFileTime);
    TRACEFMT("2b [%d] [%d]", int(LocalFileTime.dwLowDateTime), int(LocalFileTime.dwHighDateTime));
    FileTimeToSystemTime(&LocalFileTime, &SysTime);
  }
  CTRACEFMT(TRACE_TIMESTAMP, "2c [%d/%d/%d] [%d] [%d:%d:%d.%d]", int(SysTime.wYear), int(SysTime.wMonth), int(SysTime.wDay), int(SysTime.wDayOfWeek), int(SysTime.wHour), int(SysTime.wMinute), int(SysTime.wSecond), int(SysTime.wMilliseconds));
  TDateTime Result = SystemTimeToDateTime(SysTime);
  CTRACEFMT(TRACE_TIMESTAMP, "2d [%s] [%s]", Result.DateString().c_str(), Result.TimeString().c_str());
  return Result;
}
//------------------------------------------------------------------------------
__int64 ConvertTimestampToUnix(const FILETIME & FileTime,
  TDSTMode DSTMode)
{
  CCALLSTACK(TRACE_TIMESTAMP);
  __int64 Result = ((*(__int64*)&(FileTime)) / 10000000LL - 11644473600LL);

  CTRACEFMT(TRACE_TIMESTAMP, "1 [%s] [%d]", Int64ToStr(Result).c_str(), int(DSTMode));
  if (UsesDaylightHack())
  {
    if ((DSTMode == dstmUnix) || (DSTMode == dstmKeep))
    {
      FILETIME LocalFileTime;
      SYSTEMTIME SystemTime;
      FileTimeToLocalFileTime(&FileTime, &LocalFileTime);
      FileTimeToSystemTime(&LocalFileTime, &SystemTime);
      TDateTime DateTime = SystemTimeToDateTime(SystemTime);
      const TDateTimeParams * Params = GetDateTimeParams(DecodeYear(DateTime));
      Result += (IsDateInDST(DateTime) ?
        Params->DaylightDifferenceSec : Params->StandardDifferenceSec);
      CTRACEFMT(TRACE_TIMESTAMP, "2 [%s]", Int64ToStr(Result).c_str());

      if (DSTMode == dstmKeep)
      {
        const TDateTimeParams * CurrentParams = GetDateTimeParams(0);
        Result -= CurrentParams->CurrentDaylightDifferenceSec;
        CTRACEFMT(TRACE_TIMESTAMP, "3 [%s]", Int64ToStr(Result).c_str());
      }
    }
  }
  else
  {
    if (DSTMode == dstmWin)
    {
      FILETIME LocalFileTime;
      SYSTEMTIME SystemTime;
      FileTimeToLocalFileTime(&FileTime, &LocalFileTime);
      FileTimeToSystemTime(&LocalFileTime, &SystemTime);
      TDateTime DateTime = SystemTimeToDateTime(SystemTime);
      const TDateTimeParams * Params = GetDateTimeParams(DecodeYear(DateTime));
      Result -= (IsDateInDST(DateTime) ?
        Params->DaylightDifferenceSec : Params->StandardDifferenceSec);
      CTRACEFMT(TRACE_TIMESTAMP, "4 [%s]", Int64ToStr(Result).c_str());
    }
  }

  CTRACEFMT(TRACE_TIMESTAMP, "5 [%s]", Int64ToStr(Result).c_str());
  return Result;
}
//------------------------------------------------------------------------------
static TDateTime ConvertTimestampToUTC(TDateTime DateTime)
{
  CCALLSTACK(TRACE_TIMESTAMP);

  const TDateTimeParams * Params = GetDateTimeParams(DecodeYear(DateTime));
  DateTime +=
    (IsDateInDST(DateTime) ?
      Params->DaylightDifference : Params->StandardDifference);
  DateTime += Params->BaseDifference;

  if (Params->DaylightHack)
  {
    const TDateTimeParams * CurrentParams = GetDateTimeParams(0);
    DateTime += CurrentParams->CurrentDaylightDifference;
  }

  return DateTime;
}
//------------------------------------------------------------------------------
TDateTime ConvertFileTimestampFromUTC(TDateTime DateTime)
{
  CCALLSTACK(TRACE_TIMESTAMP);

  const TDateTimeParams * Params = GetDateTimeParams(DecodeYear(DateTime));
  DateTime -=
    (IsDateInDST(DateTime) ?
      // Note the difference to ConvertTimestampToUTC()
      // This is to compensate CTime::GetGmtTm for MFMT FTP conversion
      Params->DaylightDifference : -Params->DaylightDifference);

  DateTime -= Params->BaseDifference;

  if (Params->DaylightHack)
  {
    const TDateTimeParams * CurrentParams = GetDateTimeParams(0);
    DateTime -= CurrentParams->CurrentDaylightDifference;
  }

  return DateTime;
}
//------------------------------------------------------------------------------
__int64 ConvertTimestampToUnixSafe(const FILETIME & FileTime,
  TDSTMode DSTMode)
{
  CCALLSTACK(TRACE_TIMESTAMP);
  __int64 Result;
  if ((FileTime.dwLowDateTime == 0) &&
      (FileTime.dwHighDateTime == 0))
  {
    Result = ::DateTimeToUnix(Now());
  }
  else
  {
    Result = ConvertTimestampToUnix(FileTime, DSTMode);
  }
  return Result;
}
//------------------------------------------------------------------------------
TDateTime AdjustDateTimeFromUnix(TDateTime DateTime, TDSTMode DSTMode)
{
  CCALLSTACK(TRACE_TIMESTAMP);
  const TDateTimeParams * Params = GetDateTimeParams(DecodeYear(DateTime));

  if (Params->DaylightHack)
  {
    if ((DSTMode == dstmWin) || (DSTMode == dstmUnix))
    {
      const TDateTimeParams * CurrentParams = GetDateTimeParams(0);
      DateTime = DateTime - CurrentParams->CurrentDaylightDifference;
    }

    if (!IsDateInDST(DateTime))
    {
      if (DSTMode == dstmWin)
      {
        DateTime = DateTime - Params->DaylightDifference;
      }
    }
    else
    {
      DateTime = DateTime - Params->StandardDifference;
    }
  }
  else
  {
    if (DSTMode == dstmWin)
    {
      if (IsDateInDST(DateTime))
      {
        DateTime = DateTime + Params->DaylightDifference;
      }
      else
      {
        DateTime = DateTime + Params->StandardDifference;
      }
    }
  }

  return DateTime;
}
//------------------------------------------------------------------------------
UnicodeString FixedLenDateTimeFormat(const UnicodeString & Format)
{
  UnicodeString Result = Format;
  bool AsIs = false;

  intptr_t Index = 1;
  while (Index <= Result.Length())
  {
    wchar_t F = Result[Index];
    if ((F == L'\'') || (F == L'\"'))
    {
      AsIs = !AsIs;
      Index++;
    }
    else if (!AsIs && ((F == L'a') || (F == L'A')))
    {
      if (Result.SubString(Index, 5).LowerCase() == L"am/pm")
      {
        Index += 5;
      }
      else if (Result.SubString(Index, 3).LowerCase() == L"a/p")
      {
        Index += 3;
      }
      else if (Result.SubString(Index, 4).LowerCase() == L"ampm")
      {
        Index += 4;
      }
      else
      {
        Index++;
      }
    }
    else
    {
      if (!AsIs && (wcschr(L"dDeEmMhHnNsS", F) != NULL) &&
          ((Index == Result.Length()) || (Result[Index + 1] != F)))
      {
        Result.Insert(F, Index);
      }

      while ((Index <= Result.Length()) && (F == Result[Index]))
      {
        Index++;
      }
    }
  }

  return Result;
}
//------------------------------------------------------------------------------
static UnicodeString FormatTimeZone(long Sec)
{
  // TTimeSpan Span = TTimeSpan::FromSeconds(Sec);
  UnicodeString Str;
  /* if ((Span.Seconds == 0) && (Span.Minutes == 0))
  {
    Str = FORMAT(L"%d", -Span.Hours);
  }
  else if (Span.Seconds == 0)
  {
    Str = FORMAT(L"%d:%2.2d", -Span.Hours, abs(Span.Minutes));
  }
  else
  {
    Str = FORMAT(L"%d:%2.2d:%2.2d", -Span.Hours, abs(Span.Minutes), abs(Span.Seconds));
  }
  Str = ((Span <= TTimeSpan::Zero) ? L"+" : L"") + Str;
  */
  return Str;
}
//------------------------------------------------------------------------------
UnicodeString GetTimeZoneLogString()
{
  const TDateTimeParams * Params = GetDateTimeParams(0);

  UnicodeString Result =
    FORMAT("Current: GMT%s, Standard: GMT%s, DST: GMT%s, DST Start: %s, DST End: %s",
      FormatTimeZone(Params->CurrentDifferenceSec).c_str(),
       FormatTimeZone(Params->BaseDifferenceSec + Params->StandardDifferenceSec).c_str(),
       FormatTimeZone(Params->BaseDifferenceSec + Params->DaylightDifferenceSec).c_str(),
       Params->DaylightDate.DateString().c_str(),
       Params->StandardDate.DateString().c_str());
  return Result;
}
//------------------------------------------------------------------------------
UnicodeString StandardTimestamp(const TDateTime & DateTime)
{
#ifndef _MSC_VER
  return FormatDateTime(L"yyyy'-'mm'-'dd'T'hh':'nn':'ss'.'zzz'Z'", ConvertTimestampToUTC(DateTime));
#else
  unsigned short Y, M, D, H, N, S, MS;
  DateTime.DecodeDate(Y, M, D);
  DateTime.DecodeTime(H, N, S, MS);
  UnicodeString dt = FORMAT(L"%04d-%02d-%02dT%02d:%02d:%02d.%03dZ", Y, M, D, H, N, S, MS);
  return dt;
#endif
}
//------------------------------------------------------------------------------
UnicodeString StandardTimestamp()
{
  return StandardTimestamp(Now());
}
//------------------------------------------------------------------------------
static TDateTime TwoSeconds(0, 0, 2, 0);
int CompareFileTime(TDateTime T1, TDateTime T2)
{
  CCALLSTACK(TRACE_TIMESTAMP);
  // "FAT" time precision
  // (when one time is seconds-precision and other is millisecond-precision,
  // we may have times like 12:00:00.000 and 12:00:01.999, which should
  // be treated the same)
  int Result;
  CTRACEFMT(TRACE_TIMESTAMP, "2Sec [%.7f], T1 [%s] [%.7f], T2 [%s] [%.7f], T2-T1 [%.7f], T1-T2 [%.7f]", double(TwoSeconds), T1.TimeString().c_str(), double(T1), T2.TimeString().c_str(), double(T2), double(T2-T1), double(T1-T2));
  if (T1 == T2)
  {
    CTRACE(TRACE_TIMESTAMP, "1");
    // just optimalisation
    Result = 0;
  }
  else if ((T1 < T2) && (T2 - T1 >= TwoSeconds))
  {
    CTRACE(TRACE_TIMESTAMP, "2");
    Result = -1;
  }
  else if ((T1 > T2) && (T1 - T2 >= TwoSeconds))
  {
    CTRACE(TRACE_TIMESTAMP, "3");
    Result = 1;
  }
  else
  {
    CTRACE(TRACE_TIMESTAMP, "4");
    Result = 0;
  }
  return Result;
}
//------------------------------------------------------------------------------
bool RecursiveDeleteFile(const UnicodeString & FileName, bool ToRecycleBin)
{
  SHFILEOPSTRUCT Data;

  memset(&Data, 0, sizeof(Data));
  Data.hwnd = NULL;
  Data.wFunc = FO_DELETE;
  UnicodeString FileList(FileName);
  FileList.SetLength(FileList.Length() + 2);
  FileList[FileList.Length() - 1] = L'\0';
  FileList[FileList.Length()] = L'\0';
  Data.pFrom = FileList.c_str();
  Data.pTo = L"\0\0";
  Data.fFlags = FOF_NOCONFIRMATION | FOF_RENAMEONCOLLISION | FOF_NOCONFIRMMKDIR |
    FOF_NOERRORUI | FOF_SILENT;
  if (ToRecycleBin)
  {
    Data.fFlags |= FOF_ALLOWUNDO;
  }
  int ErrorCode = SHFileOperation(&Data);
  bool Result = (ErrorCode == 0);
  if (!Result)
  {
    // according to MSDN, SHFileOperation may return following non-Win32
    // error codes
    if (((ErrorCode >= 0x71) && (ErrorCode <= 0x88)) ||
        (ErrorCode == 0xB7) || (ErrorCode == 0x402) || (ErrorCode == 0x10000) ||
        (ErrorCode == 0x10074))
    {
      ErrorCode = 0;
    }
    SetLastError(ErrorCode);
  }
  return Result;
}
//------------------------------------------------------------------------------
intptr_t CancelAnswer(intptr_t Answers)
{
  intptr_t Result;
  if ((Answers & qaCancel) != 0)
  {
    Result = qaCancel;
  }
  else if ((Answers & qaNo) != 0)
  {
    Result = qaNo;
  }
  else if ((Answers & qaAbort) != 0)
  {
    Result = qaAbort;
  }
  else if ((Answers & qaOK) != 0)
  {
    Result = qaOK;
  }
  else
  {
    assert(false);
    Result = qaCancel;
  }
  return Result;
}
//------------------------------------------------------------------------------
intptr_t AbortAnswer(intptr_t Answers)
{
  intptr_t Result;
  if (FLAGSET(Answers, qaAbort))
  {
    Result = qaAbort;
  }
  else
  {
    Result = CancelAnswer(Answers);
  }
  return Result;
}
//------------------------------------------------------------------------------
intptr_t ContinueAnswer(intptr_t Answers)
{
  intptr_t Result;
  if (FLAGSET(Answers, qaSkip))
  {
    Result = qaSkip;
  }
  else if (FLAGSET(Answers, qaIgnore))
  {
    Result = qaIgnore;
  }
  else if (FLAGSET(Answers, qaYes))
  {
    Result = qaYes;
  }
  else if (FLAGSET(Answers, qaOK))
  {
    Result = qaOK;
  }
  else if (FLAGSET(Answers, qaRetry))
  {
    Result = qaRetry;
  }
  else
  {
    Result = CancelAnswer(Answers);
  }
  return Result;
}
//------------------------------------------------------------------------------
#ifndef _MSC_VER
TLibModule * FindModule(void * Instance)
{
  TLibModule * CurModule;
  CurModule = reinterpret_cast<TLibModule*>(LibModuleList);

  while (CurModule)
  {
    if (CurModule->Instance == (unsigned)Instance)
    {
      break;
    }
    else
    {
      CurModule = CurModule->Next;
    }
  }
  return CurModule;
}
#endif
//------------------------------------------------------------------------------
UnicodeString LoadStr(int Ident, intptr_t MaxLength)
{
  DEBUG_PRINTF(L"begin, Ident = %d", Ident);
  UnicodeString Result; // = Sysutils::FmtLoadStr(Ident, L"");
  Result.SetLength(MaxLength > 0 ? MaxLength : 1024);
  HINSTANCE hInstance =  GlobalFunctions ? GlobalFunctions->GetHandle() : ::GetModuleHandle(0);
  DEBUG_PRINTF(L"hInstance = %p", hInstance);
  assert(hInstance != 0);
  intptr_t Length = static_cast<intptr_t>(::LoadString(hInstance, Ident, reinterpret_cast<LPWSTR>(const_cast<wchar_t *>(Result.c_str())), (int)Result.Length()));
  Result.SetLength(Length);
  DEBUG_PRINTF(L"end, Result = %s", Result.c_str());
  return Result;
}
//------------------------------------------------------------------------------
UnicodeString LoadStrPart(int Ident, int Part)
{
  UnicodeString Result;
  UnicodeString Str = LoadStr(Ident);

  while (Part > 0)
  {
    Result = CutToChar(Str, L'|', false);
    Part--;
  }
  return Result;
}
//------------------------------------------------------------------------------
UnicodeString DecodeUrlChars(const UnicodeString & S)
{
  UnicodeString Result = S;
  int I = 1;
  while (I <= Result.Length())
  {
    switch (Result[I])
    {
      case L'+':
        Result[I] = ' ';
        break;

      case L'%':
        if (I <= Result.Length() - 2)
        {
          unsigned char B = HexToByte(Result.SubString(I + 1, 2));
          if (B > 0)
          {
            Result[I] = (wchar_t)B;
            Result.Delete(I + 1, 2);
          }
        }
        break;
    }
    I++;
  }
  return Result;
}
//------------------------------------------------------------------------------
UnicodeString DoEncodeUrl(const UnicodeString & S, const UnicodeString & Chars)
{
  UnicodeString Result = S;
  intptr_t i = 1;
  while (i <= Result.Length())
  {
    if (Chars.Pos(Result[i]) > 0)
    {
      UnicodeString H = ByteToHex(AnsiString(UnicodeString(Result[i]))[1]);
      Result.Insert(H, i + 1);
      Result[i] = '%';
      i += H.Length();
    }
    i++;
  }
  return Result;
}
//------------------------------------------------------------------------------
UnicodeString EncodeUrlChars(const UnicodeString & S, const UnicodeString & Ignore)
{
  UnicodeString Chars;
  if (Ignore.Pos(L' ') == 0)
  {
    Chars += L' ';
  }
  if (Ignore.Pos(L'/') == 0)
  {
    Chars += L'/';
  }
  return DoEncodeUrl(S, Chars);
}
//------------------------------------------------------------------------------
UnicodeString NonUrlChars()
{
  UnicodeString S;
  for (unsigned int I = 0; I <= 127; I++)
  {
    wchar_t C = static_cast<wchar_t>(I);
    if (((C >= L'a') && (C <= L'z')) ||
        ((C >= L'A') && (C <= L'Z')) ||
        ((C >= L'0') && (C <= L'9')) ||
        (C == L'_') || (C == L'-') || (C == L'.'))
    {
      // noop
    }
    else
    {
      S += C;
    }
  }
  return S;
}
//------------------------------------------------------------------------------
UnicodeString EncodeUrlString(const UnicodeString & S)
{
  return DoEncodeUrl(S, NonUrlChars());
}
//------------------------------------------------------------------------------
UnicodeString EscapeHotkey(const UnicodeString & Caption)
{
  return StringReplace(Caption, L"&", L"&&", TReplaceFlags() << rfReplaceAll);
}
//------------------------------------------------------------------------------
// duplicated in console's Main.cpp
bool CutToken(UnicodeString & Str, UnicodeString & Token,
  UnicodeString * RawToken)
{
  bool Result;

  Token = L"";

  // inspired by Putty's sftp_getcmd() from PSFTP.C
  int Index = 1;
  while ((Index <= Str.Length()) &&
    ((Str[Index] == L' ') || (Str[Index] == L'\t')))
  {
    Index++;
  }

  if (Index <= Str.Length())
  {
    bool Quoting = false;

    while (Index <= Str.Length())
    {
      if (!Quoting && ((Str[Index] == L' ') || (Str[Index] == L'\t')))
      {
        break;
      }
      else if ((Str[Index] == L'"') && (Index + 1 <= Str.Length()) &&
        (Str[Index + 1] == L'"'))
      {
        Index += 2;
        Token += L'"';
      }
      else if (Str[Index] == L'"')
      {
        Index++;
        Quoting = !Quoting;
      }
      else
      {
        Token += Str[Index];
        Index++;
      }
    }

    if (RawToken != NULL)
    {
      (*RawToken) = Str.SubString(1, Index - 1);
    }

    if (Index <= Str.Length())
    {
      Index++;
    }

    Str = Str.SubString(Index, Str.Length());

    Result = true;
  }
  else
  {
    Result = false;
    Str = L"";
  }

  return Result;
}
//------------------------------------------------------------------------------
void AddToList(UnicodeString & List, const UnicodeString & Value, const UnicodeString & Delimiter)
{
  if (!Value.IsEmpty())
  {
    if (!List.IsEmpty() &&
        ((List.Length() < Delimiter.Length()) ||
         (List.SubString(List.Length() - Delimiter.Length() + 1, Delimiter.Length()) != Delimiter)))
    {
      List += Delimiter;
    }
    List += Value;
  }
}
//------------------------------------------------------------------------------
bool Is2000()
{
  return (Win32MajorVersion >= 5);
}
//------------------------------------------------------------------------------
bool IsWin7()
{
  return
    (Win32MajorVersion > 6) ||
    ((Win32MajorVersion == 6) && (Win32MinorVersion >= 1));
}
//------------------------------------------------------------------------------
bool IsExactly2008R2()
{
  CALLSTACK;
  HINSTANCE Kernel32 = GetModuleHandle(kernel32);
  typedef BOOL (WINAPI * TGetProductInfo)(DWORD, DWORD, DWORD, DWORD, PDWORD);
  TGetProductInfo GetProductInfo =
      (TGetProductInfo)GetProcAddress(Kernel32, "GetProductInfo");
  bool Result;
  if (GetProductInfo == NULL)
  {
    Result = false;
  }
  else
  {
    DWORD Type;
    GetProductInfo(Win32MajorVersion, Win32MinorVersion, 0, 0, &Type);
    TRACEFMT("1 [%x]", int(Type));
    switch (Type)
    {
      case 0x0008 /*PRODUCT_DATACENTER_SERVER*/:
      case 0x000C /*PRODUCT_DATACENTER_SERVER_CORE}*/:
      case 0x0027 /*PRODUCT_DATACENTER_SERVER_CORE_V*/:
      case 0x0025 /*PRODUCT_DATACENTER_SERVER_V*/:
      case 0x000A /*PRODUCT_ENTERPRISE_SERVE*/:
      case 0x000E /*PRODUCT_ENTERPRISE_SERVER_COR*/:
      case 0x0029 /*PRODUCT_ENTERPRISE_SERVER_CORE_*/:
      case 0x000F /*PRODUCT_ENTERPRISE_SERVER_IA6*/:
      case 0x0026 /*PRODUCT_ENTERPRISE_SERVER_*/:
      case 0x002A /*PRODUCT_HYPER*/:
      case 0x001E /*PRODUCT_MEDIUMBUSINESS_SERVER_MANAGEMEN*/:
      case 0x0020 /*PRODUCT_MEDIUMBUSINESS_SERVER_MESSAGIN*/:
      case 0x001F /*PRODUCT_MEDIUMBUSINESS_SERVER_SECURIT*/:
      case 0x0018 /*PRODUCT_SERVER_FOR_SMALLBUSINES*/:
      case 0x0023 /*PRODUCT_SERVER_FOR_SMALLBUSINESS_*/:
      case 0x0021 /*PRODUCT_SERVER_FOUNDATIO*/:
      case 0x0009 /*PRODUCT_SMALLBUSINESS_SERVE*/:
      case 0x0038 /*PRODUCT_SOLUTION_EMBEDDEDSERVE*/:
      case 0x0007 /*PRODUCT_STANDARD_SERVE*/:
      case 0x000D /*PRODUCT_STANDARD_SERVER_COR*/:
      case 0x0028 /*PRODUCT_STANDARD_SERVER_CORE_*/:
      case 0x0024 /*PRODUCT_STANDARD_SERVER_*/:
      case 0x0017 /*PRODUCT_STORAGE_ENTERPRISE_SERVE*/:
      case 0x0014 /*PRODUCT_STORAGE_EXPRESS_SERVE*/:
      case 0x0015 /*PRODUCT_STORAGE_STANDARD_SERVE*/:
      case 0x0016 /*PRODUCT_STORAGE_WORKGROUP_SERVE*/:
      case 0x0011 /*PRODUCT_WEB_SERVE*/:
      case 0x001D /*PRODUCT_WEB_SERVER_COR*/:
        Result = true;
        break;

      default:
        Result = false;
        break;
    }
  }
  TRACEFMT("2 [%x]", int(Result));
  return Result;
}
//------------------------------------------------------------------------------
LCID GetDefaultLCID()
{
  return Is2000() ? GetUserDefaultLCID() : GetThreadLocale();
}
//------------------------------------------------------------------------------
static UnicodeString ADefaultEncodingName;
UnicodeString DefaultEncodingName()
{
  if (ADefaultEncodingName.IsEmpty())
  {
    CPINFOEX Info;
    GetCPInfoEx(CP_ACP, 0, &Info);
    ADefaultEncodingName = Info.CodePageName;
  }
  return ADefaultEncodingName;
}
//------------------------------------------------------------------------------
UnicodeString WindowsProductName()
{
  UnicodeString Result;
  TRegistry * Registry = new TRegistry();
  Registry->Access = KEY_READ;
  try
  {
    Registry->RootKey = HKEY_LOCAL_MACHINE;
    if (Registry->OpenKey("SOFTWARE", false) &&
        Registry->OpenKey("Microsoft", false) &&
        Registry->OpenKey("Windows NT", false) &&
        Registry->OpenKey("CurrentVersion", false))
    {
      Result = Registry->ReadString("ProductName");
    }
    delete Registry;
  }
  catch(...)
  {
    TRACE("E");
  }
  return Result;
}
//------------------------------------------------------------------------------
uintptr_t StrToVersionNumber(const UnicodeString & VersionMumberStr)
{
  uintptr_t Result = 0;
  UnicodeString Version = VersionMumberStr;
  int Shift = 16;
  while (!Version.IsEmpty())
  {
    UnicodeString Num = CutToChar(Version, L'.', true);
    Result += static_cast<uintptr_t>(Num.ToInt()) << Shift;
    if (Shift >= 8) Shift -= 8;
  }
  return Result;
}
//------------------------------------------------------------------------------
UnicodeString VersionNumberToStr(uintptr_t VersionNumber)
{
  DWORD Major = (VersionNumber>>16) & 0xFF;
  DWORD Minor = (VersionNumber>>8) & 0xFF; 
  DWORD Revision = (VersionNumber & 0xFF);
  UnicodeString Result = FORMAT(L"%d.%d.%d", Major, Minor, Revision);
  return Result;
}
//---------------------------------------------------------------------
UnicodeString FormatBytes(__int64 Bytes, bool UseOrders)
{
  UnicodeString Result;

  if (!UseOrders || (Bytes < static_cast<__int64>(100*1024)))
  {
    // Result = FormatFloat(L"#,##0 \"B\"", Bytes);
    Result = FORMAT(L"%.0f B", static_cast<double>(Bytes));
  }
  else if (Bytes < static_cast<__int64>(100*1024*1024))
  {
    // Result = FormatFloat(L"#,##0 \"KiB\"", Bytes / 1024);
    Result = FORMAT(L"%.0f KiB", static_cast<double>(Bytes / 1024.0));
  }
  else
  {
    // Result = FormatFloat(L"#,##0 \"MiB\"", Bytes / (1024*1024));
    Result = FORMAT(L"%.0f MiB", static_cast<double>(Bytes / (1024*1024.0)));
  }
  return Result;
}
//------------------------------------------------------------------------------
// Suppress warning about unused constants in DateUtils.hpp
#pragma warn -8080

