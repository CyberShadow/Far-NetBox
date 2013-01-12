#pragma once

/** @file subplugin.hpp
  * @brief Subplugins API for NetBox 2.1
  */

#include <stdint.h>
#include <windows.h>

#include <dl/include.hpp>

#define NBAPI WINAPI
#define NBEXP __declspec(dllexport)
#define NBIMP __declspec(dllimport)

#pragma pack(push,2)

#ifdef __cplusplus
extern "C" {
#endif

// Version of the plugin api (must change if old plugins simply can't be seen as viably working)
#define NBAPI_CORE_VER            1

// Data types
typedef void * hook_handle_t, * subs_handle_t, * intf_handle_t, * nbptr_t;
typedef enum tag_nb_bool_t { nb_false = 0, nb_true } nb_bool_t;
typedef uint64_t nbtime_t;

// Hooks (events) system - required interface
#define NBINTF_HOOKS              L"netbox.plugins.hooks"
#define NBINTF_HOOKS_VER          NBAPI_CORE_VER

// Utility and convenience functions provided by NetBox- required interface
#define NBINTF_UTILS              L"netbox.host.utils"
#define NBINTF_UTILS_VER          1

// Recommended interfaces
#define NBINTF_CONFIG             L"netbox.plugins.config"  // Config management
#define NBINTF_CONFIG_VER         1

#define NBINTF_LOGGING            L"netbox.plugins.log"    // Logging functions
#define NBINTF_LOGGING_VER        1

// Session dialog interface
#define NBINTF_SESSIONDIALOG      L"netbox.session.dialog"
#define NBINTF_SESSIONDIALOG_VER  1

// Hook GUID's for Hooks (events) system
#define HOOK_SESSION_DIALOG_INIT_TABS         L"netbox.session.dialog.init.tabs"
#define HOOK_SESSION_DIALOG_INIT_SESSION_TAB  L"netbox.session.dialog.init.session.tab"
#define HOOK_SESSION_DIALOG_AFTER_INIT_TABS   L"netbox.session.dialog.after.init.tabs"
#define HOOK_SESSION_DIALOG_UPDATE_CONTROLS   L"netbox.session.dialog.update.controls"

// Main hook events (returned by init)
typedef enum subplugin_state_enum_t
{
  ON_INSTALL = 0,                   // Replaces ON_LOAD for the very first loading of the plugin
  ON_UNINSTALL,                     // Replaces ON_UNLOAD when plugin is being uninstalled
  ON_LOAD,                          // Sent after successful call to pluginInit (obj: DCCore)
  ON_UNLOAD,                        // Sent right before plugin is unloaded (no params)
  ON_INIT,                          // Sent after all plugins are loaded
  ON_CONFIGURE                      // Sent when user wants to configure the plugin
};

// Argument types
typedef enum config_type_enum_t
{
  CFG_TYPE_UNKNOWN = -2,            // Can be used when querying core settings with magic guid: "CoreSetup"
  CFG_TYPE_REMOVE,                  // Config value will be removed
  CFG_TYPE_STRING,                  // Config value is a string
  CFG_TYPE_INT,                     // Config value is a 32bit integer
  CFG_TYPE_BOOL,                    // Config value is a boolean
  CFG_TYPE_INT64                    // Config value is a 64bit integer
};

// Config Value
struct config_value_t
{
  config_type_enum_t type;          // Indicates which type of value this is
};

// Config Value: string
struct config_str_t
{
  config_type_enum_t type;          // Indicates which type of value this is
  const wchar_t * value;
};

// Config Value: integer
struct config_int_t
{
  config_type_enum_t type;          // Indicates which type of value this is
  int32_t value;
};

// Config Value: boolean
struct config_bool_t
{
  config_type_enum_t type;          // Indicates which type of value this is
  nb_bool_t value;
};

// Config Value: integer (64bit)
struct config_int64_t
{
  config_type_enum_t type;          // Indicates which type the value holds
  int64_t value;
};

// Generic interface dummy
struct nb_interface_t
{
  // The version of the interface
  intptr_t api_version;
};

typedef intptr_t subplugin_error_t;

// Subplugin meta data
struct subplugin_meta_data_t
{
  const wchar_t * name;          // Name of the plugin
  const wchar_t * author;        // Name/Nick of the plugin author
  const wchar_t * description;   // *Short* description of plugin functionality (may be multiple lines)
  const wchar_t * web;           // Authors website if any
  const wchar_t * guid;          // Plugins unique GUID
  const wchar_t ** dependencies; // Array of plugin dependencies
  intptr_t num_dependencies;     // Number of plugin GUIDs in dependencies array
  intptr_t api_version;          // Base API version the plugin was compiled against
  intptr_t version;              // Plugin version
};

typedef enum auto_switch_enum_t
{
  as_on,
  as_off,
  as_auto
};

typedef enum fs_capability_enum_t
{
  fsc_user_group_listing = 0, fsc_mode_changing, fsc_group_changing,
  fsc_owner_changing, fsc_group_owner_changing_by_i_d, fsc_any_command, fsc_hard_link,
  fsc_symbolic_link, fsc_resolve_symlink,
  fsc_text_mode, fsc_rename, fsc_native_text_mode, fsc_newer_only_upload, fsc_remote_copy,
  fsc_timestamp_changing, fsc_remote_move, fsc_loading_additional_properties,
  fsc_checking_space_available, fsc_ignore_perm_errors, fsc_calculating_checksum,
  fsc_mode_changing_upload, fsc_preserving_timestamp_upload, fsc_shell_any_command,
  fsc_secondary_shell, fsc_count
};

// Filesystem protocol implementation
struct fs_protocol_t
{
  intptr_t id;                  // protocol id (filled by host)
  const wchar_t * plugin_guid;  // guid of subplugin
  const wchar_t * fs_name;      // protocol name (filled by subplugin, must be unique)

  intptr_t (NBAPI * init)(
    void * data);
  nb_bool_t (NBAPI * is_capable)(
    fs_capability_enum_t cap);
  const wchar_t * (NBAPI * get_session_url_prefix)();
};

// Error codes
typedef enum subplugin_error_enum_t
{
  SUBPLUGIN_NO_ERROR = 0,
  SUBPLUGIN_UNKNOWN_ERROR = 1000,
  SUBPLUGIN_ERR_WRONG_NETBOX_VERSION = 1001,
  SUBPLUGIN_ERR_VERSION_MISMATCH = 1002,
};

//------------------------------------------------------------------------------
// Macros
//------------------------------------------------------------------------------

#define NB_MAKE_VERSION(major, minor, patch, build) \
  (major << 28) + \
  (minor << 24) + \
  (patch << 16) +  \
  build

#define SUBPLUGIN_ERR(expr)                \
  do {                                     \
    subplugin_error_t err_temp = (expr);   \
    if (err_temp)                          \
      return err_temp;                     \
  } while (0)

// Core plugin system
struct nb_core_t
{
  intptr_t api_version; // Core API version
  // Interface registry
  intf_handle_t (NBAPI * register_interface)(
    const wchar_t * guid, nbptr_t intf);
  nb_interface_t * (NBAPI * query_interface)(
    const wchar_t * guid, intptr_t version);
  nb_bool_t (NBAPI * release_interface)(
    intf_handle_t intf);
  // Check if another plugin is loaded (for soft dependencies)
  nb_bool_t (NBAPI * has_subplugin)(
    const wchar_t * guid);

  // Register fs protocol.
  // @return protocol id
  intptr_t (NBAPI * register_fs_protocol)(
    fs_protocol_t * prot);
};

// Hooks (events) system - required interface

// Hook function prototype
typedef subplugin_error_t (NBAPI * nb_hook_t)(
  nbptr_t object,
  nbptr_t data,
  nbptr_t common,
  nb_bool_t * bbreak);

// Hook system functions
struct nb_hooks_t
{
  // Hooks API version
  intptr_t api_version;

  // Hook creation
  hook_handle_t (NBAPI * create_hook)(
    const wchar_t * guid, nb_hook_t def_proc);
  nb_bool_t (NBAPI * destroy_hook)(
    hook_handle_t hook);

  // Hook interaction
  subs_handle_t (NBAPI * bind_hook)(
    const wchar_t * guid, nb_hook_t hook_proc, void * common);
  nb_bool_t (NBAPI * run_hook)(
    hook_handle_t hook, nbptr_t object, nbptr_t data);
  intptr_t (NBAPI * release_hook)(
    subs_handle_t hook);
};

// Utility and convenience functions
struct nb_utils_t
{
  intptr_t api_version;

  intptr_t (NBAPI * get_unique_id)();
  const wchar_t * (NBAPI * get_msg)(
    const wchar_t * guid, const wchar_t * msg_id);

  nb_bool_t (NBAPI * versions_equal)(
    intptr_t version,
    intptr_t expected_version);
  subplugin_error_t (NBAPI * check_version)(
    intptr_t version,
    intptr_t expected_version);

  // Create memory pool
  void * (NBAPI * pool_create)(
    void * parent_pool);
  void (NBAPI * pool_destroy)(
    void * pool);
  // Allocate memory from pool
  void * (NBAPI * pcalloc)(
    size_t sz, void * pool);
  // Duplicate string
  const wchar_t * (NBAPI * pstrdup)(
    const wchar_t * str, size_t len, void * pool);

  intptr_t (NBAPI * to_utf8)(
    char * dst, const char * src, intptr_t n);
  intptr_t (NBAPI * from_utf8)(
    char * dst, const char * src, intptr_t n);

  intptr_t (NBAPI * utf8_to_wcs)(
    wchar_t * dst, const char * src, intptr_t n);
  intptr_t (NBAPI * wcs_to_utf8)(
    char * dst, const wchar_t * src, intptr_t n);
};

typedef enum nb_path_enum_type_t
{
  PATH_GLOBAL_CONFIG = 0,       // Global configuration
  PATH_USER_CONFIG,             // Per-user configuration (queue, favorites, ...)
  PATH_USER_LOCAL,              // Per-user local data (cache, temp files, ...)
  PATH_RESOURCES,               // Various resources (help files etc)
  PATH_LOCALE                   // Translations
};

// Recommended interfaces

// Config management
struct nb_config_t
{
  intptr_t api_version;

  const wchar_t * (NBAPI * get_path)(
    nb_path_enum_type_t type);

  void (NBAPI * set_cfg)(
    const wchar_t * guid, const wchar_t * setting,
    config_value_t * val);
  config_value_t * (NBAPI * get_cfg)(
    const wchar_t * guid, const wchar_t * setting, config_type_enum_t type);

  config_value_t * (NBAPI * copy)(
    const config_value_t * val);
  void (NBAPI * release)(
    config_value_t * val);
};

// Session edit dialog
typedef enum item_position_t { ip_new_line = 0, ip_below, ip_right };

struct nb_sessiondialog_t
{
  intptr_t api_version;

  intptr_t (NBAPI * add_tab)(
    nbptr_t object, intptr_t tab_id, const wchar_t * tab_caption);
  intptr_t (NBAPI * setnextitemposition)(
    nbptr_t object, item_position_t pos);
  intptr_t (NBAPI * setdefaultgroup)(
    nbptr_t object, intptr_t tab_id);
  intptr_t (NBAPI * newseparator)(
    nbptr_t object, const wchar_t * caption);

  intptr_t (NBAPI * get_property)(
    nbptr_t object, intptr_t item_id, const wchar_t * name);
  intptr_t (NBAPI * set_property)(
    nbptr_t object, intptr_t item_id, const wchar_t * name, intptr_t value);
};

// Logging functions
struct nb_log_t
{
  // Logging API version
  intptr_t api_version;

  void (NBAPI * log)(const wchar_t * msg);
};

#ifdef __cplusplus
}
#endif

#pragma pack(pop)

//------------------------------------------------------------------------------
// Define subplugin library interface
//------------------------------------------------------------------------------

DL_NS_BLOCK((nb)
(
  DL_LIBRARY(subplugin)
  (
    // Subplugin init
    (subplugin_error_t, get_meta_data,
      (subplugin_meta_data_t *, meta_data)
    )
    // Subplugin main function
    (subplugin_error_t, main,
      (subplugin_state_enum_t, state)
      (nb_core_t *, core)
      (nbptr_t, data)
    )
  )
))

