#include "afxdll.h"
#include <vcl.h>
#pragma hdrstop

#include <Sysutils.hpp>

#include "FarUtil.h"
#include "resource.h"
#include "Common.h"

#include <subplugin.hpp>
#include "GDisk.hpp"
#include "GDiskSubplugin.hpp"

static HINSTANCE HInstance = 0;
//------------------------------------------------------------------------------

static TSubplugin * CreateSubplugin(HINSTANCE HInst)
{
  return new TSubplugin(HInst);
}

//------------------------------------------------------------------------------
class TFarPluginGuard : public TFarPluginEnvGuard, public TGuard
{
public:
  inline TFarPluginGuard(TCustomFarPlugin * Subplugin) :
    TGuard(Subplugin->GetCriticalSection())
  {
  }
};

//------------------------------------------------------------------------------
extern "C"
{
//------------------------------------------------------------------------------
BOOL WINAPI DllMain(HINSTANCE HInstance, DWORD Reason, LPVOID /*ptr*/ )
{
  BOOL Result = TRUE;
  switch (Reason)
  {
    case DLL_PROCESS_ATTACH:
      ::HInstance = HInstance;
      break;

  }
  return Result;
}

} // extern "C"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
struct gdisk_ctx_t
{
  TSubplugin * Subplugin;
  // subplugin_startup_info_t startup_info;
};
//------------------------------------------------------------------------------

static const subplugin_version_t * get_min_netbox_version()
{
  SUBPLUGIN_VERSION_BODY(2, 1, 20, 200);
}

