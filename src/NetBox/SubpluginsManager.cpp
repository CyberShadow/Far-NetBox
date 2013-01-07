#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_hash.h>
#include <apr_tables.h>

#include <subplugin.hpp>
#include "SubpluginsManager.hpp"

namespace netbox {

//------------------------------------------------------------------------------
// Return plugin version info
static const subplugin_version_t *
get_plugin_version()
{
  static const subplugin_version_t versioninfo =
  {
    sizeof(subplugin_version_t),
    NETBOX_VERSION_MAJOR,
    NETBOX_VERSION_MINOR,
    NETBOX_VERSION_PATCH,
    NETBOX_VERSION_BUILD
  };
  return &versioninfo;
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
//------------------------------------------------------------------------------
static apr_pool_t *
pool_create_ex(apr_pool_t * parent_pool,
               apr_allocator_t * allocator)
{
  apr_pool_t * pool = NULL;
  apr_pool_create_ex(&pool, parent_pool, abort_on_pool_failure, allocator);
  return pool;
}
//------------------------------------------------------------------------------
static apr_pool_t *
pool_create(apr_pool_t * parent_pool)
{
  return pool_create_ex(parent_pool, NULL);
}

static void
pool_destroy(apr_pool_t * pool)
{
  return apr_pool_destroy(pool);
}

//------------------------------------------------------------------------------
// NetBox standard functions
//------------------------------------------------------------------------------

static bool NBAPI
api_versions_equal(const subplugin_version_t * version,
  const subplugin_version_t * expected_version)
{
  // DEBUG_PRINTF(L"version = %d,%d,%d,%d", version->major, version->minor, version->patch, version->build);
  // DEBUG_PRINTF(L"expected_version = %d,%d,%d,%d", expected_version->major, expected_version->minor, expected_version->patch, expected_version->build);
  return (version->major == expected_version->major &&
          version->minor == expected_version->minor &&
          version->patch >= expected_version->patch &&
          version->build >= expected_version->build);
}

static subplugin_error_t NBAPI
api_check_version(const subplugin_version_t * version,
  const subplugin_version_t * expected_version)
{
  if (!api_versions_equal(version, expected_version))
    return SUBPLUGIN_ERR_VERSION_MISMATCH;
  return SUBPLUGIN_NO_ERROR;
}

static void * NBAPI
api_pool_create(void * parent_pool)
{
  return pool_create(static_cast<apr_pool_t *>(parent_pool));
}

static void * NBAPI
api_pcalloc(size_t sz)
{
  void * Result = NULL;
  // assert(subplugin->pool);
  // return apr_pcalloc(static_cast<apr_pool_t *>(subplugin->pool), sz);
  return Result;
}

static const wchar_t * NBAPI
api_pstrdup(const wchar_t * str, apr_size_t len, apr_pool_t * pool)
{
  wchar_t * Result = NULL;
  assert(pool);
  apr_size_t clen = (len + 1) * sizeof(wchar_t);
  Result = reinterpret_cast<wchar_t *>(apr_pmemdup(pool, str, clen));
  Result[len] = 0;
  return Result;
}

/* Interface registry */
/*static intf_handle_t NBAPI
api_register_interface(
  const wchar_t * guid, nbptr_t pInterface)
{
  intf_handle_t Result = NULL;
  return Result;
}

static nb_interface_t * NBAPI
api_query_interface(
  const wchar_t * guid, intptr_t version)
{
  nb_interface_t * Result = NULL;
  return Result;
}

static nbBool NBAPI
api_release_interface(
  intf_handle_t hInterface)
{
  nbBool Result = nbFalse;
  return Result;
}

static nbBool NBAPI
api_has_subplugin(const wchar_t * guid)
{
  nbBool Result = nbFalse;
  // subplugin_info_t * info = static_cast<subplugin_info_t *>(subplugin->ctx);
  // assert(info);
  // return info->manager->HasSubplugin(guid);
  return Result;
}
*/
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Holds a loaded subplugin
struct subplugin_info_t
{
  size_t struct_size;
  const nb::subplugin * subplugin_library;
  const wchar_t * module_name;
  const wchar_t * msg_file_name_ext;
  apr_hash_t * msg_hash; // subplugin localized messages (int wchar_t * format)
  subplugin_meta_data_t * meta_data; // subplugin metadata
  TSubpluginsManager * manager;
  apr_pool_t * pool;
};
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// static intptr_t NBAPI
// api_get_next_id(subplugin_info_t * info)
// {
  // if (!check_struct_size(subplugin)) return NULL;
  // subplugin_info_t * info = static_cast<subplugin_info_t *>(subplugin->ctx);
  // assert(info);
  // return info->manager->GetNextID();
// }
//------------------------------------------------------------------------------
// static const wchar_t * NBAPI
// api_get_subplugin_msg(subplugin_info_t * info,
  // const wchar_t * msg_id)
// {
  // if (!check_struct_size(subplugin)) return NULL;
  // subplugin_info_t * info = static_cast<subplugin_info_t *>(subplugin->ctx);
  // assert(info);
  // return info->manager->GetSubpluginMsg(subplugin, msg_id);
// }
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/*static void * NBAPI
api_dialog_item_get_property(
  const property_baton_t * baton)
{
  if (!check_struct_size(baton)) return NULL;
  subplugin_info_t * info = static_cast<subplugin_info_t *>(baton->subplugin->ctx);
  assert(info);
  return info->manager->DialogItemGetProperty(baton);
}*/
//------------------------------------------------------------------------------
/*static void * NBAPI
api_dialog_item_set_property(
  const property_baton_t * baton)
{
  if (!check_struct_size(baton)) return NULL;
  subplugin_info_t * info = static_cast<subplugin_info_t *>(baton->subplugin->ctx);
  assert(info);
  return info->manager->DialogItemSetProperty(baton);
}*/
//------------------------------------------------------------------------------
/*static void * NBAPI
api_send_message(
  const send_message_baton_t * baton)
{
  if (!check_struct_size(baton)) return NULL;
  subplugin_info_t * info = static_cast<subplugin_info_t *>(baton->subplugin->ctx);
  assert(info);
  return info->manager->SendMessage(baton);
}*/
//------------------------------------------------------------------------------
// a cleanup routine attached to the pool that contains subplugin
static apr_status_t
cleanup_subplugin_info(void * ptr)
{
  DEBUG_PRINTF(L"begin");
  subplugin_info_t * info = static_cast<subplugin_info_t *>(ptr);
  assert(info);
  try
  {
    bool isSafe = true;
    // HMODULE handle = NULL;
    if (info->subplugin_library->main(ON_UNLOAD, NULL, NULL) != SUBPLUGIN_NO_ERROR)
    {
      // Plugin performs operation critical tasks (runtime unload not possible)
      // HMODULE handle = info->subplugin_library->get_hmodule();
      // isSafe = !info->manager->AddInactivePlugin(handle);
    }
    if (isSafe) // && handle != NULL)
    {
      typedef nb::subplugin subplugin_t;
      info->subplugin_library->~subplugin_t();
      // handle = NULL;
    }
  }
  catch (const std::exception & e)
  {
    DEBUG_PRINTF2("Error: %s", e.what());
    // TODO: log into file
  }
  DEBUG_PRINTF(L"end");
  return APR_SUCCESS;
}
//------------------------------------------------------------------------------
// Initialize subplugin_info_t
static subplugin_error_t
init_subplugin_info(subplugin_info_t ** subplugin_info,
  const nb::subplugin * subplugin_library,
  const UnicodeString & ModuleName,
  TSubpluginsManager * manager,
  apr_pool_t * pool)
{
  subplugin_info_t * info =
    static_cast<subplugin_info_t *>(apr_pcalloc(pool, sizeof(*info)));
  info->struct_size = sizeof(*info);
  info->subplugin_library = subplugin_library;
  info->module_name = api_pstrdup(ModuleName.c_str(), ModuleName.Length(), pool);
  info->msg_hash = apr_hash_make(pool);
  info->meta_data =
    static_cast<subplugin_meta_data_t *>(apr_pcalloc(pool, sizeof(*info->meta_data)));
  info->manager = manager;
  info->pool = pool;
  apr_pool_cleanup_register(pool, info, cleanup_subplugin_info, apr_pool_cleanup_null);
  *subplugin_info = info;
  return SUBPLUGIN_NO_ERROR;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
TSubpluginsManager::TSubpluginsManager(TWinSCPFileSystem * FileSystem) :
  FFileSystem(FileSystem),
  FSubplugins(new TList()),
  FPool(NULL),
  FSecNum(rand())
{
  memset(&FCore, 0, sizeof(FCore));
  if (apr_initialize() != APR_SUCCESS)
    throw ExtException(UnicodeString(L"Cannot init APR"));
  FPool = pool_create(NULL);
}
//------------------------------------------------------------------------------
void TSubpluginsManager::Init()
{
  apr_pool_t * pool = FPool;
  void * mem = apr_pcalloc(pool, sizeof(*FIDAllocator));
  FIDAllocator = new (mem) TIDAllocator(2000, 54999);
  FHooks = apr_hash_make(pool);
  FInterfaces = apr_hash_make(pool);
  LoadSubplugins(FPool);
}
//------------------------------------------------------------------------------
void TSubpluginsManager::Shutdown()
{
  UnloadSubplugins();
  // TODO: Notify subplugins before unload
  apr_pool_clear(FPool);
  FSubplugins->Clear();
}
//------------------------------------------------------------------------------
TSubpluginsManager::~TSubpluginsManager()
{
  // DEBUG_PRINTF(L"begin")
  delete FSubplugins;
  FSubplugins = NULL;
  apr_terminate();
  FPool = NULL;
  // DEBUG_PRINTF(L"end")
}
//------------------------------------------------------------------------------
intptr_t TSubpluginsManager::GetNextID()
{
  return FIDAllocator->allocate(1);
}
//------------------------------------------------------------------------------
// core
intf_handle_t TSubpluginsManager::register_interface(
  const wchar_t * guid, nbptr_t funcs)
{
  intf_handle_t Result = NULL;
  apr_pool_t * pool = FPool;
  bool Found = false;
  apr_hash_index_t * hi = NULL;
  for (hi = apr_hash_first(pool, FInterfaces); hi; hi = apr_hash_next(hi))
  {
    const void * key = NULL;
    void * val = NULL;
    apr_hash_this(hi, &key, NULL, &val);
    if (key && (wcscmp(guid, reinterpret_cast<const wchar_t *>(key)) == 0))
    {
      Found = true;
      break;
    }
  }
  if (!Found)
  {
    apr_ssize_t len = wcslen(guid);
    apr_ssize_t klen = (len + 1) * sizeof(wchar_t);
    apr_hash_set(FInterfaces,
      api_pstrdup(guid, len, pool), klen,
      funcs);
  }
  // Following ensures that only the original provider may remove this
  Result = reinterpret_cast<intf_handle_t>((uintptr_t)funcs ^ FSecNum);
  return Result;
}

nbptr_t TSubpluginsManager::query_interface(
  const wchar_t * guid, intptr_t version)
{
  nbptr_t Result = NULL;
  apr_pool_t * pool = FPool;
  apr_hash_index_t * hi = NULL;
  for (hi = apr_hash_first(pool, FInterfaces); hi; hi = apr_hash_next(hi))
  {
    const void * key = NULL;
    void * val = NULL;
    apr_hash_this(hi, &key, NULL, &val);
    if (key && (wcscmp(guid, reinterpret_cast<const wchar_t *>(key)) == 0))
    {
      Result = reinterpret_cast<nbptr_t>(val);
      break;
    }
  }
  return Result;
}

bool TSubpluginsManager::release_interface(
  intf_handle_t intf)
{
  bool Result = false;
  // Following ensures that only the original provider may remove this
  nbptr_t funcs = reinterpret_cast<nbptr_t>((uintptr_t)intf ^ FSecNum);
  apr_pool_t * pool = FPool;
  bool Found = false;
  apr_hash_index_t * hi = NULL;
  for (hi = apr_hash_first(pool, FInterfaces); hi; hi = apr_hash_next(hi))
  {
    const void * key = NULL;
    apr_ssize_t klen = 0;
    void * val = NULL;
    apr_hash_this(hi, &key, &klen, &val);
    if (key && reinterpret_cast<nbptr_t>(val) == funcs)
    {
      apr_hash_set(FInterfaces, key, klen, NULL); // Remove entry
      Result = true;
      break;
    }
  }
  return Result;
}

bool TSubpluginsManager::has_subplugin(const wchar_t * guid)
{
  bool Result = false;
  for (int I = 0; I < FSubplugins->Count; I++)
  {
    subplugin_info_t * info = static_cast<subplugin_info_t *>(FSubplugins->Items[I]);
    if (wcscmp(info->meta_data->guid, guid) == 0)
    {
      Result = true;
      break;
    }
  }
  return Result;
}
//------------------------------------------------------------------------------
static apr_status_t
cleanup_subplugin_hook(void * ptr)
{
  plugin_hook_t * hook = static_cast<plugin_hook_t *>(ptr);
  if (hook->subscribers)
    delete hook->subscribers;
  return APR_SUCCESS;
}

// hooks
plugin_hook_t * TSubpluginsManager::create_hook(
  const wchar_t * guid, nb_hook_t def_proc)
{
  plugin_hook_t * Result = NULL;
  apr_pool_t * pool = FPool;
  bool Found = false;
  apr_hash_index_t * hi = NULL;
  for (hi = apr_hash_first(pool, FHooks); hi; hi = apr_hash_next(hi))
  {
    const void * key = NULL;
    apr_ssize_t klen = 0;
    void * val = NULL;
    apr_hash_this(hi, &key, &klen, &val);
    if (key && (wcscmp(guid, reinterpret_cast<const wchar_t *>(key)) == 0))
    {
      Found = true;
      Result = NULL;
      break;
    }
  }
  if (!Found)
  {
    plugin_hook_t * hook = static_cast<plugin_hook_t *>(apr_pcalloc(pool, sizeof(*hook)));
    apr_ssize_t len = wcslen(guid);
    apr_ssize_t klen = (len + 1) * sizeof(wchar_t);
    hook->guid = api_pstrdup(guid, len, pool);
    hook->def_proc = def_proc;
    apr_hash_set(FHooks,
      api_pstrdup(guid, len, pool), klen,
      hook);
    // register cleanup routine
    apr_pool_cleanup_register(pool, hook, cleanup_subplugin_hook, apr_pool_cleanup_null);
    Result = hook;
  }
  return Result;
}

bool TSubpluginsManager::destroy_hook(
  plugin_hook_t * hook)
{
  bool Result = false;
  apr_pool_t * pool = FPool;
  bool Found = false;
  apr_hash_index_t * hi = NULL;
  for (hi = apr_hash_first(pool, FHooks); hi; hi = apr_hash_next(hi))
  {
    const void * key = NULL;
    apr_ssize_t klen = 0;
    void * val = NULL;
    apr_hash_this(hi, &key, &klen, &val);
    if (key && (val == hook))
    {
      apr_hash_set(FHooks, key, klen, NULL); // Remove entry
      Result = true;
      break;
    }
  }
  return Result;
}

hook_subscriber_t * TSubpluginsManager::bind_hook(
  const wchar_t * guid, nb_hook_t hook_proc, void * common)
{
  hook_subscriber_t * Result = NULL;
  apr_pool_t * pool = FPool;
  bool Found = false;
  plugin_hook_t * hook = NULL;
  apr_hash_index_t * hi = NULL;
  for (hi = apr_hash_first(pool, FHooks); hi; hi = apr_hash_next(hi))
  {
    const void * key = NULL;
    apr_ssize_t klen = 0;
    void * val = NULL;
    apr_hash_this(hi, &key, &klen, &val);
    if (key && (wcscmp(guid, reinterpret_cast<const wchar_t *>(key)) == 0))
    {
      hook = reinterpret_cast<plugin_hook_t *>(val);
      Found = true;
      break;
    }
  }
  if (Found && hook)
  {
    if (!hook->subscribers)
      hook->subscribers = apr_table_make(pool, 10);
    hook_subscriber_t * subscription = reinterpret_cast<hook_subscriber_t *>(apr_pcalloc(pool, sizeof(*subscription)));
    apr_table_set(hook->subscribers, reinterpret_cast<const char *>(subscription), reinterpret_cast<const char *>(subscription));
    subscription->hook_proc = hook_proc;
    subscription->common = common;
    subscription->owner = hook->guid;
    Result = subscription;
  }
  return Result;
}

// RunHook wrappers for host calls
bool TSubpluginsManager::RunHook(const wchar_t * guid, nbptr_t object, nbptr_t data)
{
  // if (shutdown) return false;
  // auto i = hooks.find(guid);
  // dcassert(i != hooks.end());
  // return runHook(i->second.get(), pObject, pData);
  apr_pool_t * pool = FPool;
  bool Found = false;
  plugin_hook_t * hook = NULL;
  apr_hash_index_t * hi = NULL;
  for (hi = apr_hash_first(pool, FHooks); hi; hi = apr_hash_next(hi))
  {
    const void * key = NULL;
    apr_ssize_t klen = 0;
    void * val = NULL;
    apr_hash_this(hi, &key, &klen, &val);
    if (key && (wcscmp(guid, reinterpret_cast<const wchar_t *>(key)) == 0))
    {
      hook = reinterpret_cast<plugin_hook_t *>(val);
      Found = true;
      break;
    }
  }
  assert(Found);
  return RunHook(hook, object, data);
}

bool TSubpluginsManager::RunHook(
  plugin_hook_t * hook, nbptr_t object, nbptr_t data)
{
  bool Result = false;
  nbBool bBreak = nbFalse;
  bool bRes = false;
  if (hook->subscribers)
  {
    const apr_array_header_t * arr = apr_table_elts(hook->subscribers);
    for (int I = 0; I < arr->nelts; ++I)
    {
      hook_subscriber_t * sub = &APR_ARRAY_IDX(arr, I, hook_subscriber_t);
      if (sub->hook_proc(object, data, sub->common, &bBreak) == SUBPLUGIN_NO_ERROR)
      {
        bRes = true;
      }
      if (bBreak)
        return (bRes);
    }
  }

  // Call default hook procedure for all unused hooks
  if (hook->def_proc && !hook->subscribers)
  {
    if (hook->def_proc(object, data, NULL, &bBreak))
      bRes = true;
  }

  Result = (bRes != false);
  return Result;
}

intptr_t TSubpluginsManager::release_hook(
  hook_subscriber_t * subscription)
{
  intptr_t Result = 0;
  if (subscription == NULL)
    return 0;
  apr_pool_t * pool = FPool;
  bool Found = false;
  // find hook
  // TODO: Refactor into find_hook_by_guid()
  plugin_hook_t * hook = NULL;
  apr_hash_index_t * hi = NULL;
  for (hi = apr_hash_first(pool, FHooks); hi; hi = apr_hash_next(hi))
  {
    const void * key = NULL;
    apr_ssize_t klen = 0;
    void * val = NULL;
    apr_hash_this(hi, &key, &klen, &val);
    if (key && (wcscmp(subscription->owner, reinterpret_cast<const wchar_t *>(key)) == 0))
    {
      hook = reinterpret_cast<plugin_hook_t *>(val);
      Found = true;
      break;
    }
  }
  if (!Found || !hook)
  {
    return 0;
  }
  if (hook->subscribers)
  {
    apr_table_unset(hook->subscribers, reinterpret_cast<const char *>(subscription));
    const apr_array_header_t * arr = apr_table_elts(hook->subscribers);
    Result = arr->nelts;
    if (!arr->nelts)
    {
      hook->subscribers = NULL;
    }
  }
  return Result;
}

//------------------------------------------------------------------------------
// log
void TSubpluginsManager::log(const wchar_t * msg)
{
  DEBUG_PRINTF(L"%s", msg);
}

//------------------------------------------------------------------------------
/*const wchar_t * TSubpluginsManager::GetSubpluginMsg(
  subplugin_info_t * info, const wchar_t * msg_id)
{
  if (!msg_id || !*msg_id) return L"";
  // DEBUG_PRINTF(L"msg_id = %s", msg_id);
  const wchar_t * msg = L"";
  subplugin_info_t * info = static_cast<subplugin_info_t *>(subplugin->ctx);
  // get .msg file name for current language
  UnicodeString MsgExt = GetPluginStartupInfo()->GetMsg(GetPluginStartupInfo()->ModuleNumber, SUBPLUGUN_LANGUAGE_EXTENTION);
  if (!info->msg_file_name_ext || (MsgExt != info->msg_file_name_ext))
  {
    UnicodeString MsgFileName = UnicodeString(info->module_name) + MsgExt;
    if (!::FileExists(MsgFileName))
    {
      MsgFileName = UnicodeString(info->module_name) + L".eng.msg"; // default ext;
    }
    if (::FileExists(MsgFileName))
    {
      info->msg_file_name_ext = api_pstrdup(MsgExt.c_str(), MsgExt.Length());
      // DEBUG_PRINTF(L"MsgFileName = %s", MsgFileName.c_str());
      // Load messages from file
      // LoadSubpluginMessages(subplugin, MsgFileName);
    }
  }
  // try to find msg by id
  {
    // apr_pool_t * pool = pool_create(static_cast<apr_pool_t *>(subplugin->pool));
    apr_pool_t * pool = static_cast<apr_pool_t *>(subplugin->pool);
    apr_hash_index_t * hi = NULL;
    for (hi = apr_hash_first(pool, info->msg_hash); hi; hi = apr_hash_next(hi))
    {
      const void * key = NULL;
      void * val = NULL;
      apr_hash_this(hi, &key, NULL, &val);
      // DEBUG_PRINTF(L"key = %s, val = %s", reinterpret_cast<const wchar_t *>(key), reinterpret_cast<const wchar_t *>(val));
      if (key && (wcscmp(msg_id, reinterpret_cast<const wchar_t *>(key)) == 0))
      {
        msg = reinterpret_cast<const wchar_t *>(val);
        break;
      }
    }
  }
  // DEBUG_PRINTF(L"msg = %s", msg);
  return msg;
}*/
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void * TSubpluginsManager::DialogItemGetProperty(
  const property_baton_t * baton)
{
  if (!baton->property_name || !*baton->property_name) return baton->property_value;
  ISessionDialogIntf * Dialog = static_cast<ISessionDialogIntf *>(baton->notification->param2);
  assert(Dialog);
  void * Result = Dialog->DialogItemGetProperty(baton);
  // DEBUG_PRINTF(L"end");
  return Result;
}
//------------------------------------------------------------------------------
void * TSubpluginsManager::DialogItemSetProperty(
  const property_baton_t * baton)
{
  if (!baton->item_id || !baton->property_name || !*baton->property_name) return NULL;
  ISessionDialogIntf * Dialog = static_cast<ISessionDialogIntf *>(baton->notification->param2);
  assert(Dialog);
  void * Result = Dialog->DialogItemSetProperty(baton);
  // DEBUG_PRINTF(L"end");
  return Result;
}
void * TSubpluginsManager::SendMessage(
  const send_message_baton_t * baton)
{
  if (!baton->message_id || !*baton->message_id) return NULL;
  ISessionDialogIntf * Dialog = static_cast<ISessionDialogIntf *>(baton->notification->param2);
  assert(Dialog);
  void * Result = Dialog->SendMessage(baton);
  // DEBUG_PRINTF(L"end");
  return Result;
}
//------------------------------------------------------------------------------
/*void TSubpluginsManager::LoadSubpluginMessages(subplugin_info_t * info,
  const UnicodeString & MsgFileName)
{
  TStringList StringList;
  // Strings.SetDelimiter(L'');
  StringList.LoadFromFile(MsgFileName);
  // DEBUG_PRINTF(L"Count = %d", StringList.Count.get());
  if (StringList.Count > 0)
  {
    subplugin_info_t * info = static_cast<subplugin_info_t *>(subplugin->ctx);
    apr_pool_t * pool = static_cast<apr_pool_t *>(subplugin->pool);
    apr_hash_clear(info->msg_hash);
    for (int I = 0; I < StringList.Count; I++)
    {
      UnicodeString Name = StringList.Names[I];
      // DEBUG_PRINTF(L"I = %d, Name = %s", I, Name.c_str());
      UnicodeString Value = StringList.Values[Name];
      // DEBUG_PRINTF(L"Value = %s", Value.c_str());
      if (Name.Length() > 0)
      {
        apr_ssize_t klen = Name.GetBytesCount();
        apr_hash_set(info->msg_hash,
          api_pstrdup(Name.c_str(), Name.Length(), pool), klen,
          api_pstrdup(Value.c_str(), Value.Length(), pool));
      }
    }
    // DEBUG_PRINTF(L"info->msg_hash count = %d", apr_hash_count(info->msg_hash));
  }
}*/
//------------------------------------------------------------------------------
PluginStartupInfo * TSubpluginsManager::GetPluginStartupInfo() const
{
  return FFileSystem->WinSCPPlugin()->GetStartupInfo();
}
//------------------------------------------------------------------------------
void TSubpluginsManager::MakeSubpluginsFileList(const UnicodeString & FileName,
  const TSearchRec & Rec, void * Param)
{
  TMakeLocalFileListParams & Params = *static_cast<TMakeLocalFileListParams *>(Param);
  bool IsDirectory = FLAGSET(Rec.Attr, faDirectory);
  if (IsDirectory && Params.Recursive)
  {
    ::ProcessLocalDirectory(FileName, MAKE_CALLBACK(TSubpluginsManager::MakeSubpluginsFileList, this), &Params);
  }
  if (!IsDirectory)
  {
    static TFileMasks m(L"*.subplugin"); // TODO: use constant
    if (m.Matches(Sysutils::ExtractFilename(FileName, L'\\')))
    {
      Params.FileList->Add(FileName);
    }
  }
}
//------------------------------------------------------------------------------
/*void TSubpluginsManager::InitStartupInfo(subplugin_startup_info_t ** startup_info,
  apr_pool_t * pool)
{
  static nb_core_t core =
  {
    sizeof(nb_core_t),
    NB_MAKE_VERSION(NETBOX_VERSION_MAJOR, NETBOX_VERSION_MINOR, NETBOX_VERSION_PATCH, NETBOX_VERSION_BUILD),
    api_versions_equal,
    api_check_version,
    api_pool_create,
    api_pcalloc,
    api_pstrdup,
    api_register_interface,
    api_query_interface,
    api_release_interface,
    api_has_subplugin,
  };

  subplugin_startup_info_t * info = static_cast<subplugin_startup_info_t *>(apr_pcalloc(pool, sizeof(subplugin_startup_info_t)));
  info->struct_size = sizeof(subplugin_startup_info_t);
  // info->NSF = &NSF;
  // info->get_next_id = api_get_next_id;
  // info->get_subplugin_msg = api_get_subplugin_msg;
  // info->dialog_item_get_property = api_dialog_item_get_property;
  // info->dialog_item_set_property = api_dialog_item_set_property;
  // info->send_message = api_send_message;

  *startup_info = info;
}*/
//------------------------------------------------------------------------------
void TSubpluginsManager::LoadSubplugins(apr_pool_t * pool)
{
  TSubpluginApiImpl::InitAPI(this, FCore);
  // Find all .subplugin files in plugin folder and all plugin subfolders
  TMakeLocalFileListParams Params;
  Params.FileList = new TStringList();
  std::auto_ptr<TStrings> FileListPtr(Params.FileList);
  Params.IncludeDirs = false;
  Params.Recursive = true;

  UnicodeString PluginDir = Sysutils::ExtractFilePath(FFileSystem->WinSCPPlugin()->GetModuleName());
  ::ProcessLocalDirectory(PluginDir, MAKE_CALLBACK(TSubpluginsManager::MakeSubpluginsFileList, this), &Params);
  // DEBUG_PRINTF(L"Params.FileList->Count = %d", Params.FileList->Count.get());
  for (int I = 0; I < Params.FileList->Count; I++)
  {
    try
    {
      // try to load subplugin
      UnicodeString ModuleName = Params.FileList->Strings[I];
      // DEBUG_PRINTF(L"ModuleName = %s", ModuleName.c_str());
      LoadSubplugin(ModuleName, pool);
    }
    catch (const std::exception & e)
    {
      DEBUG_PRINTF2("error: %s", e.what());
      // TODO: log into file
    }
    catch (...)
    {
      // TODO: log into file
    }
  }
  DEBUG_PRINTF2("FSubplugins.Count = %d", FSubplugins->Count.get());
}
//------------------------------------------------------------------------------
bool TSubpluginsManager::LoadSubplugin(const UnicodeString & ModuleName, apr_pool_t * pool)
{
  void * mem = apr_pcalloc(pool, sizeof(nb::subplugin));
  nb::subplugin * subplugin_library = new (mem) nb::subplugin(W2MB(ModuleName.c_str()).c_str());
  const subplugin_version_t * min_netbox_version = NULL;
  subplugin_error_t err = subplugin_library->get_min_netbox_version(&min_netbox_version);
  if ((err != SUBPLUGIN_NO_ERROR) || (min_netbox_version == NULL))
  {
    // TODO: Log
    return false;
  }
  // DEBUG_PRINTF2("ver = %d,%d", min_netbox_version->major, min_netbox_version->minor);
  err = api_check_version(netbox::get_plugin_version(), min_netbox_version);
  // DEBUG_PRINTF2("err = %d", err);
  if (err != SUBPLUGIN_NO_ERROR)
  {
    // TODO: Log
    return false;
  }
  const subplugin_version_t * subplugin_version = NULL;
  err = subplugin_library->get_subplugin_version(&subplugin_version);
  if (err != SUBPLUGIN_NO_ERROR)
  {
    // TODO: Log
    return false;
  }
  apr_pool_t * subplugin_pool = pool_create(pool);
  subplugin_startup_info_t * startup_info = NULL;
  // InitStartupInfo(&startup_info, subplugin_pool);

  subplugin_info_t * info = NULL;
  err = init_subplugin_info(&info, subplugin_library, ModuleName, this, subplugin_pool);
  if (err != SUBPLUGIN_NO_ERROR)
  {
    // TODO: Log
    return false;
  }

  err = subplugin_library->init(info->meta_data);
  if (err != SUBPLUGIN_NO_ERROR)
  {
    // TODO: Log
    return false;
  }
  DEBUG_PRINTF(L"subplugin guid: %s", info->meta_data->guid);
  DEBUG_PRINTF(L"name: %s", info->meta_data->name);
  DEBUG_PRINTF(L"description: %s", info->meta_data->description);
  DEBUG_PRINTF(L"API version: %x", info->meta_data->api_version);
  DEBUG_PRINTF(L"subplugin version: %x", info->meta_data->version);
  err = subplugin_library->main(ON_INSTALL, &FCore, NULL);
  if (err != SUBPLUGIN_NO_ERROR)
  {
    // TODO: Log
    return false;
  }
  FSubplugins->Add(info);
  return true;
}
//------------------------------------------------------------------------------
void TSubpluginsManager::UnloadSubplugins()
{
  TSubpluginApiImpl::ReleaseAPI();
}
//------------------------------------------------------------------------------
/*void TSubpluginsManager::Notify(const notification_t * notification)
{
  // DEBUG_PRINTF(L"begin");
  for (int i = 0; i < FSubplugins->Count; i++)
  {
    subplugin_info_t * info = static_cast<subplugin_t *>(FSubplugins->Items[i]);
    assert(subplugin);
    const subplugin_vtable_t * vtable = subplugin->vtable;
    if (vtable && vtable->notify)
    {
      subplugin_error_t err = vtable->notify(subplugin, notification);
      if (err != SUBPLUGIN_NO_ERROR)
      {
        // TODO: Log
      }
    }
  }
  // DEBUG_PRINTF(L"end");
}*/
//------------------------------------------------------------------------------

} // namespace netbox
