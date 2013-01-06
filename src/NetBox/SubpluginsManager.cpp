#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_hash.h>

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
api_pstrdup(const wchar_t * str, size_t len)
{
  wchar_t * Result = NULL;
  // assert(subplugin->pool);
  // wchar_t * Result = reinterpret_cast<wchar_t *>(
    // apr_pmemdup(static_cast<apr_pool_t *>(subplugin->pool),
      // reinterpret_cast<const char *>(str), (len + 1) * sizeof(wchar_t)));
  // Result[len] = 0;
  return Result;
}

/* Interface registry */
static intf_handle_t NBAPI
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
  // subplugin_descriptor_t * desc = static_cast<subplugin_descriptor_t *>(subplugin->ctx);
  // assert(desc);
  // return desc->manager->HasSubplugin(guid);
  return Result;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
struct subplugin_descriptor_t
{
  size_t struct_size;
  const wchar_t * module_name;
  const wchar_t * msg_file_name_ext;
  apr_hash_t * msg_hash; // subplugin localized messages (int wchar_t * format)
  subplugin_meta_data_t * meta_data; // subplugin metadata
  const nb::subplugin * subplugin_library;
  TSubpluginsManager * manager;
};
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static intptr_t NBAPI
api_get_next_id(subplugin_t * subplugin)
{
  if (!check_struct_size(subplugin)) return NULL;
  subplugin_descriptor_t * desc = static_cast<subplugin_descriptor_t *>(subplugin->ctx);
  assert(desc);
  return desc->manager->GetNextID();
}
//------------------------------------------------------------------------------
static const wchar_t * NBAPI
api_get_subplugin_msg(subplugin_t * subplugin,
  const wchar_t * msg_id)
{
  if (!check_struct_size(subplugin)) return NULL;
  subplugin_descriptor_t * desc = static_cast<subplugin_descriptor_t *>(subplugin->ctx);
  assert(desc);
  return desc->manager->GetSubpluginMsg(subplugin, msg_id);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void * NBAPI
api_dialog_item_get_property(
  const property_baton_t * baton)
{
  if (!check_struct_size(baton)) return NULL;
  subplugin_descriptor_t * desc = static_cast<subplugin_descriptor_t *>(baton->subplugin->ctx);
  assert(desc);
  return desc->manager->DialogItemGetProperty(baton);
}
//------------------------------------------------------------------------------
static void * NBAPI
api_dialog_item_set_property(
  const property_baton_t * baton)
{
  if (!check_struct_size(baton)) return NULL;
  subplugin_descriptor_t * desc = static_cast<subplugin_descriptor_t *>(baton->subplugin->ctx);
  assert(desc);
  return desc->manager->DialogItemSetProperty(baton);
}
//------------------------------------------------------------------------------
static void * NBAPI
api_send_message(
  const send_message_baton_t * baton)
{
  if (!check_struct_size(baton)) return NULL;
  subplugin_descriptor_t * desc = static_cast<subplugin_descriptor_t *>(baton->subplugin->ctx);
  assert(desc);
  return desc->manager->SendMessage(baton);
}
//------------------------------------------------------------------------------
// a cleanup routine attached to the pool that contains subplugin
static apr_status_t
cleanup_subplugin(void * ptr)
{
  subplugin_t * subplugin = static_cast<subplugin_t *>(ptr);
  assert(subplugin);
  subplugin_descriptor_t * desc = static_cast<subplugin_descriptor_t *>(subplugin->ctx);
  assert(desc);
  try
  {
    desc->subplugin_library->destroy(subplugin);
    // delete desc->subplugin_library;
  }
  catch (const std::exception & e)
  {
    DEBUG_PRINTF2("Error: %s", e.what());
    // TODO: log into file
  }
  return APR_SUCCESS;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
TSubpluginsManager::TSubpluginsManager(TWinSCPFileSystem * FileSystem) :
  FFileSystem(FileSystem),
  FSubplugins(new TList()),
  FPool(NULL),
  FIDAllocator(2000, 54999)
{
  memset(&FCore, 0, sizeof(FCore));
  if (apr_initialize() != APR_SUCCESS)
    throw ExtException(UnicodeString(L"Cannot init APR"));
  FPool = pool_create(NULL);
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
const wchar_t * TSubpluginsManager::GetSubpluginMsg(
  subplugin_t * subplugin, const wchar_t * msg_id)
{
  if (!msg_id || !*msg_id) return L"";
  // DEBUG_PRINTF(L"msg_id = %s", msg_id);
  const wchar_t * msg = L"";
  subplugin_descriptor_t * desc = static_cast<subplugin_descriptor_t *>(subplugin->ctx);
  // get .msg file name for current language
  UnicodeString MsgExt = GetPluginStartupInfo()->GetMsg(GetPluginStartupInfo()->ModuleNumber, SUBPLUGUN_LANGUAGE_EXTENTION);
  if (!desc->msg_file_name_ext || (MsgExt != desc->msg_file_name_ext))
  {
    UnicodeString MsgFileName = UnicodeString(desc->module_name) + MsgExt;
    if (!::FileExists(MsgFileName))
    {
      MsgFileName = UnicodeString(desc->module_name) + L".eng.msg"; // default ext;
    }
    if (::FileExists(MsgFileName))
    {
      desc->msg_file_name_ext = api_pstrdup(MsgExt.c_str(), MsgExt.Length());
      // DEBUG_PRINTF(L"MsgFileName = %s", MsgFileName.c_str());
      // Load messages from file
      LoadSubpluginMessages(subplugin, MsgFileName);
    }
  }
  // try to find msg by id
  {
    // apr_pool_t * pool = pool_create(static_cast<apr_pool_t *>(subplugin->pool));
    apr_pool_t * pool = static_cast<apr_pool_t *>(subplugin->pool);
    apr_hash_index_t * hi = NULL;
    for (hi = apr_hash_first(pool, desc->msg_hash); hi; hi = apr_hash_next(hi))
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
}
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
void TSubpluginsManager::LoadSubpluginMessages(subplugin_t * subplugin,
  const UnicodeString & MsgFileName)
{
  TStringList StringList;
  // Strings.SetDelimiter(L'');
  StringList.LoadFromFile(MsgFileName);
  // DEBUG_PRINTF(L"Count = %d", StringList.Count.get());
  if (StringList.Count > 0)
  {
    subplugin_descriptor_t * desc = static_cast<subplugin_descriptor_t *>(subplugin->ctx);
    apr_pool_t * pool = static_cast<apr_pool_t *>(subplugin->pool);
    apr_hash_clear(desc->msg_hash);
    for (int I = 0; I < StringList.Count; I++)
    {
      UnicodeString Name = StringList.Names[I];
      // DEBUG_PRINTF(L"I = %d, Name = %s", I, Name.c_str());
      UnicodeString Value = StringList.Values[Name];
      // DEBUG_PRINTF(L"Value = %s", Value.c_str());
      if (Name.Length() > 0)
      {
        apr_ssize_t klen = Name.GetBytesCount();
        apr_hash_set(desc->msg_hash,
          apr_pmemdup(pool, Name.c_str(), klen), klen,
          apr_pmemdup(pool, Value.c_str(), (Value.Length() + 1) * sizeof(wchar_t)));
      }
    }
    // DEBUG_PRINTF(L"desc->msg_hash count = %d", apr_hash_count(desc->msg_hash));
  }
}
//------------------------------------------------------------------------------
PluginStartupInfo * TSubpluginsManager::GetPluginStartupInfo() const
{
  return FFileSystem->WinSCPPlugin()->GetStartupInfo();
}
//------------------------------------------------------------------------------
void TSubpluginsManager::UnloadSubplugins()
{
  // TODO: Notify subplugins before unload
  apr_pool_clear(FPool);
  FSubplugins->Clear();
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
void TSubpluginsManager::InitStartupInfo(subplugin_startup_info_t ** startup_info,
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
  info->get_next_id = api_get_next_id;
  info->get_subplugin_msg = api_get_subplugin_msg;
  info->dialog_item_get_property = api_dialog_item_get_property;
  info->dialog_item_set_property = api_dialog_item_set_property;
  info->send_message = api_send_message;

  *startup_info = info;
}
//------------------------------------------------------------------------------
void TSubpluginsManager::InitSubplugins()
{
  apr_pool_t * pool = FPool;

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
    // try to load subplugin
    UnicodeString ModuleName = Params.FileList->Strings[I];
    // DEBUG_PRINTF(L"ModuleName = %s", ModuleName.c_str());
    try
    {
      void * lib = apr_pcalloc(pool, sizeof(nb::subplugin));
      nb::subplugin * subplugin_library = new (lib) nb::subplugin(W2MB(ModuleName.c_str()).c_str());
      const subplugin_version_t * min_netbox_version = NULL;
      subplugin_error_t err = subplugin_library->get_min_netbox_version(&min_netbox_version);
      if ((err != SUBPLUGIN_NO_ERROR) || (min_netbox_version == NULL))
      {
        // TODO: Log
        continue;
      }
      // DEBUG_PRINTF2("ver = %d,%d", min_netbox_version->major, min_netbox_version->minor);
      err = api_check_version(netbox::get_plugin_version(), min_netbox_version);
      // DEBUG_PRINTF2("err = %d", err);
      if (err != SUBPLUGIN_NO_ERROR)
      {
        // TODO: Log
        continue;
      }
      const subplugin_version_t * subplugin_version = NULL;
      err = subplugin_library->get_subplugin_version(&subplugin_version);
      if (err != SUBPLUGIN_NO_ERROR)
      {
        // TODO: Log
        continue;
      }
      apr_pool_t * subplugin_pool = pool_create(pool);
      subplugin_startup_info_t * startup_info = NULL;
      InitStartupInfo(&startup_info, subplugin_pool);

      subplugin_t * subplugin = static_cast<subplugin_t *>(apr_pcalloc(pool, sizeof(*subplugin)));
      subplugin->struct_size = sizeof(*subplugin);
      subplugin->pool = subplugin_pool;

      subplugin_descriptor_t * desc =
        static_cast<subplugin_descriptor_t *>(apr_pcalloc(pool, sizeof(*desc)));
      desc->struct_size = sizeof(*desc);
      desc->module_name = api_pstrdup(ModuleName.c_str(), ModuleName.Length());
      desc->msg_hash = apr_hash_make(pool);
      desc->meta_data =
        static_cast<subplugin_meta_data_t *>(apr_pcalloc(pool, sizeof(*desc->meta_data)));
      desc->subplugin_library = subplugin_library;
      desc->manager = this;

      subplugin->ctx = desc;

      apr_pool_cleanup_register(pool, subplugin, cleanup_subplugin, apr_pool_cleanup_null);

      // err = subplugin_library->init(
        // ON_INSTALL,
        // get_plugin_version(),
        // startup_info, subplugin);
      err = subplugin_library->init(desc->meta_data);
      if (err != SUBPLUGIN_NO_ERROR)
      {
        // TODO: Log
        continue;
      }
      if (desc->meta_data->guid)
      {
        DEBUG_PRINTF(L"subplugin guid: %s", desc->meta_data->guid);
      }
      DEBUG_PRINTF(L"name: %s", desc->meta_data->name);
      DEBUG_PRINTF(L"description: %s", desc->meta_data->description);
      DEBUG_PRINTF(L"API version: %x", desc->meta_data->api_version);
      DEBUG_PRINTF(L"subplugin version: %x", desc->meta_data->version);
      err = subplugin_library->main(ON_INSTALL, &FCore, NULL);
      if (err != SUBPLUGIN_NO_ERROR)
      {
        // TODO: Log
        continue;
      }
      FSubplugins->Add(subplugin);
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
void TSubpluginsManager::Notify(const notification_t * notification)
{
  // DEBUG_PRINTF(L"begin");
  for (int i = 0; i < FSubplugins->Count; i++)
  {
    subplugin_t * subplugin = static_cast<subplugin_t *>(FSubplugins->Items[i]);
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
}
//------------------------------------------------------------------------------

} // namespace netbox
