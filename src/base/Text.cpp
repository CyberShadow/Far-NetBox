//------------------------------------------------------------------------------
#include "Text.hpp"

//------------------------------------------------------------------------------

namespace text {

std::string emptyString;
std::wstring emptyStringW;

const std::string utf8 = "utf-8"; // optimization
std::string systemCharset;

void initialize()
{
  setlocale(LC_ALL, "");

  char * ctype = setlocale(LC_CTYPE, NULL);
  if (ctype)
  {
    systemCharset = std::string(ctype);
  }
  else
  {
    DEBUG_PRINTF(L"Unable to determine the program's locale");
  }
}

int utf8ToWc(const char* str, wchar_t& c)
{
  uint8_t c0 = (uint8_t)str[0];
  if(c0 & 0x80) {                  // 1xxx xxxx
    if(c0 & 0x40) {                // 11xx xxxx
      if(c0 & 0x20) {              // 111x xxxx
        if(c0 & 0x10) {            // 1111 xxxx
          int n = -4;
          if(c0 & 0x08) {          // 1111 1xxx
            n = -5;
            if(c0 & 0x04) {        // 1111 11xx
              if(c0 & 0x02) {      // 1111 111x
                return -1;
              }
              n = -6;
            }
          }
          int i = -1;
          while(i > n && (str[abs(i)] & 0x80) == 0x80)
            --i;
          return i;
        } else {    // 1110xxxx
          uint8_t c1 = (uint8_t)str[1];
          if((c1 & (0x80 | 0x40)) != 0x80)
            return -1;

          uint8_t c2 = (uint8_t)str[2];
          if((c2 & (0x80 | 0x40)) != 0x80)
            return -2;

          // Ugly utf-16 surrogate catch
          if((c0 & 0x0f) == 0x0d && (c1 & 0x3c) >= (0x08 << 2))
            return -3;

          // Overlong encoding
          if(c0 == (0x80 | 0x40 | 0x20) && (c1 & (0x80 | 0x40 | 0x20)) == 0x80)
            return -3;

          c = (((wchar_t)c0 & 0x0f) << 12) |
            (((wchar_t)c1 & 0x3f) << 6) |
            ((wchar_t)c2 & 0x3f);

          return 3;
        }
      } else {        // 110xxxxx
        uint8_t c1 = (uint8_t)str[1];
        if((c1 & (0x80 | 0x40)) != 0x80)
          return -1;

        // Overlong encoding
        if((c0 & ~1) == (0x80 | 0x40))
          return -2;

        c = (((wchar_t)c0 & 0x1f) << 6) |
          ((wchar_t)c1 & 0x3f);
        return 2;
      }
    } else {          // 10xxxxxx
      return -1;
    }
  } else {            // 0xxxxxxx
    c = (unsigned char)str[0];
    return 1;
  }
  assert(0);
}

void wcToUtf8(wchar_t c, std::string& str)
{
  if(c >= 0x0800) {
    str += (char)(0x80 | 0x40 | 0x20 | (c >> 12));
    str += (char)(0x80 | ((c >> 6) & 0x3f));
    str += (char)(0x80 | (c & 0x3f));
  } else if(c >= 0x0080) {
    str += (char)(0x80 | 0x40 | (c >> 6));
    str += (char)(0x80 | (c & 0x3f));
  } else {
    str += (char)c;
  }
}

wchar_t toLower(wchar_t c)
{
  return static_cast<wchar_t>(reinterpret_cast<ptrdiff_t>(::CharLowerW((LPWSTR)c)));
}

const std::wstring & toLower(const std::wstring & str, std::wstring & tmp)
{
  if(str.empty())
    return emptyStringW;
  tmp.clear();
  tmp.reserve(str.length());
  for (size_t I = 0; I < str.size(); I++) {
    tmp += toLower(str[I]);
  }
  return tmp;
}

const std::string& toLower(const std::string & str, std::string & tmp)
{
  if(str.empty())
    return emptyString;
  tmp.reserve(str.length());
  const char * end = &str[0] + str.length();
  for(const char* p = &str[0]; p < end;) {
    wchar_t c = 0;
    int n = utf8ToWc(p, c);
    if(n < 0) {
      tmp += '_';
      p += abs(n);
    } else {
      p += n;
      wcToUtf8(toLower(c), tmp);
    }
  }
  return tmp;
}

const std::string& wideToUtf8(const std::wstring & str, std::string & tgt)
{
  if(str.empty())
  {
    return emptyString;
  }

  std::string::size_type n = str.length();
  tgt.clear();
  for(std::string::size_type i = 0; i < n; ++i) {
    wcToUtf8(str[i], tgt);
  }
  return tgt;
}

const std::wstring & acpToWide(const std::string & str, std::wstring & tmp)
{
  if(str.empty())
    return emptyStringW;
  int n = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str.c_str(), (int)str.length(), NULL, 0);
  if(n == 0) {
    return emptyStringW;
  }

  tmp.resize(n);
  n = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str.c_str(), (int)str.length(), &tmp[0], n);
  if(n == 0) {
    return emptyStringW;
  }
  return tmp;
}

const std::string & acpToUtf8(const std::string & str, std::string & tmp)
{
  std::wstring wtmp;
  return wideToUtf8(acpToWide(str, wtmp), tmp);
}

const std::string& wideToAcp(const std::wstring& str, std::string& tmp)
{
  if(str.empty())
    return emptyString;
  int n = WideCharToMultiByte(CP_ACP, 0, str.c_str(), (int)str.length(), NULL, 0, NULL, NULL);
  if(n == 0) {
    return emptyString;
  }

  tmp.resize(n);
  n = WideCharToMultiByte(CP_ACP, 0, str.c_str(), (int)str.length(), &tmp[0], n, NULL, NULL);
  if(n == 0) {
    return emptyString;
  }
  return tmp;
}

const std::wstring& utf8ToWide(const std::string& str, std::wstring& tgt)
{
  tgt.reserve(str.length());
  std::string::size_type n = str.length();
  for(std::string::size_type i = 0; i < n; ) {
    wchar_t c = 0;
    int x = utf8ToWc(str.c_str() + i, c);
    if(x < 0) {
      tgt += '_';
      i += abs(x);
    } else {
      i += x;
      tgt += c;
    }
  }
  return tgt;
}

const std::string& utf8ToAcp(const std::string & str, std::string & tmp)
{
  std::wstring wtmp;
  return wideToAcp(utf8ToWide(str, wtmp), tmp);
}

const std::string & toUtf8(const std::string & str, const std::string & fromCharset, std::string & tmp)
{
  if (str.empty())
  {
    return str;
  }

  if (fromCharset == utf8 || toLower(fromCharset, tmp) == utf8)
  {
    return str;
  }

  return acpToUtf8(str, tmp);
}

const std::string & fromUtf8(const std::string & str, const std::string & toCharset, std::string & tmp)
{
  if(str.empty()) {
    return str;
  }

  if (toCharset == utf8 || toLower(toCharset, tmp) == utf8) {
    return str;
  }

  return utf8ToAcp(str, tmp);
}
//------------------------------------------------------------------------------
} // namespace text

//------------------------------------------------------------------------------
