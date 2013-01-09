#include "SessionDataProvider.h"

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

ISessionDataProviderIntf * SessionDataProvider = NULL;

//---------------------------------------------------------------------------

void InitSessionDataProvider(ISessionDataProviderIntf * Provider)
{
  assert(SessionDataProvider == NULL);
  SessionDataProvider = Provider;
}

//---------------------------------------------------------------------------
