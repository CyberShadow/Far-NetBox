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

static TSubplugin * CreateSubplugin(HINSTANCE HInst,
  const subplugin_startup_info_t * startup_info)
{
  return new TSubplugin(HInst, startup_info);
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

//------------------------------------------------------------------------------

subplugin_error_t OnLoad(intptr_t state, nb_core_t * core)
{
  host = core;

  hooks = (nb_hooks_t *)host->query_interface(NBINTF_HOOKS, NBINTF_HOOKS_VER);
  utils = (nb_utils_t *)host->query_interface(NBINTF_UTILS, NBINTF_UTILS_VER);
  config = (nb_config_t *)host->query_interface(NBINTF_CONFIG, NBINTF_CONFIG_VER);
  logging = (nb_log_t *)host->query_interface(NBINTF_LOGGING, NBINTF_LOGGING_VER);

  /*if (state == ON_INSTALL)
  {
    // Default settings
    // set_cfg("SendSuffix", "<DC++ Plugins Test>");
  }

  while (i < HOOKS_SUBSCRIBED)
  {
    subs[i] = hooks->bind_hook(hookGuids[i], hookFuncs[i], NULL);
    ++i;
  }*/

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
    // DEBUG_PRINTF(L"begin");
    /*gdisk_ctx_t * ctx = static_cast<gdisk_ctx_t *>(startup_info->NSF->pcalloc(subplugin, sizeof(*ctx)));
    assert(ctx);
    // memset(&ctx->startup_info, 0, sizeof(ctx->startup_info));
    // memmove(&ctx->startup_info, startup_info, startup_info->struct_size >= sizeof(ctx->startup_info) ?
      // sizeof(ctx->startup_info) : startup_info->struct_size);

    subplugin->vtable = &vtable;
    subplugin->impl_ctx = ctx;

    ctx->Subplugin = CreateSubplugin(::HInstance, startup_info);
    // DEBUG_PRINTF(L"ctx.Subplugin = %p", ctx->Subplugin);
    // DEBUG_PRINTF(L"end");
    */
    switch (state)
    {
      case ON_INSTALL:
      case ON_LOAD:
        return OnLoad(state, core);
      case ON_UNINSTALL:
      case ON_UNLOAD:
        // return onUnload();
      case ON_CONFIGURE:
        // return onConfig(pData);
      default:
        return SUBPLUGIN_NO_ERROR;
    }
    return SUBPLUGIN_NO_ERROR;
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
