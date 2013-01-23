//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Common.h"
#include "CoreMain.h"
// #include "FarConfiguration.h"
// #include "WinSCPPlugin.h"
#include "Queue.h"
#include "Subplugin.hpp"
//------------------------------------------------------------------------------
#pragma package(smart_init)
//------------------------------------------------------------------------------
TCustomFarPlugin * FarPlugin = NULL; // TODO: Use GlobalFunctions
ISubpluginsManagerIntf * SubpluginsManager = NULL;
//------------------------------------------------------------------------------
TConfiguration * CreateConfiguration()
{
  return NULL; // new TFarConfiguration(FarPlugin);
}
//------------------------------------------------------------------------------
void ShowExtendedException(Exception * E)
{
  assert(FarPlugin != NULL);
  // TWinSCPPlugin * WinSCPPlugin = dynamic_cast<TWinSCPPlugin *>(FarPlugin);
  // assert(WinSCPPlugin != NULL);
  // WinSCPPlugin->ShowExtendedException(E);
}
//------------------------------------------------------------------------------
UnicodeString AppNameString()
{
  return L"NetBox";
}

//------------------------------------------------------------------------------
UnicodeString GetRegistryKey()
{
  return L"";
}
//------------------------------------------------------------------------------
void Busy(bool /*Start*/)
{
  // nothing
}
//------------------------------------------------------------------------------
UnicodeString SshVersionString()
{
  return FORMAT(L"NetBox-FAR", L"");
}

//------------------------------------------------------------------------------
DWORD WINAPI threadstartroutine(void * Parameter)
{
  TSimpleThread * SimpleThread = static_cast<TSimpleThread *>(Parameter);
  return TSimpleThread::ThreadProc(SimpleThread);
}
//------------------------------------------------------------------------------
HANDLE BeginThread(void * SecurityAttributes, DWORD StackSize,
  void * Parameter, DWORD CreationFlags,
  DWORD & ThreadId)
{
  HANDLE Result = ::CreateThread(static_cast<LPSECURITY_ATTRIBUTES>(SecurityAttributes),
    static_cast<size_t>(StackSize),
    static_cast<LPTHREAD_START_ROUTINE>(&threadstartroutine),
    Parameter,
    CreationFlags, &ThreadId);
  return Result;
}

void EndThread(int ExitCode)
{
  ::ExitThread(ExitCode);
}

//------------------------------------------------------------------------------
HANDLE StartThread(void * SecurityAttributes, unsigned int StackSize,
  void * Parameter, unsigned int CreationFlags,
  DWORD & ThreadId)
{
  return BeginThread(SecurityAttributes, StackSize, Parameter,
    CreationFlags, ThreadId);
}
//------------------------------------------------------------------------------
void CopyToClipboard(const UnicodeString & Text)
{
  assert(FarPlugin != NULL);
  // FarPlugin->FarCopyToClipboard(Text);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
namespace ftp {

TGlobalFunctions::TGlobalFunctions()
{
}
//------------------------------------------------------------------------------
TGlobalFunctions::~TGlobalFunctions()
{
}
//------------------------------------------------------------------------------
HINSTANCE TGlobalFunctions::GetHandle() const
{
  assert(Subplugin);
  HINSTANCE Result = Subplugin->GetHandle();
  return Result;
}
//------------------------------------------------------------------------------
UnicodeString TGlobalFunctions::GetCurrentDirectory() const
{
  UnicodeString Result;
  wchar_t Path[MAX_PATH + 1];
  ::GetCurrentDirectory(sizeof(Path), Path);
  Result = Path;
  return Result;
}
//------------------------------------------------------------------------------
} // namespace ftp
//------------------------------------------------------------------------------
