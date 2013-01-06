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
typedef enum tag_nb_bool_t { nbFalse = 0, nbTrue } nbBool;
typedef uint64_t nbtime_t;

// #define Bool nbBool
// #define True nbTrue
// #define False nbFalse

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
  size_t struct_size;
  int32_t major;
  int32_t minor;
  int32_t patch;
  int32_t build;
};

struct subplugin_vtable_t;

/** @brief Subplugin object - holds a loaded subplugin
  *
  */
/*struct subplugin_t
{
  size_t struct_size;
  const subplugin_vtable_t * vtable; // Subplugin functions vtable
  void * pool; // Memory pool used to manage this subplugin
  void * ctx; // Private data for subplugin management
  void * impl_ctx; // Private data for subplugin implementation
};*/

// Notification API

#define SUBPLUGIN_MSG_SESSION_DIALOG_INIT               1000
#define SUBPLUGIN_MSG_SESSION_DIALOG_UPDATE_CONTROLS    1001

struct notification_t
{
  size_t struct_size;
  intptr_t message_id;
  const wchar_t * text;
  intptr_t text_length;
  intptr_t param1;
  void * param2;
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

// Functions implemented by subplugins (optional) 
struct subplugin_vtable_t
{
  size_t struct_size;
  // Notify subplugin
  subplugin_error_t (NBAPI * notify)(
    const notification_t * notification);
  // Get subplugin metadata
  subplugin_error_t (NBAPI * get_meta_data)(
    subplugin_meta_data_t * meta_data);
};

//------------------------------------------------------------------------------
// Error codes
//------------------------------------------------------------------------------

#define SUBPLUGIN_NO_ERROR                   0
#define SUBPLUGIN_UNKNOWN_ERROR              1000
#define SUBPLUGIN_ERR_WRONG_NETBOX_VERSION   1001
#define SUBPLUGIN_ERR_VERSION_MISMATCH       1002

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

struct key_value_pair_t
{
  size_t struct_size;
  intptr_t key;
  const wchar_t * value;
};

struct send_message_baton_t
{
  size_t struct_size;
  const notification_t * notification;
  const wchar_t * message_id;
  const void * message_data;
};

struct property_baton_t
{
  size_t struct_size;
  const notification_t * notification;
  intptr_t item_id;
  const wchar_t * property_name;
  void * property_value;
};

//------------------------------------------------------------------------------
// Define NetBox standard functions
//------------------------------------------------------------------------------

typedef bool (NBAPI *versions_equal_t)(
  const subplugin_version_t * version,
  const subplugin_version_t * expected_version);

typedef subplugin_error_t (NBAPI *check_version_t)(
  const subplugin_version_t * version,
  const subplugin_version_t * expected_version);

typedef void * (NBAPI *pool_create_t)(
  void * parent_pool);

typedef void * (NBAPI *pcalloc_t)(
  size_t sz);

typedef const wchar_t * (NBAPI *pstrdup_t)(
  const wchar_t * str, size_t len);

// Interface registry
typedef intf_handle_t (NBAPI *register_interface_t)(
  const wchar_t * guid, nbptr_t intf);

typedef nb_interface_t * (NBAPI *query_interface_t)(
  const wchar_t * guid, intptr_t version);

typedef nbBool (NBAPI *release_interface_t)(
  intf_handle_t intf);

// Check if another plugin is loaded (for soft dependencies)
typedef nbBool (NBAPI *has_subplugin_t)(
  const wchar_t * guid);

// Core plugin system
struct nb_core_t
{
  size_t struct_size;
  intptr_t api_version; // Core API version
  versions_equal_t versions_equal;
  check_version_t check_version; // Compare subplugin versions
  pool_create_t pool_create; // Create subpool
  pcalloc_t pcalloc; // Allocate memory from pool
  pstrdup_t pstrdup; // Duplicate string
  // Interface registry
  register_interface_t register_interface;
  query_interface_t query_interface;
  release_interface_t release_interface;
  // Check if another plugin is loaded (for soft dependencies)
  has_subplugin_t has_subplugin;
};

typedef intptr_t (NBAPI *get_next_id_t)();
typedef const wchar_t * (NBAPI *get_subplugin_msg_t)(
  const wchar_t * msg_id);
typedef void * (NBAPI *dialog_item_get_property_t)(
  const property_baton_t * baton);
typedef void * (NBAPI *dialog_item_set_property_t)(
  const property_baton_t * baton);
typedef void * (NBAPI *send_message_t)(
  const send_message_baton_t * baton);

/** Subplugin startup info
  */
 
struct subplugin_startup_info_t
{
  size_t struct_size;
  const nb_core_t * NSF;
  get_next_id_t get_next_id;
  get_subplugin_msg_t get_subplugin_msg;
  // get_dialog_item_id_t get_dialog_item_id;
  dialog_item_get_property_t dialog_item_get_property;
  dialog_item_set_property_t dialog_item_set_property;
  send_message_t send_message;
};

// Hooks (events) system - required interface

// Hook function prototype
typedef subplugin_error_t (NBAPI *nb_hook_t)(
  nbptr_t object,
  nbptr_t data,
  nbptr_t common,
  nbBool * bbreak);

// Hook system functions
struct nb_hooks_t
{
  // Hooks API version
  intptr_t api_version;

  // Hook creation
  hook_handle_t (NBAPI * create_hook)(
    const wchar_t * guid, nb_hook_t def_proc);
  nbBool (NBAPI * destroy_hook)(
    hook_handle_t hook);

  // Hook interaction
  subs_handle_t (NBAPI * bind_hook)(
    const wchar_t * guid, nb_hook_t hook_proc, void * common);
  nbBool (NBAPI * run_hook)(
    hook_handle_t hook, nbptr_t object, nbptr_t data);
  intptr_t (NBAPI * release_hook)(
    subs_handle_t hook);
};

// Utility and convenience functions
struct nb_utils_t
{
  // Utility API version
  intptr_t api_version;

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

template <typename T>
bool check_null_or_struct_size(const T * s) { return !s || (s->struct_size >= sizeof(T)); }
template <typename T>
bool check_struct_size(const T * s) { return s && (s->struct_size >= sizeof(T)); }

//------------------------------------------------------------------------------
// Define subplugin library interface
//------------------------------------------------------------------------------

DL_NS_BLOCK((nb)
(
  DL_LIBRARY(subplugin)
  (
    (subplugin_error_t, get_min_netbox_version,
      (const subplugin_version_t **, min_netbox_version))
    (subplugin_error_t, get_subplugin_version,
      (const subplugin_version_t **, version))
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
    // Hook function prototype
    (subplugin_error_t, hook,
      (nbptr_t, object)
      (nbptr_t, data)
      (nbptr_t, common)
      (nbBool *, bbreak)
    )
  )
))

