#pragma once

#include <NetBoxSubPlugin.hpp>

//------------------------------------------------------------------------------

class TSubplugin : public TBaseSubplugin
{
  friend TWinSCPFileSystem;

public:
  explicit TSubplugin(HINSTANCE HInst,
    nb_utils_t * utils,
    nb_config_t * config,
    nb_log_t * logging);
  virtual ~TSubplugin();

public:
  // virtual subplugin_error_t Notify(subplugin_t * subplugin,
    // const notification_t * notification);

public:
  subplugin_error_t NotifyEditSessionInitTabs();
  // subplugin_error_t NotifyEditSessionInitSessionTab(subplugin_t * subplugin,
    // const notification_t * notification);
  // subplugin_error_t NotifyEditSessionAfterInitSessionTabs(subplugin_t * subplugin,
    // const notification_t * notification);
  // subplugin_error_t NotifyEditSessionUpdateControls(subplugin_t * subplugin,
    // const notification_t * notification);

private:
  nb_utils_t * FUtils;
  nb_config_t * FConfig;
  nb_log_t * FLogging;
  intptr_t FTabID;
  intptr_t FTabControlID;
  intptr_t FProtocolID;
};
