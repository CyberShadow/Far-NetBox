#pragma once

#include <SubpluginDefs.hpp>
#include <Classes.hpp>

#include "Configuration.h"

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
class TTerminalSkel
{
public:
  static void NBAPI
  fatal_error(
    nb_terminal_t * object,
    nb_exception_t * E,
    wchar_t * Msg);
};
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Utility functions

// wchar_t * GuidToStr(const GUID & Guid, wchar_t * Buffer, size_t sz);
