#pragma once

#include <NetBoxSubPlugin.hpp>

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

private:
  // fs_protocol_t functions implementation
  static intptr_t NBAPI
  init(
    void * data);
  static nb_bool_t NBAPI
  is_capable(
    fs_capability_enum_t cap);
  static const wchar_t * NBAPI
  get_session_url_prefix();

  static fs_protocol_t ftp_prot;
private:
  nb_core_t * FHost;
  nb_utils_t * FUtils;
  nb_config_t * FConfig;
  nb_log_t * FLogging;
  void * FPool;
  intptr_t FTabID;
  intptr_t FTabControlID;
  intptr_t FProtocolID;
};
