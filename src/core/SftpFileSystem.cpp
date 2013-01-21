//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#define TRACE_STRINGS TRACING
#define CLEAN_SPACE_AVAILABLE

#include "SftpFileSystem.h"

#include "PuttyTools.h"
#include "Common.h"
#include "Exceptions.h"
#include "Interface.h"
#include "Terminal.h"
#include "TextsCore.h"
#include "HelpCore.h"
#include "SecureShell.h"

#include <memory>
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
#define FILE_OPERATION_LOOP_EX(ALLOW_SKIP, MESSAGE, OPERATION) \
  FILE_OPERATION_LOOP_CUSTOM(FTerminal, ALLOW_SKIP, MESSAGE, OPERATION)
//---------------------------------------------------------------------------
#define SSH_FX_OK                                 0
#define SSH_FX_EOF                                1
#define SSH_FX_NO_SUCH_FILE                       2
#define SSH_FX_PERMISSION_DENIED                  3
#define SSH_FX_OP_UNSUPPORTED                     8

#define SSH_FXP_INIT               1
#define SSH_FXP_VERSION            2
#define SSH_FXP_OPEN               3
#define SSH_FXP_CLOSE              4
#define SSH_FXP_READ               5
#define SSH_FXP_WRITE              6
#define SSH_FXP_LSTAT              7
#define SSH_FXP_FSTAT              8
#define SSH_FXP_SETSTAT            9
#define SSH_FXP_FSETSTAT           10
#define SSH_FXP_OPENDIR            11
#define SSH_FXP_READDIR            12
#define SSH_FXP_REMOVE             13
#define SSH_FXP_MKDIR              14
#define SSH_FXP_RMDIR              15
#define SSH_FXP_REALPATH           16
#define SSH_FXP_STAT               17
#define SSH_FXP_RENAME             18
#define SSH_FXP_READLINK           19
#define SSH_FXP_SYMLINK            20
#define SSH_FXP_STATUS             101
#define SSH_FXP_HANDLE             102
#define SSH_FXP_DATA               103
#define SSH_FXP_NAME               104
#define SSH_FXP_ATTRS              105
#define SSH_FXP_EXTENDED           200
#define SSH_FXP_EXTENDED_REPLY     201
#define SSH_FXP_ATTRS              105

#define SSH_FILEXFER_ATTR_SIZE              0x00000001
#define SSH_FILEXFER_ATTR_UIDGID            0x00000002
#define SSH_FILEXFER_ATTR_PERMISSIONS       0x00000004
#define SSH_FILEXFER_ATTR_ACMODTIME         0x00000008
#define SSH_FILEXFER_ATTR_EXTENDED          0x80000000
#define SSH_FILEXFER_ATTR_ACCESSTIME        0x00000008
#define SSH_FILEXFER_ATTR_CREATETIME        0x00000010
#define SSH_FILEXFER_ATTR_MODIFYTIME        0x00000020
#define SSH_FILEXFER_ATTR_ACL               0x00000040
#define SSH_FILEXFER_ATTR_OWNERGROUP        0x00000080
#define SSH_FILEXFER_ATTR_SUBSECOND_TIMES   0x00000100
#define SSH_FILEXFER_ATTR_BITS              0x00000200
#define SSH_FILEXFER_ATTR_EXTENDED          0x80000000

#define SSH_FILEXFER_ATTR_COMMON \
  (SSH_FILEXFER_ATTR_SIZE | SSH_FILEXFER_ATTR_OWNERGROUP | \
   SSH_FILEXFER_ATTR_PERMISSIONS | SSH_FILEXFER_ATTR_ACCESSTIME | \
   SSH_FILEXFER_ATTR_MODIFYTIME)

#define SSH_FILEXFER_TYPE_REGULAR          1
#define SSH_FILEXFER_TYPE_DIRECTORY        2
#define SSH_FILEXFER_TYPE_SYMLINK          3
#define SSH_FILEXFER_TYPE_SPECIAL          4
#define SSH_FILEXFER_TYPE_UNKNOWN          5

#define SSH_FXF_READ            0x00000001
#define SSH_FXF_WRITE           0x00000002
#define SSH_FXF_APPEND          0x00000004
#define SSH_FXF_CREAT           0x00000008
#define SSH_FXF_TRUNC           0x00000010
#define SSH_FXF_EXCL            0x00000020
#define SSH_FXF_TEXT            0x00000040

#define SSH_FXF_ACCESS_DISPOSITION        0x00000007
#define     SSH_FXF_CREATE_NEW            0x00000000
#define     SSH_FXF_CREATE_TRUNCATE       0x00000001
#define     SSH_FXF_OPEN_EXISTING         0x00000002
#define     SSH_FXF_OPEN_OR_CREATE        0x00000003
#define     SSH_FXF_TRUNCATE_EXISTING     0x00000004
#define SSH_FXF_ACCESS_APPEND_DATA        0x00000008
#define SSH_FXF_ACCESS_APPEND_DATA_ATOMIC 0x00000010
#define SSH_FXF_ACCESS_TEXT_MODE          0x00000020
#define SSH_FXF_ACCESS_READ_LOCK          0x00000040
#define SSH_FXF_ACCESS_WRITE_LOCK         0x00000080
#define SSH_FXF_ACCESS_DELETE_LOCK        0x00000100

#define ACE4_READ_DATA         0x00000001
#define ACE4_LIST_DIRECTORY    0x00000001
#define ACE4_WRITE_DATA        0x00000002
#define ACE4_ADD_FILE          0x00000002
#define ACE4_APPEND_DATA       0x00000004
#define ACE4_ADD_SUBDIRECTORY  0x00000004
#define ACE4_READ_NAMED_ATTRS  0x00000008
#define ACE4_WRITE_NAMED_ATTRS 0x00000010
#define ACE4_EXECUTE           0x00000020
#define ACE4_DELETE_CHILD      0x00000040
#define ACE4_READ_ATTRIBUTES   0x00000080
#define ACE4_WRITE_ATTRIBUTES  0x00000100
#define ACE4_DELETE            0x00010000
#define ACE4_READ_ACL          0x00020000
#define ACE4_WRITE_ACL         0x00040000
#define ACE4_WRITE_OWNER       0x00080000
#define ACE4_SYNCHRONIZE       0x00100000

#define SSH_FILEXFER_ATTR_FLAGS_HIDDEN           0x00000004

#define SFTP_MAX_PACKET_LEN   1024000
//---------------------------------------------------------------------------
#define SFTP_EXT_OWNER_GROUP "owner-group-query@generic-extensions"
#define SFTP_EXT_OWNER_GROUP_REPLY "owner-group-query-reply@generic-extensions"
#define SFTP_EXT_NEWLINE "newline"
#define SFTP_EXT_SUPPORTED "supported"
#define SFTP_EXT_SUPPORTED2 "supported2"
#define SFTP_EXT_FSROOTS "fs-roots@vandyke.com"
#define SFTP_EXT_VENDOR_ID "vendor-id"
#define SFTP_EXT_VERSIONS "versions"
#define SFTP_EXT_SPACE_AVAILABLE "space-available"
#define SFTP_EXT_CHECK_FILE "check-file"
#define SFTP_EXT_CHECK_FILE_NAME "check-file-name"
//---------------------------------------------------------------------------
#define OGQ_LIST_OWNERS 0x01
#define OGQ_LIST_GROUPS 0x02
//---------------------------------------------------------------------------
const int SFTPMinVersion = 0;
const int SFTPMaxVersion = 5;
const unsigned int SFTPNoMessageNumber = static_cast<unsigned int>(-1);

const int asNo =            0;
const int asOK =            1 << SSH_FX_OK;
const int asEOF =           1 << SSH_FX_EOF;
const int asPermDenied =    1 << SSH_FX_PERMISSION_DENIED;
const int asOpUnsupported = 1 << SSH_FX_OP_UNSUPPORTED;
const int asNoSuchFile =    1 << SSH_FX_NO_SUCH_FILE;
const int asAll = 0xFFFF;

#ifndef _MSC_VER
const int tfFirstLevel =   0x01;
const int tfNewDirectory = 0x02;
#endif
//---------------------------------------------------------------------------
#ifndef GET_32BIT
#define GET_32BIT(cp) \
    (((unsigned long)(unsigned char)(cp)[0] << 24) | \
    ((unsigned long)(unsigned char)(cp)[1] << 16) | \
    ((unsigned long)(unsigned char)(cp)[2] << 8) | \
    ((unsigned long)(unsigned char)(cp)[3]))
#endif
#ifndef PUT_32BIT
#define PUT_32BIT((cp), (value)) { \
    (cp)[0] = (unsigned char)((value) >> 24); \
    (cp)[1] = (unsigned char)((value) >> 16); \
    (cp)[2] = (unsigned char)((value) >> 8); \
    (cp)[3] = (unsigned char)(value); }
#endif
//---------------------------------------------------------------------------
#define SFTP_PACKET_ALLOC_DELTA 256
//---------------------------------------------------------------------------
#pragma warn -inl
//---------------------------------------------------------------------------
struct TSFTPSupport
{
  TSFTPSupport() :
    AttribExtensions(new TStringList()),
    Extensions(new TStringList())
  {
    Reset();
  }

  ~TSFTPSupport()
  {
    delete AttribExtensions;
    delete Extensions;
  }

  void Reset()
  {
    AttributeMask = 0;
    AttributeBits = 0;
    OpenFlags = 0;
    AccessMask = 0;
    MaxReadSize = 0;
    OpenBlockMasks = 0;
    BlockMasks = 0;
    AttribExtensions->Clear();
    Extensions->Clear();
    Loaded = false;
  }

  unsigned int AttributeMask;
  unsigned int AttributeBits;
  unsigned int OpenFlags;
  unsigned int AccessMask;
  unsigned int MaxReadSize;
  unsigned int OpenBlockMasks;
  unsigned int BlockMasks;
  TStrings * AttribExtensions;
  TStrings * Extensions;
  bool Loaded;
};
//---------------------------------------------------------------------------
class TSFTPPacket : public TObject
{
public:
  explicit TSFTPPacket(unsigned int codePage)
  {
    Init(codePage);
  }

  explicit TSFTPPacket(const TSFTPPacket & Source, unsigned int codePage)
  {
    Init(codePage);
    *this = Source;
  }

  explicit TSFTPPacket(unsigned char AType, unsigned int codePage)
  {
    Init(codePage);
    ChangeType(AType);
  }

  explicit TSFTPPacket(const unsigned char * Source, unsigned int Len, unsigned int codePage)
  {
    Init(codePage);
    FLength = Len;
    SetCapacity (FLength);
    memmove(GetData(), Source, Len);
  }

  explicit TSFTPPacket(const RawByteString & Source, unsigned int codePage)
  {
    Init(codePage);
    FLength = (uintptr_t)Source.Length();
    SetCapacity(FLength);
    memmove(GetData(), Source.c_str(), Source.Length());
  }

  ~TSFTPPacket()
  {
    if (FData != NULL)
    {
      delete[] (FData - FSendPrefixLen);
    }
    if (FReservedBy) { FReservedBy->UnreserveResponse(this); }
  }

  void ChangeType(unsigned char AType)
  {
    FPosition = 0;
    FLength = 0;
    SetCapacity (0);
    FType = AType;
    AddByte(FType);
    if ((FType != 1) && (FType != SSH_FXP_INIT))
    {
      AssignNumber();
      AddCardinal(FMessageNumber);
    }
  }

  void Reuse()
  {
    AssignNumber();

    assert(GetLength() >= 5);

    // duplicated in AddCardinal()
    unsigned char Buf[4];
    PUT_32BIT(Buf, FMessageNumber);

    memmove(FData + 1, Buf, sizeof(Buf));
  }

  void AddByte(unsigned char Value)
  {
    Add(&Value, sizeof(Value));
  }

  void AddCardinal(unsigned long Value)
  {
    // duplicated in Reuse()
    unsigned char Buf[4];
    PUT_32BIT(Buf, Value);
    Add(&Buf, sizeof(Buf));
  }

  void AddInt64(__int64 Value)
  {
    AddCardinal((unsigned long)(Value >> 32));
    AddCardinal((unsigned long)(Value & 0xFFFFFFFF));
  }

  void AddData(const void * Data, int ALength)
  {
    AddCardinal(ALength);
    Add(Data, ALength);
  }

  void AddStringW(const UnicodeString & ValueW)
  {
    AddString(W2MB(ValueW.c_str(), FCodePage).c_str());
  }

  void AddString(const RawByteString & Value)
  {
    AddCardinal(static_cast<unsigned long >(Value.Length()));
    Add(Value.c_str(), Value.Length());
  }

  inline void AddUtfString(const UTF8String & Value)
  {
    AddString(Value);
  }

  inline void AddUtfString(const UnicodeString & Value)
  {
    AddUtfString(UTF8String(Value));
  }

  inline void AddString(const UnicodeString & Value, bool Utf)
  {
	(void)Utf;
    AddStringW(Value);
  }

  // now purposeless alias to AddString
  inline void AddPathString(const UnicodeString & Value, bool Utf)
  {
    AddString(Value, Utf);
  }

  void AddProperties(unsigned short * Rights, TRemoteToken * Owner,
    TRemoteToken * Group, __int64 * MTime, __int64 * ATime,
    __int64 * Size, bool IsDirectory, int Version, bool Utf)
  {
    CALLSTACK;
    int Flags = 0;
    if (Size != NULL)
    {
      Flags |= SSH_FILEXFER_ATTR_SIZE;
    }
    // both or neither
    assert((Owner != NULL) == (Group != NULL));
    if ((Owner != NULL) && (Group != NULL))
    {
      if (Version < 4)
      {
        assert(Owner->GetIDValid() && Group->GetIDValid());
        Flags |= SSH_FILEXFER_ATTR_UIDGID;
      }
      else
      {
        assert(Owner->GetNameValid() && Group->GetNameValid());
        Flags |= SSH_FILEXFER_ATTR_OWNERGROUP;
      }
    }
    if (Rights != NULL)
    {
      Flags |= SSH_FILEXFER_ATTR_PERMISSIONS;
    }
    if ((Version < 4) && ((MTime != NULL) || (ATime != NULL)))
    {
      Flags |= SSH_FILEXFER_ATTR_ACMODTIME;
    }
    if ((Version >= 4) && (ATime != NULL))
    {
      Flags |= SSH_FILEXFER_ATTR_ACCESSTIME;
    }
    if ((Version >= 4) && (MTime != NULL))
    {
      Flags |= SSH_FILEXFER_ATTR_MODIFYTIME;
    }
    AddCardinal(Flags);

    if (Version >= 4)
    {
      AddByte(static_cast<unsigned char>(IsDirectory ?
        SSH_FILEXFER_TYPE_DIRECTORY : SSH_FILEXFER_TYPE_REGULAR));
    }

    if (Size != NULL)
    {
      AddInt64(*Size);
    }

    if ((Owner != NULL) && (Group != NULL))
    {
      if (Version < 4)
      {
        assert(Owner->GetIDValid() && Group->GetIDValid());
        AddCardinal(Owner->GetID());
        AddCardinal(Group->GetID());
      }
      else
      {
        assert(Owner->GetNameValid() && Group->GetNameValid());
        AddString(Owner->GetName(), Utf);
        AddString(Group->GetName(), Utf);
      }
    }

    if (Rights != NULL)
    {
      TRACEFMT("Rights [%x]", int(*Rights));
      AddCardinal(*Rights);
    }

    if ((Version < 4) && ((MTime != NULL) || (ATime != NULL)))
    {
      // any way to reflect sbSignedTS here?
      // (note that casting __int64 > 2^31 < 2^32 to unsigned long is wrapped,
      // thus we never can set time after 2038, even if the server supports it)
      AddCardinal(static_cast<unsigned long>(ATime != NULL ? *ATime : *MTime));
      AddCardinal(static_cast<unsigned long>(MTime != NULL ? *MTime : *ATime));
    }
    if ((Version >= 4) && (ATime != NULL))
    {
      AddInt64(*ATime);
    }
    if ((Version >= 4) && (MTime != NULL))
    {
      AddInt64(*MTime);
    }
    TRACE("/");
  }

  void AddProperties(const TRemoteProperties * Properties,
    unsigned short BaseRights, bool IsDirectory, int Version, bool Utf,
    TChmodSessionAction * Action)
  {
    CALLSTACK;
    enum TValid { valNone = 0, valRights = 0x01, valOwner = 0x02, valGroup = 0x04,
      valMTime = 0x08, valATime = 0x10 } Valid = valNone;
    unsigned short RightsNum = 0;
    TRemoteToken Owner;
    TRemoteToken Group;
    __int64 MTime;
    __int64 ATime;

    if (Properties != NULL)
    {
      if (Properties->Valid.Contains(vpGroup))
      {
        Valid = (TValid)(Valid | valGroup);
        Group = Properties->Group;
      }

      if (Properties->Valid.Contains(vpOwner))
      {
        Valid = (TValid)(Valid | valOwner);
        Owner = Properties->Owner;
      }

      if (Properties->Valid.Contains(vpRights))
      {
        Valid = (TValid)(Valid | valRights);
        TRights Rights = TRights(BaseRights);
        Rights |= Properties->Rights.GetNumberSet();
        Rights &= static_cast<unsigned short>(~Properties->Rights.GetNumberUnset());
        if (IsDirectory && Properties->AddXToDirectories)
        {
          Rights.AddExecute();
        }
        RightsNum = Rights;
        TRACEFMT("Rights [%x] [%x] [%x] [%d] [%x]", int(BaseRights), int(Properties->Rights.GetNumberSet()), int((unsigned short)~Properties->Rights.GetNumberUnset()), int(IsDirectory && Properties->AddXToDirectories), int(RightsNum));

        if (Action != NULL)
        {
          Action->Rights(Rights);
        }
      }

      if (Properties->Valid.Contains(vpLastAccess))
      {
        Valid = (TValid)(Valid | valATime);
        ATime = Properties->LastAccess;
      }

      if (Properties->Valid.Contains(vpModification))
      {
        Valid = (TValid)(Valid | valMTime);
        MTime = Properties->Modification;
      }
    }

    AddProperties(
      Valid & valRights ? &RightsNum : NULL,
      Valid & valOwner ? &Owner : NULL,
      Valid & valGroup ? &Group : NULL,
      Valid & valMTime ? &MTime : NULL,
      Valid & valATime ? &ATime : NULL,
      NULL, IsDirectory, Version, Utf);
    TRACE("/");
  }

  unsigned char GetByte()
  {
    Need(sizeof(unsigned char));
    unsigned char Result = FData[FPosition];
    FPosition++;
    return Result;
  }

  unsigned long GetCardinal()
  {
    unsigned long Result;
    Need(sizeof(Result));
    Result = GET_32BIT(FData + FPosition);
    FPosition += sizeof(Result);
    return Result;
  }

  unsigned long GetSmallCardinal()
  {
    unsigned long Result;
    Need(2);
    Result = (FData[FPosition] << 8) + FData[FPosition + 1];
    FPosition += 2;
    return Result;
  }

  __int64 GetInt64()
  {
    __int64 Hi = GetCardinal();
    __int64 Lo = GetCardinal();
    return (Hi << 32) + Lo;
  }

  RawByteString GetRawByteString()
  {
    RawByteString Result;
    unsigned long Len = GetCardinal();
    Need(Len);
    // cannot happen anyway as Need() would raise exception
    assert(Len < SFTP_MAX_PACKET_LEN);
    Result.SetLength(Len);
    memmove((void *)Result.c_str(), FData + FPosition, Len);
    FPosition += Len;
    CTRACEFMT(TRACE_STRINGS, "[%s]", Result.c_str());
    return Result;
  }

  inline UnicodeString GetUtfString()
  {
    return UnicodeString(UTF8String(GetRawByteString().c_str()));
  }

  // For reading string that are character strings (not byte strings as
  // as file handles), and SFTP spec does not say explicitly that they
  // are in UTF. For most of them it actually does not matter as
  // the content should be pure ASCII (e.g. extension names, etc.)
  inline UnicodeString GetAnsiString()
  {
    return UnicodeString(AnsiString(GetRawByteString().c_str()));
  }

  inline RawByteString GetFileHandle()
  {
    return GetRawByteString();
  }

  inline UnicodeString GetStringW()
  {
    return MB2W(GetRawByteString().c_str(), FCodePage);
  }

  inline UnicodeString GetString(bool Utf)
  {
	(void)Utf;
    return GetStringW();
  }

  // now purposeless alias to GetString(bool)
  inline UnicodeString GetPathString(bool Utf)
  {
    return GetString(Utf);
  }

  void GetFile(TRemoteFile * File, int Version, TDSTMode DSTMode, bool Utf, bool SignedTS, bool Complete)
  {
    CALLSTACK;
    assert(File);
    unsigned int Flags;
    UnicodeString ListingStr;
    unsigned long Permissions = 0;
    bool ParsingFailed = false;
    if (GetType() != SSH_FXP_ATTRS)
    {
      File->SetFileName(GetPathString(Utf));
      TRACEFMT("1 [%s]", File->GetFileName().c_str());
      if (Version < 4)
      {
        ListingStr = GetAnsiString();
        TRACEFMT("2 [%s]", File->GetFileName().c_str());
      }
    }
    Flags = GetCardinal();
    if (Version >= 4)
    {
      unsigned char FXType = GetByte();
      // -:regular, D:directory, L:symlink, S:special, U:unknown
      // O:socket, C:char devide, B:block device, F:fifo

      // SSH-2.0-cryptlib returns file type 0 in response to SSH_FXP_LSTAT,
      // handle this undefined value as "unknown"
      static wchar_t * Types = L"U-DLSUOCBF";
      if (FXType > (unsigned char)wcslen(Types))
      {
        throw Exception(FMTLOAD(SFTP_UNKNOWN_FILE_TYPE, static_cast<int>(FXType)));
      }
      File->SetType(Types[FXType]);
    }
    if (Flags & SSH_FILEXFER_ATTR_SIZE)
    {
      File->SetSize(GetInt64());
    }
    // SSH-2.0-3.2.0 F-SECURE SSH - Process Software MultiNet
    // sets SSH_FILEXFER_ATTR_UIDGID for v4, but does not include the UID/GUID
    if ((Flags & SSH_FILEXFER_ATTR_UIDGID) && (Version < 4))
    {
      File->GetFileOwner().SetID(GetCardinal());
      File->GetFileGroup().SetID(GetCardinal());
    }
    if (Flags & SSH_FILEXFER_ATTR_OWNERGROUP)
    {
      assert(Version >= 4);
      File->GetFileOwner().SetName(GetString(Utf));
      File->GetFileGroup().SetName(GetString(Utf));
    }
    if (Flags & SSH_FILEXFER_ATTR_PERMISSIONS)
    {
      Permissions = GetCardinal();
    }
    if (Version < 4)
    {
      if (Flags & SSH_FILEXFER_ATTR_ACMODTIME)
      {
        File->SetLastAccess(UnixToDateTime(
          SignedTS ?
            static_cast<__int64>(static_cast<signed long>(GetCardinal())) :
            static_cast<__int64>(GetCardinal()),
          DSTMode));
        File->SetModification(UnixToDateTime(
          SignedTS ?
            static_cast<__int64>(static_cast<signed long>(GetCardinal())) :
            static_cast<__int64>(GetCardinal()),
          DSTMode));
      }
    }
    else
    {
      if (Flags & SSH_FILEXFER_ATTR_ACCESSTIME)
      {
        File->SetLastAccess(UnixToDateTime(GetInt64(), DSTMode));
        if (Flags & SSH_FILEXFER_ATTR_SUBSECOND_TIMES)
        {
          GetCardinal(); // skip access time subseconds
        }
      }
      else
      {
        File->SetLastAccess(Now());
      }
      if (Flags & SSH_FILEXFER_ATTR_CREATETIME)
      {
        GetInt64(); // skip create time
        if (Flags & SSH_FILEXFER_ATTR_SUBSECOND_TIMES)
        {
          GetCardinal(); // skip create time subseconds
        }
      }
      if (Flags & SSH_FILEXFER_ATTR_MODIFYTIME)
      {
        File->SetModification(UnixToDateTime(GetInt64(), DSTMode));
        if (Flags & SSH_FILEXFER_ATTR_SUBSECOND_TIMES)
        {
          GetCardinal(); // skip modification time subseconds
        }
      }
      else
      {
        File->SetModification(Now());
      }
    }

    if (Flags & SSH_FILEXFER_ATTR_ACL)
    {
      GetRawByteString();
    }

    if (Flags & SSH_FILEXFER_ATTR_BITS)
    {
      // while SSH_FILEXFER_ATTR_BITS is defined for SFTP5 only, vandyke 2.3.3 sets it
      // for SFTP4 as well
      unsigned long Bits = GetCardinal();
      if (FLAGSET(Bits, SSH_FILEXFER_ATTR_FLAGS_HIDDEN))
      {
        File->SetIsHidden(true);
      }
    }

    if ((Version < 4) && (GetType() != SSH_FXP_ATTRS))
    {
      try
      {
        // update permissions and user/group name
        // modification time and filename is ignored
        File->SetListingStr(ListingStr);
      }
      catch(...)
      {
        // ignore any error while parsing listing line,
        // SFTP specification do not recommend to parse it
        ParsingFailed = true;
      }
    }

    if (GetType() == SSH_FXP_ATTRS || Version >= 4 || ParsingFailed)
    {
      wchar_t Type = L'-';
      if (FLAGSET(Flags, SSH_FILEXFER_ATTR_PERMISSIONS))
      {
        File->GetRights()->SetNumber(static_cast<unsigned short>(Permissions & TRights::rfAllSpecials));
        if (FLAGSET(Permissions, TRights::rfDirectory))
        {
          Type = FILETYPE_DIRECTORY;
        }
      }

      if (Version < 4)
      {
        File->SetType(Type);
      }
    }

    if (Flags & SSH_FILEXFER_ATTR_EXTENDED)
    {
      unsigned int ExtendedCount = GetCardinal();
      for (unsigned int Index = 0; Index < ExtendedCount; Index++)
      {
        GetRawByteString(); // skip extended_type
        GetRawByteString(); // skip extended_data
      }
    }

    if (Complete)
    {
      File->Complete();
    }
  }

