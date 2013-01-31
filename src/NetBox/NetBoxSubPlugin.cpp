#include <vcl.h>
#pragma hdrstop

#include "NetBoxSubPlugin.hpp"

//------------------------------------------------------------------------------

TBaseSubplugin::TBaseSubplugin()
{
  // DEBUG_PRINTF(L"begin");
  // DEBUG_PRINTF(L"end");
}
//------------------------------------------------------------------------------
TBaseSubplugin::~TBaseSubplugin()
{
  // DEBUG_PRINTF(L"begin");
  // DEBUG_PRINTF(L"end");
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void NBAPI
TTerminalSkel::fatal_error(
  nb_terminal_t * object,
  nb_exception_t * E,
  wchar_t * Msg)
{
  DEBUG_PRINTF(L"begin");
  DEBUG_PRINTF(L"end");
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
TTerminalStub::TTerminalStub(nb_terminal_t * terminal)
{
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Utility functions

/*wchar_t * GuidToStr(const GUID & Guid, wchar_t * Buffer, size_t sz)
{
  wchar_t * Result = Buffer;
  int Len = swprintf(Result, sz, L"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",Guid.Data1,Guid.Data2,Guid.Data3,Guid.Data4[0],Guid.Data4[1],Guid.Data4[2],Guid.Data4[3],Guid.Data4[4],Guid.Data4[5],Guid.Data4[6],Guid.Data4[7]);
  Result[Len] = 0;
  return Result;
}
*/
