#pragma once

#include <FarPlugin.h>
#include <subplugin.hpp>

//------------------------------------------------------------------------------

class TBaseSubplugin : public TCustomFarPlugin
{
  friend TWinSCPFileSystem;
public:
  explicit TBaseSubplugin(HINSTANCE HInst);
  virtual ~TBaseSubplugin();

  // virtual void HandleException(Exception * E, int OpMode = 0);
  virtual void GetPluginInfoEx(long unsigned & Flags,
    TStrings * DiskMenuStrings, TStrings * PluginMenuStrings,
    TStrings * PluginConfigStrings, TStrings * CommandPrefixes);
  virtual TCustomFarFileSystem * OpenPluginEx(intptr_t OpenFrom, intptr_t Item);
  virtual bool ConfigureEx(intptr_t Item);
  virtual intptr_t ProcessEditorEventEx(intptr_t Event, void * Param);
  virtual intptr_t ProcessEditorInputEx(const INPUT_RECORD * Rec);

public:
  // virtual subplugin_error_t Notify(subplugin_t * subplugin,
    // const notification_t * notification) = 0;

private:
};

//------------------------------------------------------------------------------
// Utility functions

// wchar_t * GuidToStr(const GUID & Guid, wchar_t * Buffer, size_t sz);
