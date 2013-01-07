#pragma once

#include <NetBoxSubPlugin.hpp>

//------------------------------------------------------------------------------

TBaseSubplugin::TBaseSubplugin(HINSTANCE HInst) :
  TCustomFarPlugin(HInst)
{
  // DEBUG_PRINTF(L"begin")
  // DEBUG_PRINTF(L"end")
}
//------------------------------------------------------------------------------
TBaseSubplugin::~TBaseSubplugin()
{
  // DEBUG_PRINTF(L"begin")
  // FarConfiguration->SetPlugin(NULL);
  // CoreFinalize();
  // DEBUG_PRINTF(L"end")
}
//------------------------------------------------------------------------------
void TBaseSubplugin::GetPluginInfoEx(long unsigned & Flags,
    TStrings * DiskMenuStrings, TStrings * PluginMenuStrings,
    TStrings * PluginConfigStrings, TStrings * CommandPrefixes)
{
}
//------------------------------------------------------------------------------
TCustomFarFileSystem * TBaseSubplugin::OpenPluginEx(intptr_t OpenFrom, intptr_t Item)
{
  return NULL;
}
//------------------------------------------------------------------------------
bool TBaseSubplugin::ConfigureEx(intptr_t Item)
{
  return false;
}
//------------------------------------------------------------------------------
intptr_t TBaseSubplugin::ProcessEditorEventEx(intptr_t Event, void * Param)
{
  return 0;
}
//------------------------------------------------------------------------------
intptr_t TBaseSubplugin::ProcessEditorInputEx(const INPUT_RECORD * Rec)
{
  return 0;
}
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