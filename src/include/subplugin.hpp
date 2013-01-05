#pragma once

/** @file subplugin.hpp
  * @brief Subplugins API for NetBox 2.1
  */

#include <stdint.h>
#include <windows.h>

#include <dl\include.hpp>
#include <plugin.hpp>
#include <plugin_version.hpp>

#pragma pack(push,2)

#define NBAPI WINAPI
#define NBEXP __declspec(dllexport)
#define NBIMP __declspec(dllimport)

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Typedefs
//------------------------------------------------------------------------------

// Data types
typedef void *hook_handle_t, *subs_handle_t, *intf_handle_t, *nbptr_t;
typedef enum tag_nb_bool { nbFalse = 0, nbTrue } nbBool;
typedef uint64_t nbtime_t;

// #define Bool nbBool
// #define True nbTrue
// #define False nbFalse

/* Argument types */
typedef enum config_type_enum_t
{
  CFG_TYPE_UNKNOWN = -2,                    /* Can be used when querying core settings with magic guid: "CoreSetup" */
  CFG_TYPE_REMOVE,                      /* Config value will be removed */
  CFG_TYPE_STRING,                      /* Config value is a string */
  CFG_TYPE_INT,                        /* Config value is a 32bit integer */
  CFG_TYPE_BOOL,                        /* Config value is a bool */
  CFG_TYPE_INT64                        /* Config value is a 64bit integer */
};

// Config Value: string
struct config_str_t
{
  config_type_enum_t type;                      /* Indicates which type of value this is */
  const wchar_t * value;
};

// Config Value: integer
struct config_int_t
{
  config_type_enum_t type;                      /* Indicates which type of value this is */
  int32_t value;
};

// Config Value: boolean
struct config_bool_t
{
  config_type_enum_t type;                      /* Indicates which type of value this is */
  bool value;
};

// Config Value: integer (64bit)
struct config_int64_t
{
  config_type_enum_t type;                      /* Indicates which type the value holds */
  int64_t value;
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
struct subplugin_t
{
  size_t struct_size;
  const subplugin_vtable_t * vtable; // Subplugin functions vtable
  void * pool; // Memory pool used to manage this subplugin
  void * ctx; // Private data for subplugin management
  void * impl_ctx; // Private data for subplugin implementation
};

//------------------------------------------------------------------------------
// Messages
//------------------------------------------------------------------------------

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

/* Plugin meta data */
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

struct subplugin_vtable_t
{
  size_t struct_size;
  // Notify subplugin
  subplugin_error_t (NBAPI * notify)(
    subplugin_t * subplugin, const notification_t * notification);
  // Get subplugin metadata
  subplugin_error_t (NBAPI * get_meta_data)(
    subplugin_t * subplugin, subplugin_meta_data_t * meta_data);
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
  subplugin_t * subplugin;
  const notification_t * notification;
  const wchar_t * message_id;
  const void * message_data;
};

struct property_baton_t
{
  size_t struct_size;
  subplugin_t * subplugin;
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
  subplugin_t * subplugin, size_t sz);

typedef const wchar_t * (NBAPI *pstrdup_t)(
  subplugin_t * subplugin, const wchar_t * str, size_t len);

// Check if another plugin is loaded (for soft dependencies)
typedef nbBool (NBAPI *has_plugin_t)(
  subplugin_t * subplugin, const wchar_t * guid);

struct netbox_standard_functions_t
{
  size_t struct_size;
  versions_equal_t versions_equal;
  check_version_t check_version; // Compare subplugin versions
  pool_create_t pool_create; // Create subpool
  pcalloc_t pcalloc; // Allocate memory from pool
  pstrdup_t pstrdup; // Duplicate string
  has_plugin_t has_plugin;
};

typedef intptr_t (NBAPI *get_next_id_t)(
  subplugin_t * subplugin);
typedef const wchar_t * (NBAPI *get_subplugin_msg_t)(
  subplugin_t * subplugin, const wchar_t * msg_id);
typedef void * (NBAPI *dialog_item_get_property_t)(
  const property_baton_t * baton);
typedef void * (NBAPI *dialog_item_set_property_t)(
  const property_baton_t * baton);
typedef void * (NBAPI *send_message_t)(
  const send_message_baton_t * baton);

/** Subplugin startup info
  *
  */
struct subplugin_startup_info_t
{
  size_t struct_size;
  const netbox_standard_functions_t * NSF;
  get_next_id_t get_next_id;
  get_subplugin_msg_t get_subplugin_msg;
  // get_dialog_item_id_t get_dialog_item_id;
  dialog_item_get_property_t dialog_item_get_property;
  dialog_item_set_property_t dialog_item_set_property;
  send_message_t send_message;
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
    (subplugin_error_t, init,
      (const subplugin_version_t *, netbox_version)
      (const subplugin_startup_info_t *, startup_info)
      (subplugin_t *, subplugin))
    (subplugin_error_t, destroy,
      (subplugin_t *, subplugin))
  )
))

