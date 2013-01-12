#pragma once

#include <FarPlugin.h>
#include <subplugin.hpp>

//------------------------------------------------------------------------------

class TBaseSubplugin : public TObject
{
public:
  explicit TBaseSubplugin();
  virtual ~TBaseSubplugin();

private:
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class TSessionDataProxy
{
public:
  TAutoSwitch GetFtpListAll() const;
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Utility functions

// wchar_t * GuidToStr(const GUID & Guid, wchar_t * Buffer, size_t sz);
