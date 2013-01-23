#include "Subplugin.hpp"
#include "Main.hpp"
#include "FtpFileSystem.h"

#include "PuttyIntf.h"
#include "Cryptography.h"
#ifndef NO_FILEZILLA
#include "FileZillaIntf.h"
#endif

//------------------------------------------------------------------------------
TSubplugin * Subplugin = NULL;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
TSubplugin::TSubplugin(HINSTANCE HInst,
  nb_core_t * host) :
  TBaseSubplugin(),
  FHInst(HInst),
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
  FImpls = FUtils->hash_create(FPool);
  Subplugin = this;

  InitExtensionModule(HInst);
  // From CoreInitialize
#ifdef _DEBUG
  CallstackTls = TlsAlloc();
#endif
  TRACE("CoreInitialize 1");
  // PuttyInitialize();
  #ifndef NO_FILEZILLA
  TFileZillaIntf::Initialize();
  TFileZillaIntf::SetResourceModule(0);
  #endif
  TRACE("CoreInitialize 2");
  // DEBUG_PRINTF(L"end");
}
//------------------------------------------------------------------------------
TSubplugin::~TSubplugin()
{
  // DEBUG_PRINTF(L"begin");
  FUtils->pool_destroy(FPool);

  // From CoreFinalize();
  #ifndef NO_FILEZILLA
  TFileZillaIntf::Finalize();
  #endif
  PuttyFinalize();

  CryptographyFinalize();
//!CLEANBEGIN
#ifdef _DEBUG
  TlsFree(CallstackTls);
#endif
  TermExtensionModule();
  // DEBUG_PRINTF(L"end");
}
//------------------------------------------------------------------------------
subplugin_error_t TSubplugin::Init()
{
  // DEBUG_PRINTF(L"begin");
  subplugin_error_t Result = SUBPLUGIN_NO_ERROR;
  // Register protocol
  FProtocolID = FHost->register_fs_protocol(&FFtpProtocolInfo);
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
nb_protocol_info_t TSubplugin::FFtpProtocolInfo =
{
  NBAPI_CORE_VER,

  0, // protocol id
  PLUGIN_GUID,
  PROTOCOL_NAME,

  &TSubplugin::create,
};
//------------------------------------------------------------------------------
nb_filesystem_t TSubplugin::FFileSystem =
{
  NBAPI_CORE_VER,

  &TSubplugin::init,
  &TSubplugin::destroy,
  &TSubplugin::open,
  &TSubplugin::close,
  &TSubplugin::get_active,
  &TSubplugin::is_capable,
  &TSubplugin::get_session_url_prefix
};
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
nb_filesystem_t * TSubplugin::create(
  nbptr_t data, // TTerminalIntf * ATerminal
  error_handler_t err)
{
  DEBUG_PRINTF(L"begin");

  nbptr_t FS = new TFTPFileSystem(reinterpret_cast<TTerminalIntf *>(data));
  nb_filesystem_t * object = static_cast<nb_filesystem_t *>(Subplugin->FUtils->pcalloc(sizeof(*object), Subplugin->FPool));

  // object->api_version = NBAPI_CORE_VER;
  // object->init = &TSubplugin::init;
  // object->destroy = &TSubplugin::destroy;
  // object->open = &TSubplugin::open;
  // object->close = &TSubplugin::close;
  // object->is_capable = &TSubplugin::is_capable;
  // object->get_session_url_prefix = &TSubplugin::get_session_url_prefix;
  memmove(object, &FFileSystem, sizeof(FFileSystem));

  Subplugin->FUtils->hash_set(object, FS, Subplugin->FImpls);
  DEBUG_PRINTF(L"end");
  return object;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void NBAPI
TSubplugin::init(
  nb_filesystem_t * object,
  void * data,
  error_handler_t err)
{
  DEBUG_PRINTF(L"begin");
  TFTPFileSystem * FS = static_cast<TFTPFileSystem *>(Subplugin->FUtils->hash_get(object, Subplugin->FImpls));
  DEBUG_PRINTF(L"FS = %x", FS);
  assert(FS);
  FS->Init(data);
  DEBUG_PRINTF(L"end");
}
//------------------------------------------------------------------------------
void NBAPI
TSubplugin::destroy(
  nb_filesystem_t * object,
  error_handler_t err)
{
  DEBUG_PRINTF(L"begin");
  TFTPFileSystem * FS = static_cast<TFTPFileSystem *>(Subplugin->FUtils->hash_get(object, Subplugin->FImpls));
  DEBUG_PRINTF(L"FS = %x", FS);
  assert(FS);
  delete FS;
  Subplugin->FUtils->hash_remove(object, Subplugin->FImpls);
  DEBUG_PRINTF(L"end");
}
//------------------------------------------------------------------------------
void NBAPI
TSubplugin::open(
  nb_filesystem_t * object,
  error_handler_t err)
{
  DEBUG_PRINTF(L"begin");
  TFTPFileSystem * FS = static_cast<TFTPFileSystem *>(Subplugin->FUtils->hash_get(object, Subplugin->FImpls));
  DEBUG_PRINTF(L"FS = %x", FS);
  assert(FS);
  FS->Open();
  DEBUG_PRINTF(L"end");
}
//------------------------------------------------------------------------------
void NBAPI
TSubplugin::close(
  nb_filesystem_t * object,
  error_handler_t err)
{
  DEBUG_PRINTF(L"begin");
  TFTPFileSystem * FS = static_cast<TFTPFileSystem *>(Subplugin->FUtils->hash_get(object, Subplugin->FImpls));
  DEBUG_PRINTF(L"FS = %x", FS);
  assert(FS);
  FS->Close();
  DEBUG_PRINTF(L"end");
}
//------------------------------------------------------------------------------
nb_bool_t NBAPI
TSubplugin::get_active(
  nb_filesystem_t * object,
  error_handler_t err)
{
  nb_bool_t Result = nb_false;
  TFTPFileSystem * FS = static_cast<TFTPFileSystem *>(Subplugin->FUtils->hash_get(object, Subplugin->FImpls));
  DEBUG_PRINTF(L"FS = %x", FS);
  assert(FS);
  Result = FS->GetActive() ? nb_true : nb_false;
  return Result;
}
//------------------------------------------------------------------------------
nb_bool_t NBAPI
TSubplugin::is_capable(
  nb_filesystem_t * object,
  fs_capability_enum_t cap,
  error_handler_t err)
{
  nb_bool_t Result = nb_false;
  bool error = false;
  do
  {
    /*if (!Subplugin->FTerminal)
    {
      error = true;
      break;
    }*/
  } while (0);
  if (error && err)
  {
    err(object, SUBPLUGIN_ERR_NOT_IMPLEMENTED, L"Not implemented");
  }
  return Result;
}
//------------------------------------------------------------------------------
const wchar_t * NBAPI
TSubplugin::get_session_url_prefix(
  nb_filesystem_t * object,
  error_handler_t err)
{
  const wchar_t * Result = L"";
  return Result;
}
//---------------------------------------------------------------------------
//------------------------------------------------------------------------------
