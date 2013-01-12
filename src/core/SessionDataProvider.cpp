#include "SessionDataProvider.h"

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

ISessionDataProviderIntf * SessionDataProvider = NULL;

//---------------------------------------------------------------------------

void InitSessionDataProvider(ISessionDataProviderIntf * AProvider)
{
  assert(SessionDataProvider == NULL);
  SessionDataProvider = AProvider;
}

//---------------------------------------------------------------------------
