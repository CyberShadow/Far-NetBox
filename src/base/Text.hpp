#pragma once

#include <vcl.h>
#include <headers.hpp>

//------------------------------------------------------------------------------
namespace text {

extern const std::string utf8;
extern std::string systemCharset;

void initialize();

const std::string & acpToUtf8(const std::string & str, std::string & tmp);
inline std::string acpToUtf8(const std::string & str)
{
  std::string tmp;
  return acpToUtf8(str, tmp);
}

const std::string & toUtf8(const std::string & str, const std::string & fromCharset, std::string & tmp);
inline std::string toUtf8(const std::string & str, const std::string & fromCharset = systemCharset)
{
  std::string tmp;
  return toUtf8(str, fromCharset, tmp);
}

const std::string & fromUtf8(const std::string & str, const std::string & toCharset, std::string & tmp);
inline std::string fromUtf8(const std::string & str, const std::string & toCharset = systemCharset)
{
  std::string tmp;
  return fromUtf8(str, toCharset, tmp);
}

const std::wstring & utf8ToWide(const std::string & str, std::wstring & tmp);
inline std::wstring utf8ToWide(const std::string & str)
{
  std::wstring tmp;
  return utf8ToWide(str, tmp);
}

const std::string & wideToUtf8(const std::wstring & str, std::string & tmp);
inline std::string wideToUtf8(const std::wstring & str)
{
  std::string tmp;
  return wideToUtf8(str, tmp);
}

} // namespace text
