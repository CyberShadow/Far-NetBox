#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_hash.h>
#include <apr_tables.h>

#include "SubpluginsManager.hpp"

ISubpluginsManagerIntf * SubpluginsManager;

namespace netbox {

//------------------------------------------------------------------------------

// Holds a loaded subplugin
struct subplugin_info_t
{
  const nb::subplugin * subplugin_library;
  const wchar_t * module_name;
  const wchar_t * msg_file_name_ext;
  apr_hash_t * msg_hash; // subplugin localized messages (int wchar_t * format)
  subplugin_meta_data_t * meta_data; // subplugin metadata
  apr_pool_t * pool;
};
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
TSubpluginsManager::TSubpluginsManager(TWinSCPPlugin * WinSCPPlugin) :
  FWinSCPPlugin(WinSCPPlugin),
  FSubplugins(NULL),
  FProtocols(NULL),
  FHooks(NULL),
  FInterfaces(NULL),
  FPool(NULL),
  FSecNum(rand())
{
  memset(&FCore, 0, sizeof(FCore));
  if (apr_initialize() != APR_SUCCESS)
    throw ExtException(UnicodeString(L"Cannot init APR"));
  FPool = pool_create(NULL);
  ::SubpluginsManager = this;
}
//------------------------------------------------------------------------------
TSubpluginsManager::~TSubpluginsManager()
{
  // DEBUG_PRINTF(L"begin")
  ::SubpluginsManager = NULL;
  apr_terminate();
  FPool = NULL;
  // DEBUG_PRINTF(L"end")
}
//------------------------------------------------------------------------------
void TSubpluginsManager::Init()
{
  apr_pool_t * pool = pool_create(FPool);
  FSubplugins = apr_hash_make(pool);
  FProtocols = apr_hash_make(pool);
  FHooks = apr_hash_make(pool);
  FInterfaces = apr_hash_make(pool);

  TSubpluginApiImpl::InitAPI(this, &FCore, pool);
  FUtils = reinterpret_cast<nb_utils_t *>(query_interface(NBINTF_UTILS));
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
// core
intf_handle_t TSubpluginsManager::register_interface(
  const wchar_t * guid, nbptr_t funcs)
{
  intf_handle_t Result = NULL;
  apr_pool_t * pool = pool_create(FPool);
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
  pool_destroy(pool);
  return Result;
}

nbptr_t TSubpluginsManager::query_interface(
  const wchar_t * guid)
{
  nbptr_t Result = NULL;
  apr_pool_t * pool = pool_create(FPool);
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
  pool_destroy(pool);
  return Result;
}

bool TSubpluginsManager::release_interface(
  intf_handle_t intf)
{
  bool Result = false;
  // Following ensures that only the original provider may remove this
  nbptr_t funcs = reinterpret_cast<nbptr_t>((uintptr_t)intf ^ FSecNum);
  apr_pool_t * pool = pool_create(FPool);
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
  pool_destroy(pool);
  return Result;
}

bool TSubpluginsManager::has_subplugin(
  const wchar_t * guid)
{
  bool Result = GetSubpluginByGuid(guid) != NULL;
  return Result;
}

intptr_t TSubpluginsManager::register_fs_protocol(
  nb_protocol_info_t * prot)
{
  intptr_t Result = -1;
  // First check if protocol is valid and not already registered
  if (!prot || (prot->api_version < NBAPI_CORE_VER) ||
      !prot->plugin_guid || !prot->fs_name) return Result;
  nb_protocol_info_t * found = GetFSProtocolByName(prot->fs_name);
  if (found) return Result;
  // Now register new protocol
  subplugin_info_t * info = GetSubpluginByGuid(prot->plugin_guid);
  assert(info);
  intptr_t id = FUtils->get_unique_id();
  prot->id = id;
  intptr_t cnt = apr_hash_count(FProtocols);
  apr_hash_set(FProtocols, &cnt, sizeof(cnt), prot);
  Result = id;
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
  apr_pool_t * pool = pool_create(FPool);
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
    plugin_hook_t * hook = static_cast<plugin_hook_t *>(apr_pcalloc(FPool, sizeof(*hook)));
    apr_ssize_t len = wcslen(guid);
    apr_ssize_t klen = (len + 1) * sizeof(wchar_t);
    hook->guid = StrDup(guid, len, FPool);
    hook->def_proc = def_proc;
    apr_hash_set(FHooks,
      hook->guid, klen,
      hook);
    // register cleanup routine
    apr_pool_cleanup_register(pool, hook, cleanup_subplugin_hook, apr_pool_cleanup_null);
    Result = hook;
  }
  pool_destroy(pool);
  return Result;
}

bool TSubpluginsManager::destroy_hook(
  plugin_hook_t * hook)
{
  bool Result = false;
  apr_pool_t * pool = pool_create(FPool);
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
  pool_destroy(pool);
  return Result;
}

hook_subscriber_t * TSubpluginsManager::bind_hook(
  const wchar_t * guid, nb_hook_t hook_proc, void * common)
{
  hook_subscriber_t * Result = NULL;
  if (!guid) return Result;
  apr_pool_t * pool = pool_create(FPool);
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
      hook->subscribers = apr_hash_make(FPool);
    hook_subscriber_t * sub = reinterpret_cast<hook_subscriber_t *>(apr_pcalloc(FPool, sizeof(*sub)));
    sub->hook_proc = hook_proc;
    sub->common = common;
    sub->owner = hook->guid;
    intptr_t cnt = apr_hash_count(hook->subscribers);
    apr_hash_set(hook->subscribers, &cnt, sizeof(cnt), sub);
    Result = sub;
  }
  pool_destroy(pool);
  return Result;
}

