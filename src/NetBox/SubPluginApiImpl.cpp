#pragma once

#include "SubPluginApiImpl.hpp"

namespace netbox {

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

void TSubpluginApiImpl::InitAPI(nb_core_t & core)
{
  core.struct_size = sizeof(core);
  core.api_version = NBAPI_CORE_VER; // Core API version
  // versions_equal_t versions_equal;
  // check_version_t check_version; // Compare subplugin versions
  // pool_create_t pool_create; // Create subpool
  // pcalloc_t pcalloc; // Allocate memory from pool
  // pstrdup_t pstrdup; // Duplicate string
  // Interface registry
  core.register_interface = &TSubpluginApiImpl::register_interface;
  core.query_interface = &TSubpluginApiImpl::query_interface;
  core.release_interface = &TSubpluginApiImpl::release_interface;
  // Core functions
  core.has_subplugin = &TSubpluginApiImpl::has_subplugin;

  // Interfaces (since these outlast any plugin they don't need to be explictly released)
  core.register_interface(NBINTF_HOOKS, &nbHooks);
  core.register_interface(NBINTF_UTILS, &nbUtils);
  core.register_interface(NBINTF_CONFIG, &nbConfig);
  core.register_interface(NBINTF_LOGGING, &nbLog);
  // Create provided hooks (since these outlast any plugin they don't need to be explictly released)
  // Check if another plugin is loaded (for soft dependencies)
}

void TSubpluginApiImpl::ReleaseAPI()
{
}

// core
intf_handle_t NBAPI TSubpluginApiImpl::register_interface(
  const wchar_t * guid, nbptr_t funcs)
{
  intf_handle_t Result = NULL;
  return Result;
}

nb_interface_t * NBAPI TSubpluginApiImpl::query_interface(
  const wchar_t * guid, intptr_t version)
{
  nb_interface_t * Result = NULL;
  return Result;
}

nbBool NBAPI TSubpluginApiImpl::release_interface(
  intf_handle_t intf)
{
  nbBool Result = nbFalse;
  return Result;
}

nbBool NBAPI TSubpluginApiImpl::has_subplugin(const wchar_t * guid)
{
  nbBool Result = nbFalse;
  return Result;
}

// hooks
hook_handle_t NBAPI TSubpluginApiImpl::create_hook(
  const wchar_t * guid, nb_hook_t def_proc)
{
  hook_handle_t Result = NULL;
  return Result;
}

nbBool NBAPI TSubpluginApiImpl::destroy_hook(
  hook_handle_t hook)
{
  nbBool Result = nbFalse;
  return Result;
}

subs_handle_t NBAPI TSubpluginApiImpl::bind_hook(
  const wchar_t * guid, nb_hook_t hook_proc, void * common)
{
  subs_handle_t Result = NULL;
  return Result;
}

nbBool NBAPI TSubpluginApiImpl::run_hook(
  hook_handle_t hook, nbptr_t object, nbptr_t data)
{
  nbBool Result = nbFalse;
  return Result;
}

intptr_t NBAPI TSubpluginApiImpl::release_hook(
  subs_handle_t hook)
{
  intptr_t Result = 0;
  return Result;
}

// utils
intptr_t NBAPI TSubpluginApiImpl::utils_to_utf8(
  char * dst, const char * src, intptr_t n)
{
  intptr_t Result = 0;
  return Result;
}

intptr_t NBAPI TSubpluginApiImpl::utils_from_utf8(
  char * dst, const char * src, intptr_t n)
{
  intptr_t Result = 0;
  return Result;
}

intptr_t NBAPI TSubpluginApiImpl::utils_utf8_to_wcs(
  wchar_t * dst, const char * src, intptr_t n)
{
  intptr_t Result = 0;
  return Result;
}

intptr_t NBAPI TSubpluginApiImpl::utils_wcs_to_utf8(
  char * dst, const wchar_t * src, intptr_t n)
{
  intptr_t Result = 0;
  return Result;
}

// config
const wchar_t * NBAPI TSubpluginApiImpl::config_get_path(
  nb_path_enum_type_t type)
{
  const wchar_t * Result = 0;
  return Result;
}

void NBAPI TSubpluginApiImpl::config_set_cfg(
  const wchar_t * guid, const wchar_t * setting,
  config_value_t * val)
{
}

config_value_t * NBAPI TSubpluginApiImpl::config_get_cfg(
  const wchar_t * guid, const wchar_t * setting, config_type_enum_t type)
{
  config_value_t * Result = 0;
  return Result;
}


config_value_t * NBAPI TSubpluginApiImpl::config_copy(
  const config_value_t * val)
{
  config_value_t * Result = 0;
  return Result;
}

void NBAPI TSubpluginApiImpl::config_release(
  config_value_t * val)
{
}

// log
void NBAPI TSubpluginApiImpl::log(const wchar_t * msg)
{
}

} // namespace netbox