static const subplugin_version_t * get_subplugin_version()
{
  SUBPLUGIN_VERSION_BODY(SUBPLUGIN_VERSION_MAJOR, SUBPLUGIN_VERSION_MINOR, SUBPLUGIN_VERSION_PATCH, SUBPLUGIN_VERSION_BUILD);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/*static subplugin_error_t NBAPI
notify(subplugin_t * subplugin, const notification_t * notification)
{
  // DEBUG_PRINTF(L"begin");
  gdisk_ctx_t * ctx = static_cast<gdisk_ctx_t *>(subplugin->impl_ctx);
  assert(ctx);
  subplugin_error_t err = ctx->Subplugin->Notify(subplugin, notification);
  // DEBUG_PRINTF(L"end");
  return err;
}*/
//------------------------------------------------------------------------------
/*static subplugin_error_t NBAPI
get_meta_data(subplugin_t * subplugin, subplugin_meta_data_t * meta_data)
{
  // DEBUG_PRINTF(L"begin");
  gdisk_ctx_t * ctx = static_cast<gdisk_ctx_t *>(subplugin->impl_ctx);
  assert(ctx);
  assert(ctx->Subplugin);
  // DEBUG_PRINTF(L"end");
  return ctx->Subplugin->GetMetaData(subplugin, meta_data);
}*/
//------------------------------------------------------------------------------
/*static const subplugin_vtable_t vtable =
{
  sizeof(subplugin_vtable_t),
  notify,
  get_meta_data,
};*/

//------------------------------------------------------------------------------
// Variables

static nb_core_t * host = NULL;
static nb_hooks_t * hooks = NULL;
static nb_utils_t * utils = NULL;
static nb_config_t * config = NULL;
static nb_log_t * logging = NULL;

// Event handlers
static subplugin_error_t NBAPI
OnSessionDialogInitTabs(
  nbptr_t object,
  nbptr_t data,
  nbptr_t common,
  nbBool * bbreak)
{
  DEBUG_PRINTF(L"1");
  logging->log(L"OnSessionDialogInitTabs: begin");
  logging->log(L"OnSessionDialogInitTabs: end");
  DEBUG_PRINTF(L"2");
  return SUBPLUGIN_NO_ERROR;
}

static subplugin_error_t NBAPI
OnSessionDialogInitSessionTab(
  nbptr_t object,
  nbptr_t data,
  nbptr_t common,
  nbBool * bbreak)
{
  DEBUG_PRINTF(L"1");
  logging->log(L"OnSessionDialogInitSessionTab: begin");
  logging->log(L"OnSessionDialogInitSessionTab: end");
  DEBUG_PRINTF(L"2");
  return SUBPLUGIN_NO_ERROR;
}

//------------------------------------------------------------------------------
/* Hook subscription store */
#define HOOKS_SUBSCRIBED 2

static const wchar_t * hookGuids[HOOKS_SUBSCRIBED] =
{
  HOOK_SESSION_DIALOG_INIT_TABS,
  HOOK_SESSION_DIALOG_INIT_SESSION_TAB,
};

static nb_hook_t hookFuncs[HOOKS_SUBSCRIBED] =
{
  &OnSessionDialogInitTabs,
  &OnSessionDialogInitSessionTab,
};

static subs_handle_t subs[HOOKS_SUBSCRIBED];

//------------------------------------------------------------------------------

subplugin_error_t OnLoad(intptr_t state, nb_core_t * core)
{
  DEBUG_PRINTF(L"1");
  host = core;

  hooks = (nb_hooks_t *)host->query_interface(NBINTF_HOOKS, NBINTF_HOOKS_VER);
  utils = (nb_utils_t *)host->query_interface(NBINTF_UTILS, NBINTF_UTILS_VER);
  config = (nb_config_t *)host->query_interface(NBINTF_CONFIG, NBINTF_CONFIG_VER);
  logging = (nb_log_t *)host->query_interface(NBINTF_LOGGING, NBINTF_LOGGING_VER);

  DEBUG_PRINTF(L"logging = %p", logging);
  logging->log(L"OnLoad: begin");
  /*if (state == ON_INSTALL)
  {
    // Default settings
    // set_cfg("SendSuffix", "<DC++ Plugins Test>");
  }*/

  for (intptr_t I = 0; I < HOOKS_SUBSCRIBED; ++I)
  {
    subs[I] = hooks->bind_hook(hookGuids[I], hookFuncs[I], NULL);
  }

  logging->log(L"OnLoad: end");
  DEBUG_PRINTF(L"2");
  return SUBPLUGIN_NO_ERROR;
}

subplugin_error_t OnUnload(intptr_t /* state */)
{
  DEBUG_PRINTF(L"1");
  for (intptr_t I = 0; I < HOOKS_SUBSCRIBED; ++I)
  {
    if (subs[I])
      hooks->release_hook(subs[I]);
  }
  DEBUG_PRINTF(L"2");
  return SUBPLUGIN_NO_ERROR;
}

//------------------------------------------------------------------------------

struct subplugin_impl_t
{
  static subplugin_error_t get_min_netbox_version(const subplugin_version_t ** min_netbox_version)
  {
    *min_netbox_version = ::get_min_netbox_version();
    return SUBPLUGIN_NO_ERROR;
  }

  static subplugin_error_t get_subplugin_version(const subplugin_version_t ** version)
  {
    *version = ::get_subplugin_version();
    return SUBPLUGIN_NO_ERROR;
  }

  static subplugin_error_t init(subplugin_meta_data_t * meta_data)
  {
    meta_data->name = PLUGIN_NAME;
    meta_data->author = PLUGIN_AUTHOR;
    meta_data->description = PLUGIN_DESCRIPTION;
    meta_data->web = PLUGIN_WEB;

    meta_data->guid = PLUGIN_GUID;

    meta_data->api_version = NB_MAKE_VERSION(
      NETBOX_VERSION_MAJOR,
      NETBOX_VERSION_MINOR,
      NETBOX_VERSION_PATCH,
      NETBOX_VERSION_BUILD);
    meta_data->version = NB_MAKE_VERSION(
      SUBPLUGIN_VERSION_MAJOR,
      SUBPLUGIN_VERSION_MINOR,
      SUBPLUGIN_VERSION_PATCH,
      SUBPLUGIN_VERSION_BUILD);
    // Describe plugin dependencies
    return SUBPLUGIN_NO_ERROR;
  }

  static subplugin_error_t main(
    subplugin_state_enum_t state,
    nb_core_t * core,
    nbptr_t data)
  {
    DEBUG_PRINTF(L"begin");
    /*gdisk_ctx_t * ctx = static_cast<gdisk_ctx_t *>(startup_info->NSF->pcalloc(subplugin, sizeof(*ctx)));
    assert(ctx);

    subplugin->vtable = &vtable;
    subplugin->impl_ctx = ctx;

    ctx->Subplugin = CreateSubplugin(::HInstance);
    // DEBUG_PRINTF(L"ctx.Subplugin = %p", ctx->Subplugin);
    */
    subplugin_error_t Result = SUBPLUGIN_NO_ERROR;
    switch (state)
    {
      case ON_INSTALL:
      case ON_LOAD:
        Result = OnLoad(state, core);
        break;
      case ON_UNINSTALL:
      case ON_UNLOAD:
        Result = OnUnload(state);
        break;
      case ON_CONFIGURE:
        // return onConfig(pData);
        break;
      default:
        Result = SUBPLUGIN_NO_ERROR;
        break;
    }
    DEBUG_PRINTF(L"end");
    return Result;
  }

  static subplugin_error_t hook(
    nbptr_t object,
    nbptr_t data,
    nbptr_t common,
    nbBool * bbreak)
  {
    return SUBPLUGIN_NO_ERROR;
  }
/*
  static subplugin_error_t destroy(subplugin_t * subplugin)
  {
    // DEBUG_PRINTF(L"begin");
    gdisk_ctx_t * ctx = static_cast<gdisk_ctx_t *>(subplugin->impl_ctx);
    // DEBUG_PRINTF(L"ctx->Subplugin = %p, sizeof = %d", ctx->Subplugin, sizeof(*ctx->Subplugin));
    assert(ctx->Subplugin);
    SAFE_DESTROY(ctx->Subplugin);
    // DEBUG_PRINTF(L"end");
    return SUBPLUGIN_NO_ERROR;
  }
*/
};

DL_EXPORT(nb::subplugin, subplugin_impl_t)
