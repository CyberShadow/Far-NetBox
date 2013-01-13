#pragma once

#include <map>

#include <SubpluginDefs.hpp>
#include <Interface.h>

#include "FarTexts.h"

class TWinSCPPlugin;
struct apr_pool_t;
struct apr_hash_t;
// struct apr_table_t;

namespace netbox {

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

class TSubpluginsManager : public ISubpluginsManagerIntf
{
public:
  explicit TSubpluginsManager(TWinSCPPlugin * WinSCPPlugin);
  ~TSubpluginsManager();

  void Init();
  void Shutdown();

  // core
  intf_handle_t register_interface(
    const wchar_t * guid, nbptr_t funcs);
  nbptr_t query_interface(
    const wchar_t * guids);
  bool release_interface(
    intf_handle_t intf);

  bool has_subplugin(const wchar_t * guid);

  intptr_t register_fs_protocol(
    fs_protocol_t * prot);

  // hooks
  plugin_hook_t * create_hook(
    const wchar_t * guid, nb_hook_t def_proc);
  bool destroy_hook(
    plugin_hook_t * hook);

  hook_subscriber_t * bind_hook(
    const wchar_t * guid, nb_hook_t hook_proc, void * common);

  intptr_t release_hook(
    hook_subscriber_t * subscription);

  // RunHook wrappers for host calls
  bool RunHook(
    const wchar_t * guid, nbptr_t object, nbptr_t data);
  bool RunHook(
    plugin_hook_t * hook, nbptr_t object, nbptr_t data);

  // log
  void log(const wchar_t * msg);

public:
  nb_core_t * GetCore() { return &FCore; }

  const wchar_t * GetSubpluginMsg(
    const wchar_t * guid, const wchar_t * msg_id);

public:
  // ISubpluginsManagerIntf
  virtual intptr_t GetFSProtocolsCount();
  virtual intptr_t GetFSProtocolId(intptr_t Index);
  virtual UnicodeString GetFSProtocolStr(intptr_t Index);
  virtual UnicodeString GetFSProtocolStrById(intptr_t ProtocolId);

  virtual fs_handle_t Create(intptr_t ProtocolId, void * Data);
  virtual intptr_t Init(fs_handle_t Handle, void * Data);
  virtual void Open(fs_handle_t Handle);
  virtual void Close(fs_handle_t Handle);
  virtual bool GetActive(fs_handle_t Handle);

  virtual bool IsCapable(fs_handle_t Handle, fs_capability_enum_t Capability);
  virtual UnicodeString GetCurrentDirectory(fs_handle_t Handle);

  virtual UnicodeString GetSessionUrl(fs_handle_t Handle);

private:
  const wchar_t * StrDup(
    const wchar_t * str, size_t len, apr_pool_t * pool);
  subplugin_error_t InitSubpluginInfo(
    subplugin_info_t ** subplugin_info,
    const nb::subplugin * subplugin_library,
    const wchar_t * module_name,
    apr_pool_t * pool);

  subplugin_info_t * GetSubpluginByGuid(
    const wchar_t * guid);

  fs_protocol_t * GetFSProtocolByName(
    const wchar_t * Name);
  fs_protocol_t * GetFSProtocolById(
    intptr_t Id);
  fs_protocol_t * GetFSProtocolByHandle(
    fs_handle_t Handle);

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

private:
  TWinSCPPlugin * FWinSCPPlugin;
  apr_pool_t * FPool;
  apr_hash_t * FSubplugins; // id --> subplugin_info_t *
  // Protocols implemented by subplugins
  apr_hash_t * FProtocols; // id --> fs_protocol_t *
  apr_hash_t * FHooks; // wchar_t * --> plugin_hook_t *
  apr_hash_t * FInterfaces; // wchar_t * --> nbptr_t
  nb_utils_t * FUtils;
  nb_core_t FCore;
  uintptr_t FSecNum;
};

//------------------------------------------------------------------------------

} // namespace netbox
