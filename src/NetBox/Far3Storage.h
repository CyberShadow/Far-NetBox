#pragma once

#include <vector>

#include <Classes.hpp>
#include "HierarchicalStorage.h"
#include "PluginSettings.hpp"

//---------------------------------------------------------------------------
class TFar3Storage : public THierarchicalStorage
{
public:
  explicit TFar3Storage(const UnicodeString & AStorage,
    const GUID & Guid, FARAPISETTINGSCONTROL SettingsControl);
  virtual ~TFar3Storage();

  bool Copy(TFar3Storage * Storage);

  virtual void CloseSubKey();
  virtual bool DeleteSubKey(const UnicodeString & SubKey);
  virtual bool DeleteValue(const UnicodeString & Name);
  virtual void GetSubKeyNames(TStrings * Strings);
  virtual bool ValueExists(const UnicodeString & Value) const;

  virtual size_t BinaryDataSize(const UnicodeString & Name);

  virtual bool ReadBool(const UnicodeString & Name, bool Default);
  virtual intptr_t ReadInteger(const UnicodeString & Name, intptr_t Default);
  virtual __int64 ReadInt64(const UnicodeString & Name, __int64 Default);
  virtual TDateTime ReadDateTime(const UnicodeString & Name, TDateTime Default);
  virtual double ReadFloat(const UnicodeString & Name, double Default);
  virtual UnicodeString ReadStringRaw(const UnicodeString & Name, const UnicodeString & Default);
  virtual size_t ReadBinaryData(const UnicodeString & Name, void * Buffer, size_t Size);

  virtual void WriteBool(const UnicodeString & Name, bool Value);
  virtual void WriteStringRaw(const UnicodeString & Name, const UnicodeString & Value);
  virtual void WriteInteger(const UnicodeString & Name, intptr_t Value);
  virtual void WriteInt64(const UnicodeString & Name, __int64 Value);
  virtual void WriteDateTime(const UnicodeString & Name, TDateTime AValue);
  virtual void WriteFloat(const UnicodeString & Name, double AValue);
  virtual void WriteBinaryData(const UnicodeString & Name, const void * Buffer, size_t Size);

  virtual void GetValueNames(TStrings * Strings) const;
  virtual void SetAccessMode(TStorageAccessMode Value);
  virtual bool DoKeyExists(const UnicodeString & SubKey, bool ForceAnsi);
  virtual bool DoOpenSubKey(const UnicodeString & MungedSubKey, bool CanCreate);
  virtual UnicodeString GetSource() const;

private:
  UnicodeString GetFullCurrentSubKey() { return /* GetStorage() + */ GetCurrentSubKey(); }
  intptr_t OpenSubKeyInternal(intptr_t Root, const UnicodeString & SubKey, bool CanCreate);

private:
  intptr_t FRoot;
  mutable PluginSettings FPluginSettings;
  std::vector<intptr_t> FSubKeyIds;

  void Init();
};
//---------------------------------------------------------------------------
