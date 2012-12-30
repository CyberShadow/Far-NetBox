#pragma once

#include <NetBoxSubPlugin.hpp>

//------------------------------------------------------------------------------

TBaseSubplugin::TBaseSubplugin(HINSTANCE HInst,
  const subplugin_startup_info_t * startup_info) :
  TCustomFarPlugin(HInst)
{
  // DEBUG_PRINTF(L"begin")
  assert(startup_info);
  memset(&FStartupInfo, 0, sizeof(FStartupInfo));
  memmove(&FStartupInfo, startup_info, startup_info->struct_size >= sizeof(FStartupInfo) ?
    sizeof(FStartupInfo) : startup_info->struct_size);
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
/*subplugin_error_t TBaseSubplugin::Notify(
  subplugin_t * subplugin, const notification_t * notification)
{
  // DEBUG_PRINTF(L"begin");
  assert(notification);

  // DEBUG_PRINTF(L"end");
  return SUBPLUGIN_NO_ERROR;
}*/
//------------------------------------------------------------------------------
