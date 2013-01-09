#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_hash.h>
#include <apr_tables.h>

#include <subplugin.hpp>
#include "SubpluginsManager.hpp"

namespace netbox {

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
  apr_pool_t * pool;
};
//------------------------------------------------------------------------------
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
      typedef nb::subplugin nb_subplugin_t;
      info->subplugin_library->~nb_subplugin_t();
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
//------------------------------------------------------------------------------
TSubpluginsManager::TSubpluginsManager(TWinSCPFileSystem * FileSystem) :
  FFileSystem(FileSystem),
  FSubplugins(NULL),
  FHooks(NULL),
  FInterfaces(NULL),
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
  FSubplugins = apr_hash_make(pool);
  FHooks = apr_hash_make(pool);
  FInterfaces = apr_hash_make(pool);

  TSubpluginApiImpl::InitAPI(this, &FCore, pool);
  FUtils = reinterpret_cast<nb_utils_t *>(query_interface(NBINTF_UTILS, NBINTF_UTILS_VER));
  assert(FUtils);
  LoadSubplugins(pool);
}
//------------------------------------------------------------------------------
void TSubpluginsManager::Shutdown()
{
  UnloadSubplugins();
  TSubpluginApiImpl::ReleaseAPI();
  apr_pool_clear(FPool);
}
//------------------------------------------------------------------------------
TSubpluginsManager::~TSubpluginsManager()
{
  // DEBUG_PRINTF(L"begin")
  apr_terminate();
  FPool = NULL;
  // DEBUG_PRINTF(L"end")
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
      guid, klen,
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
  bool Result = GetSubpluginByGuid(guid) != NULL;
  return Result;
}
//------------------------------------------------------------------------------
static apr_status_t
cleanup_subplugin_hook(void * ptr)
{
  plugin_hook_t * hook = static_cast<plugin_hook_t *>(ptr);
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
    hook->guid = StrDup(guid, len, pool);
    hook->def_proc = def_proc;
    apr_hash_set(FHooks,
      hook->guid, klen,
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
      hook->subscribers = apr_hash_make(pool);
    intptr_t cnt = apr_hash_count(hook->subscribers);
    hook_subscriber_t * sub = reinterpret_cast<hook_subscriber_t *>(apr_pcalloc(pool, sizeof(*sub)));
    sub->hook_proc = hook_proc;
    sub->common = common;
    sub->owner = hook->guid;
    apr_hash_set(hook->subscribers, &cnt, sizeof(cnt), sub);
    Result = sub;
  }
  return Result;
}

