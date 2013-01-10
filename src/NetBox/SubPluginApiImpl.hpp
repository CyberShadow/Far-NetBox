#pragma once

/**
 * TSubpluginApiImpl class contains implementations of callback functions
 */

#include <subplugin.hpp>

namespace netbox {

class TSubpluginsManager;
class TIDAllocator;

class TSubpluginApiImpl
{
friend class TSubpluginsManager;

public:
  static void InitAPI(TSubpluginsManager * ASubpluginsManager,
    nb_core_t * core,
    apr_pool_t * parent_pool);
  static void ReleaseAPI();

private:
  // core
  static intf_handle_t NBAPI register_interface(
    const wchar_t * guid, nbptr_t funcs);
  static nb_interface_t * NBAPI query_interface(
    const wchar_t * guid, intptr_t version);
  static nb_bool_t NBAPI release_interface(
    intf_handle_t intf);

  static nb_bool_t NBAPI has_subplugin(
    const wchar_t * guid);

  static intptr_t NBAPI register_fs_protocol(
    fs_protocol_t * prot);

  // hooks
  static hook_handle_t NBAPI create_hook(
    const wchar_t * guid, nb_hook_t def_proc);
  static nb_bool_t NBAPI destroy_hook(
    hook_handle_t hook);

  static subs_handle_t NBAPI bind_hook(
    const wchar_t * guid, nb_hook_t hook_proc, void * common);
  static nb_bool_t NBAPI run_hook(
    hook_handle_t hook, nbptr_t object, nbptr_t data);
  static intptr_t NBAPI release_hook(
    subs_handle_t hook);

  // utils
  static intptr_t NBAPI get_unique_id();
  static const wchar_t * NBAPI get_msg(
    const wchar_t * guid, const wchar_t * msg_id);

  static nb_bool_t NBAPI versions_equal(
    const subplugin_version_t * version,
    const subplugin_version_t * expected_version);
  static subplugin_error_t NBAPI check_version(
    const subplugin_version_t * version,
    const subplugin_version_t * expected_version);

  // Create memory pool
  static void * NBAPI pool_create(
    void * parent_pool);
  static void NBAPI pool_destroy(
    void * pool);
  // Allocate memory from pool
  static void * NBAPI pcalloc(
    size_t sz, void * pool);
  // Duplicate string
  static const wchar_t * NBAPI pstrdup(
    const wchar_t * str, size_t len, void * pool);

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
  static TSubpluginsManager * SubpluginsManager;
  static apr_pool_t * Pool;
  static TIDAllocator * IDAllocator;

  static nb_hooks_t nbHooks;
  static nb_utils_t nbUtils;
  static nb_config_t nbConfig;
  static nb_log_t nbLog;
};

//------------------------------------------------------------------------------

class TIDAllocator
{
public:
  TIDAllocator(intptr_t start, intptr_t maximumID) :
    FStart(start),
    FNextID(start),
    FMaximumID(maximumID)
  {}

  /// Returns -1 if not enough available
  intptr_t allocate(intptr_t quantity)
  {
    intptr_t retVal = -1;

    if (FNextID + quantity <= FMaximumID && quantity > 0)
    {
      retVal = FNextID;
      FNextID += quantity;
    }

    return retVal;
  }

  bool isInRange(intptr_t id) { return (id >= FStart && id < FNextID); }

private:
  intptr_t FStart;
  intptr_t FNextID;
  intptr_t FMaximumID;
};

} // namespace netbox
