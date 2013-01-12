#pragma once

#include <NetBoxSubPlugin.hpp>

//------------------------------------------------------------------------------
// #define GET_AUTO_SWITCH(Name) SessionData->Get ## Bame()
//------------------------------------------------------------------------------
// class TSessionDataProxy;
//------------------------------------------------------------------------------

class TFTPFileSystem : public TBaseSubplugin
{

public:
  explicit TFTPFileSystem(HINSTANCE HInst,
    nb_core_t * host);
  virtual ~TFTPFileSystem();

  subplugin_error_t Init();

public:
  subplugin_error_t OnSessionDialogInitTabs(
    nbptr_t object,
    nbptr_t data,
    nbptr_t common,
    nb_bool_t * bbreak);
  subplugin_error_t OnSessionDialogAfterInitSessionTabs(
    nbptr_t object,
    nbptr_t data,
    nbptr_t common,
    nb_bool_t * bbreak);
  subplugin_error_t OnSessionDialogUpdateControls(
    nbptr_t object,
    nbptr_t data,
    nbptr_t common,
    nb_bool_t * bbreak);

public:
  // fs_protocol_t functions implementation
  intptr_t init(void * data);
  bool is_capable(fs_capability_enum_t cap);
  const wchar_t * get_session_url_prefix();

private:
  nb_core_t * FHost;
  nb_utils_t * FUtils;
  nb_config_t * FConfig;
  nb_log_t * FLogging;
  void * FPool;
  intptr_t FTabID;
  intptr_t FTabControlID;
  intptr_t FProtocolID;
  static fs_protocol_t ftp_prot;
  TSessionDataProxy * FSessionDataProxy;

private:
  TSessionDataProxy * GetSessionData() { return FSessionDataProxy; }

private:
  void ResetReply();

private:
  int FLastCode;
  int FLastCodeClass;
  TStrings * FLastResponse;
  TStrings * FLastError;
  // auto_switch_enum_t FListAll;
  TAutoSwitch FListAll;
};

//------------------------------------------------------------------------------
extern TFTPFileSystem * FileSystem;
//------------------------------------------------------------------------------
