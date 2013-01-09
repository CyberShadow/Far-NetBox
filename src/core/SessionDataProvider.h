#pragma once
#ifndef SessionDataProviderH
#define SessionDataProviderH

//---------------------------------------------------------------------------

class ISessionDataProviderIntf
{
public:
  virtual ~ISessionDataProviderIntf() {}

  virtual intptr_t GetFSProtocolsCount() = 0;
  virtual intptr_t GetFSProtocolID(intptr_t Index) = 0;
  virtual UnicodeString GetFSProtocolStr(intptr_t Index) = 0;
};

//---------------------------------------------------------------------------

extern ISessionDataProviderIntf * SessionDataProvider;

//---------------------------------------------------------------------------

void InitSessionDataProvider(ISessionDataProviderIntf * Provider);

//---------------------------------------------------------------------------

#endif // SessionDataProviderH
