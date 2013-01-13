#pragma once

#include <apr_pools.h>
#include <apr_strings.h>

#include "SubPluginApiImpl.hpp"

namespace netbox {

TSubpluginsManager * TSubpluginApiImpl::SubpluginsManager = NULL;
TIDAllocator * TSubpluginApiImpl::IDAllocator = NULL;
apr_pool_t * TSubpluginApiImpl::Pool = NULL;

#define IMPL_HOOKS_COUNT 4

static const wchar_t * HookGuids[IMPL_HOOKS_COUNT] =
{
  HOOK_SESSION_DIALOG_INIT_TABS,
  HOOK_SESSION_DIALOG_INIT_SESSION_TAB,
  HOOK_SESSION_DIALOG_AFTER_INIT_TABS,
  HOOK_SESSION_DIALOG_UPDATE_CONTROLS,
};

nb_hooks_t TSubpluginApiImpl::nbHooks =
{
  NBINTF_HOOKS_VER,

  &TSubpluginApiImpl::create_hook,
  &TSubpluginApiImpl::destroy_hook,

  &TSubpluginApiImpl::bind_hook,
  &TSubpluginApiImpl::run_hook,
  &TSubpluginApiImpl::release_hook,
};

nb_utils_t TSubpluginApiImpl::nbUtils = {
  NBINTF_UTILS_VER,

  &TSubpluginApiImpl::get_unique_id,
  &TSubpluginApiImpl::get_msg,

  &TSubpluginApiImpl::versions_equal,
  &TSubpluginApiImpl::check_version,

  &TSubpluginApiImpl::pool_create,
  &TSubpluginApiImpl::pool_destroy,
  &TSubpluginApiImpl::pcalloc,
  &TSubpluginApiImpl::pstrdup,

  &TSubpluginApiImpl::utils_to_utf8,
  &TSubpluginApiImpl::utils_from_utf8,

  &TSubpluginApiImpl::utils_utf8_to_wcs,
  &TSubpluginApiImpl::utils_wcs_to_utf8,
};

nb_config_t TSubpluginApiImpl::nbConfig =
{
  NBINTF_CONFIG_VER,

  &TSubpluginApiImpl::config_get_path,

  &TSubpluginApiImpl::config_set_cfg,
  &TSubpluginApiImpl::config_get_cfg,

  &TSubpluginApiImpl::config_copy,
  &TSubpluginApiImpl::config_release
};

nb_log_t TSubpluginApiImpl::nbLog =
{
  NBINTF_LOGGING_VER,

  &TSubpluginApiImpl::log
};

void TSubpluginApiImpl::InitAPI(TSubpluginsManager * ASubpluginsManager,
  nb_core_t * core,
  apr_pool_t * parent_pool)
{
  SubpluginsManager = ASubpluginsManager;
  Pool = static_cast<apr_pool_t *>(pool_create(parent_pool));
  apr_pool_t * pool = Pool;
  void * mem = apr_pcalloc(pool, sizeof(*IDAllocator));
  IDAllocator = new (mem) TIDAllocator(2000, 54999);

  core->api_version = NBAPI_CORE_VER; // Core API version
  // Interface registry
  core->register_interface = &TSubpluginApiImpl::register_interface;
  core->query_interface = &TSubpluginApiImpl::query_interface;
  core->release_interface = &TSubpluginApiImpl::release_interface;
  // Core functions
  core->has_subplugin = &TSubpluginApiImpl::has_subplugin;
  core->register_fs_protocol = &TSubpluginApiImpl::register_fs_protocol;

  // Interfaces (since these outlast any plugin they don't need to be explictly released)
  core->register_interface(NBINTF_HOOKS, &nbHooks);
  core->register_interface(NBINTF_UTILS, &nbUtils);
  core->register_interface(NBINTF_CONFIG, &nbConfig);
  core->register_interface(NBINTF_LOGGING, &nbLog);
  // Create provided hooks (since these outlast any plugin they don't need to be explictly released)
  for(int I = 0; I < IMPL_HOOKS_COUNT; ++I)
    nbHooks.create_hook(HookGuids[I], NULL);
}

void TSubpluginApiImpl::ReleaseAPI()
{
  apr_pool_clear(Pool);
  Pool = NULL;
}

// core
intf_handle_t NBAPI
TSubpluginApiImpl::register_interface(
  const wchar_t * guid, nbptr_t intf)
{
  intf_handle_t Result = SubpluginsManager->register_interface(guid, intf);
  return Result;
}

nb_interface_t * NBAPI
TSubpluginApiImpl::query_interface(
  const wchar_t * guid, intptr_t version)
{
  nb_interface_t * Result = NULL;
  nb_interface_t * dci = static_cast<nb_interface_t *>(SubpluginsManager->query_interface(guid));
  Result = (!dci || dci->api_version >= version) ? dci : NULL;
  return Result;
}

nb_bool_t NBAPI
TSubpluginApiImpl::release_interface(
  intf_handle_t intf)
{
  nb_bool_t Result = SubpluginsManager->release_interface(intf) == true ? nb_true : nb_false;
  return Result;
}

nb_bool_t NBAPI
TSubpluginApiImpl::has_subplugin(const wchar_t * guid)
{
  nb_bool_t Result = SubpluginsManager->has_subplugin(guid) ? nb_true : nb_false;
  return Result;
}

intptr_t NBAPI
TSubpluginApiImpl::register_fs_protocol(fs_protocol_t * prot)
{
  intptr_t Result = SubpluginsManager->register_fs_protocol(prot);
  return Result;
}

// hooks
hook_handle_t NBAPI
TSubpluginApiImpl::create_hook(
  const wchar_t * guid, nb_hook_t def_proc)
{
  hook_handle_t Result = SubpluginsManager->create_hook(guid, def_proc);
  return Result;
}

nb_bool_t NBAPI
TSubpluginApiImpl::destroy_hook(
  hook_handle_t hook)
{
  nb_bool_t Result = SubpluginsManager->destroy_hook(reinterpret_cast<plugin_hook_t *>(hook)) ? nb_true : nb_false;
  return Result;
}

subs_handle_t NBAPI
TSubpluginApiImpl::bind_hook(
  const wchar_t * guid, nb_hook_t hook_proc, void * common)
{
  subs_handle_t Result = SubpluginsManager->bind_hook(guid, hook_proc, common);
  return Result;
}

nb_bool_t NBAPI
TSubpluginApiImpl::run_hook(
  hook_handle_t hook, nbptr_t object, nbptr_t data)
{
  nb_bool_t Result = SubpluginsManager->RunHook(reinterpret_cast<plugin_hook_t *>(hook), object, data) ? nb_true : nb_false;
  return Result;
}

intptr_t NBAPI
TSubpluginApiImpl::release_hook(
  subs_handle_t hook)
{
  intptr_t Result = SubpluginsManager->release_hook(reinterpret_cast<hook_subscriber_t *>(hook));
  return Result;
}

// utils
intptr_t NBAPI
TSubpluginApiImpl::get_unique_id()
{
  intptr_t Result = IDAllocator->allocate(1);
  return Result;
}

const wchar_t * NBAPI
TSubpluginApiImpl::get_msg(
  const wchar_t * guid, const wchar_t * msg_id)
{
  const wchar_t * Result = SubpluginsManager->GetSubpluginMsg(guid, msg_id);
  return Result;
}

//------------------------------------------------------------------------------
static void get_version_info(intptr_t version,
  intptr_t & major,
  intptr_t & minor,
  intptr_t & patch,
  intptr_t & build)
{
  major = (version >> 28) & 0x0F;
  minor = (version >> 24) & 0x0F;
  patch = (version >> 16) & 0xFF;
  build = version & 0x0FFFF;
}
//------------------------------------------------------------------------------

nb_bool_t NBAPI
TSubpluginApiImpl::versions_equal(
  intptr_t version,
  intptr_t expected_version)
{
  intptr_t major1, minor1, patch1, build1;
  intptr_t major2, minor2, patch2, build2;
  get_version_info(version, major1, minor1, patch1, build1);
  get_version_info(expected_version, major2, minor2, patch2, build2);
  return (major1 == major2 &&
          minor1 == minor2 &&
          patch1 >= patch2 &&
          build1 >= build2) ? nb_true : nb_false;
}

subplugin_error_t NBAPI
TSubpluginApiImpl::check_version(
  intptr_t version,
  intptr_t expected_version)
{
  if (!versions_equal(version, expected_version))
    return SUBPLUGIN_ERR_VERSION_MISMATCH;
  return SUBPLUGIN_NO_ERROR;
}
//------------------------------------------------------------------------------
static int
abort_on_pool_failure(int retcode)
{
  /* Don't translate this string! It requires memory allocation to do so!
     And we don't have any of it... */
  printf("Out of memory - terminating application.\n");
  abort();
  return 0;
}

static apr_pool_t *
pool_create_ex(apr_pool_t * parent_pool,
  apr_allocator_t * allocator)
{
  apr_pool_t * pool = NULL;
  apr_pool_create_ex(&pool, parent_pool, abort_on_pool_failure, allocator);
  return pool;
}

static apr_pool_t *
pool_create(apr_pool_t * parent_pool)
{
  return pool_create_ex(parent_pool, NULL);
}

static void
pool_destroy(apr_pool_t * pool)
{
  apr_pool_destroy(pool);
}
//------------------------------------------------------------------------------
// Create memory pool
void * NBAPI
TSubpluginApiImpl::pool_create(
  void * parent_pool)
{
  return netbox::pool_create(static_cast<apr_pool_t *>(parent_pool));
}

void NBAPI
TSubpluginApiImpl::pool_destroy(
  void * pool)
{
  netbox::pool_destroy(static_cast<apr_pool_t *>(pool));
}

// Allocate memory from pool
void * NBAPI
TSubpluginApiImpl::pcalloc(
  size_t sz, void * pool)
{
  void * Result = NULL;
  assert(pool);
  Result = apr_pcalloc(static_cast<apr_pool_t *>(pool), sz);
  return Result;
}

// Duplicate string
const wchar_t * NBAPI
TSubpluginApiImpl::pstrdup(
  const wchar_t * str, size_t len, void * pool)
{
  wchar_t * Result = NULL;
  assert(pool);
  apr_size_t clen = (len + 1) * sizeof(wchar_t);
  Result = reinterpret_cast<wchar_t *>(apr_pmemdup(
    static_cast<apr_pool_t *>(pool), str, clen));
  Result[len] = 0;
  return Result;
}

intptr_t NBAPI
TSubpluginApiImpl::utils_to_utf8(
  char * dst, const char * src, intptr_t n)
{
  intptr_t Result = 0;
  std::string sSrc(Sysutils::text::toUtf8(src));
  Result = (sSrc.size() < n) ? sSrc.size() : n;
  strncpy(dst, sSrc.c_str(), Result);
  return Result;
}

intptr_t NBAPI
TSubpluginApiImpl::utils_from_utf8(
  char * dst, const char * src, intptr_t n)
{
  intptr_t Result = 0;
  std::string sSrc(Sysutils::text::fromUtf8(src));
  Result = (sSrc.size() < n) ? sSrc.size() : n;
  strncpy(dst, sSrc.c_str(), n);
  return Result;
}

intptr_t NBAPI
TSubpluginApiImpl::utils_utf8_to_wcs(
  wchar_t * dst, const char * src, intptr_t n)
{
  intptr_t Result = 0;
  std::wstring sSrc(Sysutils::text::utf8ToWide(src));
  Result = (sSrc.size() < n) ? sSrc.size() : n;
  wcsncpy(dst, sSrc.c_str(), n);
  return Result;
}

intptr_t NBAPI
TSubpluginApiImpl::utils_wcs_to_utf8(
  char * dst, const wchar_t * src, intptr_t n)
{
  intptr_t Result = 0;
  std::string sSrc(Sysutils::text::wideToUtf8(src));
  Result = (sSrc.size() < n) ? sSrc.size() : n;
  strncpy(dst, sSrc.c_str(), n);
  return Result;
}

// config
const wchar_t * NBAPI
TSubpluginApiImpl::config_get_path(
  nb_path_enum_type_t type)
{
  const wchar_t * Result = 0;
  return Result;
}

void NBAPI
TSubpluginApiImpl::config_set_cfg(
  const wchar_t * guid, const wchar_t * setting,
  config_value_t * val)
{
}

config_value_t * NBAPI
TSubpluginApiImpl::config_get_cfg(
  const wchar_t * guid, const wchar_t * setting, config_type_enum_t type)
{
  config_value_t * Result = 0;
  return Result;
}

config_value_t * NBAPI
TSubpluginApiImpl::config_copy(
  const config_value_t * val)
{
  config_value_t * Result = 0;
  return Result;
}

void NBAPI
TSubpluginApiImpl::config_release(
  config_value_t * val)
{
}

// log
void NBAPI
TSubpluginApiImpl::log(const wchar_t * msg)
{
  SubpluginsManager->log(msg);
}

} // namespace netbox
