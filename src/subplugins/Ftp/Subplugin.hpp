#pragma once

#include <NetBoxSubPlugin.hpp>

//------------------------------------------------------------------------------
// #define GET_AUTO_SWITCH(Name) SessionData->Get ## Bame()
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
  static nb_filesystem_t * NBAPI
  create(
    nbptr_t data,
    error_handler_t err);

  // nb_filesystem_t functions implementation
  static void NBAPI
  init(
    nb_filesystem_t * object,
    void * data,
    error_handler_t err);
  static void NBAPI
  destroy(
    nb_filesystem_t * object,
    error_handler_t err);
  static void NBAPI
  open(
    nb_filesystem_t * object,
    error_handler_t err);
  static void NBAPI
  close(
    nb_filesystem_t * object,
    error_handler_t err);
  static nb_bool_t NBAPI
  get_active(
    nb_filesystem_t * object,
    error_handler_t err);
  static nb_bool_t NBAPI
  is_capable(
    nb_filesystem_t * object,
    fs_capability_enum_t cap,
    error_handler_t err);
  static const wchar_t * NBAPI
  get_session_url_prefix(
    nb_filesystem_t * object,
    error_handler_t err);

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
  static nb_protocol_info_t FFtpProtocolInfo;
  static nb_filesystem_t FFileSystem;
  // TSessionDataProxy * FSessionDataProxy;

private:
  // TSessionDataProxy * GetSessionData() { return FSessionDataProxy; }
};

//------------------------------------------------------------------------------
extern TSubplugin * Subplugin;
//------------------------------------------------------------------------------
