//---------------------------------------------------------------------------
#ifndef PuttyToolsH
#define PuttyToolsH
//---------------------------------------------------------------------------
enum TKeyType { ktUnopenable, ktUnknown, ktSSH1, ktSSH2, ktOpenSSH, ktSSHCom };
TKeyType KeyType(std::wstring FileName);
std::wstring KeyTypeName(TKeyType KeyType);
//---------------------------------------------------------------------------
std::wstring DecodeUTF(const std::wstring UTF);
std::wstring EncodeUTF(const std::wstring Source);
//---------------------------------------------------------------------------
__int64 ParseSize(std::wstring SizeStr);
//---------------------------------------------------------------------------
bool HasGSSAPI();
//---------------------------------------------------------------------------
void AES256EncodeWithMAC(char * Data, size_t Len, const char * Password,
  size_t PasswordLen, const char * Salt);
//---------------------------------------------------------------------------
#endif
