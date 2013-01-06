#pragma once

/**
 * TSubpluginApiImpl class contains implementations of callback functions
 */

#include <subplugin.hpp>

namespace netbox {

class TSubpluginApiImpl
{
public:
  static void InitAPI(nb_core_t & core);
  static void ReleaseAPI();

private:
  // core
  static intf_handle_t NBAPI register_interface(
    const wchar_t * guid, nbptr_t funcs);
  static nb_interface_t * NBAPI query_interface(
    const wchar_t * guid, intptr_t version);
  static nbBool NBAPI release_interface(
    intf_handle_t intf);

  static nbBool NBAPI has_subplugin(const wchar_t * guid);

  // hooks
  static hook_handle_t NBAPI create_hook(
    const wchar_t * guid, nb_hook_t def_proc);
  static nbBool NBAPI destroy_hook(
    hook_handle_t hook);

  static subs_handle_t NBAPI bind_hook(
    const wchar_t * guid, nb_hook_t hook_proc, void * common);
  static nbBool NBAPI run_hook(
    hook_handle_t hook, nbptr_t object, nbptr_t data);
  static intptr_t NBAPI release_hook(
    subs_handle_t hook);

  // utils
  static intptr_t NBAPI utils_to_utf8(
    char * dst, const char * src, intptr_t n);
  static intptr_t NBAPI utils_from_utf8(
    char * dst, const char * src, intptr_t n);

  static intptr_t NBAPI utils_utf8_to_wcs(
    wchar_t * dst, const char * src, intptr_t n);
  static intptr_t NBAPI utils_wcs_to_utf8(
    char * dst, const wchar_t * src, intptr_t n);

  // config
  static const wchar_t * NBAPI config_get_path(
    nb_path_enum_type_t type);

  static void NBAPI config_set_cfg(
    const wchar_t * guid, const wchar_t * setting,
    config_value_t * val);
  static config_value_t * NBAPI config_get_cfg(
    const wchar_t * guid, const wchar_t * setting, config_type_enum_t type);

  static config_value_t * NBAPI config_copy(
    const config_value_t * val);
  static void NBAPI config_release(
    config_value_t * val);

  // log
  static void NBAPI log(const wchar_t * msg);

private:
  static nb_hooks_t nbHooks;
  static nb_utils_t nbUtils;
  static nb_config_t nbConfig;
  static nb_log_t nbLog;
};

} // namespace netbox