  unsigned char * GetNextData(uintptr_t Size = 0)
  {
    if (Size > 0)
    {
      Need(Size);
    }
    return FPosition < FLength ? FData + FPosition : NULL;
  }

  void DataUpdated(uintptr_t ALength)
  {
    FPosition = 0;
    FLength = ALength;
    FType = GetByte();
    if (FType != SSH_FXP_VERSION)
    {
      FMessageNumber = GetCardinal();
    }
    else
    {
      FMessageNumber = SFTPNoMessageNumber;
    }
  }

  void LoadFromFile(const UnicodeString & FileName)
  {
    TStringList * DumpLines = new TStringList();
    RawByteString Dump;
    TRY_FINALLY (
    {
      DumpLines->LoadFromFile(FileName);
      Dump = AnsiString(DumpLines->Text);
    }
    ,
    {
      delete DumpLines;
    }
    );

    SetCapacity(1 * 1024 * 1024); // 20480);
    unsigned char Byte[3];
    memset(Byte, '\0', sizeof(Byte));
    int Index = 1;
    unsigned int Length = 0;
    while (Index < Dump.Length())
    {
      char C = Dump[Index];
      if (((C >= '0') && (C <= '9')) || ((C >= 'A') && (C <= 'Z')))
      {
        if (Byte[0] == '\0')
        {
          Byte[0] = C;
        }
        else
        {
          Byte[1] = C;
          assert(Length < GetCapacity());
          GetData()[Length] = HexToByte(UnicodeString(reinterpret_cast<char *>(Byte)));
          Length++;
          memset(Byte, '\0', sizeof(Byte));
        }
      }
      Index++;
    }
    DataUpdated(Length);
  }

  UnicodeString Dump() const
  {
    UnicodeString Result;
    for (unsigned int Index = 0; Index < GetLength(); Index++)
    {
      Result += ByteToHex(GetData()[Index]) + L",";
      if (((Index + 1) % 25) == 0)
      {
        Result += L"\n";
      }
    }
    return Result;
  }

  TSFTPPacket & operator = (const TSFTPPacket & Source)
  {
    SetCapacity(0);
    Add(Source.GetData(), Source.GetLength());
    DataUpdated(Source.GetLength());
    FPosition = Source.FPosition;
    return *this;
  }

  uintptr_t GetLength() const { return FLength; }
  unsigned char * GetData() const { return FData; }
  uintptr_t GetCapacity() const { return FCapacity; }
  unsigned char GetType() const { return FType; }
  uintptr_t GetMessageNumber() const { return static_cast<uintptr_t>(FMessageNumber); }
  void SetMessageNumber(unsigned long  value) { FMessageNumber = value; }
  TSFTPFileSystem * GetReservedBy() const { return FReservedBy; }
  void SetReservedBy(TSFTPFileSystem * value) { FReservedBy = value; }

private:
  unsigned char * FData;
  uintptr_t FLength;
  uintptr_t FCapacity;
  uintptr_t FPosition;
  unsigned char FType;
  unsigned long FMessageNumber;
  TSFTPFileSystem * FReservedBy;

  static int FMessageCounter;
  static const int FSendPrefixLen = 4;
  unsigned int FCodePage;

  void Init(unsigned int codePage)
  {
    FData = NULL;
    FCapacity = 0;
    FLength = 0;
    FPosition = 0;
    FMessageNumber = SFTPNoMessageNumber;
    FType = (unsigned char)-1;
    FReservedBy = NULL;
    FCodePage = codePage;
  }

  void AssignNumber()
  {
    // this is not strictly thread-safe, but as it is accessed from multiple
    // threads only for multiple connection, it is not problem if two threads get
    // the same number
    FMessageNumber = (FMessageCounter << 8) + FType;
    FMessageCounter++;
  }

public:
  unsigned char GetRequestType()
  {
    if (FMessageNumber != SFTPNoMessageNumber)
    {
      return static_cast<unsigned char>(FMessageNumber & 0xFF);
    }
    else
    {
      assert(GetType() == SSH_FXP_VERSION);
      return SSH_FXP_INIT;
    }
  }

  /* inline */ void Add(const void * AData, intptr_t ALength)
  {
    if (GetLength() + ALength > GetCapacity())
    {
      SetCapacity(GetLength() + ALength + SFTP_PACKET_ALLOC_DELTA);
    }
    memmove(FData + GetLength(), AData, ALength);
    FLength += ALength;
  }

  void SetCapacity(uintptr_t ACapacity)
  {
    if (ACapacity != GetCapacity())
    {
      FCapacity = ACapacity;
      if (FCapacity > 0)
      {
        unsigned char * NData = new unsigned char[FCapacity + FSendPrefixLen];
        NData += FSendPrefixLen;
        if (FData)
        {
          memmove(NData - FSendPrefixLen, FData - FSendPrefixLen,
            (FLength < FCapacity ? FLength : FCapacity) + FSendPrefixLen);
          delete[] (FData - FSendPrefixLen);
        }
        FData = NData;
      }
      else
      {
        if (FData) { delete[] (FData - FSendPrefixLen); }
        FData = NULL;
      }
      if (FLength > FCapacity) { FLength = FCapacity; }
    }
  }

  UnicodeString GetTypeName() const
  {
    #define TYPE_CASE(TYPE) case TYPE: return TEXT(#TYPE)
    switch (GetType()) {
      TYPE_CASE(SSH_FXP_INIT);
      TYPE_CASE(SSH_FXP_VERSION);
      TYPE_CASE(SSH_FXP_OPEN);
      TYPE_CASE(SSH_FXP_CLOSE);
      TYPE_CASE(SSH_FXP_READ);
      TYPE_CASE(SSH_FXP_WRITE);
      TYPE_CASE(SSH_FXP_LSTAT);
      TYPE_CASE(SSH_FXP_FSTAT);
      TYPE_CASE(SSH_FXP_SETSTAT);
      TYPE_CASE(SSH_FXP_FSETSTAT);
      TYPE_CASE(SSH_FXP_OPENDIR);
      TYPE_CASE(SSH_FXP_READDIR);
      TYPE_CASE(SSH_FXP_REMOVE);
      TYPE_CASE(SSH_FXP_MKDIR);
      TYPE_CASE(SSH_FXP_RMDIR);
      TYPE_CASE(SSH_FXP_REALPATH);
      TYPE_CASE(SSH_FXP_STAT);
      TYPE_CASE(SSH_FXP_RENAME);
      TYPE_CASE(SSH_FXP_READLINK);
      TYPE_CASE(SSH_FXP_SYMLINK);
      TYPE_CASE(SSH_FXP_STATUS);
      TYPE_CASE(SSH_FXP_HANDLE);
      TYPE_CASE(SSH_FXP_DATA);
      TYPE_CASE(SSH_FXP_NAME);
      TYPE_CASE(SSH_FXP_ATTRS);
      TYPE_CASE(SSH_FXP_EXTENDED);
      TYPE_CASE(SSH_FXP_EXTENDED_REPLY);
      default:
        return FORMAT(L"Unknown message (%d)", static_cast<int>(GetType()));
    }
  }

  unsigned char * GetSendData() const
  {
    unsigned char * Result = FData - FSendPrefixLen;
    // this is not strictly const-object operation
    PUT_32BIT(Result, GetLength());
    return Result;
  }

  uintptr_t GetSendLength() const
  {
    return FSendPrefixLen + GetLength();
  }

  uintptr_t GetRemainingLength() const
  {
    return GetLength() - FPosition;
  }

private:
  inline void Need(uintptr_t Size)
  {
    if (FPosition + Size > FLength)
    {
      throw Exception(FMTLOAD(SFTP_PACKET_ERROR, static_cast<int>(FPosition), static_cast<int>(Size), static_cast<int>(FLength)));
    }
  }
};
//---------------------------------------------------------------------------
int TSFTPPacket::FMessageCounter = 0;
//---------------------------------------------------------------------------
class TSFTPQueue
{
public:
  explicit TSFTPQueue(TSFTPFileSystem * AFileSystem, unsigned int codePage)
  {
    FFileSystem = AFileSystem;
    assert(FFileSystem);
    FRequests = new TList();
    FResponses = new TList();
    FCodePage = codePage;
  }

  virtual ~TSFTPQueue()
  {
    CALLSTACK;
    TRACE("4");
    TSFTPQueuePacket * Request;
    TSFTPPacket * Response;

    assert(FResponses->Count == FRequests->Count);
    for (int Index = 0; Index < FRequests->Count; Index++)
    {
      Request = static_cast<TSFTPQueuePacket*>(FRequests->Items[Index]);
      assert(Request);
      delete Request;

      Response = static_cast<TSFTPPacket*>(FResponses->Items[Index]);
      assert(Response);
      delete Response;
    }
    delete FRequests;
    delete FResponses;
    TRACE("5");
  }

  bool Init()
  {
    return SendRequests();
  }

  virtual void Dispose()
  {
    CALLSTACK;
    assert(FFileSystem->FTerminal->GetActive());

    TSFTPQueuePacket * Request;
    TSFTPPacket * Response;

    while (FRequests->Count)
    {
      assert(FResponses->Count);

      Request = static_cast<TSFTPQueuePacket*>(FRequests->Items[0]);
      assert(Request);

      Response = static_cast<TSFTPPacket*>(FResponses->Items[0]);
      assert(Response);

      try
      {
        FFileSystem->ReceiveResponse(Request, Response);
      }
      catch(Exception & E)
      {
        if (FFileSystem->FTerminal->GetActive())
        {
          FFileSystem->FTerminal->LogEvent(L"Error while disposing the SFTP queue.");
          FFileSystem->FTerminal->GetLog()->AddException(&E);
        }
        else
        {
          FFileSystem->FTerminal->LogEvent(L"Fatal error while disposing the SFTP queue.");
          throw;
        }
      }

      FRequests->Delete(0);
      delete Request;
      FResponses->Delete(0);
      delete Response;
    }
  }

  void DisposeSafe()
  {
    CALLSTACK;
    if (FFileSystem->FTerminal->GetActive())
    {
      TRACE("2");
      Dispose();
      TRACE("3");
    }
  }

  bool ReceivePacket(TSFTPPacket * Packet,
    int ExpectedType = -1, int AllowStatus = -1, void ** Token = NULL)
  {
    assert(FRequests->Count);
    bool Result;
    TSFTPQueuePacket * Request = NULL;
    TSFTPPacket * Response = NULL;
    TRY_FINALLY (
    {
      Request = static_cast<TSFTPQueuePacket*>(FRequests->Items[0]);
      FRequests->Delete(0);
      assert(Request);
      if (Token != NULL)
      {
        *Token = Request->Token;
      }

      Response = static_cast<TSFTPPacket*>(FResponses->Items[0]);
      FResponses->Delete(0);
      assert(Response);

      FFileSystem->ReceiveResponse(Request, Response,
        ExpectedType, AllowStatus);

      if (Packet)
      {
        *Packet = *Response;
      }

      Result = !End(Response);
      if (Result)
      {
        SendRequests();
      }
    }
    ,
    {
      delete Request;
      delete Response;
    }
    );

    return Result;
  }

  bool Next(int ExpectedType = -1, int AllowStatus = -1)
  {
    return ReceivePacket(NULL, ExpectedType, AllowStatus);
  }

protected:
  TList * FRequests;
  TList * FResponses;
  TSFTPFileSystem * FFileSystem;
  unsigned int FCodePage;

  class TSFTPQueuePacket : public TSFTPPacket
  {
  public:
    explicit TSFTPQueuePacket(unsigned int codePage) :
      TSFTPPacket(codePage)
    {
      Token = NULL;
    }

    void * Token;
  };

  virtual bool InitRequest(TSFTPQueuePacket * Request) = 0;

  virtual bool End(TSFTPPacket * Response) = 0;

  virtual void SendPacket(TSFTPQueuePacket * Packet)
  {
    FFileSystem->SendPacket(Packet);
  }

  // sends as many requests as allowed by implementation
  virtual bool SendRequests() = 0;

  virtual bool SendRequest()
  {
    TSFTPQueuePacket * Request = NULL;
    try
    {
      Request = new TSFTPQueuePacket(FCodePage);
      if (!InitRequest(Request))
      {
        delete Request;
        Request = NULL;
      }
    }
    catch(...)
    {
      delete Request;
      throw;
    }

    if (Request != NULL)
    {
      TSFTPPacket * Response = new TSFTPPacket(FCodePage);
      FRequests->Add(Request);
      FResponses->Add(Response);

      // make sure the response is reserved before actually ending the message
      // as we may receive response asynchronously before SendPacket finishes
      FFileSystem->ReserveResponse(Request, Response);
      SendPacket(Request);
    }

    return (Request != NULL);
  }
};
//---------------------------------------------------------------------------
class TSFTPFixedLenQueue : public TSFTPQueue
{
public:
  explicit TSFTPFixedLenQueue(TSFTPFileSystem * AFileSystem, unsigned int codePage) : TSFTPQueue(AFileSystem, codePage)
  {
    FMissedRequests = 0;
  }
  virtual ~TSFTPFixedLenQueue() {}

  bool Init(int QueueLen)
  {
    FMissedRequests = QueueLen - 1;
    return TSFTPQueue::Init();
  }

protected:
  int FMissedRequests;

  // sends as many requests as allowed by implementation
  virtual bool SendRequests()
  {
    bool Result = false;
    FMissedRequests++;
    while ((FMissedRequests > 0) && SendRequest())
    {
      Result = true;
      FMissedRequests--;
    }
    return Result;
  }
};
//---------------------------------------------------------------------------
class TSFTPAsynchronousQueue : public TSFTPQueue
{
public:
  explicit TSFTPAsynchronousQueue(TSFTPFileSystem * AFileSystem, unsigned int codePage) : TSFTPQueue(AFileSystem, codePage)
  {
    FFileSystem->FSecureShell->RegisterReceiveHandler(MAKE_CALLBACK(TSFTPAsynchronousQueue::ReceiveHandler, this));
    FReceiveHandlerRegistered = true;
  }

  virtual ~TSFTPAsynchronousQueue()
  {
    CALLSTACK;
    UnregisterReceiveHandler();
  }

  virtual void Dispose()
  {
    // we do not want to receive asynchronous notifications anymore,
    // while waiting synchronously for pending responses
    UnregisterReceiveHandler();
    TSFTPQueue::Dispose();
  }

  bool Continue()
  {
    return SendRequest();
  }

protected:

  // event handler for incoming data
  void ReceiveHandler(TObject * /*Sender*/)
  {
    while (FFileSystem->PeekPacket() && ReceivePacketAsynchronously())
    {
      // loop
    }
  }

  virtual bool ReceivePacketAsynchronously() = 0;

  // sends as many requests as allowed by implementation
  virtual bool SendRequests()
  {
    // noop
    return true;
  }

  void UnregisterReceiveHandler()
  {
    if (FReceiveHandlerRegistered)
    {
      FReceiveHandlerRegistered = false;
      FFileSystem->FSecureShell->UnregisterReceiveHandler(MAKE_CALLBACK(TSFTPAsynchronousQueue::ReceiveHandler, this));
    }
  }

private:
  bool FReceiveHandlerRegistered;
};
//---------------------------------------------------------------------------
class TSFTPDownloadQueue : public TSFTPFixedLenQueue
{
public:
  explicit TSFTPDownloadQueue(TSFTPFileSystem * AFileSystem, unsigned int codePage) :
    TSFTPFixedLenQueue(AFileSystem, codePage)
  {
  }
  virtual ~TSFTPDownloadQueue() {}

  bool Init(int QueueLen, const RawByteString & AHandle,__int64 ATransfered,
    TFileOperationProgressType * AOperationProgress)
  {
    FHandle = AHandle;
    FTransfered = ATransfered;
    OperationProgress = AOperationProgress;

    return TSFTPFixedLenQueue::Init(QueueLen);
  }

  void InitFillGapRequest(__int64 Offset, unsigned long Missing,
    TSFTPPacket * Packet)
  {
    InitRequest(Packet, Offset, Missing);
  }

  bool ReceivePacket(TSFTPPacket * Packet, unsigned long & BlockSize)
  {
    void * Token;
    bool Result = TSFTPFixedLenQueue::ReceivePacket(Packet, SSH_FXP_DATA, asEOF, &Token);
    BlockSize = reinterpret_cast<unsigned long>(Token);
    return Result;
  }

protected:
  virtual bool InitRequest(TSFTPQueuePacket * Request)
  {
    unsigned int BlockSize = FFileSystem->DownloadBlockSize(OperationProgress);
    InitRequest(Request, FTransfered, BlockSize);
    Request->Token = reinterpret_cast<void*>(BlockSize);
    FTransfered += BlockSize;
    return true;
  }

  void InitRequest(TSFTPPacket * Request, __int64 Offset,
    unsigned long Size)
  {
    Request->ChangeType(SSH_FXP_READ);
    Request->AddString(FHandle);
    Request->AddInt64(Offset);
    Request->AddCardinal(Size);
  }

  virtual bool End(TSFTPPacket * Response)
  {
    return (Response->GetType() != SSH_FXP_DATA);
  }

private:
  TFileOperationProgressType * OperationProgress;
  __int64 FTransfered;
  RawByteString FHandle;
};
//---------------------------------------------------------------------------
class TSFTPUploadQueue : public TSFTPAsynchronousQueue
{
public:
  explicit TSFTPUploadQueue(TSFTPFileSystem * AFileSystem, unsigned int codePage) :
    TSFTPAsynchronousQueue(AFileSystem, codePage)
  {
    FStream = NULL;
    OperationProgress = NULL;
    FLastBlockSize = 0;
    FEnd = false;
    FConvertToken = false;
  }

  virtual ~TSFTPUploadQueue()
  {
    CALLSTACK;
    delete FStream;
  }

  bool Init(const UnicodeString & AFileName,
    HANDLE AFile, TFileOperationProgressType * AOperationProgress,
    const RawByteString AHandle, __int64 ATransfered)
  {
    FFileName = AFileName;
    FStream = new TSafeHandleStream(AFile);
    OperationProgress = AOperationProgress;
    FHandle = AHandle;
    FTransfered = ATransfered;

    return TSFTPAsynchronousQueue::Init();
  }

protected:
  virtual bool InitRequest(TSFTPQueuePacket * Request)
  {
    FTerminal = FFileSystem->FTerminal;
    // Buffer for one block of data
    TFileBuffer BlockBuf;

    unsigned long BlockSize = GetBlockSize();
    bool Result = (BlockSize > 0);

    if (Result)
    {
      FILE_OPERATION_LOOP(FMTLOAD(READ_ERROR, FFileName.c_str()),
        BlockBuf.LoadStream(FStream, BlockSize, false);
      );

      FEnd = (BlockBuf.GetSize() == 0);
      Result = !FEnd;
      if (Result)
      {
        OperationProgress->AddLocallyUsed(BlockBuf.GetSize());

        // We do ASCII transfer: convert EOL of current block
        if (OperationProgress->AsciiTransfer)
        {
          __int64 PrevBufSize = BlockBuf.GetSize();
          BlockBuf.Convert(FTerminal->GetConfiguration()->GetLocalEOLType(),
            FFileSystem->GetEOL(), cpRemoveCtrlZ | cpRemoveBOM, FConvertToken);
          // update transfer size with difference arised from EOL conversion
          OperationProgress->ChangeTransferSize(OperationProgress->TransferSize -
            PrevBufSize + BlockBuf.GetSize());
        }

        if (FFileSystem->FTerminal->GetConfiguration()->GetActualLogProtocol() >= 1)
        {
          FFileSystem->FTerminal->LogEvent(FORMAT(L"Write request offset: %d, len: %d",
            int(FTransfered), int(BlockBuf.GetSize())));
        }

        Request->ChangeType(SSH_FXP_WRITE);
        Request->AddString(FHandle);
        Request->AddInt64(FTransfered);
        Request->AddData(BlockBuf.GetData(), (int)BlockBuf.GetSize());
        FLastBlockSize = (unsigned long)BlockBuf.GetSize();

        FTransfered += BlockBuf.GetSize();
      }
    }

    FTerminal = NULL;
    return Result;
  }

  virtual void SendPacket(TSFTPQueuePacket * Packet)
  {
    TSFTPAsynchronousQueue::SendPacket(Packet);
    OperationProgress->AddTransfered(FLastBlockSize);
  }

  virtual bool ReceivePacketAsynchronously()
  {
    // do not read response to close request
    bool Result = (FRequests->Count > 0);
    if (Result)
    {
      ReceivePacket(NULL, SSH_FXP_STATUS);
    }
    return Result;
  }

