#pragma once

#include <subplugin.hpp>

class TWinSCPFileSystem;
struct apr_pool_t;

namespace netbox {

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
  nb_interface_t * query_interface(
    const wchar_t * guid, intptr_t version);
  nbBool release_interface(
    intf_handle_t intf);
  nbBool has_subplugin(const wchar_t * guid);

  // hooks
  hook_handle_t create_hook(
    const wchar_t * guid, nb_hook_t def_proc);
  nbBool destroy_hook(
    hook_handle_t hook);

  subs_handle_t bind_hook(
    const wchar_t * guid, nb_hook_t hook_proc, void * common);
  nbBool run_hook(
    hook_handle_t hook, nbptr_t object, nbptr_t data);
  intptr_t release_hook(
    subs_handle_t hook);

  nb_core_t * GetCore() { return &FCore; }

  void Notify(const notification_t * notification);

  intptr_t GetNextID() { return FIDAllocator.allocate(1); }
  const wchar_t * GetSubpluginMsg(
    subplugin_t * subplugin,
    const wchar_t * msg_id);
  void * DialogItemGetProperty(
    const property_baton_t * baton);
  void * DialogItemSetProperty(
    const property_baton_t * baton);
  void * SendMessage(
    const send_message_baton_t * baton);

private:
  void LoadSubplugins(apr_pool_t * pool);
  void UnloadSubplugins();

  void LoadSubpluginMessages(subplugin_t * subplugin,
    const UnicodeString & MsgFileName);
  PluginStartupInfo * GetPluginStartupInfo() const;
  void InitStartupInfo(subplugin_startup_info_t ** startup_info,
    apr_pool_t * pool);
  void MakeSubpluginsFileList(const UnicodeString & FileName,
    const TSearchRec & Rec, void * Param);

private:
  TWinSCPFileSystem * FFileSystem;
  TList * FSubplugins;
  apr_pool_t * FPool;
  TIDAllocator FIDAllocator;
  nb_core_t FCore;
};

//------------------------------------------------------------------------------

} // namespace netbox
