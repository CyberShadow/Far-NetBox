#pragma once
//---------------------------------------------------------------------------

class ISessionDataProviderIntf
{
public:
  virtual ~ISessionDataProviderIntf() = 0 {}

  virtual intptr_t GetFSProtocolsCount() = 0;
  virtual intptr_t GetFSProtocolId(intptr_t Index) = 0;
  virtual UnicodeString GetFSProtocolStr(intptr_t Index) = 0;

  virtual UnicodeString GetFSProtocolStrById(intptr_t ProtocolId) = 0;
  virtual bool IsCapable(intptr_t ProtocolId, fs_capability_enum_t Capability) = 0;
  virtual UnicodeString GetSessionUrl(intptr_t ProtocolId) = 0;
};

//---------------------------------------------------------------------------

extern ISessionDataProviderIntf * SessionDataProvider;

//---------------------------------------------------------------------------

void InitSessionDataProvider(ISessionDataProviderIntf * AProvider);

//---------------------------------------------------------------------------
