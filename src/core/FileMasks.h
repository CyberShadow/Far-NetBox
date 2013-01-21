//---------------------------------------------------------------------------
#ifndef FileMasksH
#define FileMasksH
//---------------------------------------------------------------------------
#include <vector>
#include <CoreDefs.hpp>
#include <Masks.hpp>
//---------------------------------------------------------------------------
class EFileMasksException : public Exception
{
public:
  explicit /* __fastcall */ EFileMasksException(UnicodeString Message, intptr_t ErrorStart, intptr_t ErrorLen);
  intptr_t ErrorStart;
  intptr_t ErrorLen;
};
//---------------------------------------------------------------------------
extern const wchar_t IncludeExcludeFileMasksDelimiter;
#define MASK_INDEX(DIRECTORY, INCLUDE) ((DIRECTORY ? 2 : 0) + (INCLUDE ? 0 : 1))
//---------------------------------------------------------------------------
class TFileMasks
{
public:
  struct TParams
  {
    TParams();
    __int64 Size;
    TDateTime Modification;

    UnicodeString ToString() const;
  };

  static bool __fastcall IsMask(const UnicodeString & Mask);
  static UnicodeString __fastcall NormalizeMask(const UnicodeString & Mask, const UnicodeString & AnyMask = "");
  static UnicodeString __fastcall ComposeMaskStr(
    TStrings * IncludeFileMasksStr, TStrings * ExcludeFileMasksStr,
    TStrings * IncludeDirectoryMasksStr, TStrings * ExcludeDirectoryMasksStr);
  static UnicodeString __fastcall ComposeMaskStr(TStrings * MasksStr, bool Directory);

  /* __fastcall */ TFileMasks();
  explicit /* __fastcall */ TFileMasks(int ForceDirectoryMasks);
  /* __fastcall */ TFileMasks(const TFileMasks & Source);
  explicit /* __fastcall */ TFileMasks(const UnicodeString & AMasks);
  virtual /* __fastcall */ ~TFileMasks();
  TFileMasks & __fastcall operator =(const TFileMasks & rhm);
  TFileMasks & __fastcall operator =(const UnicodeString & rhs);
  bool __fastcall operator ==(const TFileMasks & rhm) const;
  bool __fastcall operator ==(const UnicodeString & rhs) const;

  void __fastcall SetMask(const UnicodeString & Mask);

  bool __fastcall Matches(const UnicodeString & FileName, bool Directory = false,
    const UnicodeString & Path = "", const TParams * Params = NULL) const;
  bool __fastcall Matches(const UnicodeString & FileName, bool Directory,
    const UnicodeString & Path, const TParams * Params,
    bool & ImplicitMatch) const;
  bool __fastcall Matches(const UnicodeString & FileName, bool Local, bool Directory,
    const TParams * Params = NULL) const;
  bool __fastcall Matches(const UnicodeString & FileName, bool Local, bool Directory,
    const TParams * Params, bool & ImplicitMatch) const;

  bool __fastcall GetIsValid() const;
  bool __fastcall GetIsValid(intptr_t & Start, intptr_t & Length) const;
  UnicodeString __fastcall GetMasks() const { return FStr; }
  void __fastcall SetMasks(const UnicodeString & Value);

  TStrings * __fastcall GetIncludeFileMasksStr() const { return GetMasksStr(MASK_INDEX(false, true)); };
  TStrings * __fastcall GetExcludeFileMasksStr() const { return GetMasksStr(MASK_INDEX(false, false)); };
  TStrings * __fastcall GetIncludeDirectoryMasksStr() const { return GetMasksStr(MASK_INDEX(true, true)); };
  TStrings * __fastcall GetExcludeDirectoryMasksStr() const { return GetMasksStr(MASK_INDEX(true, false)); };

private:
  int FForceDirectoryMasks;
  UnicodeString FStr;

  struct TMaskMask
  {
    TMaskMask() :
      Kind(Any),
      Mask(NULL)
    {}
    enum { Any, NoExt, Regular } Kind;
    Masks::TMask * Mask;
  };

  struct TMask
  {
    TMask() :
      HighSizeMask(None),
      HighSize(0),
      LowSizeMask(None),
      LowSize(0),
      HighModificationMask(None),
      LowModificationMask(None)
    {}
    TMaskMask FileNameMask;
    TMaskMask DirectoryMask;

    enum TMaskBoundary { None, Open, Close };

    TMaskBoundary HighSizeMask;
    __int64 HighSize;
    TMaskBoundary LowSizeMask;
    __int64 LowSize;

    TMaskBoundary HighModificationMask;
    TDateTime HighModification;
    TMaskBoundary LowModificationMask;
    TDateTime LowModification;

    UnicodeString MaskStr;
    UnicodeString UserStr;
  };

  typedef std::vector<TMask> TMasks;
  TMasks FMasks[4];
  mutable TStrings * FMasksStr[4];

