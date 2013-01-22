#pragma once

#include <windows.h>

#include <string>

//------------------------------------------------------------------------------

class TLibraryLoader
{
public:
  explicit TLibraryLoader(const std::wstring & libraryName, bool AllowFailure = false);
  explicit TLibraryLoader();
  ~TLibraryLoader();

  void Load(const std::wstring & LibraryName, bool AllowFailure = false);
  FARPROC GetProcAddress(const std::string & ProcedureName);
  FARPROC GetProcAddress(intptr_t ProcedureOrdinal);
  bool Loaded() const { return FHModule != NULL; }

private:
  HMODULE FHModule;

private:
  TLibraryLoader(const TLibraryLoader &);
  TLibraryLoader & operator = (const TLibraryLoader &);
};

//------------------------------------------------------------------------------
