#pragma once

#include "Subplugin.hpp"
#include "Main.hpp"

//------------------------------------------------------------------------------
TSubplugin::TSubplugin(HINSTANCE HInst,
  nb_core_t * host) :
  TBaseSubplugin(),
  FHost(host),
  FTabID(0),
  FTabControlID(0),
  FProtocolID(0)
{
  // DEBUG_PRINTF(L"begin");
  FUtils = reinterpret_cast<nb_utils_t *>(FHost->query_interface(NBINTF_UTILS, NBINTF_UTILS_VER));
  FConfig = reinterpret_cast<nb_config_t *>(FHost->query_interface(NBINTF_CONFIG, NBINTF_CONFIG_VER));
  FLogging = reinterpret_cast<nb_log_t *>(FHost->query_interface(NBINTF_LOGGING, NBINTF_LOGGING_VER));

  FPool = FUtils->pool_create(NULL);
  // DEBUG_PRINTF(L"end");
}
//------------------------------------------------------------------------------
TSubplugin::~TSubplugin()
{
  // DEBUG_PRINTF(L"begin");
  FUtils->pool_destroy(FPool);
  // CoreFinalize();
  // DEBUG_PRINTF(L"end");
}
//------------------------------------------------------------------------------
subplugin_error_t TSubplugin::Init()
{
  // DEBUG_PRINTF(L"begin");
  subplugin_error_t Result = SUBPLUGIN_NO_ERROR;
  // Register protocol
  fs_protocol_t * prot = static_cast<fs_protocol_t *>(FUtils->pcalloc(sizeof(*prot), FPool));
  prot->plugin_guid = PLUGIN_GUID;
  prot->fs_name = FUtils->pstrdup(PROTOCOL_NAME, wcslen(PROTOCOL_NAME), FPool);
  FProtocolID = FHost->register_fs_protocol(prot);
  // DEBUG_PRINTF(L"FProtocolID = %d", FProtocolID);
  // DEBUG_PRINTF(L"end");
  return Result;
}

//------------------------------------------------------------------------------
subplugin_error_t TSubplugin::OnSessionDialogInitTabs(
  nbptr_t object,
  nbptr_t data,
  nbptr_t common,
  nb_bool_t * bbreak)
{
  // DEBUG_PRINTF(L"begin");
  subplugin_error_t Result = SUBPLUGIN_NO_ERROR;
  FTabID = FUtils->get_unique_id();
  // DEBUG_PRINTF(L"FTabID = %d", FTabID);
  const wchar_t * TabCaption = FUtils->get_msg(PLUGIN_GUID, L"Tab.Caption");
  // DEBUG_PRINTF(L"TabCaption = %s", TabCaption);
  nb_sessiondialog_t * dlg = reinterpret_cast<nb_sessiondialog_t *>(FHost->query_interface(NBINTF_SESSIONDIALOG, NBINTF_SESSIONDIALOG_VER));
  assert(dlg);
  FTabControlID = dlg->add_tab(object, FTabID, TabCaption);
  // DEBUG_PRINTF(L"end, FTabControlID = %d", FTabControlID);
  return Result;
}
//------------------------------------------------------------------------------
subplugin_error_t TSubplugin::OnSessionDialogAfterInitSessionTabs(
  nbptr_t object,
  nbptr_t data,
  nbptr_t common,
  nb_bool_t * bbreak)
{
  // DEBUG_PRINTF(L"begin");
  subplugin_error_t Result = SUBPLUGIN_NO_ERROR;
  nb_sessiondialog_t * dlg = reinterpret_cast<nb_sessiondialog_t *>(FHost->query_interface(NBINTF_SESSIONDIALOG, NBINTF_SESSIONDIALOG_VER));
  assert(dlg);
  dlg->setnextitemposition(object, ip_new_line);
  dlg->setdefaultgroup(object, FTabID);
  dlg->newseparator(object, FUtils->get_msg(PLUGIN_GUID, L"Separator.Caption"));
  // DEBUG_PRINTF(L"end");
  return Result;
}
//------------------------------------------------------------------------------
subplugin_error_t TSubplugin::OnSessionDialogUpdateControls(
  nbptr_t object,
  nbptr_t data,
  nbptr_t common,
  nb_bool_t * bbreak)
{
  // DEBUG_PRINTF(L"begin");
  nb_sessiondialog_t * dlg = reinterpret_cast<nb_sessiondialog_t *>(FHost->query_interface(NBINTF_SESSIONDIALOG, NBINTF_SESSIONDIALOG_VER));
  assert(dlg);
  intptr_t CurProtocol = dlg->get_property(object, 0, L"protocol");
  dlg->set_property(object, FTabControlID,
    L"enabled", static_cast<intptr_t>(CurProtocol == FProtocolID));
  // DEBUG_PRINTF(L"end");
  return SUBPLUGIN_NO_ERROR;
}
//------------------------------------------------------------------------------
