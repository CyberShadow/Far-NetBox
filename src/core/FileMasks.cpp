//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "FileMasks.h"

#include "Common.h"
#include "TextsCore.h"
#include "RemoteFiles.h"
#include "PuttyTools.h"
#include "Terminal.h"
//---------------------------------------------------------------------------
extern const wchar_t IncludeExcludeFileMasksDelimiter = L'|';
static UnicodeString FileMasksDelimiters = L";,";
static UnicodeString AllFileMasksDelimiters = FileMasksDelimiters + IncludeExcludeFileMasksDelimiter;
static UnicodeString DirectoryMaskDelimiters = L"/\\";
static UnicodeString FileMasksDelimiterStr = UnicodeString(FileMasksDelimiters[1]) + L' ';
//---------------------------------------------------------------------------
EFileMasksException::EFileMasksException(
    UnicodeString Message, intptr_t AErrorStart, intptr_t AErrorLen) :
  Exception(Message)
{
  ErrorStart = AErrorStart;
  ErrorLen = AErrorLen;
}
//---------------------------------------------------------------------------
UnicodeString MaskFilePart(const UnicodeString & Part, const UnicodeString & Mask, bool& Masked)
{
  UnicodeString Result;
  intptr_t RestStart = 1;
  bool Delim = false;
  for (int Index = 1; Index <= Mask.Length(); Index++)
  {
    switch (Mask[Index])
    {
      case L'\\':
        if (!Delim)
        {
          Delim = true;
          Masked = true;
          break;
        }

      case L'*':
        if (!Delim)
        {
          Result += Part.SubString(RestStart, Part.Length() - RestStart + 1);
          RestStart = Part.Length() + 1;
          Masked = true;
          break;
        }

      case L'?':
        if (!Delim)
        {
          if (RestStart <= Part.Length())
          {
            Result += Part[RestStart];
            RestStart++;
          }
          Masked = true;
          break;
        }

      default:
        Result += Mask[Index];
        RestStart++;
        Delim = false;
        break;
    }
  }
  return Result;
}
//---------------------------------------------------------------------------
UnicodeString MaskFileName(const UnicodeString & FileName, const UnicodeString & Mask)
{
  UnicodeString Result = FileName;
  if (IsEffectiveFileNameMask(Mask))
  {
    bool Masked;
    intptr_t P = Mask.LastDelimiter(L".");
    if (P > 0)
    {
      intptr_t P2 = Result.LastDelimiter(".");
      // only dot at beginning of file name is not considered as
      // name/ext separator
      UnicodeString FileExt = P2 > 1 ?
        Result.SubString(P2 + 1, Result.Length() - P2) : UnicodeString();
      FileExt = MaskFilePart(FileExt, Mask.SubString(P + 1, Mask.Length() - P), Masked);
      if (P2 > 1)
      {
        Result.SetLength(P2 - 1);
      }
      Result = MaskFilePart(Result, Mask.SubString(1, P - 1), Masked);
      if (!FileExt.IsEmpty())
      {
        Result += L"." + FileExt;
      }
    }
    else
    {
      Result = MaskFilePart(Result, Mask, Masked);
    }
  }
  return Result;
}
//---------------------------------------------------------------------------
bool IsEffectiveFileNameMask(const UnicodeString & Mask)
{
  return !Mask.IsEmpty() && (Mask != L"*") && (Mask != L"*.*");
}
//---------------------------------------------------------------------------
UnicodeString DelimitFileNameMask(UnicodeString Mask)
{
  for (int i = 1; i <= Mask.Length(); i++)
  {
    if (wcschr(L"\\*?", Mask[i]) != NULL)
    {
      Mask.Insert(L"\\", i);
      i++;
    }
  }
  return Mask;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TFileMasks::TParams::TParams() :
  Size(0)
{
}
//---------------------------------------------------------------------------
UnicodeString TFileMasks::TParams::ToString() const
{
  return UnicodeString(L"[") + Int64ToStr(Size) + L"/" + DateTimeToString(Modification) + L"]";
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool TFileMasks::IsMask(const UnicodeString & Mask)
{
  return (Mask.LastDelimiter(L"?*[") > 0);
}
//---------------------------------------------------------------------------
bool TFileMasks::IsAnyMask(const UnicodeString & Mask)
{
  return Mask.IsEmpty() || (Mask == L"*.*") || (Mask == L"*");
}
//---------------------------------------------------------------------------
UnicodeString TFileMasks::NormalizeMask(const UnicodeString & Mask, const UnicodeString & AnyMask)
{
  if (IsAnyMask(Mask))
  {
    return AnyMask;
  }
  else
  {
    return Mask;
  }
}
//---------------------------------------------------------------------------
UnicodeString TFileMasks::ComposeMaskStr(
  TStrings * MasksStr, bool Directory)
{
  UnicodeString Result;
  UnicodeString ResultNoDirMask;
  for (int I = 0; I < MasksStr->Count; I++)
  {
    UnicodeString Str = MasksStr->Strings[I].Trim();
    if (!Str.IsEmpty())
    {
      for (int P = 1; P <= Str.Length(); P++)
      {
        if (Str.IsDelimiter(AllFileMasksDelimiters, P))
        {
          Str.Insert(Str[P], P);
          P++;
        }
      }

      UnicodeString StrNoDirMask;
      if (Directory)
      {
        StrNoDirMask = Str;
        Str = MakeDirectoryMask(Str);
      }
      else
      {
        while (Str.IsDelimiter(DirectoryMaskDelimiters, Str.Length()))
        {
          Str.SetLength(Str.Length() - 1);
        }
        StrNoDirMask = Str;
      }

      AddToList(Result, Str, FileMasksDelimiterStr);
      AddToList(ResultNoDirMask, StrNoDirMask, FileMasksDelimiterStr);
    }
  }

  // For directories, the above will add slash ay the end of masks,
  // breaking size and time masks and thus circumverting their validation.
  // This performes as hoc validation to cover the scenario.
  // For files this makes no difference, but no harm either
  TFileMasks Temp(Directory ? 1 : 0);
  Temp = ResultNoDirMask;

  return Result;
}
//---------------------------------------------------------------------------
UnicodeString TFileMasks::ComposeMaskStr(
  TStrings * IncludeFileMasksStr, TStrings * ExcludeFileMasksStr,
  TStrings * IncludeDirectoryMasksStr, TStrings * ExcludeDirectoryMasksStr)
{
  UnicodeString IncludeMasks = ComposeMaskStr(IncludeFileMasksStr, false);
  AddToList(IncludeMasks, ComposeMaskStr(IncludeDirectoryMasksStr, true), FileMasksDelimiterStr);
  UnicodeString ExcludeMasks = ComposeMaskStr(ExcludeFileMasksStr, false);
  AddToList(ExcludeMasks, ComposeMaskStr(ExcludeDirectoryMasksStr, true), FileMasksDelimiterStr);

  UnicodeString Result = IncludeMasks;
  if (!ExcludeMasks.IsEmpty())
  {
    if (!Result.IsEmpty())
    {
      Result += L' ';
    }
    Result += UnicodeString(IncludeExcludeFileMasksDelimiter) + L' ' + ExcludeMasks;
  }
  return Result;
}
//---------------------------------------------------------------------------
TFileMasks::TFileMasks()
{
  Init();
}
//---------------------------------------------------------------------------
TFileMasks::TFileMasks(int ForceDirectoryMasks)
{
  Init();
  FForceDirectoryMasks = ForceDirectoryMasks;
}
//---------------------------------------------------------------------------
TFileMasks::TFileMasks(const TFileMasks & Source)
{
  Init();
  FForceDirectoryMasks = Source.FForceDirectoryMasks;
  SetStr(Source.GetMasks(), false);
}
//---------------------------------------------------------------------------
TFileMasks::TFileMasks(const UnicodeString & AMasks)
{
  Init();
  SetStr(AMasks, false);
}
//---------------------------------------------------------------------------
TFileMasks::~TFileMasks()
{
  Clear();
}
//---------------------------------------------------------------------------
void TFileMasks::Init()
{
  FForceDirectoryMasks = -1;
  for (int Index = 0; Index < 4; Index++)
  {
    FMasksStr[Index] = NULL;
  }

  DoInit(false);
}
//---------------------------------------------------------------------------
void TFileMasks::DoInit(bool Delete)
{
  for (int Index = 0; Index < 4; Index++)
  {
    if (Delete)
    {
      delete FMasksStr[Index];
    }
    FMasksStr[Index] = NULL;
  }
}
//---------------------------------------------------------------------------
void TFileMasks::Clear()
{
  DoInit(true);

  for (int Index = 0; Index < 4; Index++)
  {
    Clear(FMasks[Index]);
  }
}
//---------------------------------------------------------------------------
void TFileMasks::Clear(TMasks & Masks)
{
  TMasks::iterator I = Masks.begin();
  while (I != Masks.end())
  {
    ReleaseMaskMask((*I).FileNameMask);
    ReleaseMaskMask((*I).DirectoryMask);
    ++I;
  }
  Masks.clear();
}
//---------------------------------------------------------------------------
bool TFileMasks::MatchesMasks(const UnicodeString & FileName, bool Directory,
  const UnicodeString & Path, const TParams * Params, const TMasks & Masks, bool Recurse)
{
  CALLSTACK;
  bool Result = false;

  TMasks::const_iterator I = Masks.begin();
  while (!Result && (I != Masks.end()))
  {
    const TMask & Mask = *I;
    // TRACEFMT("1 [%s]", Mask.MaskStr.c_str());
    Result =
      MatchesMaskMask(Mask.DirectoryMask, Path) &&
      MatchesMaskMask(Mask.FileNameMask, FileName);

    if (Result)
    {
      // TRACE("2");
      bool HasSize = (Params != NULL);

      switch (Mask.HighSizeMask)
      {
        case TMask::None:
          Result = true;
          break;

        case TMask::Open:
          Result = HasSize && (Params->Size < Mask.HighSize);
          break;

        case TMask::Close:
          Result = HasSize && (Params->Size <= Mask.HighSize);
          break;
      }

      if (Result)
      {
        // TRACE("3");
        switch (Mask.LowSizeMask)
        {
          case TMask::None:
            Result = true;
            break;

          case TMask::Open:
            Result = HasSize && (Params->Size > Mask.LowSize);
            break;

          case TMask::Close:
            Result = HasSize && (Params->Size >= Mask.LowSize); //-V595
            break;
        }
      }

      bool HasModification = (Params != NULL);

      if (Result)
      {
        // TRACE("4");
        switch (Mask.HighModificationMask)
        {
          case TMask::None:
            Result = true;
            break;

          case TMask::Open:
            Result = HasModification && (Params->Modification < Mask.HighModification);
            break;

          case TMask::Close:
            Result = HasModification && (Params->Modification <= Mask.HighModification);
            break;
        }
      }

      if (Result)
      {
        // TRACE("5");
        switch (Mask.LowModificationMask)
        {
          case TMask::None:
            Result = true;
            break;

          case TMask::Open:
            Result = HasModification && (Params->Modification > Mask.LowModification);
            break;

          case TMask::Close:
            Result = HasModification && (Params->Modification >= Mask.LowModification);
            break;
        }
      }
    }

    // TRACEFMT("6 [%d]", int(Result));
    ++I;
  }

  if (!Result && Directory && !IsUnixRootPath(Path) && Recurse)
  {
    UnicodeString ParentFileName = UnixExtractFileName(Path);
    UnicodeString ParentPath = UnixExcludeTrailingBackslash(UnixExtractFilePath(Path));
    // Pass Params down or not?
    // Currently it includes Size/Time only, what is not used for directories.
    // So it depends of future use. Possibly we should make a copy
    // and pass on only relevant fields.
    Result = MatchesMasks(ParentFileName, true, ParentPath, Params, Masks, Recurse);
  }

  return Result;
}
//---------------------------------------------------------------------------
bool TFileMasks::Matches(const UnicodeString & FileName, bool Directory,
  const UnicodeString & Path, const TParams * Params) const
{
  bool ImplicitMatch;
  return Matches(FileName, Directory, Path, Params, ImplicitMatch);
}
//---------------------------------------------------------------------------
bool TFileMasks::Matches(const UnicodeString & FileName, bool Directory,
  const UnicodeString & Path, const TParams * Params,
  bool & ImplicitMatch) const
{
  // TRACEFMT("1 [%s] [%d] [%s] %s", FileName.c_str(), int(Directory), Path.c_str(), UnicodeString((Params == NULL) ? L"<null>" : Params->ToString()).c_str());
  bool ImplicitIncludeMatch = (FMasks[MASK_INDEX(Directory, true)].empty());
  bool ExplicitIncludeMatch = MatchesMasks(FileName, Directory, Path, Params, FMasks[MASK_INDEX(Directory, true)], true);
  bool Result =
    (ImplicitIncludeMatch || ExplicitIncludeMatch) &&
    !MatchesMasks(FileName, Directory, Path, Params, FMasks[MASK_INDEX(Directory, false)], false);
  ImplicitMatch =
    Result && ImplicitIncludeMatch && !ExplicitIncludeMatch &&
    FMasks[MASK_INDEX(Directory, false)].empty();
  return Result;
}
//---------------------------------------------------------------------------
bool TFileMasks::Matches(const UnicodeString & FileName, bool Local,
  bool Directory, const TParams * Params) const
{
  bool ImplicitMatch;
  return Matches(FileName, Local, Directory, Params, ImplicitMatch);
}
//---------------------------------------------------------------------------
bool TFileMasks::Matches(const UnicodeString & FileName, bool Local,
  bool Directory, const TParams * Params, bool & ImplicitMatch) const
{
  CALLSTACK;
  bool Result;
  if (Local)
  {
    // TRACE("1");
    UnicodeString Path = ExtractFilePath(FileName);
    if (!Path.IsEmpty())
    {
      Path = ToUnixPath(ExcludeTrailingBackslash(Path));
    }
    Result = Matches(ExtractFileName(FileName, false), Directory, Path, Params,
      ImplicitMatch);
  }
  else
  {
    // TRACE("2");
    Result = Matches(UnixExtractFileName(FileName), Directory,
      UnixExcludeTrailingBackslash(UnixExtractFilePath(FileName)), Params,
      ImplicitMatch);
  }
  return Result;
}
//---------------------------------------------------------------------------
bool TFileMasks::GetIsValid() const
{
  intptr_t Start, Length;
  return GetIsValid(Start, Length);
}
//---------------------------------------------------------------------------
bool TFileMasks::GetIsValid(intptr_t & Start, intptr_t & Length) const
{
  if (IsMask(FStr) || FStr.IsEmpty())
  {
    Start = 0;
    Length = FStr.Length();
    return true;
  }
  else
  {
    Start = 0;
    Length = 0;
    return false;
  }
}
//---------------------------------------------------------------------------
bool TFileMasks::operator ==(const TFileMasks & rhm) const
{
  return (GetMasks() == rhm.GetMasks());
}
//---------------------------------------------------------------------------
TFileMasks & TFileMasks::operator =(const UnicodeString & rhs)
{
  SetMasks(rhs);
  return *this;
}
//---------------------------------------------------------------------------
TFileMasks & TFileMasks::operator =(const TFileMasks & rhm)
{
  FForceDirectoryMasks = rhm.FForceDirectoryMasks;
  SetMasks(rhm.GetMasks());
  return *this;
}
//---------------------------------------------------------------------------
bool TFileMasks::operator ==(const UnicodeString & rhs) const
{
  return (GetMasks() == rhs);
}
//---------------------------------------------------------------------------
void TFileMasks::ThrowError(intptr_t Start, intptr_t End) const
{
  throw EFileMasksException(
    FMTLOAD(MASK_ERROR, GetMasks().SubString(Start, End - Start + 1).c_str()),
    Start, End - Start + 1);
}
//---------------------------------------------------------------------------
void TFileMasks::CreateMaskMask(const UnicodeString & Mask, intptr_t Start, intptr_t End,
  bool Ex, TMaskMask & MaskMask) const
{
  try
  {
    assert(MaskMask.Mask == NULL);
    if (Ex && IsAnyMask(Mask))
    {
      MaskMask.Kind = TMaskMask::Any;
      MaskMask.Mask = NULL;
    }
    else
    {
      MaskMask.Kind = (Ex && (Mask == L"*.")) ? TMaskMask::NoExt : TMaskMask::Regular;
      MaskMask.Mask = new Masks::TMask(Mask);
    }
  }
  catch(...)
  {
    ThrowError(Start, End);
  }
}
//---------------------------------------------------------------------------
UnicodeString TFileMasks::MakeDirectoryMask(const UnicodeString & Str)
{
  assert(!Str.IsEmpty());
  UnicodeString Result = Str;
  if (Result.IsEmpty() || !Result.IsDelimiter(DirectoryMaskDelimiters, Result.Length()))
  {
    intptr_t D = Result.LastDelimiter(DirectoryMaskDelimiters);
    // if there's any [back]slash anywhere in str,
    // add the same [back]slash at the end, otherwise add slash
    wchar_t Delimiter = (D > 0) ? Result[D] : DirectoryMaskDelimiters[1];
    Result += Delimiter;
  }
  return Result;
}
//---------------------------------------------------------------------------
void TFileMasks::CreateMask(
  const UnicodeString & MaskStr, intptr_t MaskStart, intptr_t /*MaskEnd*/, bool Include)
{
  bool Directory = false; // shut up
  TMask Mask;

  Mask.MaskStr = MaskStr;
  Mask.UserStr = MaskStr;
  Mask.FileNameMask.Kind = TMaskMask::Any;
  Mask.FileNameMask.Mask = NULL;
  Mask.DirectoryMask.Kind = TMaskMask::Any;
  Mask.DirectoryMask.Mask = NULL;
  Mask.HighSizeMask = TMask::None;
  Mask.LowSizeMask = TMask::None;
  Mask.HighModificationMask = TMask::None;
  Mask.LowModificationMask = TMask::None;

  wchar_t NextPartDelimiter = L'\0';
  intptr_t NextPartFrom = 1;
  while (NextPartFrom <= MaskStr.Length())
  {
    wchar_t PartDelimiter = NextPartDelimiter;
    intptr_t PartFrom = NextPartFrom;
    UnicodeString PartStr = CopyToChars(MaskStr, NextPartFrom, L"<>", false, &NextPartDelimiter, true);

    intptr_t PartStart = MaskStart + PartFrom - 1;
    intptr_t PartEnd = MaskStart + NextPartFrom - 1 - 2;

    TrimEx(PartStr, PartStart, PartEnd);

    if (PartDelimiter != L'\0')
    {
      bool Low = (PartDelimiter == L'>');

      TMask::TMaskBoundary Boundary;
      if ((PartStr.Length() >= 1) && (PartStr[1] == L'='))
      {
        Boundary = TMask::Close;
        PartStr.Delete(1, 1);
      }
      else
      {
        Boundary = TMask::Open;
      }

      TFormatSettings FormatSettings = TFormatSettings::Create(GetDefaultLCID());
      FormatSettings.DateSeparator = L'-';
      FormatSettings.TimeSeparator = L':';
      FormatSettings.ShortDateFormat = "yyyy/mm/dd";
      FormatSettings.ShortTimeFormat = "hh:nn:ss";

      TDateTime Modification;
      if (TryStrToDateTime(PartStr, Modification, FormatSettings) ||
          TryRelativeStrToDateTime(PartStr, Modification))
      {
        TMask::TMaskBoundary & ModificationMask =
          (Low ? Mask.LowModificationMask : Mask.HighModificationMask);

        if ((ModificationMask != TMask::None) || Directory)
        {
          // include delimiter into size part
          ThrowError(PartStart - 1, PartEnd);
        }

        ModificationMask = Boundary;
        (Low ? Mask.LowModification : Mask.HighModification) = Modification;
      }
      else
      {
        TMask::TMaskBoundary & SizeMask = (Low ? Mask.LowSizeMask : Mask.HighSizeMask);
        __int64 & Size = (Low ? Mask.LowSize : Mask.HighSize);

        if ((SizeMask != TMask::None) || Directory)
        {
          // include delimiter into size part
          ThrowError(PartStart - 1, PartEnd);
        }

        SizeMask = Boundary;
        Size = ParseSize(PartStr);
      }
    }
    else if (!PartStr.IsEmpty())
    {
      intptr_t D = PartStr.LastDelimiter(DirectoryMaskDelimiters);

      Directory = (D > 0) && (D == PartStr.Length());

      if (Directory)
      {
        do
        {
          PartStr.SetLength(PartStr.Length() - 1);
          Mask.UserStr.Delete(PartStart - MaskStart + D, 1);
          D--;
        }
        while (PartStr.IsDelimiter(DirectoryMaskDelimiters, PartStr.Length()));

        D = PartStr.LastDelimiter(DirectoryMaskDelimiters);

        if (FForceDirectoryMasks == 0)
        {
          Directory = false;
          Mask.MaskStr = Mask.UserStr;
        }
      }
      else if (FForceDirectoryMasks > 0)
      {
        Directory = true;
        Mask.MaskStr.Insert(DirectoryMaskDelimiters[1], PartStart - MaskStart + PartStr.Length());
      }

      if (D > 0)
      {
        // make sure sole "/" (root dir) is preserved as is
        CreateMaskMask(
          UnixExcludeTrailingBackslash(ToUnixPath(PartStr.SubString(1, D))),
          PartStart, PartStart + D - 1, false,
          Mask.DirectoryMask);
        CreateMaskMask(
          PartStr.SubString(D + 1, PartStr.Length() - D),
          PartStart + D, PartEnd, true,
          Mask.FileNameMask);
      }
      else
      {
        CreateMaskMask(PartStr, PartStart, PartEnd, true, Mask.FileNameMask);
      }
    }
  }

  FMasks[MASK_INDEX(Directory, Include)].push_back(Mask);
}
//---------------------------------------------------------------------------
TStrings * TFileMasks::GetMasksStr(intptr_t Index) const
{
  if (FMasksStr[Index] == NULL)
  {
    FMasksStr[Index] = new TStringList();
    TMasks::const_iterator I = FMasks[Index].begin();
    while (I != FMasks[Index].end())
    {
      FMasksStr[Index]->Add((*I).UserStr);
      ++I;
    }
  }

  return FMasksStr[Index];
}
//---------------------------------------------------------------------------
void TFileMasks::ReleaseMaskMask(TMaskMask & MaskMask)
{
  delete MaskMask.Mask;
}
//---------------------------------------------------------------------------
void TFileMasks::TrimEx(UnicodeString & Str, intptr_t & Start, intptr_t & End)
{
  UnicodeString Buf = TrimLeft(Str);
  Start += Str.Length() - Buf.Length();
  Str = TrimRight(Buf);
  End -= Buf.Length() - Str.Length();
}
//---------------------------------------------------------------------------
bool TFileMasks::MatchesMaskMask(const TMaskMask & MaskMask, const UnicodeString & Str)
{
  bool Result;
  if (MaskMask.Kind == TMaskMask::Any)
  {
    Result = true;
  }
  else if ((MaskMask.Kind == TMaskMask::NoExt) && (Str.Pos(L".") == 0))
  {
    Result = true;
  }
  else
  {
    Result = MaskMask.Mask->Matches(Str);
  }
  return Result;
}
//---------------------------------------------------------------------------
void TFileMasks::SetMasks(const UnicodeString & Value)
{
  if (FStr != Value)
  {
    SetStr(Value, false);
  }
}
//---------------------------------------------------------------------------
void TFileMasks::SetMask(const UnicodeString & Mask)
{
  SetStr(Mask, true);
}
//---------------------------------------------------------------------------
void TFileMasks::SetStr(const UnicodeString & Str, bool SingleMask)
{
  UnicodeString Backup = FStr;
  try
  {
    FStr = Str;
    Clear();

    intptr_t NextMaskFrom = 1;
    bool Include = true;
    while (NextMaskFrom <= Str.Length())
    {
      intptr_t MaskStart = NextMaskFrom;
      wchar_t NextMaskDelimiter;
      UnicodeString MaskStr;
      if (SingleMask)
      {
        MaskStr = Str;
        NextMaskFrom = Str.Length() + 1;
        NextMaskDelimiter = L'\0';
      }
      else
      {
        MaskStr = CopyToChars(Str, NextMaskFrom, AllFileMasksDelimiters, false, &NextMaskDelimiter, true);
      }
      intptr_t MaskEnd = NextMaskFrom - 2;

      TrimEx(MaskStr, MaskStart, MaskEnd);

      if (!MaskStr.IsEmpty())
      {
        CreateMask(MaskStr, MaskStart, MaskEnd, Include);
      }

      if (NextMaskDelimiter == IncludeExcludeFileMasksDelimiter)
      {
        if (Include)
        {
          Include = false;
        }
        else
        {
          ThrowError(NextMaskFrom - 1, Str.Length());
        }
      }
    }
  }
  catch(...)
  {
    // this does not work correctly if previous mask was set using SetMask.
    // this should not fail (the mask was validated before),
    // otherwise we end in an infinite loop
    SetStr(Backup, false);
    throw;
  }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#define TEXT_TOKEN L'\255'
//---------------------------------------------------------------------------
const wchar_t TCustomCommand::NoQuote = L'\0';
const UnicodeString TCustomCommand::Quotes = L"\"'";
//---------------------------------------------------------------------------
TCustomCommand::TCustomCommand()
{
}
//---------------------------------------------------------------------------
void TCustomCommand::GetToken(
  const UnicodeString & Command, intptr_t Index, intptr_t & Len, wchar_t & PatternCmd)
{
  assert(Index <= Command.Length());
  const wchar_t * Ptr = Command.c_str() + Index - 1;

  if (Ptr[0] == L'!')
  {
    PatternCmd = Ptr[1];
    if (PatternCmd == L'!')
    {
      Len = 2;
    }
    else
    {
      Len = PatternLen(Index, PatternCmd);
    }

    if (Len < 0)
    {
      throw Exception(FMTLOAD(CUSTOM_COMMAND_UNKNOWN, PatternCmd, Index));
    }
    else if (Len > 0)
    {
      if ((Command.Length() - Index + 1) < Len)
      {
        throw Exception(FMTLOAD(CUSTOM_COMMAND_UNTERMINATED, PatternCmd, Index));
      }
    }
    else if (Len == 0)
    {
      const wchar_t * PatternEnd = wcschr(Ptr + 1, L'!');
      if (PatternEnd == NULL)
      {
        throw Exception(FMTLOAD(CUSTOM_COMMAND_UNTERMINATED, PatternCmd, Index));
      }
      Len = PatternEnd - Ptr + 1;
    }
  }
  else
  {
    PatternCmd = TEXT_TOKEN;
    const wchar_t * NextPattern = wcschr(Ptr, L'!');
    if (NextPattern == NULL)
    {
      Len = Command.Length() - Index + 1;
    }
    else
    {
      Len = NextPattern - Ptr;
    }
  }
}
//---------------------------------------------------------------------------
UnicodeString TCustomCommand::Complete(const UnicodeString & Command,
  bool LastPass)
{
  UnicodeString Result;
  intptr_t Index = 1;

  while (Index <= Command.Length())
  {
    intptr_t Len;
    wchar_t PatternCmd;
    GetToken(Command, Index, Len, PatternCmd);

    if (PatternCmd == TEXT_TOKEN)
    {
      Result += Command.SubString(Index, Len);
    }
    else if (PatternCmd == L'!')
    {
      if (LastPass)
      {
        Result += L'!';
      }
      else
      {
        Result += Command.SubString(Index, Len);
      }
    }
    else
    {
      wchar_t Quote = NoQuote;
      if ((Index > 1) && (Index + Len - 1 < Command.Length()) &&
          Command.IsDelimiter(Quotes, Index - 1) &&
          Command.IsDelimiter(Quotes, Index + Len) &&
          (Command[Index - 1] == Command[Index + Len]))
      {
        Quote = Command[Index - 1];
      }
      UnicodeString Pattern = Command.SubString(Index, Len);
      UnicodeString Replacement;
      bool Delimit = true;
      if (PatternReplacement(Index, Pattern, Replacement, Delimit))
      {
        if (!LastPass)
        {
          Replacement = StringReplace(Replacement, L"!", L"!!",
            TReplaceFlags() << rfReplaceAll);
        }
        if (Delimit)
        {
          DelimitReplacement(Replacement, Quote);
        }
        Result += Replacement;
      }
      else
      {
        Result += Pattern;
      }
    }

    Index += Len;
  }

  return Result;
}
//---------------------------------------------------------------------------
void TCustomCommand::DelimitReplacement(UnicodeString & Replacement, wchar_t Quote)
{
  Replacement = ShellDelimitStr(Replacement, Quote);
}
//---------------------------------------------------------------------------
void TCustomCommand::Validate(const UnicodeString & Command)
{
  CustomValidate(Command, NULL);
}
//---------------------------------------------------------------------------
void TCustomCommand::CustomValidate(const UnicodeString & Command,
  void * Arg)
{
  intptr_t Index = 1;

  while (Index <= Command.Length())
  {
    intptr_t Len;
    wchar_t PatternCmd;
    GetToken(Command, Index, Len, PatternCmd);
    ValidatePattern(Command, Index, Len, PatternCmd, Arg);

    Index += Len;
  }
}
//---------------------------------------------------------------------------
bool TCustomCommand::FindPattern(const UnicodeString & Command,
  wchar_t PatternCmd)
{
  bool Result = false;
  intptr_t Index = 1;

  while (!Result && (Index <= Command.Length()))
  {
    intptr_t Len;
    wchar_t APatternCmd;
    GetToken(Command, Index, Len, APatternCmd);
    if (((PatternCmd != L'!') && (PatternCmd == APatternCmd)) ||
        ((PatternCmd == L'!') && (Len == 1) && (APatternCmd != TEXT_TOKEN)))
    {
      Result = true;
    }

    Index += Len;
  }

  return Result;
}
//---------------------------------------------------------------------------
void TCustomCommand::ValidatePattern(const UnicodeString & /*Command*/,
  intptr_t /*Index*/, intptr_t /*Len*/, wchar_t /*PatternCmd*/, void * /*Arg*/)
{
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TInteractiveCustomCommand::TInteractiveCustomCommand(
  TCustomCommand * ChildCustomCommand)
{
  FChildCustomCommand = ChildCustomCommand;
}
//---------------------------------------------------------------------------
void TInteractiveCustomCommand::Prompt(intptr_t /*Index*/,
  const UnicodeString & /*Prompt*/, UnicodeString & Value)
{
  Value = L"";
}
//---------------------------------------------------------------------------
intptr_t TInteractiveCustomCommand::PatternLen(intptr_t Index, wchar_t PatternCmd)
{
  intptr_t Len = 0;
  switch (PatternCmd)
  {
    case L'?':
      Len = 0;
      break;

    default:
      Len = FChildCustomCommand->PatternLen(Index, PatternCmd);
      break;
  }
  return Len;
}
//---------------------------------------------------------------------------
bool TInteractiveCustomCommand::PatternReplacement(intptr_t Index, const UnicodeString & Pattern,
  UnicodeString & Replacement, bool & Delimit)
{
  bool Result;
  if ((Pattern.Length() >= 3) && (Pattern[2] == L'?'))
  {
    UnicodeString PromptStr;
    intptr_t Pos = Pattern.SubString(3, Pattern.Length() - 2).Pos(L"?");
    if (Pos > 0)
    {
      Replacement = Pattern.SubString(3 + Pos, Pattern.Length() - 3 - Pos);
      if ((Pos > 1) && (Pattern[3 + Pos - 2] == L'\\'))
      {
        Delimit = false;
        Pos--;
      }
      PromptStr = Pattern.SubString(3, Pos - 1);
    }
    else
    {
      PromptStr = Pattern.SubString(3, Pattern.Length() - 3);
    }

    Prompt(Index, PromptStr, Replacement);

    Result = true;
  }
  else
  {
    Result = false;
  }

  return Result;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TCustomCommandData::TCustomCommandData()
{
}
//---------------------------------------------------------------------------
TCustomCommandData::TCustomCommandData(TTerminalIntf * Terminal)
{
  HostName = Terminal->GetSessionData()->GetHostNameExpanded();
  UserName = Terminal->GetSessionData()->GetUserNameExpanded();
  Password = Terminal->GetPassword();
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TFileCustomCommand::TFileCustomCommand()
{
}
//---------------------------------------------------------------------------
TFileCustomCommand::TFileCustomCommand(const TCustomCommandData & Data,
  const UnicodeString & Path)
{
  FData = Data;
  FPath = Path;
}
//---------------------------------------------------------------------------
TFileCustomCommand::TFileCustomCommand(const TCustomCommandData & Data,
    const UnicodeString & Path, const UnicodeString & FileName,
    const UnicodeString & FileList) :
  TCustomCommand()
{
  FData = Data;
  FPath = Path;
  FFileName = FileName;
  FFileList = FileList;
}
//---------------------------------------------------------------------------
intptr_t TFileCustomCommand::PatternLen(intptr_t /*Index*/, wchar_t PatternCmd)
{
  intptr_t Len;
  switch (toupper(PatternCmd))
  {
    case L'@':
    case L'U':
    case L'P':
    case L'/':
    case L'&':
      Len = 2;
      break;

    default:
      Len = 1;
      break;
  }
  return Len;
}
//---------------------------------------------------------------------------
bool TFileCustomCommand::PatternReplacement(intptr_t /*Index*/,
  const UnicodeString & Pattern, UnicodeString & Replacement, bool & Delimit)
{
  // keep consistent with TSessionLog::OpenLogFile

  if (Pattern == L"!@")
  {
    Replacement = FData.HostName;
  }
  else if (AnsiSameText(Pattern, L"!u"))
  {
    Replacement = FData.UserName;
  }
  else if (AnsiSameText(Pattern, L"!p"))
  {
    Replacement = FData.Password;
  }
  else if (Pattern == L"!/")
  {
    Replacement = UnixIncludeTrailingBackslash(FPath);
  }
  else if (Pattern == L"!&")
  {
    Replacement = FFileList;
    // already delimited
    Delimit = false;
  }
  else
  {
    assert(Pattern.Length() == 1);
    Replacement = FFileName;
  }

  return true;
}
//---------------------------------------------------------------------------
void TFileCustomCommand::Validate(const UnicodeString & Command)
{
  int Found[2] = { 0, 0 };
  CustomValidate(Command, &Found);
  if ((Found[0] > 0) && (Found[1] > 0))
  {
    throw Exception(FMTLOAD(CUSTOM_COMMAND_FILELIST_ERROR,
      Found[1], Found[0]));
  }
}
//---------------------------------------------------------------------------
void TFileCustomCommand::ValidatePattern(const UnicodeString & /*Command*/,
  intptr_t Index, intptr_t /*Len*/, wchar_t PatternCmd, void * Arg)
{
  int * Found = static_cast<int *>(Arg);

  assert(Index > 0);

  if (PatternCmd == L'&')
  {
    Found[0] = (int)Index;
  }
  else if ((PatternCmd != TEXT_TOKEN) && (PatternLen(Index, PatternCmd) == 1))
  {
    Found[1] = (int)Index;
  }
}
//---------------------------------------------------------------------------
bool TFileCustomCommand::IsFileListCommand(const UnicodeString & Command)
{
  return FindPattern(Command, L'&');
}
//---------------------------------------------------------------------------
bool TFileCustomCommand::IsFileCommand(const UnicodeString & Command)
{
  return FindPattern(Command, L'!') || FindPattern(Command, L'&');
}
//---------------------------------------------------------------------------
