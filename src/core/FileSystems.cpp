//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "FileSystems.h"
#include "RemoteFiles.h"
#include "Common.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
TCustomFileSystem::TCustomFileSystem(TTerminalIntf * ATerminal):
  FTerminal(ATerminal)
{
  assert(FTerminal);
}
//---------------------------------------------------------------------------
TCustomFileSystem::~TCustomFileSystem()
{
}
//---------------------------------------------------------------------------
