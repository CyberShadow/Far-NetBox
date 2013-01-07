#pragma once

#include "GDiskSubplugin.hpp"
#include "plugin_version.hpp"

//------------------------------------------------------------------------------

static const wchar_t GDiskGuid[] = L"FD0439BB-31F4-4ABB-9B2A-6F3191A5D1AE";

//------------------------------------------------------------------------------
TSubplugin::TSubplugin(HINSTANCE HInst,
  nb_core_t * host) :
  TBaseSubplugin(HInst),
  FHost(host),
  FTabID(0),
  FTabControlID(0),
  FProtocolID(0)
{
  // DEBUG_PRINTF(L"begin")
  FUtils = reinterpret_cast<nb_utils_t *>(FHost->query_interface(NBINTF_UTILS, NBINTF_UTILS_VER));
  FConfig = reinterpret_cast<nb_config_t *>(FHost->query_interface(NBINTF_CONFIG, NBINTF_CONFIG_VER));
  FLogging = reinterpret_cast<nb_log_t *>(FHost->query_interface(NBINTF_LOGGING, NBINTF_LOGGING_VER));
  // DEBUG_PRINTF(L"end")
}
//------------------------------------------------------------------------------
TSubplugin::~TSubplugin()
{
  // DEBUG_PRINTF(L"begin")
  // FarConfiguration->SetPlugin(NULL);
  // CoreFinalize();
  // DEBUG_PRINTF(L"end")
}
//------------------------------------------------------------------------------
subplugin_error_t TSubplugin::OnSessionDialogInitTabs(
  nbptr_t object,
  nbptr_t data,
  nbptr_t common,
  nb_bool_t * bbreak)
{
  DEBUG_PRINTF(L"begin");
  subplugin_error_t Result = SUBPLUGIN_NO_ERROR;
  FTabID = FUtils->get_unique_id();
  DEBUG_PRINTF(L"FTabID = %d", FTabID);
  const wchar_t * TabCaption = FUtils->get_msg(PLUGIN_GUID, L"Tab.Caption");
  DEBUG_PRINTF(L"TabCaption = %s", TabCaption);
  nb_sessiondialog_t * dlg = reinterpret_cast<nb_sessiondialog_t *>(FHost->query_interface(NBINTF_SESSIONDIALOG, NBINTF_SESSIONDIALOG_VER));
  assert(dlg);
  FTabControlID = dlg->add_tab(object, FTabID, TabCaption);
  DEBUG_PRINTF(L"end, FTabControlID = %d", FTabControlID);
  return Result;
}
//------------------------------------------------------------------------------
subplugin_error_t TSubplugin::OnSessionDialogInitSessionTab(
  nbptr_t object,
  nbptr_t data,
  nbptr_t common,
  nb_bool_t * bbreak)
{
  DEBUG_PRINTF(L"begin");
  subplugin_error_t Result = SUBPLUGIN_NO_ERROR;
  // int DialogItemID = FStartupInfo.get_dialog_item_id(subplugin, notification, L"TransferProtocolCombo");
  // DEBUG_PRINTF(L"DialogItemID = %d", DialogItemID);
  FProtocolID = FUtils->get_unique_id();
  // DEBUG_PRINTF(L"FProtocolID = %d", FProtocolID);
  const wchar_t * ProtocolName = FUtils->get_msg(PLUGIN_GUID, L"Protocol.Name");
  // DEBUG_PRINTF(L"ProtocolName = %s", ProtocolName);
  nb_sessiondialog_t * dlg = reinterpret_cast<nb_sessiondialog_t *>(FHost->query_interface(NBINTF_SESSIONDIALOG, NBINTF_SESSIONDIALOG_VER));
  assert(dlg);
  dlg->add_protocol_description(object, FProtocolID, ProtocolName);
  DEBUG_PRINTF(L"end");
  return Result;
}
//------------------------------------------------------------------------------
subplugin_error_t TSubplugin::OnSessionDialogAfterInitSessionTabs(
  nbptr_t object,
  nbptr_t data,
  nbptr_t common,
  nb_bool_t * bbreak)
{
  DEBUG_PRINTF(L"begin");
  subplugin_error_t Result = SUBPLUGIN_NO_ERROR;
  nb_sessiondialog_t * dlg = reinterpret_cast<nb_sessiondialog_t *>(FHost->query_interface(NBINTF_SESSIONDIALOG, NBINTF_SESSIONDIALOG_VER));
  assert(dlg);
  dlg->setnextitemposition(object, ip_new_line);
  dlg->setdefaultgroup(object, FTabID);
  dlg->newseparator(object, FUtils->get_msg(PLUGIN_GUID, L"Separator.Caption"));
  DEBUG_PRINTF(L"end");
  return Result;
}
//------------------------------------------------------------------------------
subplugin_error_t TSubplugin::OnSessionDialogUpdateControls(
  nbptr_t object,
  nbptr_t data,
  nbptr_t common,
  nb_bool_t * bbreak)
{
  DEBUG_PRINTF(L"begin");
  /*property_baton_t baton;
  baton.struct_size = sizeof(baton);
  baton.subplugin = subplugin;
  baton.notification = notification;

  baton.item_id = 0;
  baton.property_name = L"protocol";
  baton.property_value = NULL;
  // int CurProtocol = reinterpret_cast<int>(FStartupInfo.dialog_item_get_property(subplugin, notification, 0, L"protocol", NULL));
  intptr_t CurProtocol = reinterpret_cast<intptr_t>(FStartupInfo.dialog_item_get_property(&baton));
  // DEBUG_PRINTF(L"FProtocolID = %d, CurProtocol = %d", FProtocolID, CurProtocol);
  // FStartupInfo.dialog_item_set_property(subplugin, notification, FTabControlID,
    // L"enabled", (const void *)(CurProtocol == FProtocolID));
  baton.item_id = FTabControlID;
  baton.property_name = L"enabled";
  baton.property_value = reinterpret_cast<void *>(CurProtocol == FProtocolID);
  FStartupInfo.dialog_item_set_property(&baton);
  */
  DEBUG_PRINTF(L"end");
  return SUBPLUGIN_NO_ERROR;
}
//------------------------------------------------------------------------------
