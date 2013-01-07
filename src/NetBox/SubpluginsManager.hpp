#pragma once

#include <map>

#include <subplugin.hpp>

class TWinSCPFileSystem;
struct apr_pool_t;
struct apr_hash_t;
// struct apr_table_t;

namespace netbox {

class TIDAllocator;
struct subplugin_info_t;

//------------------------------------------------------------------------------
// Represents a plugin in hook context
struct hook_subscriber_t
{
  nb_hook_t hook_proc;
  void * common;
  const wchar_t * owner;
};

// Hookable event
struct plugin_hook_t
{
  const wchar_t * guid;
  nb_hook_t def_proc;

  apr_hash_t * subscribers;
  // CriticalSection cs;
};

//------------------------------------------------------------------------------

class TSubpluginsManager
{
public:
  explicit TSubpluginsManager(TWinSCPFileSystem * FileSystem);
  ~TSubpluginsManager();
  void Init();
  void Shutdown();

  // core
  intf_handle_t register_interface(
    const wchar_t * guid, nbptr_t funcs);
  nbptr_t query_interface(
    const wchar_t * guid, intptr_t version);
  bool release_interface(
    intf_handle_t intf);
  bool has_subplugin(const wchar_t * guid);

  // hooks
  plugin_hook_t * create_hook(
    const wchar_t * guid, nb_hook_t def_proc);
  bool destroy_hook(
    plugin_hook_t * hook);

  hook_subscriber_t * bind_hook(
    const wchar_t * guid, nb_hook_t hook_proc, void * common);

  // RunHook wrappers for host calls
  bool RunHook(const wchar_t * guid, nbptr_t object, nbptr_t data);
  bool RunHook(
    plugin_hook_t * hook, nbptr_t object, nbptr_t data);
  intptr_t release_hook(
    hook_subscriber_t * subscription);

  // log
  void log(const wchar_t * msg);

  nb_core_t * GetCore() { return &FCore; }

  intptr_t GetNextID();
  const wchar_t * GetSubpluginMsg(
    const wchar_t * guid, const wchar_t * msg_id);
  void * DialogItemGetProperty(
    const property_baton_t * baton);
  void * DialogItemSetProperty(
    const property_baton_t * baton);
  void * SendMessage(
    const send_message_baton_t * baton);

private:
  void LoadSubplugins(apr_pool_t * pool);
  void UnloadSubplugins();
  // TODO: void unloadPlugin(size_t index);
  // TODO: bool addInactivePlugin(PluginHandle h);

  bool LoadSubplugin(const UnicodeString & ModuleName, apr_pool_t * pool);
  void LoadSubpluginMessages(subplugin_info_t * info,
    const UnicodeString & MsgFileName);
  PluginStartupInfo * GetPluginStartupInfo() const;
  UnicodeString GetMsgFileNameExt() { return GetPluginStartupInfo()->GetMsg(GetPluginStartupInfo()->ModuleNumber, SUBPLUGUN_LANGUAGE_EXTENTION); }
  void MakeSubpluginsFileList(const UnicodeString & FileName,
    const TSearchRec & Rec, void * Param);

  subplugin_info_t * GetSubpluginByGuid(const wchar_t * guid);

private:
  TWinSCPFileSystem * FFileSystem;
  apr_pool_t * FPool;
  TIDAllocator * FIDAllocator;
  apr_hash_t * FSubplugins; // id --> subplugin_info_t *
  apr_hash_t * FHooks; // wchar_t * --> plugin_hook_t *
  apr_hash_t * FInterfaces; // wchar_t * --> nbptr_t
  nb_core_t FCore;
  uintptr_t FSecNum;
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

//------------------------------------------------------------------------------

} // namespace netbox
