#include "afxdll.h"
#include <vcl.h>
#pragma hdrstop

#include <Sysutils.hpp>

#include "FarUtil.h"
#include "resource.h"
#include "Common.h"

#include "subplugin.hpp"
#include "FtpFz3.hpp"
#include "FtpFz3Subplugin.hpp"

static HINSTANCE HInstance = 0;
//------------------------------------------------------------------------------

static TSubplugin * __fastcall CreateSubplugin(HINSTANCE HInst,
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

    case DLL_PROCESS_DETACH:
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
static subplugin_error_t NBAPI
notify(subplugin_t * subplugin, const notification_t * notification)
{
  // DEBUG_PRINTF(L"begin");
  gdisk_ctx_t * ctx = static_cast<gdisk_ctx_t *>(subplugin->impl_ctx);
  assert(ctx);
  subplugin_error_t err = ctx->Subplugin->Notify(subplugin, notification);
  // DEBUG_PRINTF(L"end");
  return err;
}
//------------------------------------------------------------------------------
static const subplugin_vtable_t vtable =
{
  sizeof(subplugin_vtable_t),
  notify, // notify
};
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

  static subplugin_error_t init(
    const subplugin_version_t * netbox_version,
    const subplugin_startup_info_t * startup_info,
    subplugin_t * subplugin)
  {
    // DEBUG_PRINTF(L"begin");
    (void)netbox_version;
    assert(startup_info);
    assert(subplugin);
    gdisk_ctx_t * ctx = static_cast<gdisk_ctx_t *>(startup_info->NSF->pcalloc(subplugin, sizeof(*ctx)));
    assert(ctx);
    // memset(&ctx->startup_info, 0, sizeof(ctx->startup_info));
    // memmove(&ctx->startup_info, startup_info, startup_info->struct_size >= sizeof(ctx->startup_info) ?
      // sizeof(ctx->startup_info) : startup_info->struct_size);

    subplugin->vtable = &vtable;
    subplugin->impl_ctx = ctx;

    ctx->Subplugin = CreateSubplugin(::HInstance, startup_info);
    // DEBUG_PRINTF(L"ctx.Subplugin = %p", ctx->Subplugin);
    // DEBUG_PRINTF(L"end");
    return SUBPLUGIN_NO_ERROR;
  }

  static subplugin_error_t destroy(subplugin_t * subplugin)
  {
    gdisk_ctx_t * ctx = static_cast<gdisk_ctx_t *>(subplugin->impl_ctx);
    assert(ctx->Subplugin);
    SAFE_DESTROY(ctx->Subplugin);
    return SUBPLUGIN_NO_ERROR;
  }

};

DL_EXPORT(nb::subplugin, subplugin_impl_t)
