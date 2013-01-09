#pragma once
#ifndef SessionDataProviderH
#define SessionDataProviderH

//---------------------------------------------------------------------------

class ISessionDataProviderIntf
{
public:
  virtual ~ISessionDataProviderIntf() {}
};

//---------------------------------------------------------------------------

extern ISessionDataProviderIntf * SessionDataProvider;

//---------------------------------------------------------------------------

void InitSessionDataProvider(ISessionDataProviderIntf * Provider);

//---------------------------------------------------------------------------

#endif // SessionDataProviderH
