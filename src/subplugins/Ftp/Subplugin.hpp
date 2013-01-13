#pragma once

#include <NetBoxSubPlugin.hpp>

//------------------------------------------------------------------------------
// #define GET_AUTO_SWITCH(Name) SessionData->Get ## Bame()
//------------------------------------------------------------------------------
// class TSessionDataProxy;
//------------------------------------------------------------------------------

class TSubplugin : public TBaseSubplugin
{

public:
  explicit TSubplugin(HINSTANCE HInst,
    nb_core_t * host);
  virtual ~TSubplugin();

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
  // nb_protocol_info_t functions implementation
  nb_filesystem_t * create(nbptr_t data);

  // nb_filesystem_t functions implementation
  void init(nb_filesystem_t * fs, void * data);
  void destroy(nb_filesystem_t * fs);
  nb_bool_t is_capable(nb_filesystem_t * fs, fs_capability_enum_t cap);
  const wchar_t * get_session_url_prefix(nb_filesystem_t * fs);

private:
  nb_core_t * FHost;
  nb_utils_t * FUtils;
  nb_config_t * FConfig;
  nb_log_t * FLogging;
  void * FPool;
  void * FImpls;
  intptr_t FTabID;
  intptr_t FTabControlID;
  intptr_t FProtocolID;
  static nb_protocol_info_t FFtpProtocol;
  // TSessionDataProxy * FSessionDataProxy;

private:
  // TSessionDataProxy * GetSessionData() { return FSessionDataProxy; }
};

//------------------------------------------------------------------------------
extern TSubplugin * Subplugin;
//------------------------------------------------------------------------------