  void __fastcall SetStr(const UnicodeString & Value, bool SingleMask);
  void __fastcall CreateMaskMask(const UnicodeString & Mask, intptr_t Start, intptr_t End,
    bool Ex, TMaskMask & MaskMask) const;
  void __fastcall CreateMask(const UnicodeString & MaskStr, intptr_t MaskStart,
    intptr_t MaskEnd, bool Include);
  TStrings * __fastcall GetMasksStr(intptr_t Index) const;
private:
  static UnicodeString __fastcall MakeDirectoryMask(const UnicodeString & Str);
  static inline void __fastcall ReleaseMaskMask(TMaskMask & MaskMask);
  inline void __fastcall Init();
  void __fastcall DoInit(bool Delete);
  void __fastcall Clear();
  static void __fastcall Clear(TMasks & Masks);
  static void __fastcall TrimEx(UnicodeString & Str, intptr_t & Start, intptr_t & End);
  static bool __fastcall MatchesMasks(const UnicodeString & FileName, bool Directory,
    const UnicodeString & Path, const TParams * Params, const TMasks & Masks, bool Recurse);
  static inline bool __fastcall MatchesMaskMask(const TMaskMask & MaskMask, const UnicodeString & Str);
  static inline bool __fastcall IsAnyMask(const UnicodeString & Mask);
  void __fastcall ThrowError(intptr_t Start, intptr_t End) const;
};
//---------------------------------------------------------------------------
UnicodeString __fastcall MaskFileName(const UnicodeString & FileName, const UnicodeString & Mask);
bool __fastcall IsEffectiveFileNameMask(const UnicodeString & Mask);
UnicodeString __fastcall DelimitFileNameMask(UnicodeString Mask);
//---------------------------------------------------------------------------
DEFINE_CALLBACK_TYPE5(TCustomCommandPatternEvent, void,
  int /* Index */, const UnicodeString & /* Pattern */, void * /* Arg */, UnicodeString & /* Replacement */,
  bool & /* LastPass */);
//---------------------------------------------------------------------------
class TCustomCommand
{
friend class TInteractiveCustomCommand;

public:
  TCustomCommand();
  virtual ~TCustomCommand() {}

  UnicodeString __fastcall Complete(const UnicodeString & Command, bool LastPass);
  virtual void __fastcall Validate(const UnicodeString & Command);

protected:
  static const wchar_t NoQuote;
  static const UnicodeString Quotes;
  void __fastcall GetToken(const UnicodeString & Command,
    intptr_t Index, intptr_t & Len, wchar_t & PatternCmd);
  void __fastcall CustomValidate(const UnicodeString & Command, void * Arg);
  bool __fastcall FindPattern(const UnicodeString & Command, wchar_t PatternCmd);

  virtual void __fastcall ValidatePattern(const UnicodeString & Command,
    intptr_t Index, intptr_t Len, wchar_t PatternCmd, void * Arg);

  virtual intptr_t __fastcall PatternLen(intptr_t Index, wchar_t PatternCmd) = 0;
  virtual bool __fastcall PatternReplacement(intptr_t Index, const UnicodeString & Pattern,
    UnicodeString & Replacement, bool & Delimit) = 0;
  virtual void __fastcall DelimitReplacement(UnicodeString & Replacement, wchar_t Quote);
};
//---------------------------------------------------------------------------
class TInteractiveCustomCommand : public TCustomCommand
{
public:
  explicit /* __fastcall */ TInteractiveCustomCommand(TCustomCommand * ChildCustomCommand);

protected:
  virtual void __fastcall Prompt(intptr_t Index, const UnicodeString & Prompt,
    UnicodeString & Value);
  virtual intptr_t __fastcall PatternLen(intptr_t Index, wchar_t PatternCmd);
  virtual bool __fastcall PatternReplacement(intptr_t Index, const UnicodeString & Pattern,
    UnicodeString & Replacement, bool & Delimit);

private:
  TCustomCommand * FChildCustomCommand;
};
//---------------------------------------------------------------------------
class TTerminalIntf;
struct TCustomCommandData
{
  /* __fastcall */ TCustomCommandData();
  explicit /* __fastcall */ TCustomCommandData(TTerminalIntf * Terminal);

  UnicodeString HostName;
  UnicodeString UserName;
  UnicodeString Password;
};
//---------------------------------------------------------------------------
class TFileCustomCommand : public TCustomCommand
{
public:
  /* __fastcall */ TFileCustomCommand();
  explicit /* __fastcall */ TFileCustomCommand(const TCustomCommandData & Data, const UnicodeString & Path);
  explicit /* __fastcall */ TFileCustomCommand(const TCustomCommandData & Data, const UnicodeString & Path,
    const UnicodeString & FileName, const UnicodeString & FileList);
  virtual /* __fastcall */ ~TFileCustomCommand() {}

  virtual void __fastcall Validate(const UnicodeString & Command);
  virtual void __fastcall ValidatePattern(const UnicodeString & Command,
    intptr_t Index, intptr_t Len, wchar_t PatternCmd, void * Arg);

  bool __fastcall IsFileListCommand(const UnicodeString & Command);
  virtual bool __fastcall IsFileCommand(const UnicodeString & Command);

protected:
  virtual intptr_t __fastcall PatternLen(intptr_t Index, wchar_t PatternCmd);
  virtual bool __fastcall PatternReplacement(intptr_t Index, const UnicodeString & Pattern,
    UnicodeString & Replacement, bool & Delimit);

private:
  TCustomCommandData FData;
  UnicodeString FPath;
  UnicodeString FFileName;
  UnicodeString FFileList;
};
//---------------------------------------------------------------------------
typedef TFileCustomCommand TRemoteCustomCommand;
//---------------------------------------------------------------------------
#endif