// RunHook wrappers for host calls
bool TSubpluginsManager::RunHook(const wchar_t * guid, nbptr_t object, nbptr_t data)
{
  // if (shutdown) return false;
  bool Result = false;
  if (!guid) return Result;
  apr_pool_t * pool = pool_create(FPool);
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
  Result = RunHook(hook, object, data);
  pool_destroy(pool);
  return Result;
}

bool TSubpluginsManager::RunHook(
  plugin_hook_t * hook, nbptr_t object, nbptr_t data)
{
  bool Result = false;
  apr_pool_t * pool = pool_create(FPool);
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
      {
        pool_destroy(pool);
        return (bRes);
      }
    }
  }

  // Call default hook procedure for all unused hooks
  if (hook->def_proc && !hook->subscribers)
  {
    if (hook->def_proc(object, data, NULL, &bBreak))
      bRes = true;
  }

  Result = (bRes != false);
  pool_destroy(pool);
  return Result;
}

intptr_t TSubpluginsManager::release_hook(
  hook_subscriber_t * subscription)
{
  intptr_t Result = 0;
  if (subscription == NULL)
    return 0;
  apr_pool_t * pool = pool_create(FPool);
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
    pool_destroy(pool);
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
  pool_destroy(pool);
  return Result;
}

//------------------------------------------------------------------------------
const wchar_t * TSubpluginsManager::GetSubpluginMsg(
  const wchar_t * guid, const wchar_t * msg_id)
{
  // DEBUG_PRINTF(L"begin");
  // DEBUG_PRINTF(L"msg_id = %s", msg_id);
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
  // DEBUG_PRINTF(L"end");
  return Result;
}
//------------------------------------------------------------------------------
// log
void TSubpluginsManager::log(const wchar_t * msg)
{
  DEBUG_PRINTF(L"%s", msg);
}
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
  return FWinSCPPlugin->GetStartupInfo();
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

  UnicodeString PluginDir = Sysutils::ExtractFilePath(FWinSCPPlugin->GetModuleName());
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
  // Now initialize all loaded subplugins
  apr_hash_index_t * hi = NULL;
  for (hi = apr_hash_first(pool, FSubplugins); hi; hi = apr_hash_next(hi))
  {
    const void * key = NULL;
    apr_ssize_t klen = 0;
    void * val = NULL;
    apr_hash_this(hi, &key, &klen, &val);
    subplugin_info_t * info = static_cast<subplugin_info_t *>(val);
    if (info)
    {
      subplugin_error_t err = info->subplugin_library->main(ON_INIT, NULL, NULL);
      if (err != SUBPLUGIN_NO_ERROR)
      {
        log(FORMAT(L"Cannot init module: %s", info->module_name).c_str());
        // TODO: unload subplugin
      }
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
// a cleanup routine attached to the pool that contains subplugin
typedef nb::subplugin nb_subplugin_t;

static apr_status_t
cleanup_subplugin_info(void * ptr)
{
  // DEBUG_PRINTF(L"begin");
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
      info->subplugin_library->~nb_subplugin_t();
      // handle = NULL;
    }
  }
  catch (const std::exception & e)
  {
    DEBUG_PRINTF2("Error: %s", e.what());
    // TODO: log into file
  }
  // DEBUG_PRINTF(L"end");
  return APR_SUCCESS;
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
  info->pool = subplugin_pool;
  info->subplugin_library = subplugin_library;
  info->module_name = StrDup(module_name, wcslen(module_name), subplugin_pool);
  info->msg_hash = apr_hash_make(subplugin_pool);
  info->meta_data =
    static_cast<subplugin_meta_data_t *>(apr_pcalloc(subplugin_pool, sizeof(*info->meta_data)));
  *subplugin_info = info;
  return SUBPLUGIN_NO_ERROR;
}
//------------------------------------------------------------------------------
bool TSubpluginsManager::LoadSubplugin(const UnicodeString & ModuleName, apr_pool_t * pool)
{
  subplugin_info_t * info = NULL;
  try
  {
    void * mem = apr_pcalloc(pool, sizeof(nb::subplugin));
    nb::subplugin * subplugin_library = new (mem) nb::subplugin(W2MB(ModuleName.c_str()).c_str());
    subplugin_error_t err = 0;
    InitSubpluginInfo(&info, subplugin_library, ModuleName.c_str(), pool);
    err = subplugin_library->get_meta_data(info->meta_data);
    if (err != SUBPLUGIN_NO_ERROR)
    {
      log(FORMAT(L"Cannot get metadata for module: %s", ModuleName.c_str()).c_str());
      subplugin_library->~nb_subplugin_t();
      return false;
    }
    if ((info->meta_data->api_version < NBAPI_CORE_VER) || (info->meta_data->api_version > 100000))
    {
      log(FORMAT(L"Cannot get metadata for module %s: wrong API version", ModuleName.c_str()).c_str());
      subplugin_library->~nb_subplugin_t();
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
      subplugin_library->~nb_subplugin_t();
      return false;
    }
  }
  catch (const std::exception & e)
  {
    DEBUG_PRINTF2("Error while loading subplugin %s: %s", ModuleName.c_str(), e.what());
    // TODO: log into file
  }
  catch (...)
  {
    DEBUG_PRINTF2("Error while loading subplugin: %s", ModuleName.c_str());
    // TODO: log into file
  }
  intptr_t cnt = apr_hash_count(FSubplugins);
  apr_hash_set(FSubplugins, &cnt, sizeof(cnt), info);
  apr_pool_cleanup_register(info->pool, info, cleanup_subplugin_info, apr_pool_cleanup_null);
  return true;
}
//------------------------------------------------------------------------------
void TSubpluginsManager::UnloadSubplugins()
{
  // TODO: Notify subplugins before unload
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// ISessionDataProviderIntf
intptr_t TSubpluginsManager::GetFSProtocolsCount()
{
  // DEBUG_PRINTF(L"begin");
  intptr_t Result = apr_hash_count(FProtocols);
  // DEBUG_PRINTF(L"end, Result = %d", Result);
  return Result;
}
//------------------------------------------------------------------------------
intptr_t TSubpluginsManager::GetFSProtocolId(intptr_t Index)
{
  // DEBUG_PRINTF(L"begin, Index = %d", Index);
  intptr_t Result = -1;
  apr_pool_t * pool = pool_create(FPool);
  intptr_t I = 0;
  apr_hash_index_t * hi = NULL;
  for (hi = apr_hash_first(pool, FProtocols); hi; hi = apr_hash_next(hi))
  {
    const void * key = NULL;
    apr_ssize_t klen = 0;
    void * val = NULL;
    apr_hash_this(hi, &key, &klen, &val);
    nb_protocol_info_t * prot = static_cast<nb_protocol_info_t *>(val);
    if (prot && (Index == I))
    {
      Result = prot->id;
      break;
    }
    ++I;
  }
  pool_destroy(pool);
  // DEBUG_PRINTF(L"end, Result = %d", Result);
  assert(Result != -1);
  return Result;
}
//------------------------------------------------------------------------------
UnicodeString TSubpluginsManager::GetFSProtocolStr(intptr_t Index)
{
  // DEBUG_PRINTF(L"begin, Index = %d", Index);
  UnicodeString Result = L"";
  apr_pool_t * pool = pool_create(FPool);
  intptr_t I = 0;
  apr_hash_index_t * hi = NULL;
  for (hi = apr_hash_first(pool, FProtocols); hi; hi = apr_hash_next(hi))
  {
    const void * key = NULL;
    apr_ssize_t klen = 0;
    void * val = NULL;
    apr_hash_this(hi, &key, &klen, &val);
    nb_protocol_info_t * prot = static_cast<nb_protocol_info_t *>(val);
    if (prot && (Index == I))
    {
      Result = prot->fs_name;
      break;
    }
    ++I;
  }
  pool_destroy(pool);
  // DEBUG_PRINTF(L"end, Result = %s", Result.c_str());
  assert(!Result.IsEmpty());
  return Result;
}
//------------------------------------------------------------------------------
subplugin_info_t * TSubpluginsManager::GetSubpluginByGuid(
  const wchar_t * guid)
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
UnicodeString TSubpluginsManager::GetFSProtocolStrById(
  intptr_t ProtocolId)
{
  UnicodeString Result = L"";
  nb_protocol_info_t * prot = GetFSProtocolById(ProtocolId);
  assert(prot);
  Result = prot->fs_name;
  assert(!Result.IsEmpty());
  return Result;
}
//------------------------------------------------------------------------------
nb_filesystem_t * TSubpluginsManager::Create(intptr_t ProtocolId, void * Data)
{
  nb_filesystem_t * Result = NULL;
  nb_protocol_info_t * prot = GetFSProtocolById(ProtocolId);
  assert(prot);
  if (prot->create)
  {
    Result = prot->create(Data, NULL);
  }
  return Result;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
nb_protocol_info_t * TSubpluginsManager::GetFSProtocolByName(
  const wchar_t * Name)
{
  // DEBUG_PRINTF(L"begin");
  nb_protocol_info_t * Result = NULL;
  apr_pool_t * pool = pool_create(FPool);
  apr_hash_index_t * hi = NULL;
  for (hi = apr_hash_first(pool, FProtocols); hi; hi = apr_hash_next(hi))
  {
    const void * key = NULL;
    apr_ssize_t klen = 0;
    void * val = NULL;
    apr_hash_this(hi, &key, &klen, &val);
    nb_protocol_info_t * prot = static_cast<nb_protocol_info_t *>(val);
    if (prot && wcscmp(prot->fs_name, Name) == 0)
    {
      Result = prot;
      break;
    }
  }
  pool_destroy(pool);
  // DEBUG_PRINTF(L"end, Result = %p", Result);
  return Result;
}
//------------------------------------------------------------------------------
nb_protocol_info_t * TSubpluginsManager::GetFSProtocolById(
  intptr_t Id)
{
  // DEBUG_PRINTF(L"begin");
  nb_protocol_info_t * Result = NULL;
  apr_pool_t * pool = pool_create(FPool);
  apr_hash_index_t * hi = NULL;
  for (hi = apr_hash_first(pool, FProtocols); hi; hi = apr_hash_next(hi))
  {
    const void * key = NULL;
    apr_ssize_t klen = 0;
    void * val = NULL;
    apr_hash_this(hi, &key, &klen, &val);
    nb_protocol_info_t * prot = static_cast<nb_protocol_info_t *>(val);
    if (prot && (prot->id == Id))
    {
      Result = prot;
      break;
    }
  }
  pool_destroy(pool);
  assert(Result);
  // DEBUG_PRINTF(L"end, Result = %p", Result);
  return Result;
}
//------------------------------------------------------------------------------
/*nb_protocol_info_t * TSubpluginsManager::GetFSProtocolByHandle(
  fs_handle_t Handle)
{
  // DEBUG_PRINTF(L"begin");
  nb_protocol_info_t * Result = NULL;
  apr_pool_t * pool = pool_create(FPool);
  apr_hash_index_t * hi = NULL;
  for (hi = apr_hash_first(pool, FFileSystems); hi; hi = apr_hash_next(hi))
  {
    const void * key = NULL;
    apr_ssize_t klen = 0;
    void * val = NULL;
    apr_hash_this(hi, &key, &klen, &val);
    fs_handle_t fs = static_cast<fs_handle_t *>(val);
    if (fs && (fs == Handle))
    {
      Result = fs->prot;
      break;
    }
  }
  pool_destroy(pool);
  assert(Result);
  // DEBUG_PRINTF(L"end, Result = %p", Result);
  return Result;
}*/
//------------------------------------------------------------------------------

} // namespace netbox