// RunHook wrappers for host calls
bool TSubpluginsManager::RunHook(const wchar_t * guid, nbptr_t object, nbptr_t data)
{
  // if (shutdown) return false;
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
  apr_pool_t * pool = FPool;
  nb_bool_t bBreak = nb_false;
  bool bRes = false;
  if (hook->subscribers)
  {
    intptr_t cnt = apr_hash_count(hook->subscribers);
    apr_hash_index_t * hi = NULL;
    for (hi = apr_hash_first(pool, hook->subscribers); hi; hi = apr_hash_next(hi))
    {
      const void * key = NULL;
      apr_ssize_t klen = 0;
      void * val = NULL;
      apr_hash_this(hi, &key, &klen, &val);
      hook_subscriber_t * sub = static_cast<hook_subscriber_t *>(val);
      if (sub && sub->hook_proc && sub->hook_proc(object, data, sub->common, &bBreak) == SUBPLUGIN_NO_ERROR)
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
    apr_hash_index_t * hi = NULL;
    for (hi = apr_hash_first(pool, hook->subscribers); hi; hi = apr_hash_next(hi))
    {
      const void * key = NULL;
      apr_ssize_t klen = 0;
      void * val = NULL;
      apr_hash_this(hi, &key, &klen, &val);
      hook_subscriber_t * sub = static_cast<hook_subscriber_t *>(val);
      if (sub == subscription)
      {
        apr_hash_set(hook->subscribers, key, klen, NULL); // Remove entry
        break;
      }
    }
    Result = apr_hash_count(hook->subscribers);
    if (!Result)
    {
      apr_hash_clear(hook->subscribers);
      hook->subscribers = NULL;
    }
  }
  return Result;
}

//------------------------------------------------------------------------------
const wchar_t * TSubpluginsManager::GetSubpluginMsg(
  const wchar_t * guid, const wchar_t * msg_id)
{
  DEBUG_PRINTF(L"begin");
  DEBUG_PRINTF(L"msg_id = %s", msg_id);
  const wchar_t * Result = L"";
  if (!guid || !msg_id || !*msg_id) return Result;
  subplugin_info_t * info = GetSubpluginByGuid(guid);
  if (!info) return Result;
  apr_pool_t * pool = info->pool;
  // get .msg file name for current language
  UnicodeString MsgExt = GetMsgFileNameExt();
  if (!info->msg_file_name_ext || (MsgExt != info->msg_file_name_ext))
  {
    UnicodeString MsgFileName = UnicodeString(info->module_name) + MsgExt;
    if (!::FileExists(MsgFileName))
    {
      MsgFileName = UnicodeString(info->module_name) + L".eng.msg"; // default ext;
    }
    if (::FileExists(MsgFileName))
    {
      info->msg_file_name_ext = StrDup(MsgExt.c_str(), MsgExt.Length(), pool);
      // DEBUG_PRINTF(L"MsgFileName = %s", MsgFileName.c_str());
      // Load messages from file
      LoadSubpluginMessages(info, MsgFileName);
    }
  }
  // try to find msg by id
  {
    apr_hash_index_t * hi = NULL;
    for (hi = apr_hash_first(pool, info->msg_hash); hi; hi = apr_hash_next(hi))
    {
      const void * key = NULL;
      void * val = NULL;
      apr_hash_this(hi, &key, NULL, &val);
      // DEBUG_PRINTF(L"key = %s, val = %s", reinterpret_cast<const wchar_t *>(key), reinterpret_cast<const wchar_t *>(val));
      if (key && (wcscmp(msg_id, reinterpret_cast<const wchar_t *>(key)) == 0))
      {
        Result = reinterpret_cast<const wchar_t *>(val);
        break;
      }
    }
  }
  DEBUG_PRINTF(L"end");
  return Result;
}
//------------------------------------------------------------------------------
// log
void TSubpluginsManager::log(const wchar_t * msg)
{
  DEBUG_PRINTF(L"%s", msg);
}

//------------------------------------------------------------------------------
subplugin_info_t * TSubpluginsManager::GetSubpluginByGuid(const wchar_t * guid)
{
  subplugin_info_t * Result = NULL;
  apr_pool_t * pool = pool_create(FPool);
  apr_hash_index_t * hi = NULL;
  for (hi = apr_hash_first(pool, FSubplugins); hi; hi = apr_hash_next(hi))
  {
    const void * key = NULL;
    apr_ssize_t klen = 0;
    void * val = NULL;
    apr_hash_this(hi, &key, &klen, &val);
    subplugin_info_t * info = static_cast<subplugin_info_t *>(val);
    if (info && (wcscmp(info->meta_data->guid, guid) == 0))
    {
      Result = info;
      break;
    }
  }
  pool_destroy(pool);
  return Result;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void TSubpluginsManager::LoadSubpluginMessages(subplugin_info_t * info,
  const UnicodeString & MsgFileName)
{
  TStringList StringList;
  // Strings.SetDelimiter(L'');
  StringList.LoadFromFile(MsgFileName);
  // DEBUG_PRINTF(L"Count = %d", StringList.Count.get());
  if (StringList.Count > 0)
  {
    apr_pool_t * pool = info->pool;
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
          StrDup(Name.c_str(), Name.Length(), pool), klen,
          StrDup(Value.c_str(), Value.Length(), pool));
      }
    }
    // DEBUG_PRINTF(L"info->msg_hash count = %d", apr_hash_count(info->msg_hash));
  }
}
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
void TSubpluginsManager::LoadSubplugins(apr_pool_t * pool)
{
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
  DEBUG_PRINTF2("FSubplugins Count = %d", apr_hash_count(FSubplugins));
}
//------------------------------------------------------------------------------
const wchar_t *
TSubpluginsManager::StrDup(
  const wchar_t * str, size_t len, apr_pool_t * pool)
{
  // return FUtils->pstrdup(str, len, pool);
  return TSubpluginApiImpl::pstrdup(str, len, pool);
}
//------------------------------------------------------------------------------
subplugin_error_t
TSubpluginsManager::InitSubpluginInfo(
  subplugin_info_t ** subplugin_info,
  const nb::subplugin * subplugin_library,
  const wchar_t * module_name,
  apr_pool_t * pool)
{
  apr_pool_t * subplugin_pool = pool_create(pool);
  subplugin_info_t * info =
    static_cast<subplugin_info_t *>(apr_pcalloc(subplugin_pool, sizeof(*info)));
  info->struct_size = sizeof(*info);
  info->subplugin_library = subplugin_library;
  info->module_name = StrDup(module_name, wcslen(module_name), subplugin_pool);
  info->msg_hash = apr_hash_make(subplugin_pool);
  info->meta_data =
    static_cast<subplugin_meta_data_t *>(apr_pcalloc(subplugin_pool, sizeof(*info->meta_data)));
  info->pool = subplugin_pool;
  apr_pool_cleanup_register(subplugin_pool, info, cleanup_subplugin_info, apr_pool_cleanup_null);
  *subplugin_info = info;
  return SUBPLUGIN_NO_ERROR;
}
//------------------------------------------------------------------------------
bool TSubpluginsManager::LoadSubplugin(const UnicodeString & ModuleName, apr_pool_t * pool)
{
  void * mem = apr_pcalloc(pool, sizeof(nb::subplugin));
  nb::subplugin * subplugin_library = new (mem) nb::subplugin(W2MB(ModuleName.c_str()).c_str());
  subplugin_error_t err = 0;
  subplugin_info_t * info = NULL;
  InitSubpluginInfo(&info, subplugin_library, ModuleName.c_str(), pool);
  err = subplugin_library->init(info->meta_data);
  if (err != SUBPLUGIN_NO_ERROR)
  {
    log(FORMAT(L"Cannot init module: %s", ModuleName.c_str()).c_str());
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
    log(FORMAT(L"Cannot load module: %s", ModuleName.c_str()).c_str());
    return false;
  }
  intptr_t cnt = apr_hash_count(FSubplugins);
  apr_hash_set(FSubplugins, &cnt, sizeof(cnt), info);
  return true;
}
//------------------------------------------------------------------------------
void TSubpluginsManager::UnloadSubplugins()
{
  // TODO: Notify subplugins before unload
}
//------------------------------------------------------------------------------

} // namespace netbox
