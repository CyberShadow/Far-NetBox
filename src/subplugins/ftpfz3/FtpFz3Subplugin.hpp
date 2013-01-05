#pragma once

#include <NetBoxSubPlugin.hpp>

//------------------------------------------------------------------------------

class TSubplugin : public TBaseSubplugin
{
  friend TWinSCPFileSystem;
public:
  explicit TSubplugin(HINSTANCE HInst,
    const subplugin_startup_info_t * startup_info);
  virtual ~TSubplugin();

public:
  virtual subplugin_error_t Notify(subplugin_t * subplugin,
    const notification_t * notification);

private:
  subplugin_error_t NotifyEditSessionInitTabs(subplugin_t * subplugin,
    const notification_t * notification);
  subplugin_error_t NotifyEditSessionInitSessionTab(subplugin_t * subplugin,
    const notification_t * notification);
  subplugin_error_t NotifyEditSessionAfterInitSessionTabs(subplugin_t * subplugin,
    const notification_t * notification);
  subplugin_error_t NotifyEditSessionUpdateControls(subplugin_t * subplugin,
    const notification_t * notification);

private:
  intptr_t FTabID;
  intptr_t FTabControlID;
  intptr_t FProtocolID;
};