  inline int GetBlockSize()
  {
    return FFileSystem->UploadBlockSize(FHandle, OperationProgress);
  }

  virtual bool End(TSFTPPacket * /*Response*/)
  {
    return FEnd;
  }

private:
  TStream * FStream;
  TFileOperationProgressType * OperationProgress;
  UnicodeString FFileName;
  unsigned long FLastBlockSize;
  bool FEnd;
  __int64 FTransfered;
  RawByteString FHandle;
  bool FConvertToken;
  TTerminalIntf * FTerminal;
};
//---------------------------------------------------------------------------
class TSFTPLoadFilesPropertiesQueue : public TSFTPFixedLenQueue
{
public:
  explicit TSFTPLoadFilesPropertiesQueue(TSFTPFileSystem * AFileSystem, unsigned int codePage) :
    TSFTPFixedLenQueue(AFileSystem, codePage)
  {
    FIndex = 0;
  }
  virtual ~TSFTPLoadFilesPropertiesQueue() {}

  bool Init(int QueueLen, TStrings * FileList)
  {
    FFileList = FileList;

    return TSFTPFixedLenQueue::Init(QueueLen);
  }

  bool ReceivePacket(TSFTPPacket * Packet, TRemoteFile *& File)
  {
    void * Token;
    bool Result = TSFTPFixedLenQueue::ReceivePacket(Packet, SSH_FXP_ATTRS, asAll, &Token);
    File = static_cast<TRemoteFile *>(Token);
    return Result;
  }

protected:
  virtual bool InitRequest(TSFTPQueuePacket * Request)
  {
    bool Result = false;
    while (!Result && (FIndex < FFileList->Count))
    {
      TRemoteFile * File = static_cast<TRemoteFile *>(FFileList->Objects[FIndex]);
      FIndex++;

      bool MissingRights =
        (FLAGSET(FFileSystem->FSupport->AttributeMask, SSH_FILEXFER_ATTR_PERMISSIONS) &&
           File->GetRights()->GetUnknown());
      bool MissingOwnerGroup =
        (FLAGSET(FFileSystem->FSupport->AttributeMask, SSH_FILEXFER_ATTR_OWNERGROUP) &&
           !File->GetFileOwner().GetIsSet() || !File->GetFileGroup().GetIsSet());

      Result = (MissingRights || MissingOwnerGroup);
      if (Result)
      {
        Request->ChangeType(SSH_FXP_LSTAT);
        Request->AddPathString(FFileSystem->LocalCanonify(File->GetFileName()),
          FFileSystem->FUtfStrings);
        if (FFileSystem->FVersion >= 4)
        {
          Request->AddCardinal(
            FLAGMASK(MissingRights, SSH_FILEXFER_ATTR_PERMISSIONS) |
            FLAGMASK(MissingOwnerGroup, SSH_FILEXFER_ATTR_OWNERGROUP));
        }
        Request->Token = File;
      }
    }

    return Result;
  }

  virtual bool SendRequest()
  {
    bool Result =
      (FIndex < FFileList->Count) &&
      TSFTPFixedLenQueue::SendRequest();
    return Result;
  }

  virtual bool End(TSFTPPacket * /*Response*/)
  {
    return (FRequests->Count == 0);
  }

private:
  TStrings * FFileList;
  int FIndex;
};
//---------------------------------------------------------------------------
class TSFTPCalculateFilesChecksumQueue : public TSFTPFixedLenQueue
{
public:
  explicit TSFTPCalculateFilesChecksumQueue(TSFTPFileSystem * AFileSystem, unsigned int codePage) :
    TSFTPFixedLenQueue(AFileSystem, codePage)
  {
    FIndex = 0;
  }
  virtual ~TSFTPCalculateFilesChecksumQueue() {}

  bool Init(int QueueLen, const UnicodeString & Alg, TStrings * FileList)
  {
    FAlg = Alg;
    FFileList = FileList;

    return TSFTPFixedLenQueue::Init(QueueLen);
  }

  bool ReceivePacket(TSFTPPacket * Packet, TRemoteFile *& File)
  {
    void * Token;
    bool Result;
    TRY_FINALLY (
    {
      Result = TSFTPFixedLenQueue::ReceivePacket(Packet, SSH_FXP_EXTENDED_REPLY, asNo, &Token);
    }
    ,
    {
      File = static_cast<TRemoteFile *>(Token);
    }
    );
    return Result;
  }

protected:
  virtual bool InitRequest(TSFTPQueuePacket * Request)
  {
    bool Result = false;
    while (!Result && (FIndex < FFileList->Count))
    {
      TRemoteFile * File = static_cast<TRemoteFile *>(FFileList->Objects[FIndex]);
      assert(File != NULL);
      FIndex++;

      Result = !File->GetIsDirectory();
      if (Result)
      {
        assert(!File->GetIsParentDirectory() && !File->GetIsThisDirectory());

        Request->ChangeType(SSH_FXP_EXTENDED);
        Request->AddString(SFTP_EXT_CHECK_FILE_NAME);
        Request->AddPathString(FFileSystem->LocalCanonify(File->GetFullFileName()),
          FFileSystem->FUtfStrings);
        Request->AddString(FAlg);
        Request->AddInt64(0); // offset
        Request->AddInt64(0); // length (0 = till end)
        Request->AddCardinal(0); // block size (0 = no blocks or "one block")

        Request->Token = File;
      }
    }

    return Result;
  }

  virtual bool SendRequest()
  {
    bool Result =
      (FIndex < FFileList->Count) &&
      TSFTPFixedLenQueue::SendRequest();
    return Result;
  }

  virtual bool End(TSFTPPacket * /*Response*/)
  {
    return (FRequests->Count == 0);
  }

private:
  UnicodeString FAlg;
  TStrings * FFileList;
  int FIndex;
};
//---------------------------------------------------------------------------
#pragma warn .inl
//---------------------------------------------------------------------------
class TSFTPBusy
{
public:
  explicit TSFTPBusy(TSFTPFileSystem * FileSystem)
  {
    FFileSystem = FileSystem;
    assert(FFileSystem != NULL);
    FFileSystem->BusyStart();
  }

  ~TSFTPBusy()
  {
    FFileSystem->BusyEnd();
  }

private:
  TSFTPFileSystem * FFileSystem;
};
//===========================================================================
#ifndef _MSC_VER
struct TOpenRemoteFileParams
{
  int LocalFileAttrs;
  UnicodeString RemoteFileName;
  TFileOperationProgressType * OperationProgress;
  const TCopyParamType * CopyParam;
  int Params;
  bool Resume;
  bool Resuming;
  TOverwriteMode OverwriteMode;
  __int64 DestFileSize; // output
  RawByteString RemoteFileHandle; // output
  TOverwriteFileParams * FileParams;
  bool Confirmed;
};
//---------------------------------------------------------------------------
struct TSinkFileParams
{
  UnicodeString TargetDir;
  const TCopyParamType * CopyParam;
  int Params;
  TFileOperationProgressType * OperationProgress;
  bool Skipped;
  unsigned int Flags;
};
#endif
//===========================================================================
TSFTPFileSystem::TSFTPFileSystem(TTerminal * ATerminal) :
  TCustomFileSystem(ATerminal)
{
}

