#include "LibraryLoader.hpp"

//------------------------------------------------------------------------------
TLibraryLoader::TLibraryLoader(const std::wstring & LibraryName, bool AllowFailure) :
  FHModule(NULL)
{
  Load(LibraryName, AllowFailure);
}
//------------------------------------------------------------------------------
TLibraryLoader::TLibraryLoader() :
  FHModule(NULL)
{
}
//------------------------------------------------------------------------------
TLibraryLoader::~TLibraryLoader()
{
  Unload();
}
//------------------------------------------------------------------------------
void TLibraryLoader::Load(const std::wstring & LibraryName, bool AllowFailure)
{
  assert(FHModule == NULL);

  // Loading library
  FHModule = ::LoadLibrary(LibraryName.c_str());

  if (!AllowFailure)
  {
    assert(FHModule != 0);
  }
}
//------------------------------------------------------------------------------
void TLibraryLoader::Unload()
{
  if (FHModule != NULL)
  {
    ::FreeLibrary(FHModule);
    FHModule = NULL;
  }
}
//------------------------------------------------------------------------------
// Get procedure address from loaded library by name
FARPROC TLibraryLoader::GetProcAddress(const std::string & ProcedureName)
{
  return ::GetProcAddress(FHModule, ProcedureName.c_str());
}
//------------------------------------------------------------------------------
// Get procedure address from loaded library by ordinal value
FARPROC TLibraryLoader::GetProcAddress(intptr_t ProcedureOrdinal)
{
  return ::GetProcAddress(FHModule, (LPCSTR)0 + ProcedureOrdinal);
}

//------------------------------------------------------------------------------
