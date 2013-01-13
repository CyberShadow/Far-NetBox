#pragma once

#include "Subplugin.hpp"
#include "Main.hpp"

//------------------------------------------------------------------------------
TSubplugin * Subplugin = NULL;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
TSubplugin::TSubplugin(HINSTANCE HInst,
  nb_core_t * host) :
  TBaseSubplugin(),
  FHost(host),
  FTabID(0),
  FTabControlID(0),
  FProtocolID(0),
  FLastCode(0),
  FLastCodeClass(0),
  FLastResponse(new TStringList()),
  FLastError(new TStringList()),
  FListAll(asAuto)
{
  // DEBUG_PRINTF(L"begin");
  FUtils = reinterpret_cast<nb_utils_t *>(FHost->query_interface(NBINTF_UTILS, NBINTF_UTILS_VER));
  FConfig = reinterpret_cast<nb_config_t *>(FHost->query_interface(NBINTF_CONFIG, NBINTF_CONFIG_VER));
  FLogging = reinterpret_cast<nb_log_t *>(FHost->query_interface(NBINTF_LOGGING, NBINTF_LOGGING_VER));

  FPool = FUtils->pool_create(NULL);
  Subplugin = this;
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
  FProtocolID = FHost->register_fs_protocol(&ftp_prot);
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
//------------------------------------------------------------------------------
static fs_handle_t NBAPI
create(void * data)
{
  fs_handle_t Result = Subplugin->create(data);
  return Result;
}
//------------------------------------------------------------------------------
static intptr_t NBAPI
init(fs_handle_t fs, void * data)
{
  intptr_t Result = Subplugin->init(fs, data);
  return Result;
}
//------------------------------------------------------------------------------
static nb_bool_t NBAPI
is_capable(fs_handle_t fs, fs_capability_enum_t cap)
{
  nb_bool_t Result = Subplugin->is_capable(fs, cap) ? nb_true : nb_false;
  return Result;
}
//------------------------------------------------------------------------------
static const wchar_t * NBAPI
get_session_url_prefix(fs_handle_t fs)
{
  const wchar_t * Result = Subplugin->get_session_url_prefix(fs);
  return Result;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
fs_protocol_t TSubplugin::ftp_prot =
{
  0,
  PLUGIN_GUID, // plugin_guid
  PROTOCOL_NAME, // fs_name

  &::create,
  &::init,
  &::is_capable,
  &::get_session_url_prefix,
};
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
fs_handle_t TSubplugin::create(void * data)
{
  fs_handle_t Result = NULL;
  DEBUG_PRINTF(L"begin");
  ResetReply();

  // FListAll = FTerminal->GetSessionData()->GetFtpListAll();
  // FListAll = GET_AUTO_SWITCH(ListAll);
  FListAll = GetSessionData()->GetFtpListAll();
  // FTimeoutStatus = LoadStr(IDS_ERRORMSG_TIMEOUT);
  // FDisconnectStatus = LoadStr(IDS_STATUSMSG_DISCONNECTED);
  // FServerCapabilities = new TFTPServerCapabilities();
  DEBUG_PRINTF(L"end");
  return Result;
}
//------------------------------------------------------------------------------
intptr_t TSubplugin::init(fs_handle_t fs, void * data)
{
  intptr_t Result = 0;
  DEBUG_PRINTF(L"begin");
  ResetReply();

  // FListAll = FTerminal->GetSessionData()->GetFtpListAll();
  // FListAll = GET_AUTO_SWITCH(ListAll);
  FListAll = GetSessionData()->GetFtpListAll();
  // FTimeoutStatus = LoadStr(IDS_ERRORMSG_TIMEOUT);
  // FDisconnectStatus = LoadStr(IDS_STATUSMSG_DISCONNECTED);
  // FServerCapabilities = new TFTPServerCapabilities();
  DEBUG_PRINTF(L"end");
  return Result;
}
//------------------------------------------------------------------------------
bool TSubplugin::is_capable(fs_handle_t fs, fs_capability_enum_t cap)
{
  bool Result = false;
  return Result;
}
//------------------------------------------------------------------------------
const wchar_t * TSubplugin::get_session_url_prefix(fs_handle_t fs)
{
  const wchar_t * Result = L"";
  return Result;
}
//------------------------------------------------------------------------------
void  TSubplugin::ResetReply()
{
  FLastCode = 0;
  FLastCodeClass = 0;
  assert(FLastResponse != NULL);
  FLastResponse->Clear();
  assert(FLastError != NULL);
  FLastError->Clear();
}
//---------------------------------------------------------------------------
//------------------------------------------------------------------------------