void TSFTPFileSystem::Init(void * Data)
{
  FSecureShell = reinterpret_cast<TSecureShell *>(Data);
  assert(FSecureShell);
  FFileSystemInfoValid = false;
  FVersion = NPOS;
  FPacketReservations = new TList();
#ifndef _MSC_VER
  FPacketNumbers = VarArrayCreate(OPENARRAY(int, (0, 1)), varLongWord);
#else
  FPacketNumbers.clear();
#endif
  FPreviousLoggedPacket = 0;
  FNotLoggedPackets = 0;
  FBusy = 0;
  FAvoidBusy = false;
  FUtfStrings = false;
  FUtfNever = false;
  FSignedTS = false;
  FSupport = new TSFTPSupport();
  FExtensions = new TStringList();
  FFixedPaths = NULL;
  FFileSystemInfoValid = false;
}
//---------------------------------------------------------------------------
TSFTPFileSystem::~TSFTPFileSystem()
{
  delete FSupport;
  ResetConnection();
  delete FPacketReservations;
  delete FExtensions;
  delete FFixedPaths;
  delete FSecureShell;
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::Open()
{
  CALLSTACK;
  ResetConnection();
  FSecureShell->Open();
  TRACE("/");
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::Close()
{
  CALLSTACK;
  FSecureShell->Close();
  TRACE("/");
}
//---------------------------------------------------------------------------
bool TSFTPFileSystem::GetActive()
{
  return FSecureShell->GetActive();
}
//---------------------------------------------------------------------------
const TSessionInfo & TSFTPFileSystem::GetSessionInfo()
{
  return FSecureShell->GetSessionInfo();
}
//---------------------------------------------------------------------------
const TSessionDataIntf * TSFTPFileSystem::GetSessionData() const
{
  return FTerminal->GetSessionData();
}
//---------------------------------------------------------------------------
const TFileSystemInfo & TSFTPFileSystem::GetFileSystemInfo(bool /*Retrieve*/)
{
  if (!FFileSystemInfoValid)
  {
    FFileSystemInfo.AdditionalInfo = L"";

    if (!IsCapable(fcRename))
    {
      FFileSystemInfo.AdditionalInfo += LoadStr(FS_RENAME_NOT_SUPPORTED) + L"\r\n\r\n";
    }

    if (FExtensions->Count > 0)
    {
      UnicodeString Name;
      UnicodeString Value;
      UnicodeString Line;
      FFileSystemInfo.AdditionalInfo += LoadStr(SFTP_EXTENSION_INFO) + L"\r\n";
      for (int Index = 0; Index < FExtensions->Count; Index++)
      {
        UnicodeString Name = FExtensions->Names[Index];
        UnicodeString Value = FExtensions->Values[Name];
        UnicodeString Line;
        if (Value.IsEmpty())
        {
          Line = Name;
        }
        else
        {
          Line = FORMAT(L"%s=%s", Name.c_str(), DisplayableStr(Value).c_str());
        }
        FFileSystemInfo.AdditionalInfo += FORMAT(L"  %s\r\n", Line.c_str());
      }
    }
    else
    {
      FFileSystemInfo.AdditionalInfo += LoadStr(SFTP_NO_EXTENSION_INFO) + L"\r\n";
    }

    FFileSystemInfo.ProtocolBaseName = L"SFTP";
    FFileSystemInfo.ProtocolName = FMTLOAD(SFTP_PROTOCOL_NAME2, FVersion);
    for (int Index = 0; Index < fcCount; Index++)
    {
      FFileSystemInfo.IsCapable[Index] = IsCapable(static_cast<TFSCapability>(Index));
    }

    FFileSystemInfoValid = true;
  }

  return FFileSystemInfo;
}
//---------------------------------------------------------------------------
bool TSFTPFileSystem::TemporaryTransferFile(const UnicodeString & FileName)
{
  return AnsiSameText(UnixExtractFileExt(FileName), PARTIAL_EXT);
}
//---------------------------------------------------------------------------
bool TSFTPFileSystem::GetStoredCredentialsTried()
{
  return FSecureShell->GetStoredCredentialsTried();
}
//---------------------------------------------------------------------------
UnicodeString TSFTPFileSystem::GetUserName()
{
  return FSecureShell->GetUserName();
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::Idle()
{
  CALLSTACK;
  // Keep session alive
  if ((GetSessionData()->GetPingType() != ptOff) &&
      ((Now() - FSecureShell->GetLastDataSent()) > GetSessionData()->GetPingIntervalDT()))
  {
    if ((GetSessionData()->GetPingType() == ptDummyCommand) &&
        FSecureShell->GetReady())
    {
      TSFTPPacket Packet(SSH_FXP_REALPATH, GetSessionData()->GetCodePageAsNumber());
      Packet.AddPathString(L"/", FUtfStrings);
      SendPacketAndReceiveResponse(&Packet, &Packet);
    }
    else
    {
      FSecureShell->KeepAlive();
    }
  }

  FSecureShell->Idle();
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::ResetConnection()
{
  // there must be no valid packet reservation at the end
  for (int i = 0; i < FPacketReservations->Count; i++)
  {
    assert(FPacketReservations->Items[i] == NULL);
    delete static_cast<TSFTPPacket *>(FPacketReservations->Items[i]);
  }
  FPacketReservations->Clear();
  FPacketNumbers.clear();
}
//---------------------------------------------------------------------------
bool TSFTPFileSystem::IsCapable(int Capability) const
{
  assert(FTerminal);
  switch (Capability) {
    case fcAnyCommand:
    case fcShellAnyCommand:
    case fcHardLink:
    case fcRemoteCopy:
      return false;

    case fcNewerOnlyUpload:
    case fcTimestampChanging:
    case fcIgnorePermErrors:
    case fcPreservingTimestampUpload:
    case fcSecondaryShell:
      return true;

    case fcRename:
    case fcRemoteMove:
      return (FVersion >= 2);

    case fcSymbolicLink:
    case fcResolveSymlink:
      return (FVersion >= 3);

    case fcModeChanging:
    case fcModeChangingUpload:
      return !FSupport->Loaded ||
        FLAGSET(FSupport->AttributeMask, SSH_FILEXFER_ATTR_PERMISSIONS);

    case fcGroupOwnerChangingByID:
      return (FVersion <= 3);

    case fcOwnerChanging:
    case fcGroupChanging:
      return
        (FVersion <= 3) ||
        ((FVersion >= 4) &&
         (!FSupport->Loaded ||
          FLAGSET(FSupport->AttributeMask, SSH_FILEXFER_ATTR_OWNERGROUP)));

    case fcNativeTextMode:
      return (FVersion >= 4);

    case fcTextMode:
      return (FVersion >= 4) ||
        strcmp(GetEOL(), EOLToStr(FTerminal->GetConfiguration()->GetLocalEOLType())) != 0;

    case fcUserGroupListing:
      return SupportsExtension(SFTP_EXT_OWNER_GROUP);

    case fcLoadingAdditionalProperties:
      // we allow loading properties only, if "suported" extension is supported and
      // the server support "permissions" and/or "owner/group" attributes
      // (no other attributes are loaded)
      return FSupport->Loaded &&
        ((FSupport->AttributeMask &
          (SSH_FILEXFER_ATTR_PERMISSIONS | SSH_FILEXFER_ATTR_OWNERGROUP)) != 0);

    case fcCheckingSpaceAvailable:
//!CLEANBEGIN
      #ifdef CLEAN_SPACE_AVAILABLE
      return true;
      #else
//!CLEANEND
      return SupportsExtension(SFTP_EXT_SPACE_AVAILABLE);
//!CLEANBEGIN
      #endif
//!CLEANEND


    case fcCalculatingChecksum:
      return SupportsExtension(SFTP_EXT_CHECK_FILE);

    default:
      assert(false);
      return false;
  }
}
//---------------------------------------------------------------------------
bool TSFTPFileSystem::SupportsExtension(const UnicodeString & Extension) const
{
  return FSupport->Loaded && (FSupport->Extensions->IndexOf(Extension.c_str()) >= 0);
}
//---------------------------------------------------------------------------
inline void TSFTPFileSystem::BusyStart()
{
  CALLSTACK;
  if (FBusy == 0 && FTerminal->GetUseBusyCursor() && !FAvoidBusy)
  {
    Busy(true);
  }
  FBusy++;
  assert(FBusy < 10);
}
//---------------------------------------------------------------------------
inline void TSFTPFileSystem::BusyEnd()
{
  CALLSTACK;
  assert(FBusy > 0);
  FBusy--;
  if (FBusy == 0 && FTerminal->GetUseBusyCursor() && !FAvoidBusy)
  {
    Busy(false);
  }
}
//---------------------------------------------------------------------------
unsigned long TSFTPFileSystem::TransferBlockSize(unsigned long Overhead,
  TFileOperationProgressType * OperationProgress,
  unsigned long MinPacketSize,
  unsigned long MaxPacketSize)
{
  const unsigned long minPacketSize = MinPacketSize ? MinPacketSize : 4096;

  // size + message number + type
  const unsigned long SFTPPacketOverhead = 4 + 4 + 1;
  unsigned long AMinPacketSize = FSecureShell->MinPacketSize();
  unsigned long AMaxPacketSize = FSecureShell->MaxPacketSize();
  bool MaxPacketSizeValid = (AMaxPacketSize > 0);
  unsigned long Result = OperationProgress->CPS();

  if ((MaxPacketSize > 0) &&
      ((MaxPacketSize < AMaxPacketSize) || !MaxPacketSizeValid))
  {
    AMaxPacketSize = MaxPacketSize;
    MaxPacketSizeValid = true;
  }

  if ((FMaxPacketSize > 0) &&
      ((FMaxPacketSize < AMaxPacketSize) || !MaxPacketSizeValid))
  {
    AMaxPacketSize = FMaxPacketSize;
    MaxPacketSizeValid = true;
  }

  if (Result == 0)
  {
    Result = OperationProgress->StaticBlockSize();
  }

  if (Result < minPacketSize)
  {
    Result = minPacketSize;
  }

  if (MaxPacketSizeValid)
  {
    Overhead += SFTPPacketOverhead;
    if (AMaxPacketSize < Overhead)
    {
      // do not send another request
      // (generally should happen only if upload buffer if full)
      Result = 0;
    }
    else
    {
      AMaxPacketSize -= Overhead;
      if (Result > AMaxPacketSize)
      {
        Result = AMaxPacketSize;
      }
    }
  }

  Result = OperationProgress->AdjustToCPSLimit(Result);

  return Result;
}
//---------------------------------------------------------------------------
unsigned long TSFTPFileSystem::UploadBlockSize(const RawByteString & Handle,
  TFileOperationProgressType * OperationProgress)
{
  // handle length + offset + data size
  const unsigned long UploadPacketOverhead =
    sizeof(unsigned long) + sizeof(__int64) + sizeof(unsigned long);
  return TransferBlockSize(UploadPacketOverhead + static_cast<unsigned long >(Handle.Length()), OperationProgress,
    GetSessionData()->GetSFTPMinPacketSize(),
    GetSessionData()->GetSFTPMaxPacketSize());
}
//---------------------------------------------------------------------------
unsigned long TSFTPFileSystem::DownloadBlockSize(
  TFileOperationProgressType * OperationProgress)
{
  unsigned long Result = TransferBlockSize(sizeof(unsigned long), OperationProgress,
    GetSessionData()->GetSFTPMinPacketSize(),
    GetSessionData()->GetSFTPMaxPacketSize());
  if (FSupport->Loaded && (FSupport->MaxReadSize > 0) &&
      (Result > FSupport->MaxReadSize))
  {
    Result = FSupport->MaxReadSize;
  }
  return Result;
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::SendPacket(const TSFTPPacket * Packet)
{
  CALLSTACK;
  BusyStart();
  TRY_FINALLY (
  {
    if (FTerminal->GetLog()->GetLogging())
    {
      if ((FPreviousLoggedPacket != SSH_FXP_READ &&
           FPreviousLoggedPacket != SSH_FXP_WRITE) ||
          (Packet->GetType() != FPreviousLoggedPacket) ||
          (FTerminal->GetConfiguration()->GetActualLogProtocol() >= 1))
      {
        if (FNotLoggedPackets)
        {
          FTerminal->LogEvent(FORMAT(L"%d skipped SSH_FXP_WRITE, SSH_FXP_READ, SSH_FXP_DATA and SSH_FXP_STATUS packets.",
            FNotLoggedPackets));
          FNotLoggedPackets = 0;
        }
        FTerminal->GetLog()->Add(llInput, FORMAT(L"Type: %s, Size: %d, Number: %d",
          Packet->GetTypeName().c_str(), (int)Packet->GetLength(), (int)Packet->GetMessageNumber()));
        if (FTerminal->GetConfiguration()->GetActualLogProtocol() >= 2)
        {
          FTerminal->GetLog()->Add(llInput, Packet->Dump());
        }
        FPreviousLoggedPacket = Packet->GetType();
      }
      else
      {
        FNotLoggedPackets++;
      }
    }
    FSecureShell->Send(Packet->GetSendData(), (int)Packet->GetSendLength());
  }
  ,
  {
    BusyEnd();
  }
  );
  TRACE("/");
}
//---------------------------------------------------------------------------
unsigned long TSFTPFileSystem::GotStatusPacket(TSFTPPacket * Packet,
  int AllowStatus)
{
  unsigned long Code = Packet->GetCardinal();

  static int Messages[] = {
    SFTP_STATUS_OK,
    SFTP_STATUS_EOF,
    SFTP_STATUS_NO_SUCH_FILE,
    SFTP_STATUS_PERMISSION_DENIED,
    SFTP_STATUS_FAILURE,
    SFTP_STATUS_BAD_MESSAGE,
    SFTP_STATUS_NO_CONNECTION,
    SFTP_STATUS_CONNECTION_LOST,
    SFTP_STATUS_OP_UNSUPPORTED,
    SFTP_STATUS_INVALID_HANDLE,
    SFTP_STATUS_NO_SUCH_PATH,
    SFTP_STATUS_FILE_ALREADY_EXISTS,
    SFTP_STATUS_WRITE_PROTECT,
    SFTP_STATUS_NO_MEDIA,
    SFTP_STATUS_NO_SPACE_ON_FILESYSTEM,
    SFTP_STATUS_QUOTA_EXCEEDED,
    SFTP_STATUS_UNKNOWN_PRINCIPAL,
    SFTP_STATUS_LOCK_CONFLICT,
    SFTP_STATUS_DIR_NOT_EMPTY,
    SFTP_STATUS_NOT_A_DIRECTORY,
    SFTP_STATUS_INVALID_FILENAME,
    SFTP_STATUS_LINK_LOOP,
    SFTP_STATUS_CANNOT_DELETE,
    SFTP_STATUS_INVALID_PARAMETER,
    SFTP_STATUS_FILE_IS_A_DIRECTORY,
    SFTP_STATUS_BYTE_RANGE_LOCK_CONFLICT,
    SFTP_STATUS_BYTE_RANGE_LOCK_REFUSED,
    SFTP_STATUS_DELETE_PENDING,
    SFTP_STATUS_FILE_CORRUPT
  };
  int Message;
  if ((AllowStatus & (0x01 << Code)) == 0)
  {
    if (Code >= LENOF(Messages))
    {
      Message = SFTP_STATUS_UNKNOWN;
    }
    else
    {
      Message = Messages[Code];
    }
    UnicodeString MessageStr = LoadStr(Message);
    UnicodeString ServerMessage;
    UnicodeString LanguageTag;
    if ((FVersion >= 3) ||
        // if version is not decided yet (i.e. this is status response
        // to the init request), go on, only if there are any more data
        ((FVersion < 0) && (Packet->GetRemainingLength() > 0)))
    {
      // message is in UTF only since SFTP specification 01 (specification 00
      // is also version 3)
      // (in other words, always use UTF unless server is known to be buggy)
      ServerMessage = Packet->GetString(!FUtfNever);
      // SSH-2.0-Maverick_SSHD and SSH-2.0-CIGNA SFTP Server Ready! omit the language tag
      // and I believe I've seen one more server doing the same.
      if (Packet->GetRemainingLength() > 0)
      {
        LanguageTag = Packet->GetAnsiString();
        if ((FVersion >= 5) && (Message == SFTP_STATUS_UNKNOWN_PRINCIPAL))
        {
          UnicodeString Principals;
          while (Packet->GetNextData() != NULL)
          {
            if (!Principals.IsEmpty())
            {
              Principals += L", ";
            }
            Principals += Packet->GetAnsiString();
          }
          MessageStr = FORMAT(MessageStr.c_str(), Principals.c_str());
        }
      }
    }
    else
    {
      ServerMessage = LoadStr(SFTP_SERVER_MESSAGE_UNSUPPORTED);
    }
    if (FTerminal->GetLog()->GetLogging())
    {
      FTerminal->GetLog()->Add(llOutput, FORMAT(L"Status code: %d, Message: %d, Server: %s, Language: %s ",
        int(Code), (int)Packet->GetMessageNumber(), ServerMessage.c_str(), LanguageTag.c_str()));
    }
    if (!LanguageTag.IsEmpty())
    {
      LanguageTag = FORMAT(L" (%s)", LanguageTag.c_str());
    }
    UnicodeString Error = FMTLOAD(SFTP_ERROR_FORMAT2, MessageStr.c_str(),
      int(Code), LanguageTag.c_str(), ServerMessage.c_str(), int(Packet->GetRequestType()));
    FTerminal->TerminalError(NULL, Error);
    return 0;
  }
  else
  {
    if (!FNotLoggedPackets || Code)
    {
      FTerminal->GetLog()->Add(llOutput, FORMAT(L"Status code: %d", static_cast<int>(Code)));
    }
    return Code;
  }
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::RemoveReservation(intptr_t Reservation)
{
  for (intptr_t Index = Reservation+1; Index < FPacketReservations->Count; Index++)
  {
    FPacketNumbers[Index-1] = FPacketNumbers[Index];
  }
  TSFTPPacket * Packet = static_cast<TSFTPPacket *>(FPacketReservations->Items[Reservation]);
  if (Packet)
  {
    assert(Packet->GetReservedBy() == this);
    Packet->SetReservedBy(NULL);
  }
  FPacketReservations->Delete(Reservation);
}
//---------------------------------------------------------------------------
/* inline */ int TSFTPFileSystem::PacketLength(unsigned char * LenBuf, int ExpectedType)
{
  int Length = GET_32BIT(LenBuf);
  if (Length > SFTP_MAX_PACKET_LEN)
  {
    UnicodeString Message = FMTLOAD(SFTP_PACKET_TOO_BIG,
      int(Length), SFTP_MAX_PACKET_LEN);
    if (ExpectedType == SSH_FXP_VERSION)
    {
      RawByteString LenString(reinterpret_cast<char *>(LenBuf), 4);
      Message = FMTLOAD(SFTP_PACKET_TOO_BIG_INIT_EXPLAIN,
        Message.c_str(), DisplayableStr(LenString).c_str());
    }
    FTerminal->FatalError(NULL, Message);
  }
  return Length;
}
//---------------------------------------------------------------------------
bool TSFTPFileSystem::PeekPacket()
{
  bool Result;
  unsigned char * Buf = NULL;
  Result = FSecureShell->Peek(Buf, 4);
  if (Result)
  {
    int Length = PacketLength(Buf, -1);
    Result = FSecureShell->Peek(Buf, 4 + Length);
  }
  return Result;
}
//---------------------------------------------------------------------------
uintptr_t TSFTPFileSystem::ReceivePacket(TSFTPPacket * Packet,
  int ExpectedType, int AllowStatus)
{
  CALLSTACK;
  TSFTPBusy Busy(this);

  uintptr_t Result = SSH_FX_OK;
  intptr_t Reservation = FPacketReservations->IndexOf(Packet);

  if ((Reservation < 0) || (Packet->GetCapacity() == 0))
  {
    TRACE("1");
    bool IsReserved;
    do
    {
      IsReserved = false;

      assert(Packet);
      unsigned char LenBuf[4];
      FSecureShell->Receive(LenBuf, sizeof(LenBuf));
      int Length = PacketLength(LenBuf, ExpectedType);
      Packet->SetCapacity(Length);
      FSecureShell->Receive(Packet->GetData(), Length);
      Packet->DataUpdated(Length);

      if (FTerminal->GetLog()->GetLogging())
      {
        TRACE("2");
        if ((FPreviousLoggedPacket != SSH_FXP_READ &&
             FPreviousLoggedPacket != SSH_FXP_WRITE) ||
            (Packet->GetType() != SSH_FXP_STATUS && Packet->GetType() != SSH_FXP_DATA) ||
            (FTerminal->GetConfiguration()->GetActualLogProtocol() >= 1))
        {
          if (FNotLoggedPackets)
          {
            FTerminal->LogEvent(FORMAT(L"%d skipped SSH_FXP_WRITE, SSH_FXP_READ, SSH_FXP_DATA and SSH_FXP_STATUS packets.",
              FNotLoggedPackets));
            FNotLoggedPackets = 0;
          }
          FTerminal->GetLog()->Add(llOutput, FORMAT(L"Type: %s, Size: %d, Number: %d",
            Packet->GetTypeName().c_str(), (int)Packet->GetLength(), (int)Packet->GetMessageNumber()));
          if (FTerminal->GetConfiguration()->GetActualLogProtocol() >= 2)
          {
            FTerminal->GetLog()->Add(llOutput, Packet->Dump());
          }
        }
        else
        {
          FNotLoggedPackets++;
        }
      }

      if ((Reservation < 0) ||
          Packet->GetMessageNumber() != FPacketNumbers[Reservation])
      {
        TRACE("4");
        TSFTPPacket * ReservedPacket;
        unsigned int MessageNumber;
        for (int Index = 0; Index < FPacketReservations->Count; Index++)
        {
          TRACE("4a");
          MessageNumber = (unsigned int)FPacketNumbers[Index];
          if (MessageNumber == Packet->GetMessageNumber())
          {
            TRACE("4b");
            ReservedPacket = static_cast<TSFTPPacket *>(FPacketReservations->Items[Index]);
            IsReserved = true;
            if (ReservedPacket)
            {
              TRACE("4c");
              FTerminal->LogEvent(L"Storing reserved response");
              *ReservedPacket = *Packet;
            }
            else
            {
              TRACE("4d");
              FTerminal->LogEvent(L"Discarding reserved response");
              RemoveReservation(Index);
              if ((Reservation >= 0) && (Reservation > Index))
              {
                TRACE("4e");
                Reservation--;
                assert(Reservation == FPacketReservations->IndexOf(Packet));
              }
            }
            break;
          }
        }
      }
    }
    while (IsReserved);
  }

  // before we removed the reservation after check for packet type,
  // but if it raises exception, removal is unnecessarily
  // postponed until the packet is removed
  // (and it have not worked anyway until recent fix to UnreserveResponse)
  if (Reservation >= 0)
  {
    TRACE("5");
    assert(Packet->GetMessageNumber() == FPacketNumbers[Reservation]);
    RemoveReservation(Reservation);
  }

  if (ExpectedType >= 0)
  {
    TRACE("6");
    if (Packet->GetType() == SSH_FXP_STATUS)
    {
      if (AllowStatus < 0)
      {
        AllowStatus = (ExpectedType == SSH_FXP_STATUS ? asOK : asNo);
      }
      Result = GotStatusPacket(Packet, AllowStatus);
    }
    else if (ExpectedType != Packet->GetType())
    {
      FTerminal->FatalError(NULL, FMTLOAD(SFTP_INVALID_TYPE, static_cast<int>(Packet->GetType())));
    }
  }

  TRACE("/");
  return Result;
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::ReserveResponse(const TSFTPPacket * Packet,
  TSFTPPacket * Response)
{
  if (Response != NULL)
  {
    assert(FPacketReservations->IndexOf(Response) < 0);
    // mark response as not received yet
    Response->SetCapacity(0);
    Response->SetReservedBy(this);
  }
  FPacketReservations->Add(Response);
  if ((size_t)FPacketReservations->Count >= FPacketNumbers.size())
  {
    FPacketNumbers.resize(FPacketReservations->Count + 10);
  }
  FPacketNumbers[FPacketReservations->Count - 1] = Packet->GetMessageNumber();
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::UnreserveResponse(TSFTPPacket * Response)
{
  intptr_t Reservation = FPacketReservations->IndexOf(Response);
  if (Response->GetCapacity() != 0)
  {
    // added check for already received packet
    // (it happens when the reserved response is received out of order,
    // unexpectedly soon, and then receivepacket() on the packet
    // is not actually called, due to exception)
    RemoveReservation(Reservation);
  }
  else
  {
    if (Reservation >= 0)
    {
      // we probably do not remove the item at all, because
      // we must remember that the response was expected, so we skip it
      // in receivepacket()
      FPacketReservations->Items(Reservation, NULL);
    }
  }
}
//---------------------------------------------------------------------------
uintptr_t TSFTPFileSystem::ReceiveResponse(
  const TSFTPPacket * Packet, TSFTPPacket * Response, int ExpectedType,
  int AllowStatus)
{
  CALLSTACK;
  uintptr_t Result;
  uintptr_t MessageNumber = Packet->GetMessageNumber();
  TSFTPPacket * AResponse = (Response ? Response : new TSFTPPacket(GetSessionData()->GetCodePageAsNumber()));
  TRY_FINALLY (
  {
    Result = ReceivePacket(AResponse, ExpectedType, AllowStatus);
    if (MessageNumber != AResponse->GetMessageNumber())
    {
      TRACE("1");
      FTerminal->FatalError(NULL, FMTLOAD(SFTP_MESSAGE_NUMBER,
        (int)AResponse->GetMessageNumber(), (int)MessageNumber));
    }
  }
  ,
  {
    if (!Response)
    {
      delete AResponse;
    }
  }
  );
  TRACE("/");
  return Result;
}
//---------------------------------------------------------------------------
uintptr_t TSFTPFileSystem::SendPacketAndReceiveResponse(
  const TSFTPPacket * Packet, TSFTPPacket * Response, int ExpectedType,
  int AllowStatus)
{
  CALLSTACK;
  uintptr_t Result;
  TSFTPBusy Busy(this);
  SendPacket(Packet);
  Result = ReceiveResponse(Packet, Response, ExpectedType, AllowStatus);
  TRACE("/");
  return Result;
}
//---------------------------------------------------------------------------
UnicodeString TSFTPFileSystem::RealPath(const UnicodeString & Path)
{
  try
  {
    FTerminal->LogEvent(FORMAT(L"Getting real path for '%s'",
      Path.c_str()));

    TSFTPPacket Packet(SSH_FXP_REALPATH, GetSessionData()->GetCodePageAsNumber());
    Packet.AddPathString(Path, FUtfStrings);
    SendPacketAndReceiveResponse(&Packet, &Packet, SSH_FXP_NAME);
    if (Packet.GetCardinal() != 1)
    {
      FTerminal->FatalError(NULL, LoadStr(SFTP_NON_ONE_FXP_NAME_PACKET));
    }

    UnicodeString RealDir = UnixExcludeTrailingBackslash(Packet.GetPathString(FUtfStrings));
    // ignore rest of SSH_FXP_NAME packet

    FTerminal->LogEvent(FORMAT(L"Real path is '%s'", RealDir.c_str()));

    return RealDir;
  }
  catch(Exception & E)
  {
    if (FTerminal->GetActive())
    {
      throw ExtException(&E, FMTLOAD(SFTP_REALPATH_ERROR, Path.c_str()));
    }
    else
    {
      throw;
    }
  }
  return UnicodeString();
}
//---------------------------------------------------------------------------
UnicodeString TSFTPFileSystem::RealPath(const UnicodeString & Path,
  const UnicodeString & BaseDir)
{
  UnicodeString APath;

  if (FTerminal->IsAbsolutePath(Path))
  {
    APath = Path;
  }
  else
  {
    if (!Path.IsEmpty())
    {
      // this condition/block was outside (before) current block
      // but it dod not work when Path was empty
      if (!BaseDir.IsEmpty())
      {
        APath = UnixIncludeTrailingBackslash(BaseDir);
      }
      APath = APath + Path;
    }
    if (APath.IsEmpty()) { APath = UnixIncludeTrailingBackslash(L"."); }
  }
  return RealPath(APath);
}
//---------------------------------------------------------------------------
UnicodeString TSFTPFileSystem::LocalCanonify(const UnicodeString & Path)
{
  // TODO: improve (handle .. etc.)
  if (FTerminal->IsAbsolutePath(Path) ||
      (!FCurrentDirectory.IsEmpty() && UnixComparePaths(FCurrentDirectory, Path)))
  {
    return Path;
  }
  else
  {
    return ::AbsolutePath(FCurrentDirectory, Path);
  }
}
//---------------------------------------------------------------------------
UnicodeString TSFTPFileSystem::Canonify(UnicodeString Path)
{
  // inspired by canonify() from PSFTP.C
  UnicodeString Result;
  FTerminal->LogEvent(FORMAT(L"Canonifying: \"%s\"", Path.c_str()));
  Path = LocalCanonify(Path);
  bool TryParent = false;
  try
  {
    Result = RealPath(Path);
  }
  catch(...)
  {
    if (FTerminal->GetActive())
    {
      TryParent = true;
    }
    else
    {
      throw;
    }
  }

  if (TryParent)
  {
    UnicodeString APath = UnixExcludeTrailingBackslash(Path);
    UnicodeString Name = UnixExtractFileName(APath);
    if (Name == THISDIRECTORY || Name == PARENTDIRECTORY)
    {
      Result = Path;
    }
    else
    {
      UnicodeString FPath = UnixExtractFilePath(APath);
      try
      {
        Result = RealPath(FPath);
        Result = UnixIncludeTrailingBackslash(Result) + Name;
      }
      catch(...)
      {
        if (FTerminal->GetActive())
        {
          Result = Path;
        }
        else
        {
          throw;
        }
      }
    }
  }

  FTerminal->LogEvent(FORMAT(L"Canonified: \"%s\"", Result.c_str()));

  return Result;
}
//---------------------------------------------------------------------------
UnicodeString TSFTPFileSystem::AbsolutePath(const UnicodeString & Path, bool Local)
{
  if (Local)
  {
    return LocalCanonify(Path);
  }
  else
  {
    return RealPath(Path, GetCurrentDirectory());
  }
}
//---------------------------------------------------------------------------
UnicodeString TSFTPFileSystem::GetHomeDirectory()
{
  if (FHomeDirectory.IsEmpty())
  {
    FHomeDirectory = RealPath(THISDIRECTORY);
  }
  return FHomeDirectory;
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::LoadFile(TRemoteFile * File, TSFTPPacket * Packet,
  bool Complete)
{
  CALLSTACK;
  Packet->GetFile(File, FVersion, GetSessionData()->GetDSTMode(),
    FUtfStrings, FSignedTS, Complete);
}
//---------------------------------------------------------------------------
TRemoteFile * TSFTPFileSystem::LoadFile(TSFTPPacket * Packet,
  TRemoteFile * ALinkedByFile, const UnicodeString & FileName,
  TRemoteFileList * TempFileList, bool Complete)
{
  TRemoteFile * File = new TRemoteFile(ALinkedByFile);
  try
  {
    File->SetTerminal(FTerminal);
    if (!FileName.IsEmpty())
    {
      File->SetFileName(FileName);
    }
    // to get full path for symlink completion
    File->SetDirectory(TempFileList);
    LoadFile(File, Packet, Complete);
    File->SetDirectory(NULL);
  }
  catch(...)
  {
    delete File;
    throw;
  }
  return File;
}
//---------------------------------------------------------------------------
UnicodeString TSFTPFileSystem::GetCurrentDirectory()
{
  return FCurrentDirectory;
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::DoStartup()
{
  CALLSTACK;
  // do not know yet
  FVersion = -1;
  FFileSystemInfoValid = false;
  TSFTPPacket Packet(SSH_FXP_INIT, GetSessionData()->GetCodePageAsNumber());
  int MaxVersion = GetSessionData()->GetSFTPMaxVersion();
  if (MaxVersion > SFTPMaxVersion)
  {
    MaxVersion = SFTPMaxVersion;
  }
  Packet.AddCardinal(MaxVersion);

  try
  {
    SendPacketAndReceiveResponse(&Packet, &Packet, SSH_FXP_VERSION);
  }
  catch(Exception &E)
  {
    FTerminal->FatalError(&E, LoadStr(SFTP_INITIALIZE_ERROR));
  }

  FVersion = Packet.GetCardinal();
  FTerminal->LogEvent(FORMAT(L"SFTP version %d negotiated.", FVersion));
  if (FVersion < SFTPMinVersion || FVersion > SFTPMaxVersion)
  {
    FTerminal->FatalError(NULL, FMTLOAD(SFTP_VERSION_NOT_SUPPORTED,
      FVersion, SFTPMinVersion, SFTPMaxVersion));
  }

  FExtensions->Clear();
  FEOL = "\r\n";
  FSupport->Loaded = false;
  SAFE_DESTROY(FFixedPaths);

  if (FVersion >= 3)
  {
    while (Packet.GetNextData() != NULL)
    {
      UnicodeString ExtensionName = Packet.GetAnsiString();
      RawByteString ExtensionData = Packet.GetRawByteString();
      UnicodeString ExtensionDisplayData = DisplayableStr(ExtensionData);

      if (ExtensionName == SFTP_EXT_NEWLINE)
      {
        FEOL = AnsiString(ExtensionData.c_str());
        FTerminal->LogEvent(FORMAT(L"Server requests EOL sequence %s.",
          ExtensionDisplayData.c_str()));
        if (FEOL.Length() < 1 || FEOL.Length() > 2)
        {
          FTerminal->FatalError(NULL, FMTLOAD(SFTP_INVALID_EOL, ExtensionDisplayData.c_str()));
        }
      }
      // do not allow "supported" to override "supported2" if both are received
      else if (((ExtensionName == SFTP_EXT_SUPPORTED) && !FSupport->Loaded) ||
               (ExtensionName == SFTP_EXT_SUPPORTED2))
      {
        FSupport->Reset();
        TSFTPPacket SupportedStruct(ExtensionData, GetSessionData()->GetCodePageAsNumber());
        FSupport->Loaded = true;
        FSupport->AttributeMask = SupportedStruct.GetCardinal();
        FSupport->AttributeBits = SupportedStruct.GetCardinal();
        FSupport->OpenFlags = SupportedStruct.GetCardinal();
        FSupport->AccessMask = SupportedStruct.GetCardinal();
        FSupport->MaxReadSize = SupportedStruct.GetCardinal();
        if (ExtensionName == SFTP_EXT_SUPPORTED)
        {
          while (SupportedStruct.GetNextData() != NULL)
          {
            FSupport->Extensions->Add(SupportedStruct.GetAnsiString());
          }
        }
        else
        {
          FSupport->OpenBlockMasks = SupportedStruct.GetSmallCardinal();
          FSupport->BlockMasks = SupportedStruct.GetSmallCardinal();
          unsigned int ExtensionCount;
          ExtensionCount = SupportedStruct.GetCardinal();
          for (unsigned int i = 0; i < ExtensionCount; i++)
          {
            FSupport->AttribExtensions->Add(SupportedStruct.GetAnsiString());
          }
          ExtensionCount = SupportedStruct.GetCardinal();
          for (unsigned int i = 0; i < ExtensionCount; i++)
          {
            FSupport->Extensions->Add(SupportedStruct.GetAnsiString());
          }
        }

        if (FTerminal->GetLog()->GetLogging())
        {
          FTerminal->LogEvent(FORMAT(
            L"Server support information:\n"
            L"  Attribute mask: %x, Attribute bits: %x, Open flags: %x\n"
            L"  Access mask: %x, Open block masks: %x, Block masks: %x, Max read size: %d\n",
            int(FSupport->AttributeMask),
             int(FSupport->AttributeBits),
             int(FSupport->OpenFlags),
             int(FSupport->AccessMask),
             int(FSupport->OpenBlockMasks),
             int(FSupport->BlockMasks),
             int(FSupport->MaxReadSize)));
          FTerminal->LogEvent(FORMAT(L"  Attribute extensions (%d)\n", FSupport->AttribExtensions->Count.get()));
          for (int Index = 0; Index < FSupport->AttribExtensions->Count; Index++)
          {
            FTerminal->LogEvent(
              FORMAT(L"    %s", FSupport->AttribExtensions->Strings[Index].c_str()));
          }
          FTerminal->LogEvent(FORMAT(L"  Extensions (%d)\n", FSupport->Extensions->Count.get()));
          for (int Index = 0; Index < FSupport->Extensions->Count; Index++)
          {
            FTerminal->LogEvent(
              FORMAT(L"    %s", FSupport->Extensions->Strings[Index].c_str()));
          }
        }
      }
      else if (ExtensionName == SFTP_EXT_VENDOR_ID)
      {
        TSFTPPacket VendorIdStruct(ExtensionData, GetSessionData()->GetCodePageAsNumber());
        UnicodeString VendorName(VendorIdStruct.GetAnsiString());
        UnicodeString ProductName(VendorIdStruct.GetAnsiString());
        UnicodeString ProductVersion(VendorIdStruct.GetAnsiString());
        __int64 ProductBuildNumber = VendorIdStruct.GetInt64();
        FTerminal->LogEvent(FORMAT(L"Server software: %s %s (%d) by %s",
          ProductName.c_str(), ProductVersion.c_str(), int(ProductBuildNumber), VendorName.c_str()));
      }
      else if (ExtensionName == SFTP_EXT_FSROOTS)
      {
        FTerminal->LogEvent(L"File system roots:\n");
        assert(FFixedPaths == NULL);
        FFixedPaths = new TStringList();
        try
        {
          TSFTPPacket RootsPacket(ExtensionData, GetSessionData()->GetCodePageAsNumber());
          while (RootsPacket.GetNextData() != NULL)
          {
            unsigned long Dummy = RootsPacket.GetCardinal();
            if (Dummy != 1)
            {
              break;
            }
            else
            {
              unsigned char Drive = RootsPacket.GetByte();
              unsigned char MaybeType = RootsPacket.GetByte();
              FTerminal->LogEvent(FORMAT(L"  %c: (type %d)", static_cast<char>(Drive), static_cast<int>(MaybeType)));
              FFixedPaths->Add(FORMAT(L"%c:", static_cast<char>(Drive)));
            }
          }
        }
        catch(Exception & E)
        {
          DEBUG_PRINTF(L"before FTerminal->HandleException");
          FFixedPaths->Clear();
          FTerminal->LogEvent(FORMAT(L"Failed to decode %s extension",
            SFTP_EXT_FSROOTS));
          FTerminal->HandleException(&E);
        }
      }
      else if (ExtensionName == SFTP_EXT_VERSIONS)
      {
        try
        {
          // first try legacy decoding according to incorrect encoding
          // (structure-like) as of VShell.
          TSFTPPacket VersionsPacket(ExtensionData, GetSessionData()->GetCodePageAsNumber());
          UnicodeString Versions = VersionsPacket.GetAnsiString();
          if (VersionsPacket.GetNextData() != NULL)
          {
            Abort();
          }
          FTerminal->LogEvent(FORMAT(L"SFTP versions supported by the server (VShell format): %s",
            Versions.c_str()));
        }
        catch(...)
        {
          // if that fails, fallback to proper decoding
          FTerminal->LogEvent(FORMAT(L"SFTP versions supported by the server: %s",
            AnsiString(ExtensionData.c_str()).c_str()));
        }
      }
      else
      {
        FTerminal->LogEvent(FORMAT(L"Unknown server extension %s=%s",
          ExtensionName.c_str(), ExtensionDisplayData.c_str()));
      }
      FExtensions->Values(ExtensionName, ExtensionDisplayData);
    }

    if (SupportsExtension(SFTP_EXT_VENDOR_ID))
    {
      TSFTPPacket Packet((unsigned char)SSH_FXP_EXTENDED, GetSessionData()->GetCodePageAsNumber());
      Packet.AddString(RawByteString(SFTP_EXT_VENDOR_ID));
      Packet.AddString(FTerminal->GetConfiguration()->GetCompanyName());
      Packet.AddString(FTerminal->GetConfiguration()->GetProductName());
      Packet.AddString(FTerminal->GetConfiguration()->GetProductVersion());
      Packet.AddInt64(LOWORD(FTerminal->GetConfiguration()->GetFixedApplicationInfo()->dwFileVersionLS));
      SendPacket(&Packet);
      // we are not interested in the response, do not wait for it
      ReserveResponse(&Packet, NULL);
    }
  }

  if (FVersion < 4)
  {
    // currently enable the bug for all servers (really known on OpenSSH)
    FSignedTS = (GetSessionData()->GetSFTPBug(sbSignedTS) == asOn) ||
      (GetSessionData()->GetSFTPBug(sbSignedTS) == asAuto);
    if (FSignedTS)
    {
      FTerminal->LogEvent(L"We believe the server has signed timestamps bug");
    }
  }
  else
  {
    FSignedTS = false;
  }

  // use UTF when forced or ...
  // when "auto" and version is at least 4 and the server is not know not to use UTF
  FUtfNever = (GetSessionInfo().SshImplementation.Pos(L"Foxit-WAC-Server") == 1) ||
    (GetSessionData()->GetNotUtf() == asOn);
  FUtfStrings =
    (GetSessionData()->GetNotUtf() == asOff) ||
    ((GetSessionData()->GetNotUtf() == asAuto) &&
      (FVersion >= 4) && !FUtfNever);

  if (FUtfStrings)
  {
    FTerminal->LogEvent(L"We will use UTF-8 strings when appropriate");
  }
  else if (FUtfNever)
  {
    FTerminal->LogEvent(L"We will never use UTF-8 strings");
  }
  else
  {
    FTerminal->LogEvent(L"We will use UTF-8 strings for status messages only");
  }

  FOpenSSH =
    // Sun SSH is based on OpenSSH (suffers the same bugs)
    (GetSessionInfo().SshImplementation.Pos(L"OpenSSH") == 1) ||
    (GetSessionInfo().SshImplementation.Pos(L"Sun_SSH") == 1);

  FMaxPacketSize = GetSessionData()->GetSFTPMaxPacketSize();
  if (FMaxPacketSize == 0)
  {
    if (FOpenSSH && (FVersion == 3) && !FSupport->Loaded)
    {
      FMaxPacketSize = 4 + (256 * 1024); // len + 256kB payload
      FTerminal->LogEvent(FORMAT(L"Limiting packet size to OpenSSH sftp-server limit of %d bytes",
        int(FMaxPacketSize)));
    }
    // full string is "1.77 sshlib: Momentum SSH Server",
    // possibly it is sshlib-related
    else if (GetSessionInfo().SshImplementation.Pos(L"Momentum SSH Server") != 0)
    {
      FMaxPacketSize = 4 + (32 * 1024);
      FTerminal->LogEvent(FORMAT(L"Limiting packet size to Momentum sftp-server limit of %d bytes",
        int(FMaxPacketSize)));
    }
  }
  TRACE("/");
}
//---------------------------------------------------------------------------
char * TSFTPFileSystem::GetEOL() const
{
  if (FVersion >= 4)
  {
    assert(!FEOL.IsEmpty());
    return const_cast<char *>(FEOL.c_str());
  }
  else
  {
    return EOLToStr(GetSessionData()->GetEOLType());
  }
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::LookupUsersGroups()
{
  assert(SupportsExtension(SFTP_EXT_OWNER_GROUP));

  TSFTPPacket PacketOwners((unsigned char)SSH_FXP_EXTENDED, GetSessionData()->GetCodePageAsNumber());
  TSFTPPacket PacketGroups((unsigned char)SSH_FXP_EXTENDED, GetSessionData()->GetCodePageAsNumber());

  TSFTPPacket * Packets[] = { &PacketOwners, &PacketGroups };
  TRemoteTokenList * Lists[] = { FTerminal->GetUsers(), FTerminal->GetGroups() };
  wchar_t ListTypes[] = { OGQ_LIST_OWNERS, OGQ_LIST_GROUPS };

  for (size_t Index = 0; Index < LENOF(Packets); Index++)
  {
    TSFTPPacket * Packet = Packets[Index];
    Packet->AddString(RawByteString(SFTP_EXT_OWNER_GROUP));
    Packet->AddByte((unsigned char)ListTypes[Index]);
    SendPacket(Packet);
    ReserveResponse(Packet, Packet);
  }

  for (size_t Index = 0; Index < LENOF(Packets); Index++)
  {
    TSFTPPacket * Packet = Packets[Index];

    ReceiveResponse(Packet, Packet, SSH_FXP_EXTENDED_REPLY, asOpUnsupported);

    if ((Packet->GetType() != SSH_FXP_EXTENDED_REPLY) ||
        (Packet->GetAnsiString() != SFTP_EXT_OWNER_GROUP_REPLY))
    {
      FTerminal->LogEvent(FORMAT(L"Invalid response to %s", SFTP_EXT_OWNER_GROUP));
    }
    else
    {
      TRemoteTokenList & List = *Lists[Index];
      unsigned long Count = Packet->GetCardinal();

      List.Clear();
      for (unsigned long Item = 0; Item < Count; Item++)
      {
        TRemoteToken Token(Packet->GetString(!FUtfNever));
        List.Add(Token);
        if (&List == FTerminal->GetGroups())
        {
          FTerminal->GetMembership()->Add(Token);
        }
      }
    }
  }
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::ReadCurrentDirectory()
{
  if (!FDirectoryToChangeTo.IsEmpty())
  {
    FCurrentDirectory = FDirectoryToChangeTo;
    FDirectoryToChangeTo = L"";
  }
  else if (FCurrentDirectory.IsEmpty())
  {
    // this happens only after startup when default remote directory is not specified
    FCurrentDirectory = GetHomeDirectory();
  }
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::HomeDirectory()
{
  ChangeDirectory(GetHomeDirectory());
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::TryOpenDirectory(const UnicodeString & Directory)
{
  FTerminal->LogEvent(FORMAT(L"Trying to open directory \"%s\".", Directory.c_str()));
  TRemoteFile * File;
  CustomReadFile(Directory, File, SSH_FXP_LSTAT, NULL, asOpUnsupported);
  if (File == NULL)
  {
    // File can be NULL only when server does not support SSH_FXP_LSTAT.
    // Fallback to legacy solution, which in turn does not allow entering
    // traverse-only (chmod 110) directories.
    // This is workaround for http://www.ftpshell.com/
    TSFTPPacket Packet(SSH_FXP_OPENDIR, GetSessionData()->GetCodePageAsNumber());
    Packet.AddPathString(UnixExcludeTrailingBackslash(Directory), FUtfStrings);
    SendPacketAndReceiveResponse(&Packet, &Packet, SSH_FXP_HANDLE);
    RawByteString Handle = Packet.GetFileHandle();
    Packet.ChangeType(SSH_FXP_CLOSE);
    Packet.AddString(Handle);
    SendPacketAndReceiveResponse(&Packet, &Packet, SSH_FXP_STATUS, asAll);
  }
  else
  {
    delete File;
  }
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::AnnounceFileListOperation()
{
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::ChangeDirectory(const UnicodeString & Directory)
{
  UnicodeString Path, Current;

  Current = !FDirectoryToChangeTo.IsEmpty() ? FDirectoryToChangeTo : FCurrentDirectory;
  Path = RealPath(Directory, Current);

  // to verify existence of directory try to open it (SSH_FXP_REALPATH succeeds
  // for invalid paths on some systems, like CygWin)
  TryOpenDirectory(Path);

  // if open dir did not fail, directory exists -> success.
  FDirectoryToChangeTo = Path;
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::CachedChangeDirectory(const UnicodeString & Directory)
{
  FDirectoryToChangeTo = UnixExcludeTrailingBackslash(Directory);
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::ReadDirectory(TRemoteFileList * FileList)
{
  CALLSTACK;
  assert(FileList && !FileList->GetDirectory().IsEmpty());

  UnicodeString Directory;
  Directory = UnixExcludeTrailingBackslash(LocalCanonify(FileList->GetDirectory()));
  FTerminal->LogEvent(FORMAT(L"Listing directory \"%s\".", Directory.c_str()));

  // moved before SSH_FXP_OPENDIR, so directory listing does not retain
  // old data (e.g. parent directory) when reading fails
  FileList->Clear();

  TSFTPPacket Packet(SSH_FXP_OPENDIR, GetSessionData()->GetCodePageAsNumber());
  RawByteString Handle;

  try
  {
    Packet.AddPathString(Directory, FUtfStrings);

    SendPacketAndReceiveResponse(&Packet, &Packet, SSH_FXP_HANDLE);

    Handle = Packet.GetFileHandle();
  }
  catch(...)
  {
    if (FTerminal->GetActive())
    {
      FileList->AddFile(new TRemoteParentDirectory(FTerminal));
    }
    throw;
  }

  TRACE("1");
  TSFTPPacket Response(GetSessionData()->GetCodePageAsNumber());
  TRY_FINALLY (
  {
    bool isEOF = false;
    int Total = 0;
    TRemoteFile * File;

    Packet.ChangeType(SSH_FXP_READDIR);
    Packet.AddString(Handle);

    SendPacket(&Packet);

    TRACE("2");
    do
    {
      ReceiveResponse(&Packet, &Response);
      TRACE("3");
      if (Response.GetType() == SSH_FXP_NAME)
      {
        TSFTPPacket ListingPacket(Response, GetSessionData()->GetCodePageAsNumber());

        Packet.ChangeType(SSH_FXP_READDIR);
        Packet.AddString(Handle);

        SendPacket(&Packet);
        ReserveResponse(&Packet, &Response);

        unsigned int Count = ListingPacket.GetCardinal();

        TRACE("4");
        for (unsigned long Index = 0; !isEOF && (Index < Count); Index++)
        {
          File = LoadFile(&ListingPacket, NULL, L"", FileList);
          if (FTerminal->GetConfiguration()->GetActualLogProtocol() >= 1)
          {
            FTerminal->LogEvent(FORMAT(L"Read file '%s' from listing", File->GetFileName().c_str()));
          }
          FileList->AddFile(File);
          Total++;

          if (Total % 10 == 0)
          {
            FTerminal->DoReadDirectoryProgress(Total, isEOF);
            if (isEOF)
            {
              FTerminal->DoReadDirectoryProgress(-2, isEOF);
            }
          }
        }

        if (Count == 0)
        {
          FTerminal->LogEvent(L"Empty directory listing packet. Aborting directory reading.");
          isEOF = true;
        }
        TRACE("6");
      }
      else if (Response.GetType() == SSH_FXP_STATUS)
      {
        TRACE("7");
        isEOF = (GotStatusPacket(&Response, asEOF) == SSH_FX_EOF);
      }
      else
      {
        TRACE("8");
        FTerminal->FatalError(NULL, FMTLOAD(SFTP_INVALID_TYPE, static_cast<int>(Response.GetType())));
      }
    }
    while (!isEOF);

    TRACE("9");
    if (Total == 0)
    {
      TRACE("10");
      bool Failure = false;
      // no point reading parent of root directory,
      // moreover CompleteFTP terminates session upon attempt to do so
      if (IsUnixRootPath(FileList->GetDirectory()))
      {
        File = NULL;
      }
      else
      {
        // Empty file list -> probably "permission denied", we
        // at least get link to parent directory ("..")
        try
        {
          FTerminal->SetExceptionOnFail(true);
          TRY_FINALLY (
          {
            File = NULL;
            FTerminal->ReadFile(
              UnixIncludeTrailingBackslash(FileList->GetDirectory()) + PARENTDIRECTORY, File);
          }
          ,
          {
            FTerminal->SetExceptionOnFail(false);
          }
          );
        }
        catch(Exception &E)
        {
          TRACE("11");
          if ((dynamic_cast<EFatal *>(&E) != NULL))
          {
            throw;
          }
          else
          {
            File = NULL;
            Failure = true;
          }
        }
      }

      // on some systems even getting ".." fails, we create dummy ".." instead
      if (File == NULL)
      {
        TRACE("12");
        File = new TRemoteParentDirectory(FTerminal);
      }

      assert(File && File->GetIsParentDirectory());
      FileList->AddFile(File);

      if (Failure)
      {
        TRACE("13");
        throw Exception(FMTLOAD(EMPTY_DIRECTORY, FileList->GetDirectory().c_str()));
      }
    }
  }
  ,
  {
    TRACE("14");
    if (FTerminal->GetActive())
    {
      TRACE("15");
      Packet.ChangeType(SSH_FXP_CLOSE);
      Packet.AddString(Handle);
      SendPacket(&Packet);
      // we are not interested in the response, do not wait for it
      ReserveResponse(&Packet, NULL);
      TRACE("16");
    }
  }
  );
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::ReadSymlink(TRemoteFile * SymlinkFile,
  TRemoteFile *& File)
{
  assert(SymlinkFile && SymlinkFile->GetIsSymLink());
  assert(FVersion >= 3); // symlinks are supported with SFTP version 3 and later

  // need to use full filename when resolving links within subdirectory
  // (i.e. for download)
  UnicodeString FileName = LocalCanonify(
    SymlinkFile->GetDirectory() != NULL ? SymlinkFile->GetFullFileName() : SymlinkFile->GetFileName());

  TSFTPPacket ReadLinkPacket(SSH_FXP_READLINK, GetSessionData()->GetCodePageAsNumber());
  ReadLinkPacket.AddPathString(FileName, FUtfStrings);
  SendPacket(&ReadLinkPacket);
  ReserveResponse(&ReadLinkPacket, &ReadLinkPacket);

  // send second request before reading response to first one
  // (performance benefit)
  TSFTPPacket AttrsPacket(SSH_FXP_STAT, GetSessionData()->GetCodePageAsNumber());
  AttrsPacket.AddPathString(FileName, FUtfStrings);
  if (FVersion >= 4)
  {
    AttrsPacket.AddCardinal(SSH_FILEXFER_ATTR_COMMON);
  }
  SendPacket(&AttrsPacket);
  ReserveResponse(&AttrsPacket, &AttrsPacket);

  ReceiveResponse(&ReadLinkPacket, &ReadLinkPacket, SSH_FXP_NAME);
  if (ReadLinkPacket.GetCardinal() != 1)
  {
    FTerminal->FatalError(NULL, LoadStr(SFTP_NON_ONE_FXP_NAME_PACKET));
  }
  SymlinkFile->SetLinkTo(ReadLinkPacket.GetPathString(FUtfStrings));

  ReceiveResponse(&AttrsPacket, &AttrsPacket, SSH_FXP_ATTRS);
  // SymlinkFile->FileName was used instead SymlinkFile->LinkTo before, why?
  File = LoadFile(&AttrsPacket, SymlinkFile,
    UnixExtractFileName(SymlinkFile->GetLinkTo()));
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::ReadFile(const UnicodeString & FileName,
  TRemoteFile *& File)
{
  CustomReadFile(FileName, File, SSH_FXP_LSTAT);
}
//---------------------------------------------------------------------------
bool TSFTPFileSystem::RemoteFileExists(const UnicodeString & FullPath,
  TRemoteFile ** File)
{
  bool Result;
  try
  {
    TRemoteFile * AFile;
    CustomReadFile(FullPath, AFile, SSH_FXP_LSTAT, NULL, asNoSuchFile);
    Result = (AFile != NULL);
    if (Result)
    {
      if (File)
      {
        *File = AFile;
      }
      else
      {
        delete AFile;
      }
    }
  }
  catch(...)
  {
    if (!FTerminal->GetActive())
    {
      throw;
    }
    Result = false;
  }
  return Result;
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::SendCustomReadFile(TSFTPPacket * Packet,
  TSFTPPacket * Response, unsigned long Flags)
{
  if (FVersion >= 4)
  {
    Packet->AddCardinal(Flags);
  }
  SendPacket(Packet);
  ReserveResponse(Packet, Response);
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::CustomReadFile(const UnicodeString & FileName,
  TRemoteFile *& File, unsigned char Type, TRemoteFile * ALinkedByFile,
  int AllowStatus)
{
  unsigned long Flags = SSH_FILEXFER_ATTR_SIZE | SSH_FILEXFER_ATTR_PERMISSIONS |
    SSH_FILEXFER_ATTR_ACCESSTIME | SSH_FILEXFER_ATTR_MODIFYTIME |
    SSH_FILEXFER_ATTR_OWNERGROUP;
  TSFTPPacket Packet(Type, GetSessionData()->GetCodePageAsNumber());
  Packet.AddPathString(LocalCanonify(FileName), FUtfStrings);
  SendCustomReadFile(&Packet, &Packet, Flags);
  ReceiveResponse(&Packet, &Packet, SSH_FXP_ATTRS, AllowStatus);

  if (Packet.GetType() == SSH_FXP_ATTRS)
  {
    File = LoadFile(&Packet, ALinkedByFile, UnixExtractFileName(FileName));
  }
  else
  {
    assert(AllowStatus > 0);
    File = NULL;
  }
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::DoDeleteFile(const UnicodeString & FileName, unsigned char Type)
{
  TSFTPPacket Packet(Type, GetSessionData()->GetCodePageAsNumber());
  UnicodeString RealFileName = LocalCanonify(FileName);
  Packet.AddPathString(RealFileName, FUtfStrings);
  SendPacketAndReceiveResponse(&Packet, &Packet, SSH_FXP_STATUS);
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::DeleteFile(const UnicodeString & FileName,
  const TRemoteFile * File, int Params, TRmSessionAction & Action)
{
  unsigned char Type;
  if (File && File->GetIsDirectory() && !File->GetIsSymLink())
  {
    if (FLAGCLEAR(Params, dfNoRecursive))
    {
      try
      {
        FTerminal->ProcessDirectory(FileName, MAKE_CALLBACK(TTerminalIntf::DeleteFile, FTerminal), &Params);
      }
      catch(...)
      {
        Action.Cancel();
        throw;
      }
    }
    Type = SSH_FXP_RMDIR;
  }
  else
  {
    Type = SSH_FXP_REMOVE;
  }

  DoDeleteFile(FileName, Type);
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::RenameFile(const UnicodeString & FileName,
  const UnicodeString & NewName)
{
  TSFTPPacket Packet(SSH_FXP_RENAME, GetSessionData()->GetCodePageAsNumber());
  UnicodeString RealName = LocalCanonify(FileName);
  Packet.AddPathString(RealName, FUtfStrings);
  UnicodeString TargetName;
  if (UnixExtractFilePath(NewName).IsEmpty())
  {
    // rename case (TTerminalIntf::RenameFile)
    TargetName = UnixExtractFilePath(RealName) + NewName;
  }
  else
  {
    TargetName = LocalCanonify(NewName);
  }
  Packet.AddPathString(TargetName, FUtfStrings);
  if (FVersion >= 5)
  {
    Packet.AddCardinal(0);
  }
  SendPacketAndReceiveResponse(&Packet, &Packet, SSH_FXP_STATUS);
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::CopyFile(const UnicodeString & /*FileName*/,
  const UnicodeString & /*NewName*/)
{
  assert(false);
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::CreateDirectory(const UnicodeString & DirName)
{
  CALLSTACK;
  TSFTPPacket Packet(SSH_FXP_MKDIR, GetSessionData()->GetCodePageAsNumber());
  UnicodeString CanonifiedName = Canonify(DirName);
  Packet.AddPathString(CanonifiedName, FUtfStrings);
  Packet.AddProperties(NULL, 0, true, FVersion, FUtfStrings, NULL);
  SendPacketAndReceiveResponse(&Packet, &Packet, SSH_FXP_STATUS);
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::CreateLink(const UnicodeString & FileName,
  const UnicodeString & PointTo, bool Symbolic)
{
  USEDPARAM(Symbolic);
  assert(Symbolic); // only symlinks are supported by SFTP
  assert(FVersion >= 3); // symlinks are supported with SFTP version 3 and later
  TSFTPPacket Packet(SSH_FXP_SYMLINK, GetSessionData()->GetCodePageAsNumber());

  bool Buggy = (GetSessionData()->GetSFTPBug(sbSymlink) == asOn) ||
    ((GetSessionData()->GetSFTPBug(sbSymlink) == asAuto) && FOpenSSH);

  if (!Buggy)
  {
    Packet.AddPathString(Canonify(FileName), FUtfStrings);
    Packet.AddPathString(PointTo, FUtfStrings);
  }
  else
  {
    FTerminal->LogEvent(L"We believe the server has SFTP symlink bug");
    Packet.AddPathString(PointTo, FUtfStrings);
    Packet.AddPathString(Canonify(FileName), FUtfStrings);
  }
  SendPacketAndReceiveResponse(&Packet, &Packet, SSH_FXP_STATUS);
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::ChangeFileProperties(const UnicodeString & FileName,
  const TRemoteFile * /*File*/, const TRemoteProperties * AProperties,
  TChmodSessionAction & Action)
{
  CALLSTACK;
  assert(AProperties != NULL);

  TRemoteFile * File;

  UnicodeString RealFileName = LocalCanonify(FileName);
  ReadFile(RealFileName, File);
  assert(File);
  TRY_FINALLY (
  {
    if (File->GetIsDirectory() && !File->GetIsSymLink() && AProperties->Recursive)
    {
      try
      {
        FTerminal->ProcessDirectory(FileName, MAKE_CALLBACK(TTerminalIntf::ChangeFileProperties, FTerminal),
          static_cast<void *>(const_cast<TRemoteProperties *>(AProperties)));
      }
      catch(...)
      {
        Action.Cancel();
        throw;
      }
    }

    // SFTP can change owner and group at the same time only, not individually.
    // Fortunatelly we know current owner/group, so if only one is present,
    // we can supplement the other.
    TRemoteProperties Properties(*AProperties);
    if (Properties.Valid.Contains(vpGroup) &&
        !Properties.Valid.Contains(vpOwner))
    {
      Properties.Owner = File->GetFileOwner();
      Properties.Valid << vpOwner;
    }
    else if (Properties.Valid.Contains(vpOwner) &&
             !Properties.Valid.Contains(vpGroup))
    {
      Properties.Group = File->GetFileGroup();
      Properties.Valid << vpGroup;
    }

    TRACEFMT("Rights [%s]", File->GetRights()->GetText().c_str());
    TSFTPPacket Packet(SSH_FXP_SETSTAT, GetSessionData()->GetCodePageAsNumber());
    Packet.AddPathString(RealFileName, FUtfStrings);
    Packet.AddProperties(&Properties, *File->GetRights(), File->GetIsDirectory(), FVersion, FUtfStrings, &Action);
    SendPacketAndReceiveResponse(&Packet, &Packet, SSH_FXP_STATUS);
  }
  ,
  {
    delete File;
  }
  );
}
//---------------------------------------------------------------------------
bool TSFTPFileSystem::LoadFilesProperties(TStrings * FileList)
{
  bool Result = false;
  // without knowledge of server's capabilities, this all make no sense
  if (FSupport->Loaded)
  {
    TFileOperationProgressType Progress(MAKE_CALLBACK(TTerminalIntf::DoProgress, FTerminal), MAKE_CALLBACK(TTerminalIntf::DoFinished, FTerminal));
    Progress.Start(foGetProperties, osRemote, FileList->Count);

    FTerminal->SetOperationProgress(&Progress); //-V506

    static int LoadFilesPropertiesQueueLen = 5;
    TSFTPLoadFilesPropertiesQueue Queue(this, GetSessionData()->GetCodePageAsNumber());
    TRY_FINALLY (
    {
      if (Queue.Init(LoadFilesPropertiesQueueLen, FileList))
      {
        TRemoteFile * File;
        TSFTPPacket Packet(GetSessionData()->GetCodePageAsNumber());
        bool Next;
        do
        {
          Next = Queue.ReceivePacket(&Packet, File);
          assert((Packet.GetType() == SSH_FXP_ATTRS) || (Packet.GetType() == SSH_FXP_STATUS));
          if (Packet.GetType() == SSH_FXP_ATTRS)
          {
            assert(File != NULL);
            Progress.SetFile(File->GetFileName());
            LoadFile(File, &Packet);
            Result = true;
            TOnceDoneOperation OnceDoneOperation;
            Progress.Finish(File->GetFileName(), true, OnceDoneOperation);
          }

          if (Progress.Cancel != csContinue)
          {
            Next = false;
          }
        }
        while (Next);
      }
    }
    ,
    {
      Queue.DisposeSafe();
      FTerminal->SetOperationProgress(NULL);
      Progress.Stop();
    }
    );
    // queue is discarded here
  }

  return Result;
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::DoCalculateFilesChecksum(const UnicodeString & Alg,
  TStrings * FileList, TStrings * Checksums,
  TCalculatedChecksumEvent OnCalculatedChecksum,
  TFileOperationProgressType * OperationProgress, bool FirstLevel)
{
  CALLSTACK;
  TOnceDoneOperation OnceDoneOperation; // not used

  // recurse into subdirectories only if we have callback function
  if (OnCalculatedChecksum != NULL)
  {
    for (int Index = 0; Index < FileList->Count; Index++)
    {
      TRemoteFile * File = static_cast<TRemoteFile *>(FileList->Objects[Index]);
      assert(File != NULL);
      if (File->GetIsDirectory() && !File->GetIsSymLink() &&
          !File->GetIsParentDirectory() && !File->GetIsThisDirectory())
      {
        OperationProgress->SetFile(File->GetFileName());
        TRemoteFileList * SubFiles =
          FTerminal->CustomReadDirectoryListing(File->GetFullFileName(), false);

        if (SubFiles != NULL)
        {
          TStrings * SubFileList = new TStringList();
          bool Success = false;
          TRY_FINALLY (
          {
            OperationProgress->SetFile(File->GetFileName());

            for (int Index = 0; Index < SubFiles->Count; Index++)
            {
              TRemoteFile * SubFile = SubFiles->GetFiles(Index);
              SubFileList->AddObject(SubFile->GetFullFileName(), SubFile);
            }

            // do not collect checksums for files in subdirectories,
            // only send back checksums via callback
            DoCalculateFilesChecksum(Alg, SubFileList, NULL,
              OnCalculatedChecksum, OperationProgress, false);

            Success = true;
          }
          ,
          {
            delete SubFiles;
            delete SubFileList;

            if (FirstLevel)
            {
              OperationProgress->Finish(File->GetFileName(), Success, OnceDoneOperation);
            }
          }
          );
        }
      }
    }
  }

  static int CalculateFilesChecksumQueueLen = 5;
  TSFTPCalculateFilesChecksumQueue Queue(this, GetSessionData()->GetCodePageAsNumber());
  TRY_FINALLY (
  {
    if (Queue.Init(CalculateFilesChecksumQueueLen, Alg, FileList))
    {
      TSFTPPacket Packet(GetSessionData()->GetCodePageAsNumber());
      bool Next = false;
      do
      {
        bool Success = false;
        UnicodeString Alg;
        UnicodeString Checksum;
        TRemoteFile * File = NULL;

        TRY_FINALLY (
        {
          try
          {
            Next = Queue.ReceivePacket(&Packet, File);
            assert(Packet.GetType() == SSH_FXP_EXTENDED_REPLY);

            OperationProgress->SetFile(File->GetFileName());

            Alg = Packet.GetAnsiString();
            Checksum = BytesToHex(reinterpret_cast<const unsigned char*>(Packet.GetNextData(Packet.GetRemainingLength())), Packet.GetRemainingLength());
            OnCalculatedChecksum(File->GetFileName(), Alg, Checksum);

            Success = true;
          }
          catch (Exception & E)
          {
            FTerminal->CommandError(&E, FMTLOAD(CHECKSUM_ERROR,
              File != NULL ? File->GetFullFileName().c_str() : UnicodeString(L"")));
            // TODO: retries? resume?
            Next = false;
          }

          if (Checksums != NULL)
          {
            Checksums->Add(L"");
          }
        }
        ,
        {
          if (FirstLevel)
          {
            OperationProgress->Finish(File->GetFileName(), Success, OnceDoneOperation);
          }
        }
        );

        if (OperationProgress->Cancel != csContinue)
        {
          Next = false;
        }
      }
      while (Next);
    }
  }
  ,
  {
    Queue.DisposeSafe();
  }
  );
  // queue is discarded here
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::CalculateFilesChecksum(const UnicodeString & Alg,
  TStrings * FileList, TStrings * Checksums,
  TCalculatedChecksumEvent OnCalculatedChecksum)
{
  TFileOperationProgressType Progress(MAKE_CALLBACK(TTerminalIntf::DoProgress, FTerminal), MAKE_CALLBACK(TTerminalIntf::DoFinished, FTerminal));
  Progress.Start(foCalculateChecksum, osRemote, FileList->Count);

  FTerminal->SetOperationProgress(&Progress); //-V506

  TRY_FINALLY (
  {
    DoCalculateFilesChecksum(Alg, FileList, Checksums, OnCalculatedChecksum,
      &Progress, true);
  }
  ,
  {
    FTerminal->SetOperationProgress(NULL);
    Progress.Stop();
  }
  );
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::CustomCommandOnFile(const UnicodeString & /* FileName */,
  const TRemoteFile * /* File */, const UnicodeString & /* Command */, int /* Params */,
  TCaptureOutputEvent /* OutputEvent */)
{
  assert(false);
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::AnyCommand(const UnicodeString & /*Command*/,
  TCaptureOutputEvent /* OutputEvent */)
{
  assert(false);
}
//---------------------------------------------------------------------------
UnicodeString TSFTPFileSystem::FileUrl(const UnicodeString & FileName)
{
  return FTerminal->FileUrl(L"sftp", FileName);
}
//---------------------------------------------------------------------------
TStrings * TSFTPFileSystem::GetFixedPaths()
{
  return FFixedPaths;
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::SpaceAvailable(const UnicodeString & Path,
  TSpaceAvailable & ASpaceAvailable)
{
//!CLEANBEGIN
  #ifdef CLEAN_SPACE_AVAILABLE
  ASpaceAvailable.BytesOnDevice = 144304308224LL;
  ASpaceAvailable.UnusedBytesOnDevice = 114290327554LL;
  ASpaceAvailable.BytesAvailableToUser = 0;
  ASpaceAvailable.UnusedBytesAvailableToUser = 114290327554LL;
  ASpaceAvailable.BytesPerAllocationUnit = 0;
  #else
//!CLEANEND
  TSFTPPacket Packet(SSH_FXP_EXTENDED, GetSessionData()->GetCodePageAsNumber());
  Packet.AddString(SFTP_EXT_SPACE_AVAILABLE);
  Packet.AddPathString(LocalCanonify(Path), FUtfStrings);
  SendPacketAndReceiveResponse(&Packet, &Packet, SSH_FXP_EXTENDED_REPLY);
  ASpaceAvailable.BytesOnDevice = Packet.GetInt64();
  ASpaceAvailable.UnusedBytesOnDevice = Packet.GetInt64();
  ASpaceAvailable.BytesAvailableToUser = Packet.GetInt64();
  ASpaceAvailable.UnusedBytesAvailableToUser = Packet.GetInt64();
  ASpaceAvailable.BytesPerAllocationUnit = Packet.GetCardinal();
//!CLEANBEGIN
  #endif
//!CLEANEND
}
//---------------------------------------------------------------------------
// transfer protocol
//---------------------------------------------------------------------------
void TSFTPFileSystem::CopyToRemote(TStrings * FilesToCopy,
  const UnicodeString & TargetDir, const TCopyParamType * CopyParam,
  int Params, TFileOperationProgressType * OperationProgress,
  TOnceDoneOperation & OnceDoneOperation)
{
  CALLSTACK;
  assert(FilesToCopy && OperationProgress);

  UnicodeString FileName, FileNameOnly;
  UnicodeString FullTargetDir = UnixIncludeTrailingBackslash(TargetDir);
  int Index = 0;
  while (Index < FilesToCopy->Count && !OperationProgress->Cancel)
  {
    bool Success = false;
    FileName = FilesToCopy->Strings[Index];
    TRemoteFile * File = dynamic_cast<TRemoteFile *>(FilesToCopy->Objects[Index]);
    UnicodeString RealFileName = File ? File->GetFileName() : FileName;
    FileNameOnly = ExtractFileName(RealFileName, false);
    TRACEFMT("FileName [%s]", FileName.c_str());
    assert(!FAvoidBusy);
    FAvoidBusy = true;

    TRY_FINALLY (
    {
      try
      {
        if (GetSessionData()->GetCacheDirectories())
        {
          FTerminal->DirectoryModified(TargetDir, false);

          if (DirectoryExists(ExtractFilePath(FileName)))
          {
            FTerminal->DirectoryModified(UnixIncludeTrailingBackslash(TargetDir) +
              FileNameOnly, true);
          }
        }
        SFTPSourceRobust(FileName, File, FullTargetDir, CopyParam, Params, OperationProgress,
          tfFirstLevel);
        Success = true;
      }
      catch(EScpSkipFile & E)
      {
        TRACE("1");
        DEBUG_PRINTF(L"before FTerminal->HandleException");
        SUSPEND_OPERATION (
          if (!FTerminal->HandleException(&E))
          {
            throw;
          }
        );
      }
    }
    ,
    {
      TRACE("3");
      FAvoidBusy = false;
      OperationProgress->Finish(RealFileName, Success, OnceDoneOperation);
    }
    );
    Index++;
  }
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::SFTPConfirmOverwrite(UnicodeString & FileName,
  int Params, TFileOperationProgressType * OperationProgress,
  TOverwriteMode & OverwriteMode, const TOverwriteFileParams * FileParams)
{
  CALLSTACK;
  bool CanAppend = (FVersion < 4) || !OperationProgress->AsciiTransfer;
  bool CanResume =
    (FileParams != NULL) &&
    (FileParams->DestSize < FileParams->SourceSize);
  unsigned int Answer = 0;
  SUSPEND_OPERATION
  (
    // abort = "append"
    // retry = "resume"
    // all = "yes to newer"
    // ignore = "rename"
    int Answers = qaYes | qaNo | qaCancel | qaYesToAll | qaNoToAll | qaAll | qaIgnore;

    // possibly we can allow alternate resume at least in some cases
    if (CanAppend)
    {
      Answers |= qaAbort | qaRetry;
    }
    TQueryButtonAlias Aliases[4];
    Aliases[0].Button = qaAbort;
    Aliases[0].Alias = LoadStr(APPEND_BUTTON);
    Aliases[1].Button = qaRetry;
    Aliases[1].Alias = LoadStr(RESUME_BUTTON);
    Aliases[2].Button = qaAll;
    Aliases[2].Alias = LoadStr(YES_TO_NEWER_BUTTON);
    Aliases[3].Button = qaIgnore;
    Aliases[3].Alias = LoadStr(RENAME_BUTTON);
    TQueryParams QueryParams(qpNeverAskAgainCheck);
    QueryParams.NoBatchAnswers = qaIgnore | qaAbort | qaRetry | qaAll;
    QueryParams.Aliases = Aliases;
    QueryParams.AliasesCount = LENOF(Aliases);
    Answer = FTerminal->ConfirmFileOverwrite(FileName, FileParams,
      Answers, &QueryParams,
      OperationProgress->Side == osLocal ? osRemote : osLocal,
      Params, OperationProgress);
  );

  if (CanAppend &&
      ((Answer == qaAbort) || (Answer == qaRetry) || (Answer == qaSkip)))
  {
    switch (Answer)
    {
      // append
      case qaAbort:
        Params |= cpAppend;
        break;
        // resume
      case qaRetry:
        Params |= cpResume;
        break;
    }
    // duplicated in TTerminalIntf::ConfirmFileOverwrite
    bool CanAlternateResume =
      FileParams ? (FileParams->DestSize < FileParams->SourceSize) && !OperationProgress->AsciiTransfer : false;
    TBatchOverwrite BatchOverwrite =
      FTerminal->EffectiveBatchOverwrite(Params, OperationProgress, true);
    // when mode is forced by batch, never query user
    if (BatchOverwrite == boAppend)
    {
      OverwriteMode = omAppend;
    }
    else if (CanAlternateResume && CanResume &&
             ((BatchOverwrite == boResume) || (BatchOverwrite == boAlternateResume)))
    {
      OverwriteMode = omResume;
    }
    // no other option, but append
    else if (!CanAlternateResume)
    {
      OverwriteMode = omAppend;
    }
    else
    {
      TQueryParams Params(0, HELP_APPEND_OR_RESUME);
      SUSPEND_OPERATION
      (
        Answer = FTerminal->QueryUser(FORMAT(LoadStr(APPEND_OR_RESUME).c_str(), FileName.c_str()),
          NULL, qaYes | qaNo | qaNoToAll | qaCancel, &Params);
      );

      switch (Answer)
      {
        case qaYes:
          OverwriteMode = omAppend;
          break;

        case qaNo:
          OverwriteMode = omResume;
          break;

        case qaNoToAll:
          OverwriteMode = omResume;
          OperationProgress->BatchOverwrite = boAlternateResume;
          break;

        default: assert(false); //fallthru
        case qaCancel:
          if (!OperationProgress->Cancel)
          {
            OperationProgress->Cancel = csCancel;
          }
          Abort();
          break;
      }
    }
  }
  else if (Answer == qaIgnore)
  {
    if (FTerminal->PromptUser(FTerminal->GetSessionData(), pkFileName, LoadStr(RENAME_TITLE), L"",
          LoadStr(RENAME_PROMPT2), true, 0, FileName))
    {
      OverwriteMode = omOverwrite;
    }
    else
    {
      if (!OperationProgress->Cancel)
      {
        OperationProgress->Cancel = csCancel;
      }
      Abort();
    }
  }
  else
  {
    OverwriteMode = omOverwrite;
    switch (Answer)
    {
      case qaCancel:
        if (!OperationProgress->Cancel)
        {
          OperationProgress->Cancel = csCancel;
        }
        Abort();
        break;

      case qaNo:
        THROW_SKIP_FILE_NULL;
    }
  }
}
//---------------------------------------------------------------------------
bool TSFTPFileSystem::SFTPConfirmResume(const UnicodeString & DestFileName,
  bool PartialBiggerThanSource, TFileOperationProgressType * OperationProgress)
{
  bool ResumeTransfer = false;
  assert(OperationProgress);
  if (PartialBiggerThanSource)
  {
    unsigned int Answer = 0;
    SUSPEND_OPERATION
    (
      TQueryParams Params(qpAllowContinueOnError, HELP_PARTIAL_BIGGER_THAN_SOURCE);
      Answer = FTerminal->QueryUser(
        FMTLOAD(PARTIAL_BIGGER_THAN_SOURCE, DestFileName.c_str()), NULL,
          qaOK | qaAbort, &Params, qtWarning);
    )

    if (Answer == qaAbort)
    {
      if (!OperationProgress->Cancel)
      {
        OperationProgress->Cancel = csCancel;
      }
      Abort();
    }
    ResumeTransfer = false;
  }
  else if (FTerminal->GetConfiguration()->GetConfirmResume())
  {
    unsigned int Answer = 0;
    SUSPEND_OPERATION
    (
      TQueryParams Params(qpAllowContinueOnError | qpNeverAskAgainCheck,
        HELP_RESUME_TRANSFER);
      // "abort" replaced with "cancel" to unify with "append/resume" query
      Answer = FTerminal->QueryUser(
        FMTLOAD(RESUME_TRANSFER, DestFileName.c_str()), NULL, qaYes | qaNo | qaCancel,
        &Params);
    );

    switch (Answer) {
      case qaNeverAskAgain:
        FTerminal->GetConfiguration()->SetConfirmResume(false);
      case qaYes:
        ResumeTransfer = true;
        break;

      case qaNo:
        ResumeTransfer = false;
        break;

      case qaCancel:
        if (!OperationProgress->Cancel)
        {
          OperationProgress->Cancel = csCancel;
        }
        Abort();
        break;
    }
  }
  else
  {
    ResumeTransfer = true;
  }
  return ResumeTransfer;
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::SFTPSourceRobust(const UnicodeString & FileName,
  const TRemoteFile * File,
  const UnicodeString & TargetDir, const TCopyParamType * CopyParam, int Params,
  TFileOperationProgressType * OperationProgress, unsigned int Flags)
{
  CALLSTACK;
  // the same in TFTPFileSystem
  bool Retry;

  TUploadSessionAction Action(FTerminal->GetActionLog());
  TOpenRemoteFileParams OpenParams;
  OpenParams.OverwriteMode = omOverwrite;
  TOverwriteFileParams FileParams;

  do
  {
    Retry = false;
    bool ChildError = false;
    try
    {
      SFTPSource(FileName, File, TargetDir, CopyParam, Params,
        OpenParams, FileParams,
        OperationProgress,
        Flags, Action, ChildError);
    }
    catch(Exception & E)
    {
      TRACE("1");
      Retry = true;
      if (FTerminal->GetActive() ||
          !FTerminal->QueryReopen(&E, ropNoReadDirectory, OperationProgress))
      {
        if (!ChildError)
        {
          FTerminal->RollbackAction(Action, OperationProgress, &E);
        }
        TRACE("2");
        throw;
      }
      TRACE("3");
    }

    if (Retry)
    {
      OperationProgress->RollbackTransfer();
      Action.Restart();
      // prevent overwrite and resume confirmations
      // (should not be set for directories!)
      Params |= cpNoConfirmation;
      // enable resume even if we are uploading into new directory
      Flags &= ~tfNewDirectory;
    }
  }
  while (Retry);
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::SFTPSource(const UnicodeString & FileName,
  const TRemoteFile * File,
  const UnicodeString & TargetDir, const TCopyParamType * CopyParam, int Params,
  TOpenRemoteFileParams & OpenParams,
  TOverwriteFileParams & FileParams,
  TFileOperationProgressType * OperationProgress, unsigned int Flags,
  TUploadSessionAction & Action, bool & ChildError)
{
  CALLSTACK;
  UnicodeString RealFileName = File ? File->GetFileName() : FileName;
  FTerminal->LogEvent(FORMAT(L"File: \"%s\"", RealFileName.c_str()));

  TRACE("1");
  Action.FileName(ExpandUNCFileName(RealFileName));

  TRACE("2");
  OperationProgress->SetFile(RealFileName, false);

  if (!FTerminal->AllowLocalFileTransfer(FileName, CopyParam))
  {
    TRACE("2a");
    FTerminal->LogEvent(FORMAT(L"File \"%s\" excluded from transfer", RealFileName.c_str()));
    THROW_SKIP_FILE_NULL;
  }

  TRACE("3");
  // TOpenRemoteFileParams OpenParams;
  // OpenParams.OverwriteMode = omOverwrite;

  HANDLE FileHandle = 0;
  __int64 MTime = 0, ATime = 0;
  __int64 Size = 0;

  TRACE("4");
  FTerminal->OpenLocalFile(FileName, GENERIC_READ, &OpenParams.LocalFileAttrs,
    &FileHandle, NULL, &MTime, &ATime, &Size);

  bool Dir = FLAGSET(OpenParams.LocalFileAttrs, faDirectory);

  TRY_FINALLY ( //-V614
  {
    TRACE("7");
    OperationProgress->SetFileInProgress();

    if (Dir)
    {
      TRACE("8");
      Action.Cancel();
      SFTPDirectorySource(IncludeTrailingBackslash(FileName), TargetDir,
        OpenParams.LocalFileAttrs, CopyParam, Params, OperationProgress, Flags);
    }
    else
    {
      TRACE("9");
      // File is regular file (not directory)
      assert(FileHandle);

      UnicodeString DestFileName = CopyParam->ChangeFileName(ExtractFileName(RealFileName, false),
        osLocal, FLAGSET(Flags, tfFirstLevel));
      UnicodeString DestFullName = LocalCanonify(TargetDir + DestFileName);
      UnicodeString DestPartialFullName;
      bool ResumeAllowed;
      bool ResumeTransfer = false;
      bool DestFileExists = false;
      TRights DestRights;

      __int64 ResumeOffset = 0;

      FTerminal->LogEvent(FORMAT(L"Copying \"%s\" to remote directory started.", RealFileName.c_str()));

      OperationProgress->SetLocalSize(Size);

      // Suppose same data size to transfer as to read
      // (not true with ASCII transfer)
      OperationProgress->SetTransferSize(OperationProgress->LocalSize);
      OperationProgress->TransferingFile = false;

      TDateTime Modification = UnixToDateTime(MTime, GetSessionData()->GetDSTMode());

      // Will we use ASCII of BINARY file tranfer?
      TFileMasks::TParams MaskParams;
      MaskParams.Size = Size;
      MaskParams.Modification = Modification;
      OperationProgress->SetAsciiTransfer(
        CopyParam->UseAsciiTransfer(RealFileName, osLocal, MaskParams));
      FTerminal->LogEvent(
        UnicodeString((OperationProgress->AsciiTransfer ? L"Ascii" : L"Binary")) +
          L" transfer mode selected.");

      // should we check for interrupted transfer?
      ResumeAllowed = !OperationProgress->AsciiTransfer &&
        CopyParam->AllowResume(OperationProgress->LocalSize) &&
        IsCapable(fcRename);
      OperationProgress->SetResumeStatus(ResumeAllowed ? rsEnabled : rsDisabled);

      // TOverwriteFileParams FileParams;
      FileParams.SourceSize = OperationProgress->LocalSize;
      FileParams.SourceTimestamp = Modification;

      if (ResumeAllowed)
      {
        DestPartialFullName = DestFullName + FTerminal->GetConfiguration()->GetPartialExt();

        if (FLAGCLEAR(Flags, tfNewDirectory))
        {
          FTerminal->LogEvent(L"Checking existence of file.");
          TRemoteFile * File = NULL;
          DestFileExists = RemoteFileExists(DestFullName, &File);
          if (DestFileExists)
          {
            OpenParams.DestFileSize = File->GetSize();
            FileParams.DestSize = OpenParams.DestFileSize;
            FileParams.DestTimestamp = File->GetModification();
            DestRights = *File->GetRights();
            // if destination file is symlink, never do resumable transfer,
            // as it would delete the symlink.
            // also bit of heuristics to detect symlink on SFTP-3 and older
            // (which does not indicate symlink in SSH_FXP_ATTRS).
            // if file has all permissions and is small, then it is likely symlink.
            // also it is not likely that such a small file (if it is not symlink)
            // gets overwritten by large file (that would trigger resumable transfer).
            if (File->GetIsSymLink() ||
                ((FVersion < 4) &&
                 ((*File->GetRights() & static_cast<unsigned short>(TRights::rfAll)) == static_cast<unsigned short>(TRights::rfAll)) &&
                 (File->GetSize() < 100)))
            {
              ResumeAllowed = false;
              OperationProgress->SetResumeStatus(rsDisabled);
            }

            delete File;
            File = NULL;
          }

          if (ResumeAllowed)
          {
            FTerminal->LogEvent(L"Checking existence of partially transfered file.");
            if (RemoteFileExists(DestPartialFullName, &File))
            {
              ResumeOffset = File->GetSize();
              delete File;
              File = NULL;

              bool PartialBiggerThanSource = (ResumeOffset > OperationProgress->LocalSize);
              if (FLAGCLEAR(Params, cpNoConfirmation) &&
                  FLAGCLEAR(Params, cpResume))
              {
                ResumeTransfer = SFTPConfirmResume(DestFileName,
                  PartialBiggerThanSource, OperationProgress);
              }
              else
              {
                ResumeTransfer = !PartialBiggerThanSource;
              }

              if (!ResumeTransfer)
              {
                DoDeleteFile(DestPartialFullName, SSH_FXP_REMOVE);
              }
              else
              {
                FTerminal->LogEvent(L"Resuming file transfer.");
              }
            }
            else
            {
              // partial upload file does not exists, check for full file
              if (DestFileExists)
              {
                UnicodeString PrevDestFileName = DestFileName;
                SFTPConfirmOverwrite(DestFileName,
                  Params, OperationProgress, OpenParams.OverwriteMode, &FileParams);
                if (PrevDestFileName != DestFileName)
                {
                  // update paths in case user changes the file name
                  DestFullName = LocalCanonify(TargetDir + DestFileName);
                  DestPartialFullName = DestFullName + FTerminal->GetConfiguration()->GetPartialExt();
                  FTerminal->LogEvent(L"Checking existence of new file.");
                  DestFileExists = RemoteFileExists(DestFullName, NULL);
                }
              }
            }
          }
        }
      }

      // will the transfer be resumable?
      bool DoResume = (ResumeAllowed && (OpenParams.OverwriteMode == omOverwrite));

      UnicodeString RemoteFileName = DoResume ? DestPartialFullName : DestFullName;
      OpenParams.RemoteFileName = RemoteFileName;
      OpenParams.Resume = DoResume;
      OpenParams.Resuming = ResumeTransfer;
      OpenParams.OperationProgress = OperationProgress;
      OpenParams.CopyParam = CopyParam;
      OpenParams.Params = Params;
      OpenParams.FileParams = &FileParams;
      OpenParams.Confirmed = false;

      FTerminal->LogEvent(L"Opening remote file.");
      FTerminal->FileOperationLoop(MAKE_CALLBACK(TSFTPFileSystem::SFTPOpenRemote, this), OperationProgress, true,
        FMTLOAD(SFTP_CREATE_FILE_ERROR, OpenParams.RemoteFileName.c_str()),
        &OpenParams);

      if (OpenParams.RemoteFileName != RemoteFileName)
      {
        assert(!DoResume);
        assert(UnixExtractFilePath(OpenParams.RemoteFileName) == UnixExtractFilePath(RemoteFileName));
        DestFullName = OpenParams.RemoteFileName;
        UnicodeString NewFileName = UnixExtractFileName(DestFullName);
        assert(DestFileName != NewFileName);
        DestFileName = NewFileName;
      }

      Action.Destination(DestFullName);

      bool TransferFinished = false;
      __int64 DestWriteOffset = 0;
      TSFTPPacket CloseRequest(GetSessionData()->GetCodePageAsNumber());
      bool SetRights = ((DoResume && DestFileExists) || CopyParam->GetPreserveRights());
      bool SetProperties = (CopyParam->GetPreserveTime() || SetRights);
      TSFTPPacket PropertiesRequest(SSH_FXP_SETSTAT, GetSessionData()->GetCodePageAsNumber());
      TSFTPPacket PropertiesResponse(GetSessionData()->GetCodePageAsNumber());
      TRights Rights;
      if (SetProperties)
      {
        PropertiesRequest.AddPathString(DestFullName, FUtfStrings);
        if (CopyParam->GetPreserveRights())
        {
          Rights = CopyParam->RemoteFileRights(OpenParams.LocalFileAttrs);
        }
        else if (DoResume && DestFileExists)
        {
          Rights = DestRights;
        }
        else
        {
          assert(!SetRights);
        }

        unsigned short RightsNumber = Rights.GetNumberSet();
        PropertiesRequest.AddProperties(
          SetRights ? &RightsNumber : NULL, NULL, NULL,
          CopyParam->GetPreserveTime() ? &MTime : NULL,
          CopyParam->GetPreserveTime() ? &ATime : NULL,
          NULL, false, FVersion, FUtfStrings);
      }

      TRY_FINALLY (
      {
        if (OpenParams.OverwriteMode == omAppend)
        {
          FTerminal->LogEvent(L"Appending file.");
          DestWriteOffset = OpenParams.DestFileSize;
        }
        else if (ResumeTransfer || (OpenParams.OverwriteMode == omResume))
        {
          if (OpenParams.OverwriteMode == omResume)
          {
            FTerminal->LogEvent(L"Resuming file transfer (append style).");
            ResumeOffset = OpenParams.DestFileSize;
          }
          FileSeek(FileHandle, ResumeOffset, 0);
          OperationProgress->AddResumed(ResumeOffset);
        }

        TRACE("10");
        TSFTPUploadQueue Queue(this, GetSessionData()->GetCodePageAsNumber());
        TRY_FINALLY (
        {
          Queue.Init(FileName, FileHandle, OperationProgress,
            OpenParams.RemoteFileHandle,
            DestWriteOffset + OperationProgress->TransferedSize);

          TRACE("11");
          while (Queue.Continue())
          {
            if (OperationProgress->Cancel)
            {
              Abort();
            }
          }

          TRACE("12");
          // send close request before waiting for pending read responses
          SFTPCloseRemote(OpenParams.RemoteFileHandle, DestFileName,
            OperationProgress, false, true, &CloseRequest);
          OpenParams.RemoteFileHandle = L"";

          // when resuming is disabled, we can send "set properties"
          // request before waiting for pending read/close responses
          if (SetProperties && !DoResume)
          {
            SendPacket(&PropertiesRequest);
            ReserveResponse(&PropertiesRequest, &PropertiesResponse);
          }
          TRACE("13");
        }
        ,
        {
          Queue.DisposeSafe();
        }
        );

        TransferFinished = true;
        // queue is discarded here
      }
      ,
      {
        TRACE("14");
        if (FTerminal->GetActive())
        {
          // if file transfer was finished, the close request was already sent
          if (!OpenParams.RemoteFileHandle.IsEmpty())
          {
            SFTPCloseRemote(OpenParams.RemoteFileHandle, DestFileName,
              OperationProgress, TransferFinished, true, &CloseRequest);
          }
          // wait for the response
          SFTPCloseRemote(OpenParams.RemoteFileHandle, DestFileName,
            OperationProgress, TransferFinished, false, &CloseRequest);

          // delete file if transfer was not completed, resuming was not allowed and
          // we were not appending (incl. alternate resume),
          // shortly after plain transfer completes (eq. !ResumeAllowed)
          if (!TransferFinished && !DoResume && (OpenParams.OverwriteMode == omOverwrite))
          {
            DoDeleteFile(OpenParams.RemoteFileName, SSH_FXP_REMOVE);
          }
        }
      }
      );

      if (DoResume)
      {
        if (DestFileExists)
        {
          FILE_OPERATION_LOOP(FMTLOAD(DELETE_ON_RESUME_ERROR,
              UnixExtractFileName(DestFullName).c_str(), DestFullName.c_str()),

            if (GetSessionData()->GetOverwrittenToRecycleBin() &&
                !FTerminal->GetSessionData()->GetRecycleBinPath().IsEmpty())
            {
              FTerminal->RecycleFile(DestFullName, NULL);
            }
            else
            {
              DoDeleteFile(DestFullName, SSH_FXP_REMOVE);
            }
          );
        }

        // originally this was before CLOSE (last __finally statement),
        // on VShell it failed
        FILE_OPERATION_LOOP(FMTLOAD(RENAME_AFTER_RESUME_ERROR,
            UnixExtractFileName(OpenParams.RemoteFileName.c_str()).c_str(), DestFileName.c_str()),
          RenameFile(OpenParams.RemoteFileName, DestFileName);
        );
      }

      if (SetProperties)
      {
        std::auto_ptr<TTouchSessionAction> TouchAction;
        if (CopyParam->GetPreserveTime())
        {
          TouchAction.reset(new TTouchSessionAction(FTerminal->GetActionLog(), DestFullName,
            UnixToDateTime(MTime, GetSessionData()->GetDSTMode())));
        }
        std::auto_ptr<TChmodSessionAction> ChmodAction;
        // do record chmod only if it was explicitly requested,
        // not when it was implicitly performed to apply timestamp
        // of overwritten file to new file
        if (CopyParam->GetPreserveRights())
        {
          ChmodAction.reset(new TChmodSessionAction(FTerminal->GetActionLog(), DestFullName, Rights));
        }
        try
        {
          // when resuming is enabled, the set properties request was not sent yet
          if (DoResume)
          {
            SendPacket(&PropertiesRequest);
          }
          bool Resend = false;
          FILE_OPERATION_LOOP(FMTLOAD(PRESERVE_TIME_PERM_ERROR, DestFileName.c_str()),
            try
            {
              TSFTPPacket DummyResponse(GetSessionData()->GetCodePageAsNumber());
              TSFTPPacket * Response = &PropertiesResponse;
              if (Resend)
              {
                PropertiesRequest.Reuse();
                SendPacket(&PropertiesRequest);
                // ReceiveResponse currently cannot receive twice into same packet,
                // so DummyResponse is temporary workaround
                Response = &DummyResponse;
              }
              Resend = true;
              ReceiveResponse(&PropertiesRequest, Response, SSH_FXP_STATUS,
                asOK | FLAGMASK(CopyParam->GetIgnorePermErrors(), asPermDenied));
            }
            catch (...)
            {
              if (FTerminal->GetActive() &&
                  (!CopyParam->GetPreserveRights() && !CopyParam->GetPreserveTime()))
              {
                assert(DoResume);
                FTerminal->LogEvent(L"Ignoring error preserving permissions of overwritten file");
              }
              else
              {
                throw;
              }
            }
          );
        }
        catch(Exception & E)
        {
          if (TouchAction.get() != NULL)
          {
            TouchAction->Rollback(&E);
          }
          if (ChmodAction.get() != NULL)
          {
            ChmodAction->Rollback(&E);
          }
          ChildError = true;
          throw;
        }
      }
    }
  }
  ,
  {
    TRACE("15");
    if (FileHandle != NULL)
    {
      ::CloseHandle(FileHandle);
    }
  }
  );

  /* TODO : Delete also read-only files. */
  if (FLAGSET(Params, cpDelete))
  {
    TRACE("16");
    if (!Dir)
    {
      FILE_OPERATION_LOOP (FMTLOAD(DELETE_LOCAL_FILE_ERROR, FileName.c_str()),
        THROWOSIFFALSE(Sysutils::DeleteFile(FileName));
      )
    }
  }
  else if (CopyParam->GetClearArchive() && FLAGSET(OpenParams.LocalFileAttrs, faArchive))
  {
    TRACE("17");
    FILE_OPERATION_LOOP (FMTLOAD(CANT_SET_ATTRS, FileName.c_str()),
      THROWOSIFFALSE(FTerminal->SetLocalFileAttributes(FileName, OpenParams.LocalFileAttrs & ~faArchive) == 0);
    )
  }
  TRACE("/");
}
//---------------------------------------------------------------------------
RawByteString TSFTPFileSystem::SFTPOpenRemoteFile(
  const UnicodeString & FileName, unsigned int OpenType, __int64 Size)
{
  CALLSTACK;
  TSFTPPacket Packet(SSH_FXP_OPEN, GetSessionData()->GetCodePageAsNumber());

  Packet.AddPathString(FileName, FUtfStrings);
  if (FVersion < 5)
  {
    TRACE("1");
    Packet.AddCardinal(OpenType);
  }
  else
  {
    TRACE("2");
    unsigned long Access =
      FLAGMASK(FLAGSET(OpenType, SSH_FXF_READ), ACE4_READ_DATA) |
      FLAGMASK(FLAGSET(OpenType, SSH_FXF_WRITE), ACE4_WRITE_DATA | ACE4_APPEND_DATA);

    unsigned long Flags = 0;

    if (FLAGSET(OpenType, SSH_FXF_CREAT | SSH_FXF_EXCL))
    {
      Flags = SSH_FXF_CREATE_NEW;
    }
    else if (FLAGSET(OpenType, SSH_FXF_CREAT | SSH_FXF_TRUNC))
    {
      Flags = SSH_FXF_CREATE_TRUNCATE;
    }
    else if (FLAGSET(OpenType, SSH_FXF_CREAT))
    {
      Flags = SSH_FXF_OPEN_OR_CREATE;
    }
    else
    {
      Flags = SSH_FXF_OPEN_EXISTING;
    }

    Flags |=
      FLAGMASK(FLAGSET(OpenType, SSH_FXF_APPEND), SSH_FXF_ACCESS_APPEND_DATA) |
      FLAGMASK(FLAGSET(OpenType, SSH_FXF_TEXT), SSH_FXF_ACCESS_TEXT_MODE);

    TRACE("3");
    Packet.AddCardinal(Access);
    Packet.AddCardinal(Flags);
  }

  TRACE("4");
  bool SendSize =
    (Size >= 0) &&
    FLAGSET(OpenType, SSH_FXF_CREAT | SSH_FXF_TRUNC);
  Packet.AddProperties(NULL, NULL, NULL, NULL, NULL,
    SendSize ? &Size : NULL, false, FVersion, FUtfStrings);

  SendPacketAndReceiveResponse(&Packet, &Packet, SSH_FXP_HANDLE);

  TRACE("/");
  return Packet.GetFileHandle();
}
//---------------------------------------------------------------------------
int TSFTPFileSystem::SFTPOpenRemote(void * AOpenParams, void * /*Param2*/)
{
  CALLSTACK;
  TOpenRemoteFileParams * OpenParams = static_cast<TOpenRemoteFileParams *>(AOpenParams);
  assert(OpenParams);
  TFileOperationProgressType * OperationProgress = OpenParams->OperationProgress;

  int OpenType = 0;
  bool Success = false;
  bool ConfirmOverwriting = false;

  do
  {
    try
    {
      ConfirmOverwriting =
        !OpenParams->Confirmed && !OpenParams->Resume &&
        FTerminal->CheckRemoteFile(OpenParams->Params, OperationProgress);
      OpenType = SSH_FXF_WRITE | SSH_FXF_CREAT;
      // when we want to preserve overwritten files, we need to find out that
      // they exist first... even if overwrite confirmation is disabled.
      // but not when we already know we are not going to overwrite (but e.g. to append)
      if ((ConfirmOverwriting || GetSessionData()->GetOverwrittenToRecycleBin()) &&
          (OpenParams->OverwriteMode == omOverwrite))
      {
        OpenType |= SSH_FXF_EXCL;
      }
      if (!OpenParams->Resuming && (OpenParams->OverwriteMode == omOverwrite))
      {
        OpenType |= SSH_FXF_TRUNC;
      }
      if ((FVersion >= 4) && OpenParams->OperationProgress->AsciiTransfer)
      {
        OpenType |= SSH_FXF_TEXT;
      }

      TRACE("1");
      OpenParams->RemoteFileHandle = SFTPOpenRemoteFile(
        OpenParams->RemoteFileName, OpenType, OperationProgress->LocalSize);

      Success = true;
    }
    catch(Exception & E)
    {
      TRACE("E1");
      if (!OpenParams->Confirmed && (OpenType & SSH_FXF_EXCL) && FTerminal->GetActive())
      {
        TRACE("2");
        bool ThrowOriginal = false;

        // When exclusive opening of file fails, try to detect if file exists.
        // When file does not exist, failure was probably caused by 'permission denied'
        // or similar error. In this case throw original exception.
        try
        {
          TRACE("3");
          TRemoteFile * File = NULL;
          UnicodeString RealFileName = LocalCanonify(OpenParams->RemoteFileName);
          ReadFile(RealFileName, File);
          assert(File);
          OpenParams->DestFileSize = File->GetSize();
          if (OpenParams->FileParams != NULL)
          {
            TRACE("4");
            OpenParams->FileParams->DestTimestamp = File->GetModification();
            OpenParams->FileParams->DestSize = OpenParams->DestFileSize;
          }
          // file exists (otherwise exception was thrown)
          SAFE_DESTROY(File);
        }
        catch(...)
        {
          TRACE("E2");
          if (!FTerminal->GetActive())
          {
            TRACE("5");
            throw;
          }
          else
          {
            TRACE("6");
            ThrowOriginal = true;
          }
        }

        if (ThrowOriginal)
        {
          TRACE("7");
          throw;
        }

        // we may get here even if confirmation is disabled,
        // when we have preserving of overwritten files enabled
        if (ConfirmOverwriting)
        {
          TRACE("8");
          // confirmation duplicated in SFTPSource for resumable file transfers.
          UnicodeString RemoteFileNameOnly = UnixExtractFileName(OpenParams->RemoteFileName);
          SFTPConfirmOverwrite(RemoteFileNameOnly,
            OpenParams->Params, OperationProgress, OpenParams->OverwriteMode, OpenParams->FileParams);
          if (RemoteFileNameOnly != UnixExtractFileName(OpenParams->RemoteFileName))
          {
            TRACE("9");
            OpenParams->RemoteFileName =
              UnixExtractFilePath(OpenParams->RemoteFileName) + RemoteFileNameOnly;
          }
          OpenParams->Confirmed = true;
        }
        else
        {
          TRACE("10");
          assert(GetSessionData()->GetOverwrittenToRecycleBin());
        }

        if ((OpenParams->OverwriteMode == omOverwrite) &&
            GetSessionData()->GetOverwrittenToRecycleBin() &&
            !FTerminal->GetSessionData()->GetRecycleBinPath().IsEmpty())
        {
          TRACE("11");
          FTerminal->RecycleFile(OpenParams->RemoteFileName, NULL);
        }
      }
      else if (FTerminal->GetActive())
      {
        TRACE("12");
        // if file overwritting was confirmed, it means that the file already exists,
        // if not, check now
        if (!OpenParams->Confirmed)
        {
          TRACE("13");
          bool ThrowOriginal = false;

          // When file does not exist, failure was probably caused by 'permission denied'
          // or similar error. In this case throw original exception.
          try
          {
            TRemoteFile * File;
            UnicodeString RealFileName = LocalCanonify(OpenParams->RemoteFileName);
            ReadFile(RealFileName, File);
            SAFE_DESTROY(File);
          }
          catch(...)
          {
            TRACE("E3");
            if (!FTerminal->GetActive())
            {
              TRACE("14");
              throw;
            }
            else
            {
              TRACE("15");
              ThrowOriginal = true;
            }
          }

          if (ThrowOriginal)
          {
            TRACE("16");
            throw;
          }
        }

        // now we know that the file exists

        if (FTerminal->FileOperationLoopQuery(E, OperationProgress,
              FMTLOAD(SFTP_OVERWRITE_FILE_ERROR, OpenParams->RemoteFileName.c_str()),
              true, LoadStr(SFTP_OVERWRITE_DELETE_BUTTON)))
        {
          TRACE("17");
          int Params = dfNoRecursive;
          FTerminal->DeleteFile(OpenParams->RemoteFileName, NULL, &Params);
        }
      }
      else
      {
        TRACE("18");
        throw;
      }
    }
  }
  while (!Success);

  TRACE("/");
  return 0;
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::SFTPCloseRemote(const RawByteString Handle,
  const UnicodeString & FileName, TFileOperationProgressType * OperationProgress,
  bool TransferFinished, bool Request, TSFTPPacket * Packet)
{
  // Moving this out of SFTPSource() fixed external exception 0xC0000029 error
  FILE_OPERATION_LOOP(FMTLOAD(SFTP_CLOSE_FILE_ERROR, FileName.c_str()),
    try
    {
      TSFTPPacket CloseRequest(GetSessionData()->GetCodePageAsNumber());
      TSFTPPacket * P = (Packet == NULL ? &CloseRequest : Packet);

      if (Request)
      {
        P->ChangeType(SSH_FXP_CLOSE);
        P->AddString(Handle);
        SendPacket(P);
        ReserveResponse(P, Packet);
      }
      else
      {
        assert(Packet != NULL);
        ReceiveResponse(P, Packet, SSH_FXP_STATUS);
      }
    }
    catch(...)
    {
      if (!FTerminal->GetActive() || TransferFinished)
      {
        throw;
      }
    }
  );
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::SFTPDirectorySource(const UnicodeString & DirectoryName,
  const UnicodeString & TargetDir, int Attrs, const TCopyParamType * CopyParam,
  int Params, TFileOperationProgressType * OperationProgress, unsigned int Flags)
{
  UnicodeString DestDirectoryName = CopyParam->ChangeFileName(
    ExtractFileName(ExcludeTrailingBackslash(DirectoryName), false), osLocal,
    FLAGSET(Flags, tfFirstLevel));
  UnicodeString DestFullName = UnixIncludeTrailingBackslash(TargetDir + DestDirectoryName);

  OperationProgress->SetFile(DirectoryName);

  bool CreateDir = false;
  try
  {
    TryOpenDirectory(DestFullName);
  }
  catch(...)
  {
    if (FTerminal->GetActive())
    {
      // opening directory failed, it probably does not exists, try to
      // create it
      CreateDir = true;
    }
    else
    {
      throw;
    }
  }

  if (CreateDir)
  {
    TRemoteProperties Properties;
    if (CopyParam->GetPreserveRights())
    {
      Properties.Valid = TValidProperties() << vpRights;
      Properties.Rights = CopyParam->RemoteFileRights(Attrs);
    }
    FTerminal->CreateDirectory(DestFullName, &Properties);
    Flags |= tfNewDirectory;
  }

  int FindAttrs = faReadOnly | faHidden | faSysFile | faDirectory | faArchive;
  TSearchRec SearchRec;
  bool FindOK = false;

  FILE_OPERATION_LOOP (FMTLOAD(LIST_DIR_ERROR, DirectoryName.c_str()),
    FindOK = (bool)(FindFirst(DirectoryName + L"*.*",
      FindAttrs, SearchRec) == 0);
  );

  TRY_FINALLY (
  {
    while (FindOK && !OperationProgress->Cancel)
    {
      UnicodeString FileName = DirectoryName + SearchRec.Name;
      try
      {
        if ((SearchRec.Name != THISDIRECTORY) && (SearchRec.Name != PARENTDIRECTORY))
        {
          SFTPSourceRobust(FileName, NULL, DestFullName, CopyParam, Params, OperationProgress,
            Flags & ~tfFirstLevel);
        }
      }
      catch (EScpSkipFile &E)
      {
        // If ESkipFile occurs, just log it and continue with next file
        SUSPEND_OPERATION (
          // here a message to user was displayed, which was not appropriate
          // when user refused to overwrite the file in subdirectory.
          // hopefuly it won't be missing in other situations.
          if (!FTerminal->HandleException(&E)) throw;
        );
      }

      FILE_OPERATION_LOOP (FMTLOAD(LIST_DIR_ERROR, DirectoryName.c_str()),
        FindOK = (FindNext(SearchRec) == 0);
      );
    }
  }
  ,
  {
    FindClose(SearchRec);
  }
  );

  /* TODO : Delete also read-only directories. */
  /* TODO : Show error message on failure. */
  if (!OperationProgress->Cancel)
  {
    if (FLAGSET(Params, cpDelete))
    {
      FTerminal->RemoveLocalDirectory(DirectoryName);
    }
    else if (CopyParam->GetClearArchive() && FLAGSET(Attrs, faArchive))
    {
      FILE_OPERATION_LOOP (FMTLOAD(CANT_SET_ATTRS, DirectoryName.c_str()),
        THROWOSIFFALSE(FTerminal->SetLocalFileAttributes(DirectoryName, Attrs & ~faArchive) == 0);
      )
    }
  }
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::CopyToLocal(TStrings * FilesToCopy,
  const UnicodeString & TargetDir, const TCopyParamType * CopyParam,
  int Params, TFileOperationProgressType * OperationProgress,
  TOnceDoneOperation & OnceDoneOperation)
{
  CALLSTACK;
  assert(FilesToCopy && OperationProgress);

  UnicodeString FileName;
  UnicodeString FullTargetDir = IncludeTrailingBackslash(TargetDir);
  const TRemoteFile * File;
  bool Success;
  int Index = 0;
  while (Index < FilesToCopy->Count && !OperationProgress->Cancel)
  {
    Success = false;
    FileName = FilesToCopy->Strings[Index];
    File = static_cast<TRemoteFile *>(FilesToCopy->Objects[Index]);

    assert(!FAvoidBusy);
    FAvoidBusy = true;

    TRY_FINALLY (
    {
      try
      {
        SFTPSinkRobust(LocalCanonify(FileName), File, FullTargetDir, CopyParam,
          Params, OperationProgress, tfFirstLevel);
        Success = true;
      }
      catch(EScpSkipFile & E)
      {
        SUSPEND_OPERATION (
          if (!FTerminal->HandleException(&E)) throw;
        );
      }
      catch(...)
      {
        // TODO: remove the block?
        throw;
      }
    }
    ,
    {
      FAvoidBusy = false;
      OperationProgress->Finish(FileName, Success, OnceDoneOperation);
    }
    );
    Index++;
  }
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::SFTPSinkRobust(const UnicodeString & FileName,
  const TRemoteFile * File, const UnicodeString & TargetDir,
  const TCopyParamType * CopyParam, int Params,
  TFileOperationProgressType * OperationProgress, unsigned int Flags)
{
  CALLSTACK;
  // the same in TFTPFileSystem
  bool Retry;

  TDownloadSessionAction Action(FTerminal->GetActionLog());

  do
  {
    Retry = false;
    bool ChildError = false;
    try
    {
      SFTPSink(FileName, File, TargetDir, CopyParam, Params, OperationProgress,
        Flags, Action, ChildError);
    }
    catch(Exception & E)
    {
      TRACE("1");
      Retry = true;
      if (FTerminal->GetActive() ||
          !FTerminal->QueryReopen(&E, ropNoReadDirectory, OperationProgress))
      {
        if (!ChildError)
        {
          FTerminal->RollbackAction(Action, OperationProgress, &E);
        }
        TRACE("2");
        throw;
      }
    }

    if (Retry)
    {
      OperationProgress->RollbackTransfer();
      Action.Restart();
      assert(File != NULL);
      if (!File->GetIsDirectory())
      {
        // prevent overwrite and resume confirmations
        Params |= cpNoConfirmation;
      }
    }
  }
  while (Retry);
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::SFTPSink(const UnicodeString & FileName,
  const TRemoteFile * File, const UnicodeString & TargetDir,
  const TCopyParamType * CopyParam, int Params,
  TFileOperationProgressType * OperationProgress, unsigned int Flags,
  TDownloadSessionAction & Action, bool & ChildError)
{
  CALLSTACK;

  Action.FileName(FileName);

  UnicodeString OnlyFileName = UnixExtractFileName(FileName);

  TFileMasks::TParams MaskParams;
  assert(File);
  MaskParams.Size = File->GetSize();
  MaskParams.Modification = File->GetModification();

  if (!CopyParam->AllowTransfer(FileName, osRemote, File->GetIsDirectory(), MaskParams))
  {
    FTerminal->LogEvent(FORMAT(L"File \"%s\" excluded from transfer", FileName.c_str()));
    THROW_SKIP_FILE_NULL;
  }

  FTerminal->LogEvent(FORMAT(L"File: \"%s\"", FileName.c_str()));

  OperationProgress->SetFile(OnlyFileName);

  UnicodeString DestFileName = CopyParam->ChangeFileName(UnixExtractFileName(File->GetFileName()),
    osRemote, FLAGSET(Flags, tfFirstLevel));
  UnicodeString DestFullName = TargetDir + DestFileName;

  if (File->GetIsDirectory())
  {
    Action.Cancel();
    if (!File->GetIsSymLink())
    {
      FILE_OPERATION_LOOP (FMTLOAD(NOT_DIRECTORY_ERROR, DestFullName.c_str()),
        int Attrs = FTerminal->GetLocalFileAttributes(DestFullName);
        if ((Attrs & faDirectory) == 0) { EXCEPTION; }
      );

      FILE_OPERATION_LOOP (FMTLOAD(CREATE_DIR_ERROR, DestFullName.c_str()),
        if (!ForceDirectories(DestFullName)) { RaiseLastOSError(); }
      );

      TSinkFileParams SinkFileParams;
      SinkFileParams.TargetDir = IncludeTrailingBackslash(DestFullName);
      SinkFileParams.CopyParam = CopyParam;
      SinkFileParams.Params = Params;
      SinkFileParams.OperationProgress = OperationProgress;
      SinkFileParams.Skipped = false;
      SinkFileParams.Flags = Flags & ~tfFirstLevel;

      FTerminal->ProcessDirectory(FileName, MAKE_CALLBACK(TSFTPFileSystem::SFTPSinkFile, this), &SinkFileParams);

      // Do not delete directory if some of its files were skip.
      // Throw "skip file" for the directory to avoid attempt to deletion
      // of any parent directory
      if ((Params & cpDelete) && SinkFileParams.Skipped)
      {
        THROW_SKIP_FILE_NULL;
      }
    }
    else
    {
      // file is symlink to directory, currently do nothing, but it should be
      // reported to user
    }
  }
  else
  {
    FTerminal->LogEvent(FORMAT(L"Copying \"%s\" to local directory started.", FileName.c_str()));

    UnicodeString DestPartialFullName;
    bool ResumeAllowed;
    bool ResumeTransfer = false;
    __int64 ResumeOffset;

    // Will we use ASCII of BINARY file tranfer?
    OperationProgress->SetAsciiTransfer(
      CopyParam->UseAsciiTransfer(FileName, osRemote, MaskParams));
    FTerminal->LogEvent(UnicodeString((OperationProgress->AsciiTransfer ? L"Ascii" : L"Binary")) +
      L" transfer mode selected.");

    // Suppose same data size to transfer as to write
    // (not true with ASCII transfer)
    OperationProgress->SetTransferSize(File->GetSize());
    OperationProgress->SetLocalSize(OperationProgress->TransferSize);

    // resume has no sense for temporary downloads
    ResumeAllowed = ((Params & cpTemporary) == 0) &&
      !OperationProgress->AsciiTransfer &&
      CopyParam->AllowResume(OperationProgress->TransferSize);
    OperationProgress->SetResumeStatus(ResumeAllowed ? rsEnabled : rsDisabled);

    int Attrs = 0;
    FILE_OPERATION_LOOP (FMTLOAD(NOT_FILE_ERROR, DestFullName.c_str()),
      Attrs = FTerminal->GetLocalFileAttributes(DestFullName);
      if ((Attrs >= 0) && (Attrs & faDirectory)) { EXCEPTION; }
    );

    OperationProgress->TransferingFile = false; // not set with SFTP protocol

    HANDLE LocalHandle = NULL;
    TStream * FileStream = NULL;
    bool DeleteLocalFile = false;
    RawByteString RemoteHandle;
    UnicodeString LocalFileName = DestFullName;
    TOverwriteMode OverwriteMode = omOverwrite;
    TRY_FINALLY (
    {
      if (ResumeAllowed)
      {
        DestPartialFullName = DestFullName + FTerminal->GetConfiguration()->GetPartialExt();
        LocalFileName = DestPartialFullName;

        FTerminal->LogEvent(L"Checking existence of partially transfered file.");
        if (FileExists(DestPartialFullName))
        {
          FTerminal->OpenLocalFile(DestPartialFullName, GENERIC_WRITE,
            NULL, &LocalHandle, NULL, NULL, NULL, &ResumeOffset);

          bool PartialBiggerThanSource = (ResumeOffset > OperationProgress->TransferSize);
          if (FLAGCLEAR(Params, cpNoConfirmation))
          {
            ResumeTransfer = SFTPConfirmResume(DestFileName,
              PartialBiggerThanSource, OperationProgress);
          }
          else
          {
            ResumeTransfer = !PartialBiggerThanSource;
          }

          if (!ResumeTransfer)
          {
            CloseHandle(LocalHandle);
            LocalHandle = NULL;
            FILE_OPERATION_LOOP (FMTLOAD(DELETE_LOCAL_FILE_ERROR, DestPartialFullName.c_str()),
              THROWOSIFFALSE(Sysutils::DeleteFile(DestPartialFullName));
            )
          }
          else
          {
            FTerminal->LogEvent(L"Resuming file transfer.");
            FileSeek(LocalHandle, ResumeOffset, 0);
            OperationProgress->AddResumed(ResumeOffset);
          }
        }
      }

      // first open source file, not to loose the destination file,
      // if we cannot open the source one in the first place
      FTerminal->LogEvent(L"Opening remote file.");
      FILE_OPERATION_LOOP (FMTLOAD(SFTP_OPEN_FILE_ERROR, FileName.c_str()),
        int OpenType = SSH_FXF_READ;
        if ((FVersion >= 4) && OperationProgress->AsciiTransfer)
        {
          OpenType |= SSH_FXF_TEXT;
        }
        RemoteHandle = SFTPOpenRemoteFile(FileName, OpenType);
      );

      TDateTime Modification(0.0);
      FILETIME AcTime = {0};
      FILETIME WrTime = {0};

      TRACE("1");
      TSFTPPacket RemoteFilePacket(SSH_FXP_FSTAT, GetSessionData()->GetCodePageAsNumber());
      TRACE("1a");
      RemoteFilePacket.AddString(RemoteHandle);
      SendCustomReadFile(&RemoteFilePacket, &RemoteFilePacket,
        SSH_FILEXFER_ATTR_MODIFYTIME);
      ReceiveResponse(&RemoteFilePacket, &RemoteFilePacket);

      const TRemoteFile * AFile = File;
      TRY_FINALLY (
      {
        // ignore errors
        if (RemoteFilePacket.GetType() == SSH_FXP_ATTRS)
        {
          // load file, avoid completion (resolving symlinks) as we do not need that
          AFile = LoadFile(&RemoteFilePacket, NULL, UnixExtractFileName(FileName),
            NULL, false);
        }

        Modification = AFile->GetModification();
        AcTime = DateTimeToFileTime(AFile->GetLastAccess(),
          FTerminal->GetSessionData()->GetDSTMode());
        WrTime = DateTimeToFileTime(AFile->GetModification(),
          FTerminal->GetSessionData()->GetDSTMode());
        // TRACEFMT("WrTime %s/%s [%s %s]", IntToStr(__int64(WrTime.dwLowDateTime)).c_str(), IntToStr(__int64(WrTime.dwHighDateTime)), AFile->GetModification().DateString().c_str(), AFile->GetModification().TimeString().c_str());
      }
      ,
      {
        if (AFile != File)
        {
          delete AFile;
          AFile = NULL;
        }
      }
      );

      if ((Attrs >= 0) && !ResumeTransfer)
      {
        __int64 DestFileSize = 0;
        __int64 MTime = 0;
        FTerminal->OpenLocalFile(DestFullName, GENERIC_WRITE,
          NULL, &LocalHandle, NULL, &MTime, NULL, &DestFileSize, false);

        FTerminal->LogEvent(L"Confirming overwriting of file.");
        TOverwriteFileParams FileParams;
        FileParams.SourceSize = OperationProgress->TransferSize;
        FileParams.SourceTimestamp = File->GetModification();
        FileParams.DestTimestamp = UnixToDateTime(MTime,
          GetSessionData()->GetDSTMode());
        FileParams.DestSize = DestFileSize;
        UnicodeString PrevDestFileName = DestFileName;
        SFTPConfirmOverwrite(DestFileName, Params, OperationProgress, OverwriteMode, &FileParams);
        if (PrevDestFileName != DestFileName)
        {
          DestFullName = TargetDir + DestFileName;
          DestPartialFullName = DestFullName + FTerminal->GetConfiguration()->GetPartialExt();
          if (ResumeAllowed)
          {
            if (FileExists(DestPartialFullName))
            {
              FILE_OPERATION_LOOP (FMTLOAD(DELETE_LOCAL_FILE_ERROR, DestPartialFullName.c_str()),
                THROWOSIFFALSE(Sysutils::DeleteFile(DestPartialFullName));
              )
            }
            LocalFileName = DestPartialFullName;
          }
          else
          {
            LocalFileName = DestFullName;
          }
        }

        if (OverwriteMode == omOverwrite)
        {
          // is NULL when overwritting read-only file
          if (LocalHandle)
          {
            CloseHandle(LocalHandle);
            LocalHandle = NULL;
          }
        }
        else
        {
          // is NULL when overwritting read-only file, so following will
          // probably fail anyway
          if (LocalHandle == NULL)
          {
            FTerminal->OpenLocalFile(DestFullName, GENERIC_WRITE,
              NULL, &LocalHandle, NULL, NULL, NULL, NULL);
          }
          ResumeAllowed = false;
          FileSeek(LocalHandle, DestFileSize, 0);
          if (OverwriteMode == omAppend)
          {
            FTerminal->LogEvent(L"Appending to file.");
          }
          else
          {
            FTerminal->LogEvent(L"Resuming file transfer (append style).");
            assert(OverwriteMode == omResume);
            OperationProgress->AddResumed(DestFileSize);
          }
        }
      }

      Action.Destination(ExpandUNCFileName(DestFullName));

      // if not already opened (resume, append...), create new empty file
      if (!LocalHandle)
      {
        if (!FTerminal->CreateLocalFile(LocalFileName, OperationProgress,
               &LocalHandle, FLAGSET(Params, cpNoConfirmation)))
        {
          THROW_SKIP_FILE_NULL;
        }
      }
      assert(LocalHandle);

      DeleteLocalFile = true;

      TRACE("2");
      FileStream = new TSafeHandleStream((THandle)LocalHandle);

      // at end of this block queue is discarded
      {
        TRACE("3");
        TSFTPDownloadQueue Queue(this, GetSessionData()->GetCodePageAsNumber());
        TRY_FINALLY (
        {
          TSFTPPacket DataPacket(GetSessionData()->GetCodePageAsNumber());

          TRACE("4");
          int QueueLen = int(File->GetSize() / DownloadBlockSize(OperationProgress)) + 1;
          if ((QueueLen > GetSessionData()->GetSFTPDownloadQueue()) ||
              (QueueLen < 0))
          {
            QueueLen = GetSessionData()->GetSFTPDownloadQueue();
          }
          if (QueueLen < 1)
          {
            QueueLen = 1;
          }
          TRACE("5");
          Queue.Init(QueueLen, RemoteHandle, OperationProgress->TransferedSize,
            OperationProgress);

          bool Eof = false;
          bool PrevIncomplete = false;
          int GapFillCount = 0;
          int GapCount = 0;
          unsigned long Missing = 0;
          unsigned long DataLen = 0;
          unsigned long BlockSize = 0;
          bool ConvertToken = false;

          while (!Eof)
          {
            TRACE("6");
            if (Missing > 0)
            {
              Queue.InitFillGapRequest(OperationProgress->TransferedSize, Missing,
                &DataPacket);
              GapFillCount++;
              SendPacketAndReceiveResponse(&DataPacket, &DataPacket,
                SSH_FXP_DATA, asEOF);
            }
            else
            {
              Queue.ReceivePacket(&DataPacket, BlockSize);
            }

            if (DataPacket.GetType() == SSH_FXP_STATUS)
            {
              // must be SSH_FX_EOF, any other status packet would raise exception
              Eof = true;
              // close file right away, before waiting for pending responses
              SFTPCloseRemote(RemoteHandle, DestFileName, OperationProgress,
                true, true, NULL);
              RemoteHandle = L""; // do not close file again in __finally block
            }

            if (!Eof)
            {
              if ((Missing == 0) && PrevIncomplete)
              {
                // This can happen only if last request returned less bytes
                // than expected, but exacly number of bytes missing to last
                // known file size, but actually EOF was not reached.
                // Can happen only when filesize has changed since directory
                // listing and server returns less bytes than requested and
                // file has some special file size.
                FTerminal->LogEvent(FORMAT(
                  L"Received incomplete data packet before end of file, "
                  L"offset: %s, size: %d, requested: %d",
                  Int64ToStr(OperationProgress->TransferedSize).c_str(), static_cast<int>(DataLen),
                  static_cast<int>(BlockSize)));
                FTerminal->TerminalError(NULL, LoadStr(SFTP_INCOMPLETE_BEFORE_EOF));
              }

              // Buffer for one block of data
              TFileBuffer BlockBuf;

              DataLen = DataPacket.GetCardinal();

              PrevIncomplete = false;
              if (Missing > 0)
              {
                assert(DataLen <= Missing);
                Missing -= DataLen;
              }
              else if (DataLen < BlockSize)
              {
                if (OperationProgress->TransferedSize + static_cast<__int64>(DataLen) !=
                      OperationProgress->TransferSize)
                {
                  // with native text transfer mode (SFTP>=4), do not bother about
                  // getting less than requested, read offset is ignored anyway
                  if ((FVersion < 4) || !OperationProgress->AsciiTransfer)
                  {
                    GapCount++;
                    Missing = BlockSize - DataLen;
                  }
                }
                else
                {
                  PrevIncomplete = true;
                }
              }

              assert(DataLen <= BlockSize);
              BlockBuf.Insert(0, reinterpret_cast<const char *>(DataPacket.GetNextData(DataLen)), DataLen);
              OperationProgress->AddTransfered(DataLen);

              if (OperationProgress->AsciiTransfer)
              {
                assert(!ResumeTransfer && !ResumeAllowed);

                __int64 PrevBlockSize = BlockBuf.GetSize();
                BlockBuf.Convert(GetEOL(), FTerminal->GetConfiguration()->GetLocalEOLType(), 0, ConvertToken);
                OperationProgress->SetLocalSize(
                  OperationProgress->LocalSize - PrevBlockSize + BlockBuf.GetSize());
              }

              FILE_OPERATION_LOOP (FMTLOAD(WRITE_ERROR, LocalFileName.c_str()),
                BlockBuf.WriteToStream(FileStream, BlockBuf.GetSize());
              );

              OperationProgress->AddLocallyUsed(BlockBuf.GetSize());
            }

            if (OperationProgress->Cancel == csCancel)
            {
              Abort();
            }
          }

          if (GapCount > 0)
          {
            FTerminal->LogEvent(FORMAT(
              L"%d requests to fill %d data gaps were issued.",
              GapFillCount, GapCount));
          }
        }
        ,
        {
          Queue.DisposeSafe();
        }
        );
        // queue is discarded here
      }

      if (CopyParam->GetPreserveTime())
      {
        SetFileTime(LocalHandle, NULL, &AcTime, &WrTime);
        // TRACEFMT("FileTime %d [%s %s]", FileGetDate(int(LocalHandle)), FileDateToDateTime(FileGetDate(int(LocalHandle))).DateString().c_str(), FileDateToDateTime(FileGetDate(int(LocalHandle))).TimeString().c_str());
      }

      CloseHandle(LocalHandle);
      LocalHandle = NULL;

      if (ResumeAllowed)
      {
        FILE_OPERATION_LOOP (FMTLOAD(RENAME_AFTER_RESUME_ERROR,
            ExtractFileName(DestPartialFullName, true).c_str(), DestFileName.c_str()),

          if (FileExists(DestFullName))
          {
            THROWOSIFFALSE(Sysutils::DeleteFile(DestFullName));
          }
          if (!Sysutils::RenameFile(DestPartialFullName, DestFullName))
          {
            RaiseLastOSError();
          }
        );
      }

      DeleteLocalFile = false;

      if (Attrs == -1)
      {
        Attrs = faArchive;
      }
      int NewAttrs = CopyParam->LocalFileAttrs(*File->GetRights());
      if ((NewAttrs & Attrs) != NewAttrs)
      {
        FILE_OPERATION_LOOP (FMTLOAD(CANT_SET_ATTRS, DestFullName.c_str()),
          THROWOSIFFALSE(FTerminal->SetLocalFileAttributes(DestFullName, Attrs | NewAttrs) == 0);
        );
      }

      TRACE("1");
    }
    ,
    {
      TRACE("2");
      if (LocalHandle) { ::CloseHandle(LocalHandle); }
      if (FileStream) { delete FileStream; }
      if (DeleteLocalFile && (!ResumeAllowed || OperationProgress->LocallyUsed == 0) &&
          (OverwriteMode == omOverwrite))
      {
        FILE_OPERATION_LOOP (FMTLOAD(DELETE_LOCAL_FILE_ERROR, LocalFileName.c_str()),
          THROWOSIFFALSE(Sysutils::DeleteFile(LocalFileName));
        )
      }

      // if the transfer was finished, the file is closed already
      if (FTerminal->GetActive() && !RemoteHandle.IsEmpty())
      {
        // do not wait for response
        SFTPCloseRemote(RemoteHandle, DestFileName, OperationProgress,
          true, true, NULL);
      }
      TRACE("3");
    }
    );
  }

  if (Params & cpDelete)
  {
    ChildError = true;
    // If file is directory, do not delete it recursively, because it should be
    // empty already. If not, it should not be deleted (some files were
    // skipped or some new files were copied to it, while we were downloading)
    int Params2 = dfNoRecursive;
    FTerminal->DeleteFile(FileName, File, &Params2);
    ChildError = false;
  }
}
//---------------------------------------------------------------------------
void TSFTPFileSystem::SFTPSinkFile(const UnicodeString & FileName,
  const TRemoteFile * File, void * Param)
{
  CALLSTACK;
  TSinkFileParams * Params = static_cast<TSinkFileParams *>(Param);
  assert(Params->OperationProgress);
  try
  {
    SFTPSinkRobust(FileName, File, Params->TargetDir, Params->CopyParam,
      Params->Params, Params->OperationProgress, Params->Flags);
  }
  catch(EScpSkipFile & E)
  {
    TRACE("1");
    TFileOperationProgressType * OperationProgress = Params->OperationProgress;

    Params->Skipped = true;

    SUSPEND_OPERATION (
      if (!FTerminal->HandleException(&E)) throw;
    );

    if (OperationProgress->Cancel)
    {
      Abort();
    }
    TRACE("2");
  }
}
