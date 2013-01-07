#pragma once

/** @file subplugin.hpp
  * @brief Subplugins API for NetBox 2.1
  */

#include <stdint.h>
#include <windows.h>

#include <dl\include.hpp>
#include <plugin.hpp>
#include <plugin_version.hpp>

#define NBAPI WINAPI
#define NBEXP __declspec(dllexport)
#define NBIMP __declspec(dllimport)

#pragma pack(push,2)

#ifdef __cplusplus
extern "C" {
#endif

// Version of the plugin api (must change if old plugins simply can't be seen as viably working)
#define NBAPI_CORE_VER            1

//------------------------------------------------------------------------------
// Typedefs
//------------------------------------------------------------------------------

// Data types
typedef void * hook_handle_t, * subs_handle_t, * intf_handle_t, * nbptr_t;
typedef enum tag_nb_bool_t { nb_false = 0, nb_true } nb_bool_t;
typedef uint64_t nbtime_t;

// Hooks (events) system - required interface
#define NBINTF_HOOKS              L"netbox.plugins.hooks"
#define NBINTF_HOOKS_VER          NBAPI_CORE_VER

// Utility and convenience functions - required interface
#define NBINTF_UTILS              L"netbox.plugins.utils"
#define NBINTF_UTILS_VER          1

// Recommended interfaces
#define NBINTF_CONFIG             L"netbox.plugins.config"  // Config management
#define NBINTF_CONFIG_VER         1

#define NBINTF_LOGGING            L"netbox.plugins.log"    // Logging functions
#define NBINTF_LOGGING_VER        1

// Optional interfaces
// #define NBINTF_NETBOX_UTILS       L"netbox.utils.nbutils"    // Utility and convenience functions
// #define NBINTF_NETBOX_UTILS_VER   1

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
  ON_CONFIGURE                      // Sent when user wants to configure the plugin (obj: DCCore, data: impl. dependant)
};

// Argument types
typedef enum config_type_enum_t
{
  CFG_TYPE_UNKNOWN = -2,                    // Can be used when querying core settings with magic guid: "CoreSetup"
  CFG_TYPE_REMOVE,                      // Config value will be removed
  CFG_TYPE_STRING,                      // Config value is a string
  CFG_TYPE_INT,                        // Config value is a 32bit integer
  CFG_TYPE_BOOL,                        // Config value is a bool
  CFG_TYPE_INT64                        // Config value is a 64bit integer
};

// Config Value
struct config_value_t
{
  config_type_enum_t type;                      // Indicates which type of value this is
};

// Config Value: string
struct config_str_t
{
  config_type_enum_t type;                      // Indicates which type of value this is
  const wchar_t * value;
};

// Config Value: integer
struct config_int_t
{
  config_type_enum_t type;                      // Indicates which type of value this is
  int32_t value;
};

// Config Value: boolean
struct config_bool_t
{
  config_type_enum_t type;                      // Indicates which type of value this is
  bool value;
};

// Config Value: integer (64bit)
struct config_int64_t
{
  config_type_enum_t type;                      // Indicates which type the value holds
  int64_t value;
};

// Generic interface dummy
struct nb_interface_t
{
  // The version of the interface
  intptr_t api_version;
};

typedef intptr_t subplugin_error_t;

struct subplugin_version_t
{
  int32_t major;
  int32_t minor;
  int32_t patch;
  int32_t build;
};

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

//------------------------------------------------------------------------------
// Error codes
//------------------------------------------------------------------------------

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
  (major << 24) + \
  (minor << 16) + \
  (patch << 8) + \
  build

#define SUBPLUGIN_VERSION_DEFINE(name, major, minor, patch, build) \
  static const subplugin_version_t name = \
    { \
      sizeof(subplugin_version_t), \
      major, minor, patch, build \
    }

#define SUBPLUGIN_VERSION_BODY(major, minor, patch, build) \
  SUBPLUGIN_VERSION_DEFINE(versioninfo, major, minor, patch, build);              \
  return &versioninfo

#define SUBPLUGIN_ERR(expr)                 \
  do {                                   \
    subplugin_error_t err__temp = (expr);   \
    if (err__temp)                       \
      return err__temp;                  \
  } while (0)

//------------------------------------------------------------------------------
// Define NetBox standard functions
//------------------------------------------------------------------------------

// Interface registry
typedef intf_handle_t (NBAPI *register_interface_t)(
  const wchar_t * guid, nbptr_t intf);

typedef nb_interface_t * (NBAPI *query_interface_t)(
  const wchar_t * guid, intptr_t version);

typedef nb_bool_t (NBAPI *release_interface_t)(
  intf_handle_t intf);

// Check if another plugin is loaded (for soft dependencies)
typedef nb_bool_t (NBAPI *has_subplugin_t)(
  const wchar_t * guid);

// Core plugin system
struct nb_core_t
{
  intptr_t api_version; // Core API version
  // Interface registry
  register_interface_t register_interface;
  query_interface_t query_interface;
  release_interface_t release_interface;
  // Check if another plugin is loaded (for soft dependencies)
  has_subplugin_t has_subplugin;
};

// Hooks (events) system - required interface

// Hook function prototype
typedef subplugin_error_t (NBAPI *nb_hook_t)(
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
  // Utility API version
  intptr_t api_version;

  intptr_t (NBAPI * get_unique_id)();
  const wchar_t * (NBAPI * get_msg)(
    const wchar_t * guid, const wchar_t * msg_id);

  nb_bool_t (NBAPI * versions_equal)(
    const subplugin_version_t * version,
    const subplugin_version_t * expected_version);
  subplugin_error_t (NBAPI * check_version)(
    const subplugin_version_t * version,
    const subplugin_version_t * expected_version);

  // Create memory pool
  void * (NBAPI * pool_create)(
    void * parent_pool);
  void (NBAPI * pool_destroy)(
    void * pool);
  // Allocate memory from pool
  void * (NBAPI * pcalloc)(
    size_t sz);
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

  // intptr_t (NBAPI * to_base32)(char * dst, const uint8_t * src, intptr_t n);
  // intptr_t (NBAPI * from_base32)(uint8_t * dst, const char * src, intptr_t n);
};

typedef enum nb_path_enum_type_t
{
  PATH_GLOBAL_CONFIG = 0,                    // Global configuration
  PATH_USER_CONFIG,                      // Per-user configuration (queue, favorites, ...)
  PATH_USER_LOCAL,                      // Per-user local data (cache, temp files, ...)
  PATH_RESOURCES,                        // Various resources (help files etc)
  PATH_LOCALE                          // Translations
};

// Recommended interfaces

// Config management
struct nb_config_t
{
  // Config API version
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
  // API version
  intptr_t api_version;

  intptr_t (NBAPI * add_tab)(
    nbptr_t object, intptr_t tab_id, const wchar_t * tab_caption);
  intptr_t (NBAPI * add_protocol_description)(
    nbptr_t object, intptr_t protocol_id, const wchar_t * protocol_name);
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
    (subplugin_error_t, init,
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

