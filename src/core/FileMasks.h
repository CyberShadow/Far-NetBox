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
  explicit EFileMasksException(UnicodeString Message, intptr_t ErrorStart, intptr_t ErrorLen);
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

  static bool IsMask(const UnicodeString & Mask);
  static UnicodeString NormalizeMask(const UnicodeString & Mask, const UnicodeString & AnyMask = "");
  static UnicodeString ComposeMaskStr(
    TStrings * IncludeFileMasksStr, TStrings * ExcludeFileMasksStr,
    TStrings * IncludeDirectoryMasksStr, TStrings * ExcludeDirectoryMasksStr);
  static UnicodeString ComposeMaskStr(TStrings * MasksStr, bool Directory);

  TFileMasks();
  explicit TFileMasks(int ForceDirectoryMasks);
  TFileMasks(const TFileMasks & Source);
  explicit TFileMasks(const UnicodeString & AMasks);
  virtual ~TFileMasks();
  TFileMasks & operator =(const TFileMasks & rhm);
  TFileMasks & operator =(const UnicodeString & rhs);
  bool operator ==(const TFileMasks & rhm) const;
  bool operator ==(const UnicodeString & rhs) const;

  void SetMask(const UnicodeString & Mask);

  bool Matches(const UnicodeString & FileName, bool Directory = false,
    const UnicodeString & Path = "", const TParams * Params = NULL) const;
  bool Matches(const UnicodeString & FileName, bool Directory,
    const UnicodeString & Path, const TParams * Params,
    bool & ImplicitMatch) const;
  bool Matches(const UnicodeString & FileName, bool Local, bool Directory,
    const TParams * Params = NULL) const;
  bool Matches(const UnicodeString & FileName, bool Local, bool Directory,
    const TParams * Params, bool & ImplicitMatch) const;

  bool GetIsValid() const;
  bool GetIsValid(intptr_t & Start, intptr_t & Length) const;
  UnicodeString GetMasks() const { return FStr; }
  void SetMasks(const UnicodeString & Value);

  TStrings * GetIncludeFileMasksStr() const { return GetMasksStr(MASK_INDEX(false, true)); };
  TStrings * GetExcludeFileMasksStr() const { return GetMasksStr(MASK_INDEX(false, false)); };
  TStrings * GetIncludeDirectoryMasksStr() const { return GetMasksStr(MASK_INDEX(true, true)); };
  TStrings * GetExcludeDirectoryMasksStr() const { return GetMasksStr(MASK_INDEX(true, false)); };

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

  void SetStr(const UnicodeString & Value, bool SingleMask);
  void CreateMaskMask(const UnicodeString & Mask, intptr_t Start, intptr_t End,
    bool Ex, TMaskMask & MaskMask) const;
  void CreateMask(const UnicodeString & MaskStr, intptr_t MaskStart,
    intptr_t MaskEnd, bool Include);
  TStrings * GetMasksStr(intptr_t Index) const;
private:
  static UnicodeString MakeDirectoryMask(const UnicodeString & Str);
  static inline void ReleaseMaskMask(TMaskMask & MaskMask);
  inline void Init();
  void DoInit(bool Delete);
  void Clear();
  static void Clear(TMasks & Masks);
  static void TrimEx(UnicodeString & Str, intptr_t & Start, intptr_t & End);
  static bool MatchesMasks(const UnicodeString & FileName, bool Directory,
    const UnicodeString & Path, const TParams * Params, const TMasks & Masks, bool Recurse);
  static inline bool MatchesMaskMask(const TMaskMask & MaskMask, const UnicodeString & Str);
  static inline bool IsAnyMask(const UnicodeString & Mask);
  void ThrowError(intptr_t Start, intptr_t End) const;
};
//---------------------------------------------------------------------------
UnicodeString MaskFileName(const UnicodeString & FileName, const UnicodeString & Mask);
bool IsEffectiveFileNameMask(const UnicodeString & Mask);
UnicodeString DelimitFileNameMask(UnicodeString Mask);
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

  UnicodeString Complete(const UnicodeString & Command, bool LastPass);
  virtual void Validate(const UnicodeString & Command);

protected:
  static const wchar_t NoQuote;
  static const UnicodeString Quotes;
  void GetToken(const UnicodeString & Command,
    intptr_t Index, intptr_t & Len, wchar_t & PatternCmd);
  void CustomValidate(const UnicodeString & Command, void * Arg);
  bool FindPattern(const UnicodeString & Command, wchar_t PatternCmd);

  virtual void ValidatePattern(const UnicodeString & Command,
    intptr_t Index, intptr_t Len, wchar_t PatternCmd, void * Arg);

  virtual intptr_t PatternLen(intptr_t Index, wchar_t PatternCmd) = 0;
  virtual bool PatternReplacement(intptr_t Index, const UnicodeString & Pattern,
    UnicodeString & Replacement, bool & Delimit) = 0;
  virtual void DelimitReplacement(UnicodeString & Replacement, wchar_t Quote);
};
//---------------------------------------------------------------------------
class TInteractiveCustomCommand : public TCustomCommand
{
public:
  explicit TInteractiveCustomCommand(TCustomCommand * ChildCustomCommand);

protected:
  virtual void Prompt(intptr_t Index, const UnicodeString & Prompt,
    UnicodeString & Value);
  virtual intptr_t PatternLen(intptr_t Index, wchar_t PatternCmd);
  virtual bool PatternReplacement(intptr_t Index, const UnicodeString & Pattern,
    UnicodeString & Replacement, bool & Delimit);

private:
  TCustomCommand * FChildCustomCommand;
};
//---------------------------------------------------------------------------
class TTerminalIntf;
struct TCustomCommandData
{
  TCustomCommandData();
  explicit TCustomCommandData(TTerminalIntf * Terminal);

  UnicodeString HostName;
  UnicodeString UserName;
  UnicodeString Password;
};
//---------------------------------------------------------------------------
class TFileCustomCommand : public TCustomCommand
{
public:
  TFileCustomCommand();
  explicit TFileCustomCommand(const TCustomCommandData & Data, const UnicodeString & Path);
  explicit TFileCustomCommand(const TCustomCommandData & Data, const UnicodeString & Path,
    const UnicodeString & FileName, const UnicodeString & FileList);
  virtual ~TFileCustomCommand() {}

  virtual void Validate(const UnicodeString & Command);
  virtual void ValidatePattern(const UnicodeString & Command,
    intptr_t Index, intptr_t Len, wchar_t PatternCmd, void * Arg);

  bool IsFileListCommand(const UnicodeString & Command);
  virtual bool IsFileCommand(const UnicodeString & Command);

protected:
  virtual intptr_t PatternLen(intptr_t Index, wchar_t PatternCmd);
  virtual bool PatternReplacement(intptr_t Index, const UnicodeString & Pattern,
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
