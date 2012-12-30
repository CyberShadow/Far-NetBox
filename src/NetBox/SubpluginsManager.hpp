#pragma once

#include <apr_pools.h>

//------------------------------------------------------------------------------

class TIDAllocator
{
public:
  TIDAllocator(int start, int maximumID) :
    FStart(start),
    FNextID(start),
    FMaximumID(maximumID)
  {}

  /// Returns -1 if not enough available
  int allocate(int quantity)
  {
    int retVal = -1;

    if (FNextID + quantity <= FMaximumID && quantity > 0)
    {
      retVal = FNextID;
      FNextID += quantity;
    }

    return retVal;
  }

  bool isInRange(int id) { return (id >= FStart && id < FNextID); }

private:
  int FStart;
  int FNextID;
  int FMaximumID;
};

//------------------------------------------------------------------------------
class TWinSCPFileSystem;

class TSubpluginsManager
{
public:
  explicit TSubpluginsManager(TWinSCPFileSystem * FileSystem);
  ~TSubpluginsManager();

  void InitSubplugins();
  void UnloadSubplugins();
  void Notify(const notification_t * notification);

  int GetNextID() { return FIDAllocator.allocate(1); }
  const wchar_t * GetSubpluginMsg(
    subplugin_t * subplugin,
    const wchar_t * msg_id);
  /* int GetDialogItemID(
    const notification_t * notification,
    const wchar_t * dialog_item_str_id); */
  /* int AddItem(
    const notification_t * notification,
    int dialog_item_id,
    const wchar_t * item);*/
  void * DialogItemGetProperty(
    const property_baton_t * baton);
  void * DialogItemSetProperty(
    const property_baton_t * baton);
  void * SendMessage(
    const send_message_baton_t * baton);

private:
  TWinSCPFileSystem * FFileSystem;
  TList * FSubplugins;
  apr_pool_t * FPool;
  TIDAllocator FIDAllocator;

  void LoadSubpluginMessages(subplugin_t * subplugin,
    const char * msg_file_name);
  PluginStartupInfo * GetPluginStartupInfo() const;
  void InitStartupInfo(subplugin_startup_info_t ** startup_info,
    apr_pool_t * pool);
  void MakeSubpluginsFileList(const UnicodeString & FileName,
    const TSearchRec & Rec, void * Param);
};

//------------------------------------------------------------------------------
