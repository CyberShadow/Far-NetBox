//---------------------------------------------------------------------------
#ifndef SessionDataH
#define SessionDataH

#include <vector>

#include "Common.h"
#include "Option.h"
#include "FileBuffer.h"
#include "NamedObjs.h"
#include "HierarchicalStorage.h"
#include "Configuration.h"
//---------------------------------------------------------------------------
#define SET_SESSION_PROPERTY(Property) \
  if (F##Property != Value) { F##Property = Value; Modify(); }
//---------------------------------------------------------------------------
enum TCipher { cipWarn, cip3DES, cipBlowfish, cipAES, cipDES, cipArcfour };
#define CIPHER_COUNT (cipArcfour+1)
enum TProtocol { ptRaw, ptTelnet, ptRLogin, ptSSH };
#define PROTOCOL_COUNT (ptSSH+1)
// explicit values to skip obsoleted fsExternalSSH, fsExternalSFTP
enum TFSProtocol_219 { fsFTPS_219 = 6, fsHTTP_219 = 7, fsHTTPS_219 = 8 };
enum TFSProtocol { fsSCPonly = 0, fsSFTP = 1, fsSFTPonly = 2, fsFTP = 5, fsWebDAV = 6 };
// typedef int TFSProtocol;
#define FSPROTOCOL_COUNT (fsWebDAV+1)
enum TLoginType { ltAnonymous = 0, ltNormal = 1 };
enum TProxyMethod { pmNone, pmSocks4, pmSocks5, pmHTTP, pmTelnet, pmCmd, pmSystem };
enum TSshProt { ssh1only, ssh1, ssh2, ssh2only };
enum TKex { kexWarn, kexDHGroup1, kexDHGroup14, kexDHGEx, kexRSA };
#define KEX_COUNT (kexRSA+1)
enum TSshBug { sbIgnore1, sbPlainPW1, sbRSA1, sbHMAC2, sbDeriveKey2, sbRSAPad2,
  sbPKSessID2, sbRekey2, sbMaxPkt2, sbIgnore2 };
#define BUG_COUNT (sbIgnore2+1)
enum TSftpBug { sbSymlink, sbSignedTS };
#define SFTP_BUG_COUNT (sbSignedTS+1)
enum TPingType { ptOff, ptNullPacket, ptDummyCommand };
enum TAddressFamily { afAuto, afIPv4, afIPv6 };
enum TFtps { ftpsNone, ftpsImplicit, ftpsExplicitSsl, ftpsExplicitTls };
enum TSessionSource { ssNone, ssStored, ssStoredModified };
//---------------------------------------------------------------------------
extern const wchar_t CipherNames[CIPHER_COUNT][10];
extern const wchar_t KexNames[KEX_COUNT][20];
extern const wchar_t ProtocolNames[PROTOCOL_COUNT][10];
extern const wchar_t SshProtList[][10];
extern const wchar_t ProxyMethodList[][10];
extern const TCipher DefaultCipherList[CIPHER_COUNT];
extern const TKex DefaultKexList[KEX_COUNT];
extern const int DefaultSendBuf;
extern const UnicodeString AnonymousUserName;
extern const UnicodeString AnonymousPassword;
extern const int SshPortNumber;
extern const int FtpPortNumber;
extern const int FtpsImplicitPortNumber;
extern const int HTTPPortNumber;
extern const int HTTPSPortNumber;
//---------------------------------------------------------------------------
struct TIEProxyConfig
{
  TIEProxyConfig() :
    AutoDetect(false),
    ProxyPort(0),
    ProxyMethod(pmNone)
  {}
  bool AutoDetect; // not used
  UnicodeString AutoConfigUrl; // not used
  UnicodeString Proxy; //< string in format "http=host:80;https=host:443;ftp=ftpproxy:20;socks=socksproxy:1080"
  UnicodeString ProxyBypass; //< string in format "*.local, foo.com, google.com"
  UnicodeString ProxyHost;
  int ProxyPort;
  TProxyMethod ProxyMethod;
};
//---------------------------------------------------------------------------
class TStoredSessionList;
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class TSessionDataIntf
{
public:
  virtual ~TSessionDataIntf() = 0 {}

  virtual UnicodeString GetName() const = 0;
  virtual void SetName(const UnicodeString & Value) = 0;

  virtual void SetHostName(const UnicodeString & Value) = 0;
  virtual UnicodeString GetHostNameExpanded() = 0;
  virtual void SetPortNumber(int Value) = 0;
  virtual void SetUserName(const UnicodeString & Value) = 0;
  virtual UnicodeString GetUserNameExpanded() = 0;
  virtual void SetPassword(const UnicodeString & Value) = 0;
  virtual UnicodeString GetPassword() const;
  virtual void SetPasswordless(bool Value) = 0;
  virtual void SetPingInterval(int Value) = 0;
  virtual void SetTryAgent(bool Value) = 0;
  virtual void SetAgentFwd(bool Value) = 0;
  virtual void SetAuthTIS(bool Value) = 0;
  virtual void SetAuthKI(bool Value) = 0;
  virtual void SetAuthKIPassword(bool Value) = 0;
  virtual void SetAuthGSSAPI(bool Value) = 0;
  virtual void SetGSSAPIFwdTGT(bool Value) = 0;
  virtual void SetGSSAPIServerRealm(const UnicodeString & Value) = 0;
  virtual void SetChangeUsername(bool Value) = 0;
  virtual void SetCompression(bool Value) = 0;
  virtual void SetSshProt(TSshProt Value) = 0;
  virtual void SetSsh2DES(bool Value) = 0;
  virtual void SetSshNoUserAuth(bool Value) = 0;
  virtual void SetCipher(int Index, TCipher Value) = 0;
  virtual TCipher GetCipher(int Index) const;
  virtual void SetKex(int Index, TKex Value) = 0;
  virtual TKex GetKex(int Index) const;
  virtual void SetPublicKeyFile(const UnicodeString & Value) = 0;

  virtual void SetProtocolStr(const UnicodeString & Value) = 0;
  virtual UnicodeString GetProtocolStr() const;
  virtual bool GetCanLogin() = 0;
  virtual void SetPingIntervalDT(TDateTime Value) = 0;
  virtual TDateTime GetPingIntervalDT() const;
  virtual TDateTime GetFtpPingIntervalDT() = 0;
  virtual void SetTimeDifference(TDateTime Value) = 0;
  virtual void SetPingType(TPingType Value) = 0;
  virtual UnicodeString GetSessionName() = 0;
  virtual bool HasSessionName() = 0;
  virtual UnicodeString GetDefaultSessionName() = 0;
  virtual void SetProtocol(TProtocol Value) = 0;
  virtual void SetFSProtocol(TFSProtocol Value) = 0;
  virtual UnicodeString GetFSProtocolStr() const;
  virtual void SetLocalDirectory(const UnicodeString & Value) = 0;
  virtual void SetRemoteDirectory(const UnicodeString & Value) = 0;
  virtual void SetSynchronizeBrowsing(bool Value) = 0;
  virtual void SetUpdateDirectories(bool Value) = 0;
  virtual void SetCacheDirectories(bool Value) = 0;
  virtual void SetCacheDirectoryChanges(bool Value) = 0;
  virtual void SetPreserveDirectoryChanges(bool Value) = 0;
  virtual void SetLockInHome(bool Value) = 0;
  virtual void SetSpecial(bool Value) = 0;
  virtual UnicodeString GetInfoTip() = 0;
  virtual bool GetDefaultShell() = 0;
  virtual void SetDetectReturnVar(bool Value) = 0;
  virtual bool GetDetectReturnVar() = 0;
  virtual void SetListingCommand(const UnicodeString & Value) = 0;
  virtual void SetClearAliases(bool Value) = 0;
  virtual void SetDefaultShell(bool Value) = 0;
  virtual void SetEOLType(TEOLType Value) = 0;
  virtual void SetLookupUserGroups(TAutoSwitch Value) = 0;
  virtual void SetReturnVar(const UnicodeString & Value) = 0;
  virtual void SetScp1Compatibility(bool Value) = 0;
  virtual void SetShell(const UnicodeString & Value) = 0;
  virtual void SetSftpServer(const UnicodeString & Value) = 0;
  virtual void SetTimeout(int Value) = 0;
  virtual void SetUnsetNationalVars(bool Value) = 0;
  virtual void SetIgnoreLsWarnings(bool Value) = 0;
  virtual void SetTcpNoDelay(bool Value) = 0;
  virtual void SetSendBuf(int Value) = 0;
  virtual void SetSshSimple(bool Value) = 0;
  virtual UnicodeString GetSshProtStr() = 0;
  virtual bool GetUsesSsh() = 0;
  virtual void SetCipherList(const UnicodeString & Value) = 0;
  virtual UnicodeString GetCipherList() const;
  virtual void SetKexList(const UnicodeString & Value) = 0;
  virtual UnicodeString GetKexList() const;
  virtual void SetProxyMethod(TProxyMethod Value) = 0;
  virtual void SetProxyHost(const UnicodeString & Value) = 0;
  virtual void SetProxyPort(int Value) = 0;
  virtual void SetProxyUsername(const UnicodeString & Value) = 0;
  virtual void SetProxyPassword(const UnicodeString & Value) = 0;
  virtual void SetProxyTelnetCommand(const UnicodeString & Value) = 0;
  virtual void SetProxyLocalCommand(const UnicodeString & Value) = 0;
  virtual void SetProxyDNS(TAutoSwitch Value) = 0;
  virtual void SetProxyLocalhost(bool Value) = 0;
  virtual UnicodeString GetProxyPassword() const;
  virtual void SetFtpProxyLogonType(int Value) = 0;
  virtual void SetBug(TSshBug Bug, TAutoSwitch Value) = 0;
  virtual TAutoSwitch GetBug(TSshBug Bug) const;
  virtual UnicodeString GetSessionKey() = 0;
  virtual void SetCustomParam1(const UnicodeString & Value) = 0;
  virtual void SetCustomParam2(const UnicodeString & Value) = 0;
  virtual void SetResolveSymlinks(bool Value) = 0;
  virtual void SetSFTPDownloadQueue(int Value) = 0;
  virtual void SetSFTPUploadQueue(int Value) = 0;
  virtual void SetSFTPListingQueue(int Value) = 0;
  virtual void SetSFTPMaxVersion(int Value) = 0;
  virtual void SetSFTPMinPacketSize(unsigned long Value) = 0;
  virtual void SetSFTPMaxPacketSize(unsigned long Value) = 0;
  virtual void SetSFTPBug(TSftpBug Bug, TAutoSwitch Value) = 0;
  virtual TAutoSwitch GetSFTPBug(TSftpBug Bug) const;
  virtual void SetSCPLsFullTime(TAutoSwitch Value) = 0;
  virtual void SetFtpListAll(TAutoSwitch Value) = 0;
  virtual void SetSslSessionReuse(bool Value) = 0;
  virtual UnicodeString GetStorageKey() = 0;
  virtual UnicodeString GetInternalStorageKey() = 0;
  virtual void SetDSTMode(TDSTMode Value) = 0;
  virtual void SetDeleteToRecycleBin(bool Value) = 0;
  virtual void SetOverwrittenToRecycleBin(bool Value) = 0;
  virtual void SetRecycleBinPath(const UnicodeString & Value) = 0;
  virtual void SetPostLoginCommands(const UnicodeString & Value) = 0;
  virtual void SetAddressFamily(TAddressFamily Value) = 0;
  virtual void SetRekeyData(const UnicodeString & Value) = 0;
  virtual void SetRekeyTime(unsigned int Value) = 0;
  virtual void SetColor(int Value) = 0;
  virtual void SetTunnel(bool Value) = 0;
  virtual void SetTunnelHostName(const UnicodeString & Value) = 0;
  virtual void SetTunnelPortNumber(int Value) = 0;
  virtual void SetTunnelUserName(const UnicodeString & Value) = 0;
  virtual void SetTunnelPassword(const UnicodeString & Value) = 0;
  virtual UnicodeString GetTunnelPassword() const;
  virtual void SetTunnelPublicKeyFile(const UnicodeString & Value) = 0;
  virtual void SetTunnelPortFwd(const UnicodeString & Value) = 0;
  virtual void SetTunnelLocalPortNumber(int Value) = 0;
  virtual bool GetTunnelAutoassignLocalPortNumber() = 0;
  virtual void SetTunnelHostKey(const UnicodeString & Value) = 0;
  virtual void SetFtpPasvMode(bool Value) = 0;
  virtual void SetFtpForcePasvIp(TAutoSwitch Value) = 0;
  virtual void SetFtpUseMlsd(TAutoSwitch Value) = 0;
  virtual void SetFtpAccount(const UnicodeString & Value) = 0;
  virtual void SetFtpPingInterval(int Value) = 0;
  virtual void SetFtpPingType(TPingType Value) = 0;
  virtual void SetFtps(TFtps Value) = 0;
  virtual void SetNotUtf(TAutoSwitch Value) = 0;
  virtual void SetHostKey(const UnicodeString & Value) = 0;
  virtual TDateTime GetTimeoutDT() = 0;
  virtual void SavePasswords(THierarchicalStorage * Storage, bool PuttyExport) = 0;
  virtual UnicodeString GetLocalName() = 0;
  virtual void Modify() = 0;
  virtual UnicodeString GetSource() = 0;
  virtual void DoLoad(THierarchicalStorage * Storage, bool & RewritePassword) = 0;

  virtual void Default() = 0;
  virtual void NonPersistant() = 0;
  virtual void Load(THierarchicalStorage * Storage) = 0;
  virtual void Save(THierarchicalStorage * Storage, bool PuttyExport,
    const TSessionDataIntf * Default = NULL) = 0;
  virtual void SaveRecryptedPasswords(THierarchicalStorage * Storage) = 0;
  virtual void RecryptPasswords() = 0;
  virtual bool HasAnyPassword() = 0;
  virtual void Remove() = 0;
  virtual void Assign(TSessionDataIntf * Source) = 0;

  virtual bool ParseUrl(const UnicodeString & Url, TOptions * Options,
    TStoredSessionList * StoredSessions, bool & DefaultsOnly,
    UnicodeString * FileName, bool * AProtocolDefined, UnicodeString * MaskedUrl) = 0;
  virtual bool ParseOptions(TOptions * Options) = 0;
  virtual void ConfigureTunnel(int PortNumber) = 0;
  virtual void RollbackTunnel() = 0;
  virtual void ExpandEnvironmentVariables() = 0;
  virtual bool IsSame(const TSessionDataIntf * Default, bool AdvancedOnly) = 0;
  virtual UnicodeString GetHostName() const= 0;
  virtual int GetPortNumber() const = 0;
  virtual TLoginType GetLoginType() const;
  virtual void SetLoginType(TLoginType Value) = 0;
  virtual UnicodeString GetUserName() const = 0;
  virtual int GetPingInterval() const = 0;
  virtual bool GetTryAgent() const = 0;
  virtual bool GetAgentFwd() const = 0;
  virtual const UnicodeString GetListingCommand() const = 0;
  virtual bool GetAuthTIS() const = 0;
  virtual bool GetAuthKI() const = 0;
  virtual bool GetAuthKIPassword() const = 0;
  virtual bool GetAuthGSSAPI() const = 0;
  virtual bool GetGSSAPIFwdTGT() const = 0;
  virtual const UnicodeString GetGSSAPIServerRealm() const = 0;
  virtual bool GetChangeUsername() const = 0;
  virtual bool GetCompression() const = 0;
  virtual TSshProt GetSshProt() const = 0;
  virtual bool GetSsh2DES() const = 0;
  virtual bool GetSshNoUserAuth() const = 0;
  virtual const UnicodeString GetPublicKeyFile() const = 0;
  virtual TProtocol GetProtocol() const = 0;
  virtual TFSProtocol GetFSProtocol() const = 0;
  virtual bool GetModified() const = 0;
  virtual void SetModified(bool Value) = 0;
  virtual bool GetClearAliases() const = 0;
  virtual TDateTime GetTimeDifference() const = 0;
  virtual TPingType GetPingType() const = 0;
  virtual UnicodeString GetLocalDirectory() const = 0;
  virtual UnicodeString GetRemoteDirectory() const = 0;
  virtual bool GetSynchronizeBrowsing() const = 0;
  virtual bool GetUpdateDirectories() const = 0;
  virtual bool GetCacheDirectories() const = 0;
  virtual bool GetCacheDirectoryChanges() const = 0;
  virtual bool GetPreserveDirectoryChanges() const = 0;
  virtual bool GetLockInHome() const = 0;
  virtual bool GetSpecial() const = 0;
  virtual bool GetSelected() const = 0;
  virtual void SetSelected(bool Value) = 0;
  virtual TEOLType GetEOLType() const = 0;
  virtual TAutoSwitch GetLookupUserGroups() const = 0;
  virtual UnicodeString GetReturnVar() const = 0;
  virtual bool GetScp1Compatibility() const = 0;
  virtual UnicodeString GetShell() const = 0;
  virtual UnicodeString GetSftpServer() const = 0;
  virtual int GetTimeout() const = 0;
  virtual bool GetUnsetNationalVars() const = 0;
  virtual bool GetIgnoreLsWarnings() const = 0;
  virtual bool GetTcpNoDelay() const = 0;
  virtual int GetSendBuf() const = 0;
  virtual bool GetSshSimple() const = 0;
  virtual TProxyMethod GetProxyMethod() const = 0;
  virtual TProxyMethod GetActualProxyMethod() const = 0;
  virtual UnicodeString GetProxyHost() const;
  virtual int GetProxyPort() const;
  virtual UnicodeString GetProxyUsername() const;
  virtual UnicodeString GetProxyTelnetCommand() const = 0;
  virtual UnicodeString GetProxyLocalCommand() const = 0;
  virtual TAutoSwitch GetProxyDNS() const = 0;
  virtual bool GetProxyLocalhost() const = 0;
  virtual int GetFtpProxyLogonType() const = 0;
  virtual UnicodeString GetCustomParam1() const = 0;
  virtual UnicodeString GetCustomParam2() const = 0;
  virtual bool GetResolveSymlinks() const = 0;
  virtual int GetSFTPDownloadQueue() const = 0;
  virtual int GetSFTPUploadQueue() const = 0;
  virtual int GetSFTPListingQueue() const = 0;
  virtual int GetSFTPMaxVersion() const = 0;
  virtual unsigned long GetSFTPMinPacketSize() const = 0;
  virtual unsigned long GetSFTPMaxPacketSize() const = 0;
  virtual TAutoSwitch GetSCPLsFullTime() const = 0;
  virtual TAutoSwitch GetFtpListAll() const = 0;
  virtual bool GetSslSessionReuse() const = 0;
  virtual TDSTMode GetDSTMode() const = 0;
  virtual bool GetDeleteToRecycleBin() const = 0;
  virtual bool GetOverwrittenToRecycleBin() const = 0;
  virtual UnicodeString GetRecycleBinPath() const = 0;
  virtual UnicodeString GetPostLoginCommands() const = 0;
  virtual TAddressFamily GetAddressFamily() const = 0;
  virtual UnicodeString GetCodePage() const = 0;
  virtual void SetCodePage(const UnicodeString & Value) = 0;
  virtual unsigned int GetCodePageAsNumber() const;
  virtual UnicodeString GetRekeyData() const = 0;
  virtual unsigned int GetRekeyTime() const = 0;
  virtual int GetColor() const = 0;
  virtual bool GetTunnel() const = 0;
  virtual UnicodeString GetTunnelHostName() const = 0;
  virtual int GetTunnelPortNumber() const = 0;
  virtual UnicodeString GetTunnelUserName() const = 0;
  virtual UnicodeString GetTunnelPublicKeyFile() const = 0;
  virtual int GetTunnelLocalPortNumber() const = 0;
  virtual UnicodeString GetTunnelPortFwd() const = 0;
  virtual UnicodeString GetTunnelHostKey() const = 0;
  virtual bool GetFtpPasvMode() const = 0;
  virtual bool GetFtpAllowEmptyPassword() const = 0;
  virtual void SetFtpAllowEmptyPassword(bool Value) = 0;
  virtual TAutoSwitch GetFtpForcePasvIp() const = 0;
  virtual TAutoSwitch GetFtpUseMlsd() const = 0;
  virtual UnicodeString GetFtpAccount() const = 0;
  virtual int GetFtpPingInterval() const = 0;
  virtual TPingType GetFtpPingType() const = 0;
  virtual TFtps GetFtps() const = 0;
  virtual TAutoSwitch GetNotUtf() const = 0;
  virtual UnicodeString GetHostKey() const = 0;
  virtual UnicodeString GetOrigHostName() const = 0;
  virtual int GetOrigPortNumber() const = 0;

  virtual int GetNumberOfRetries() const = 0;
  virtual void SetNumberOfRetries(int Value) = 0;
  virtual uintptr_t GetSessionVersion() const = 0;
  virtual void SetSessionVersion(uintptr_t Value) = 0;
};
//---------------------------------------------------------------------------
class TSessionData : public TNamedObject, public TSessionDataIntf
{
friend class TStoredSessionList;

public:
  // TSessionDataIntf implementation
  virtual UnicodeString GetName() const { return TNamedObject::GetName(); }
  virtual void SetName(const UnicodeString & Value) { TNamedObject::SetName(Value); }

  virtual void SetHostName(const UnicodeString & Value);
  virtual UnicodeString GetHostNameExpanded();
  virtual void SetPortNumber(int Value);
  virtual void SetUserName(const UnicodeString & Value);
  virtual UnicodeString GetUserNameExpanded();
  virtual void SetPassword(const UnicodeString & Value);
  virtual UnicodeString GetPassword() const;
  virtual void SetPasswordless(bool Value);
  virtual void SetPingInterval(int Value);
  virtual void SetTryAgent(bool Value);
  virtual void SetAgentFwd(bool Value);
  virtual void SetAuthTIS(bool Value);
  virtual void SetAuthKI(bool Value);
  virtual void SetAuthKIPassword(bool Value);
  virtual void SetAuthGSSAPI(bool Value);
  virtual void SetGSSAPIFwdTGT(bool Value);
  virtual void SetGSSAPIServerRealm(const UnicodeString & Value);
  virtual void SetChangeUsername(bool Value);
  virtual void SetCompression(bool Value);
  virtual void SetSshProt(TSshProt Value);
  virtual void SetSsh2DES(bool Value);
  virtual void SetSshNoUserAuth(bool Value);
  virtual void SetCipher(int Index, TCipher Value);
  virtual TCipher GetCipher(int Index) const;
  virtual void SetKex(int Index, TKex Value);
  virtual TKex GetKex(int Index) const;
  virtual void SetPublicKeyFile(const UnicodeString & Value);

  virtual void SetProtocolStr(const UnicodeString & Value);
  virtual UnicodeString GetProtocolStr() const;
  virtual bool GetCanLogin();
  virtual void SetPingIntervalDT(TDateTime Value);
  virtual TDateTime GetPingIntervalDT() const;
  virtual TDateTime GetFtpPingIntervalDT();
  virtual void SetTimeDifference(TDateTime Value);
  virtual void SetPingType(TPingType Value);
  virtual UnicodeString GetSessionName();
  virtual bool HasSessionName();
  virtual UnicodeString GetDefaultSessionName();
  virtual void SetProtocol(TProtocol Value);
  virtual void SetFSProtocol(TFSProtocol Value);
  virtual UnicodeString GetFSProtocolStr() const;
  virtual void SetLocalDirectory(const UnicodeString & Value);
  virtual void SetRemoteDirectory(const UnicodeString & Value);
  virtual void SetSynchronizeBrowsing(bool Value);
  virtual void SetUpdateDirectories(bool Value);
  virtual void SetCacheDirectories(bool Value);
  virtual void SetCacheDirectoryChanges(bool Value);
  virtual void SetPreserveDirectoryChanges(bool Value);
  virtual void SetLockInHome(bool Value);
  virtual void SetSpecial(bool Value);
  virtual UnicodeString GetInfoTip();
  virtual bool GetDefaultShell();
  virtual void SetDetectReturnVar(bool Value);
  virtual bool GetDetectReturnVar();
  virtual void SetListingCommand(const UnicodeString & Value);
  virtual void SetClearAliases(bool Value);
  virtual void SetDefaultShell(bool Value);
  virtual void SetEOLType(TEOLType Value);
  virtual void SetLookupUserGroups(TAutoSwitch Value);
  virtual void SetReturnVar(const UnicodeString & Value);
  virtual void SetScp1Compatibility(bool Value);
  virtual void SetShell(const UnicodeString & Value);
  virtual void SetSftpServer(const UnicodeString & Value);
  virtual void SetTimeout(int Value);
  virtual void SetUnsetNationalVars(bool Value);
  virtual void SetIgnoreLsWarnings(bool Value);
  virtual void SetTcpNoDelay(bool Value);
  virtual void SetSendBuf(int Value);
  virtual void SetSshSimple(bool Value);
  virtual UnicodeString GetSshProtStr();
  virtual bool GetUsesSsh();
  virtual void SetCipherList(const UnicodeString & Value);
  virtual UnicodeString GetCipherList() const;
  virtual void SetKexList(const UnicodeString & Value);
  virtual UnicodeString GetKexList() const;
  virtual void SetProxyMethod(TProxyMethod Value);
  virtual void SetProxyHost(const UnicodeString & Value);
  virtual void SetProxyPort(int Value);
  virtual void SetProxyUsername(const UnicodeString & Value);
  virtual void SetProxyPassword(const UnicodeString & Value);
  virtual void SetProxyTelnetCommand(const UnicodeString & Value);
  virtual void SetProxyLocalCommand(const UnicodeString & Value);
  virtual void SetProxyDNS(TAutoSwitch Value);
  virtual void SetProxyLocalhost(bool Value);
  virtual UnicodeString GetProxyPassword() const;
  virtual void SetFtpProxyLogonType(int Value);
  virtual void SetBug(TSshBug Bug, TAutoSwitch Value);
  virtual TAutoSwitch GetBug(TSshBug Bug) const;
  virtual UnicodeString GetSessionKey();
  virtual void SetCustomParam1(const UnicodeString & Value);
  virtual void SetCustomParam2(const UnicodeString & Value);
  virtual void SetResolveSymlinks(bool Value);
  virtual void SetSFTPDownloadQueue(int Value);
  virtual void SetSFTPUploadQueue(int Value);
  virtual void SetSFTPListingQueue(int Value);
  virtual void SetSFTPMaxVersion(int Value);
  virtual void SetSFTPMinPacketSize(unsigned long Value);
  virtual void SetSFTPMaxPacketSize(unsigned long Value);
  virtual void SetSFTPBug(TSftpBug Bug, TAutoSwitch Value);
  virtual TAutoSwitch GetSFTPBug(TSftpBug Bug) const;
  virtual void SetSCPLsFullTime(TAutoSwitch Value);
  virtual void SetFtpListAll(TAutoSwitch Value);
  virtual void SetSslSessionReuse(bool Value);
  virtual UnicodeString GetStorageKey();
  virtual UnicodeString GetInternalStorageKey();
  virtual void SetDSTMode(TDSTMode Value);
  virtual void SetDeleteToRecycleBin(bool Value);
  virtual void SetOverwrittenToRecycleBin(bool Value);
  virtual void SetRecycleBinPath(const UnicodeString & Value);
  virtual void SetPostLoginCommands(const UnicodeString & Value);
  virtual void SetAddressFamily(TAddressFamily Value);
  virtual void SetRekeyData(const UnicodeString & Value);
  virtual void SetRekeyTime(unsigned int Value);
  virtual void SetColor(int Value);
  virtual void SetTunnel(bool Value);
  virtual void SetTunnelHostName(const UnicodeString & Value);
  virtual void SetTunnelPortNumber(int Value);
  virtual void SetTunnelUserName(const UnicodeString & Value);
  virtual void SetTunnelPassword(const UnicodeString & Value);
  virtual UnicodeString GetTunnelPassword() const;
  virtual void SetTunnelPublicKeyFile(const UnicodeString & Value);
  virtual void SetTunnelPortFwd(const UnicodeString & Value);
  virtual void SetTunnelLocalPortNumber(int Value);
  virtual bool GetTunnelAutoassignLocalPortNumber();
  virtual void SetTunnelHostKey(const UnicodeString & Value);
  virtual void SetFtpPasvMode(bool Value);
  virtual void SetFtpForcePasvIp(TAutoSwitch Value);
  virtual void SetFtpUseMlsd(TAutoSwitch Value);
  virtual void SetFtpAccount(const UnicodeString & Value);
  virtual void SetFtpPingInterval(int Value);
  virtual void SetFtpPingType(TPingType Value);
  virtual void SetFtps(TFtps Value);
  virtual void SetNotUtf(TAutoSwitch Value);
  virtual void SetHostKey(const UnicodeString & Value);
  virtual TDateTime GetTimeoutDT();
  virtual void SavePasswords(THierarchicalStorage * Storage, bool PuttyExport);
  virtual UnicodeString GetLocalName();
  virtual void Modify();
  virtual UnicodeString GetSource();
  virtual void DoLoad(THierarchicalStorage * Storage, bool & RewritePassword);

  virtual void Default();
  virtual void NonPersistant();
  virtual void Load(THierarchicalStorage * Storage);
  virtual void Save(THierarchicalStorage * Storage, bool PuttyExport,
    const TSessionDataIntf * Default = NULL);
  virtual void SaveRecryptedPasswords(THierarchicalStorage * Storage);
  virtual void RecryptPasswords();
  virtual bool HasAnyPassword();
  virtual void Remove();
  virtual void Assign(TSessionDataIntf * Source);

  virtual bool ParseUrl(const UnicodeString & Url, TOptions * Options,
    TStoredSessionList * StoredSessions, bool & DefaultsOnly,
    UnicodeString * FileName, bool * AProtocolDefined, UnicodeString * MaskedUrl);
  virtual bool ParseOptions(TOptions * Options);
  virtual void ConfigureTunnel(int PortNumber);
  virtual void RollbackTunnel();
  virtual void ExpandEnvironmentVariables();
  virtual bool IsSame(const TSessionDataIntf * Default, bool AdvancedOnly);
  // static void ValidatePath(const UnicodeString & Path);
  // static void ValidateName(const UnicodeString & Name);
  virtual UnicodeString GetHostName() const { return FHostName; }
  virtual int GetPortNumber() const { return FPortNumber; }
  virtual TLoginType GetLoginType() const;
  virtual void SetLoginType(TLoginType Value);
  virtual UnicodeString GetUserName() const { return FUserName; }
  virtual int GetPingInterval() const { return FPingInterval; }
  virtual bool GetTryAgent() const { return FTryAgent; }
  virtual bool GetAgentFwd() const { return FAgentFwd; }
  virtual const UnicodeString GetListingCommand() const { return FListingCommand; }
  virtual bool GetAuthTIS() const { return FAuthTIS; }
  virtual bool GetAuthKI() const { return FAuthKI; }
  virtual bool GetAuthKIPassword() const { return FAuthKIPassword; }
  virtual bool GetAuthGSSAPI() const { return FAuthGSSAPI; }
  virtual bool GetGSSAPIFwdTGT() const { return FGSSAPIFwdTGT; }
  virtual const UnicodeString GetGSSAPIServerRealm() const { return FGSSAPIServerRealm; }
  virtual bool GetChangeUsername() const { return FChangeUsername; }
  virtual bool GetCompression() const { return FCompression; }
  virtual TSshProt GetSshProt() const { return FSshProt; }
  virtual bool GetSsh2DES() const { return FSsh2DES; }
  virtual bool GetSshNoUserAuth() const { return FSshNoUserAuth; }
  virtual const UnicodeString GetPublicKeyFile() const { return FPublicKeyFile; }
  virtual TProtocol GetProtocol() const { return FProtocol; }
  virtual TFSProtocol GetFSProtocol() const { return FFSProtocol; }
  virtual bool GetModified() const { return FModified; }
  virtual void SetModified(bool Value) { FModified = Value; }
  virtual bool GetClearAliases() const { return FClearAliases; }
  virtual TDateTime GetTimeDifference() const { return FTimeDifference; }
  virtual TPingType GetPingType() const { return FPingType; }
  virtual UnicodeString GetLocalDirectory() const { return FLocalDirectory; }
  virtual UnicodeString GetRemoteDirectory() const { return FRemoteDirectory; }
  virtual bool GetSynchronizeBrowsing() const { return FSynchronizeBrowsing; }
  virtual bool GetUpdateDirectories() const { return FUpdateDirectories; }
  virtual bool GetCacheDirectories() const { return FCacheDirectories; }
  virtual bool GetCacheDirectoryChanges() const { return FCacheDirectoryChanges; }
  virtual bool GetPreserveDirectoryChanges() const { return FPreserveDirectoryChanges; }
  virtual bool GetLockInHome() const { return FLockInHome; }
  virtual bool GetSpecial() const { return FSpecial; }
  virtual bool GetSelected() const { return FSelected; }
  virtual void SetSelected(bool Value) { FSelected = Value; }
  virtual TEOLType GetEOLType() const { return FEOLType; }
  virtual TAutoSwitch GetLookupUserGroups() const { return FLookupUserGroups; }
  virtual UnicodeString GetReturnVar() const { return FReturnVar; }
  virtual bool GetScp1Compatibility() const { return FScp1Compatibility; }
  virtual UnicodeString GetShell() const { return FShell; }
  virtual UnicodeString GetSftpServer() const { return FSftpServer; }
  virtual int GetTimeout() const { return FTimeout; }
  virtual bool GetUnsetNationalVars() const { return FUnsetNationalVars; }
  virtual bool GetIgnoreLsWarnings() const { return FIgnoreLsWarnings; }
  virtual bool GetTcpNoDelay() const { return FTcpNoDelay; }
  virtual int GetSendBuf() const { return FSendBuf; }
  virtual bool GetSshSimple() const { return FSshSimple; }
  virtual TProxyMethod GetProxyMethod() const { return FProxyMethod; }
  virtual TProxyMethod GetActualProxyMethod() const
  {
    return GetProxyMethod() == pmSystem ? GetSystemProxyMethod() : GetProxyMethod();
  }
  virtual UnicodeString GetProxyHost() const;
  virtual int GetProxyPort() const;
  virtual UnicodeString GetProxyUsername() const;
  virtual UnicodeString GetProxyTelnetCommand() const { return FProxyTelnetCommand; }
  virtual UnicodeString GetProxyLocalCommand() const { return FProxyLocalCommand; }
  virtual TAutoSwitch GetProxyDNS() const { return FProxyDNS; }
  virtual bool GetProxyLocalhost() const { return FProxyLocalhost; }
  virtual int GetFtpProxyLogonType() const { return FFtpProxyLogonType; }
  virtual UnicodeString GetCustomParam1() const { return FCustomParam1; }
  virtual UnicodeString GetCustomParam2() const { return FCustomParam2; }
  virtual bool GetResolveSymlinks() const { return FResolveSymlinks; }
  virtual int GetSFTPDownloadQueue() const { return FSFTPDownloadQueue; }
  virtual int GetSFTPUploadQueue() const { return FSFTPUploadQueue; }
  virtual int GetSFTPListingQueue() const { return FSFTPListingQueue; }
  virtual int GetSFTPMaxVersion() const { return FSFTPMaxVersion; }
  virtual unsigned long GetSFTPMinPacketSize() const { return FSFTPMinPacketSize; }
  virtual unsigned long GetSFTPMaxPacketSize() const { return FSFTPMaxPacketSize; }
  virtual TAutoSwitch GetSCPLsFullTime() const { return FSCPLsFullTime; }
  virtual TAutoSwitch GetFtpListAll() const { return FFtpListAll; }
  virtual bool GetSslSessionReuse() const { return FSslSessionReuse; }
  virtual TDSTMode GetDSTMode() const { return FDSTMode; }
  virtual bool GetDeleteToRecycleBin() const { return FDeleteToRecycleBin; }
  virtual bool GetOverwrittenToRecycleBin() const { return FOverwrittenToRecycleBin; }
  virtual UnicodeString GetRecycleBinPath() const { return FRecycleBinPath; }
  virtual UnicodeString GetPostLoginCommands() const { return FPostLoginCommands; }
  virtual TAddressFamily GetAddressFamily() const { return FAddressFamily; }
  virtual UnicodeString GetCodePage() const { return FCodePage; }
  virtual void SetCodePage(const UnicodeString & Value);
  virtual unsigned int GetCodePageAsNumber() const;
  virtual UnicodeString GetRekeyData() const { return FRekeyData; }
  virtual unsigned int GetRekeyTime() const { return FRekeyTime; }
  virtual int GetColor() const { return FColor; }
  virtual bool GetTunnel() const { return FTunnel; }
  virtual UnicodeString GetTunnelHostName() const { return FTunnelHostName; }
  virtual int GetTunnelPortNumber() const { return FTunnelPortNumber; }
  virtual UnicodeString GetTunnelUserName() const { return FTunnelUserName; }
  virtual UnicodeString GetTunnelPublicKeyFile() const { return FTunnelPublicKeyFile; }
  virtual int GetTunnelLocalPortNumber() const { return FTunnelLocalPortNumber; }
  virtual UnicodeString GetTunnelPortFwd() const { return FTunnelPortFwd; }
  virtual UnicodeString GetTunnelHostKey() const { return FTunnelHostKey; }
  virtual bool GetFtpPasvMode() const { return FFtpPasvMode; }
  virtual bool GetFtpAllowEmptyPassword() const { return FFtpAllowEmptyPassword; }
  virtual void SetFtpAllowEmptyPassword(bool Value);
  virtual TAutoSwitch GetFtpForcePasvIp() const { return FFtpForcePasvIp; }
  virtual TAutoSwitch GetFtpUseMlsd() const { return FFtpUseMlsd; }
  virtual UnicodeString GetFtpAccount() const { return FFtpAccount; }
  virtual int GetFtpPingInterval() const { return FFtpPingInterval; }
  virtual TPingType GetFtpPingType() const { return FFtpPingType; }
  virtual TFtps GetFtps() const { return FFtps; }
  virtual TAutoSwitch GetNotUtf() const { return FNotUtf; }
  virtual UnicodeString GetHostKey() const { return FHostKey; }
  virtual UnicodeString GetOrigHostName() const { return FOrigHostName; }
  virtual int GetOrigPortNumber() const { return FOrigPortNumber; }

  virtual int GetNumberOfRetries() const { return FNumberOfRetries; }
  virtual void SetNumberOfRetries(int Value) { FNumberOfRetries = Value; }
  virtual uintptr_t GetSessionVersion() const { return FSessionVersion; }
  virtual void SetSessionVersion(uintptr_t Value) { FSessionVersion = Value; }

private:
  UnicodeString FHostName;
  int FPortNumber;
  UnicodeString FUserName;
  RawByteString FPassword;
  int FPingInterval;
  TPingType FPingType;
  bool FTryAgent;
  bool FAgentFwd;
  UnicodeString FListingCommand;
  bool FAuthTIS;
  bool FAuthKI;
  bool FAuthKIPassword;
  bool FAuthGSSAPI;
  bool FGSSAPIFwdTGT; // not supported anymore
  UnicodeString FGSSAPIServerRealm; // not supported anymore
  bool FChangeUsername;
  bool FCompression;
  TSshProt FSshProt;
  bool FSsh2DES;
  bool FSshNoUserAuth;
  TCipher FCiphers[CIPHER_COUNT];
  TKex FKex[KEX_COUNT];
  bool FClearAliases;
  TEOLType FEOLType;
  UnicodeString FPublicKeyFile;
  TProtocol FProtocol;
  TFSProtocol FFSProtocol;
  bool FModified;
  UnicodeString FLocalDirectory;
  UnicodeString FRemoteDirectory;
  bool FLockInHome;
  bool FSpecial;
  bool FSynchronizeBrowsing;
  bool FUpdateDirectories;
  bool FCacheDirectories;
  bool FCacheDirectoryChanges;
  bool FPreserveDirectoryChanges;
  bool FSelected;
  TAutoSwitch FLookupUserGroups;
  UnicodeString FReturnVar;
  bool FScp1Compatibility;
  UnicodeString FShell;
  UnicodeString FSftpServer;
  int FTimeout;
  bool FUnsetNationalVars;
  bool FIgnoreLsWarnings;
  bool FTcpNoDelay;
  int FSendBuf;
  bool FSshSimple;
  TProxyMethod FProxyMethod;
  UnicodeString FProxyHost;
  int FProxyPort;
  UnicodeString FProxyUsername;
  RawByteString FProxyPassword;
  UnicodeString FProxyTelnetCommand;
  UnicodeString FProxyLocalCommand;
  TAutoSwitch FProxyDNS;
  bool FProxyLocalhost;
  int FFtpProxyLogonType;
  TAutoSwitch FBugs[BUG_COUNT];
  UnicodeString FCustomParam1;
  UnicodeString FCustomParam2;
  bool FResolveSymlinks;
  TDateTime FTimeDifference;
  int FSFTPDownloadQueue;
  int FSFTPUploadQueue;
  int FSFTPListingQueue;
  int FSFTPMaxVersion;
  unsigned long FSFTPMinPacketSize;
  unsigned long FSFTPMaxPacketSize;
  TDSTMode FDSTMode;
  TAutoSwitch FSFTPBugs[SFTP_BUG_COUNT];
  bool FDeleteToRecycleBin;
  bool FOverwrittenToRecycleBin;
  UnicodeString FRecycleBinPath;
  UnicodeString FPostLoginCommands;
  TAutoSwitch FSCPLsFullTime;
  TAutoSwitch FFtpListAll;
  bool FSslSessionReuse;
  TAddressFamily FAddressFamily;
  UnicodeString FRekeyData;
  unsigned int FRekeyTime;
  int FColor;
  bool FTunnel;
  UnicodeString FTunnelHostName;
  int FTunnelPortNumber;
  UnicodeString FTunnelUserName;
  RawByteString FTunnelPassword;
  UnicodeString FTunnelPublicKeyFile;
  int FTunnelLocalPortNumber;
  UnicodeString FTunnelPortFwd;
  UnicodeString FTunnelHostKey;
  bool FFtpPasvMode;
  TAutoSwitch FFtpForcePasvIp;
  TAutoSwitch FFtpUseMlsd;
  UnicodeString FFtpAccount;
  int FFtpPingInterval;
  TPingType FFtpPingType;
  TFtps FFtps;
  TAutoSwitch FNotUtf;
  UnicodeString FHostKey;

  UnicodeString FOrigHostName;
  int FOrigPortNumber;
  TProxyMethod FOrigProxyMethod;
  TSessionSource FSource;
  UnicodeString FCodePage;
  bool FFtpAllowEmptyPassword;
  TLoginType FLoginType;
  int FNumberOfRetries;
  uintptr_t FSessionVersion;

public:
  // void SetHostName(const UnicodeString & Value);
  // UnicodeString GetHostNameExpanded();
  // void SetPortNumber(int Value);
  // void SetUserName(const UnicodeString & Value);
  // UnicodeString GetUserNameExpanded();
  // void SetPassword(const UnicodeString & Value);
  // UnicodeString GetPassword() const;
  // void SetPasswordless(bool Value);
  // void SetPingInterval(int Value);
  // void SetTryAgent(bool Value);
  // void SetAgentFwd(bool Value);
  // void SetAuthTIS(bool Value);
  // void SetAuthKI(bool Value);
  // void SetAuthKIPassword(bool Value);
  // void SetAuthGSSAPI(bool Value);
  // void SetGSSAPIFwdTGT(bool Value);
  // void SetGSSAPIServerRealm(const UnicodeString & Value);
  // void SetChangeUsername(bool Value);
  // void SetCompression(bool Value);
  // void SetSshProt(TSshProt Value);
  // void SetSsh2DES(bool Value);
  // void SetSshNoUserAuth(bool Value);
  // void SetCipher(int Index, TCipher Value);
  // TCipher GetCipher(int Index) const;
  // void SetKex(int Index, TKex Value);
  // TKex GetKex(int Index) const;
  // void SetPublicKeyFile(const UnicodeString & Value);

  // void SetProtocolStr(const UnicodeString & Value);
  // UnicodeString GetProtocolStr() const;
  // bool GetCanLogin();
  // void SetPingIntervalDT(TDateTime Value);
  // TDateTime GetPingIntervalDT() const;
  // TDateTime GetFtpPingIntervalDT();
  // void SetTimeDifference(TDateTime Value);
  // void SetPingType(TPingType Value);
  // UnicodeString GetSessionName();
  // bool HasSessionName();
  // UnicodeString GetDefaultSessionName();
  // void SetProtocol(TProtocol Value);
  // void SetFSProtocol(TFSProtocol Value);
  // UnicodeString GetFSProtocolStr() const;
  // void SetLocalDirectory(const UnicodeString & Value);
  // void SetRemoteDirectory(const UnicodeString & Value);
  // void SetSynchronizeBrowsing(bool Value);
  // void SetUpdateDirectories(bool Value);
  // void SetCacheDirectories(bool Value);
  // void SetCacheDirectoryChanges(bool Value);
  // void SetPreserveDirectoryChanges(bool Value);
  // void SetLockInHome(bool Value);
  // void SetSpecial(bool Value);
  // UnicodeString GetInfoTip();
  // bool GetDefaultShell();
  // void SetDetectReturnVar(bool Value);
  // bool GetDetectReturnVar() const;
  // void SetListingCommand(const UnicodeString & Value);
  // void SetClearAliases(bool Value);
  // void SetDefaultShell(bool Value);
  // void SetEOLType(TEOLType Value);
  // void SetLookupUserGroups(TAutoSwitch Value);
  // void SetReturnVar(const UnicodeString & Value);
  // void SetScp1Compatibility(bool Value);
  // void SetShell(const UnicodeString & Value);
  // void SetSftpServer(const UnicodeString & Value);
  // void SetTimeout(int Value);
  // void SetUnsetNationalVars(bool Value);
  // void SetIgnoreLsWarnings(bool Value);
  // void SetTcpNoDelay(bool Value);
  // void SetSendBuf(int Value);
  // void SetSshSimple(bool Value);
  // UnicodeString GetSshProtStr();
  // bool GetUsesSsh();
  // void SetCipherList(const UnicodeString & Value);
  // UnicodeString GetCipherList() const;
  // void SetKexList(const UnicodeString & Value);
  // UnicodeString GetKexList() const;
  // void SetProxyMethod(TProxyMethod Value);
  // void SetProxyHost(const UnicodeString & Value);
  // void SetProxyPort(int Value);
  // void SetProxyUsername(const UnicodeString & Value);
  // void SetProxyPassword(const UnicodeString & Value);
  // void SetProxyTelnetCommand(const UnicodeString & Value);
  // void SetProxyLocalCommand(const UnicodeString & Value);
  // void SetProxyDNS(TAutoSwitch Value);
  // void SetProxyLocalhost(bool Value);
  // UnicodeString GetProxyPassword() const;
  // void SetFtpProxyLogonType(int Value);
  // void SetBug(TSshBug Bug, TAutoSwitch Value);
  // TAutoSwitch GetBug(TSshBug Bug) const;
  // UnicodeString GetSessionKey();
  // void SetCustomParam1(const UnicodeString & Value);
  // void SetCustomParam2(const UnicodeString & Value);
  // void SetResolveSymlinks(bool Value);
  // void SetSFTPDownloadQueue(int Value);
  // void SetSFTPUploadQueue(int Value);
  // void SetSFTPListingQueue(int Value);
  // void SetSFTPMaxVersion(int Value);
  // void SetSFTPMinPacketSize(unsigned long Value);
  // void SetSFTPMaxPacketSize(unsigned long Value);
  // void SetSFTPBug(TSftpBug Bug, TAutoSwitch Value);
  // TAutoSwitch GetSFTPBug(TSftpBug Bug) const;
  // void SetSCPLsFullTime(TAutoSwitch Value);
  // void SetFtpListAll(TAutoSwitch Value);
  // void SetSslSessionReuse(bool Value);
  // UnicodeString GetStorageKey();
  // UnicodeString GetInternalStorageKey();
  // void SetDSTMode(TDSTMode Value);
  // void SetDeleteToRecycleBin(bool Value);
  // void SetOverwrittenToRecycleBin(bool Value);
  // void SetRecycleBinPath(const UnicodeString & Value);
  // void SetPostLoginCommands(const UnicodeString & Value);
  // void SetAddressFamily(TAddressFamily Value);
  // void SetRekeyData(const UnicodeString & Value);
  // void SetRekeyTime(unsigned int Value);
  // void SetColor(int Value);
  // void SetTunnel(bool Value);
  // void SetTunnelHostName(const UnicodeString & Value);
  // void SetTunnelPortNumber(int Value);
  // void SetTunnelUserName(const UnicodeString & Value);
  // void SetTunnelPassword(const UnicodeString & Value);
  // UnicodeString GetTunnelPassword() const;
  // void SetTunnelPublicKeyFile(const UnicodeString & Value);
  // void SetTunnelPortFwd(const UnicodeString & Value);
  // void SetTunnelLocalPortNumber(int Value);
  // bool GetTunnelAutoassignLocalPortNumber();
  // void SetTunnelHostKey(const UnicodeString & Value);
  // void SetFtpPasvMode(bool Value);
  // void SetFtpForcePasvIp(TAutoSwitch Value);
  // void SetFtpUseMlsd(TAutoSwitch Value);
  // void SetFtpAccount(const UnicodeString & Value);
  // void SetFtpPingInterval(int Value);
  // void SetFtpPingType(TPingType Value);
  // void SetFtps(TFtps Value);
  // void SetNotUtf(TAutoSwitch Value);
  // void SetHostKey(const UnicodeString & Value);
  // TDateTime GetTimeoutDT();
  // void SavePasswords(THierarchicalStorage * Storage, bool PuttyExport);
  // UnicodeString GetLocalName();
  // void Modify();
  // UnicodeString GetSource();
  // void DoLoad(THierarchicalStorage * Storage, bool & RewritePassword);
  static RawByteString EncryptPassword(const UnicodeString & Password, UnicodeString Key);
  static UnicodeString DecryptPassword(const RawByteString & Password, UnicodeString Key);
  static RawByteString StronglyRecryptPassword(const RawByteString & Password, UnicodeString Key);

public:
  explicit TSessionData(const UnicodeString & AName);
  virtual ~TSessionData();
  // void Default();
  // void NonPersistant();
  // void Load(THierarchicalStorage * Storage);
  // void Save(THierarchicalStorage * Storage, bool PuttyExport,
    // const TSessionData * Default = NULL);
  // void SaveRecryptedPasswords(THierarchicalStorage * Storage);
  // void RecryptPasswords();
  // bool HasAnyPassword();
  // void Remove();
  // virtual void Assign(TPersistent * Source);

  // bool ParseUrl(const UnicodeString & Url, TOptions * Options,
    // TStoredSessionList * StoredSessions, bool & DefaultsOnly,
    // UnicodeString * FileName, bool * AProtocolDefined, UnicodeString * MaskedUrl);
  // bool ParseOptions(TOptions * Options);
  // void ConfigureTunnel(int PortNumber);
  // void RollbackTunnel();
  // void ExpandEnvironmentVariables();
  // bool IsSame(const TSessionData * Default, bool AdvancedOnly);
  static void ValidatePath(const UnicodeString & Path);
  static void ValidateName(const UnicodeString & Name);
  // UnicodeString GetHostName() const { return FHostName; }
  // int GetPortNumber() const { return FPortNumber; }
  // TLoginType GetLoginType() const;
  // void SetLoginType(TLoginType Value);
  // UnicodeString GetUserName() const { return FUserName; }
  // int GetPingInterval() const { return FPingInterval; }
  // bool GetTryAgent() const { return FTryAgent; }
  // bool GetAgentFwd() const { return FAgentFwd; }
  // const UnicodeString GetListingCommand() const { return FListingCommand; }
  // bool GetAuthTIS() const { return FAuthTIS; }
  // bool GetAuthKI() const { return FAuthKI; }
  // bool GetAuthKIPassword() const { return FAuthKIPassword; }
  // bool GetAuthGSSAPI() const { return FAuthGSSAPI; }
  // bool GetGSSAPIFwdTGT() const { return FGSSAPIFwdTGT; }
  // const UnicodeString GetGSSAPIServerRealm() const { return FGSSAPIServerRealm; }
  // bool GetChangeUsername() const { return FChangeUsername; }
  // bool GetCompression() const { return FCompression; }
  // TSshProt GetSshProt() const { return FSshProt; }
  // bool GetSsh2DES() const { return FSsh2DES; }
  // bool GetSshNoUserAuth() const { return FSshNoUserAuth; }
  // const UnicodeString GetPublicKeyFile() const { return FPublicKeyFile; }
  // TProtocol GetProtocol() const { return FProtocol; }
  // TFSProtocol GetFSProtocol() const { return FFSProtocol; }
  // bool GetModified() const { return FModified; }
  // void SetModified(bool Value) { FModified = Value; }
  // bool GetClearAliases() const { return FClearAliases; }
  // TDateTime GetTimeDifference() const { return FTimeDifference; }
  // TPingType GetPingType() const { return FPingType; }
  // UnicodeString GetLocalDirectory() const { return FLocalDirectory; }
  // UnicodeString GetRemoteDirectory() const { return FRemoteDirectory; }
  // bool GetSynchronizeBrowsing() const { return FSynchronizeBrowsing; }
  // bool GetUpdateDirectories() const { return FUpdateDirectories; }
  // bool GetCacheDirectories() const { return FCacheDirectories; }
  // bool GetCacheDirectoryChanges() const { return FCacheDirectoryChanges; }
  // bool GetPreserveDirectoryChanges() const { return FPreserveDirectoryChanges; }
  // bool GetLockInHome() const { return FLockInHome; }
  // bool GetSpecial() const { return FSpecial; }
  // bool GetSelected() const { return FSelected; }
  // void SetSelected(bool Value) { FSelected = Value; }
  // TEOLType GetEOLType() const { return FEOLType; }
  // TAutoSwitch GetLookupUserGroups() const { return FLookupUserGroups; }
  // UnicodeString GetReturnVar() const { return FReturnVar; }
  // bool GetScp1Compatibility() const { return FScp1Compatibility; }
  // UnicodeString GetShell() const { return FShell; }
  // UnicodeString GetSftpServer() const { return FSftpServer; }
  // int GetTimeout() const { return FTimeout; }
  // bool GetUnsetNationalVars() const { return FUnsetNationalVars; }
  // bool GetIgnoreLsWarnings() const { return FIgnoreLsWarnings; }
  // bool GetTcpNoDelay() const { return FTcpNoDelay; }
  // int GetSendBuf() const { return FSendBuf; }
  // bool GetSshSimple() const { return FSshSimple; }
  // TProxyMethod GetProxyMethod() const { return FProxyMethod; }
  // TProxyMethod GetActualProxyMethod() const
  // {
    // return GetProxyMethod() == pmSystem ? GetSystemProxyMethod() : GetProxyMethod();
  // }
  // UnicodeString GetProxyHost() const;
  // int GetProxyPort() const;
  // UnicodeString GetProxyUsername() const;
  // UnicodeString GetProxyTelnetCommand() const { return FProxyTelnetCommand; }
  // UnicodeString GetProxyLocalCommand() const { return FProxyLocalCommand; }
  // TAutoSwitch GetProxyDNS() const { return FProxyDNS; }
  // bool GetProxyLocalhost() const { return FProxyLocalhost; }
  // int GetFtpProxyLogonType() const { return FFtpProxyLogonType; }
  // UnicodeString GetCustomParam1() const { return FCustomParam1; }
  // UnicodeString GetCustomParam2() const { return FCustomParam2; }
  // bool GetResolveSymlinks() const { return FResolveSymlinks; }
  // int GetSFTPDownloadQueue() const { return FSFTPDownloadQueue; }
  // int GetSFTPUploadQueue() const { return FSFTPUploadQueue; }
  // int GetSFTPListingQueue() const { return FSFTPListingQueue; }
  // int GetSFTPMaxVersion() const { return FSFTPMaxVersion; }
  // unsigned long GetSFTPMinPacketSize() const { return FSFTPMinPacketSize; }
  // unsigned long GetSFTPMaxPacketSize() const { return FSFTPMaxPacketSize; }
  // TAutoSwitch GetSCPLsFullTime() const { return FSCPLsFullTime; }
  // TAutoSwitch GetFtpListAll() const { return FFtpListAll; }
  // bool GetSslSessionReuse() const { return FSslSessionReuse; }
  // TDSTMode GetDSTMode() const { return FDSTMode; }
  // bool GetDeleteToRecycleBin() const { return FDeleteToRecycleBin; }
  // bool GetOverwrittenToRecycleBin() const { return FOverwrittenToRecycleBin; }
  // UnicodeString GetRecycleBinPath() const { return FRecycleBinPath; }
  // UnicodeString GetPostLoginCommands() const { return FPostLoginCommands; }
  // TAddressFamily GetAddressFamily() const { return FAddressFamily; }
  // UnicodeString GetCodePage() const { return FCodePage; }
  // void SetCodePage(const UnicodeString & Value);
  // unsigned int GetCodePageAsNumber() const;
  // UnicodeString GetRekeyData() const { return FRekeyData; }
  // unsigned int GetRekeyTime() const { return FRekeyTime; }
  // int GetColor() const { return FColor; }
  // bool GetTunnel() const { return FTunnel; }
  // UnicodeString GetTunnelHostName() const { return FTunnelHostName; }
  // int GetTunnelPortNumber() const { return FTunnelPortNumber; }
  // UnicodeString GetTunnelUserName() const { return FTunnelUserName; }
  // UnicodeString GetTunnelPublicKeyFile() const { return FTunnelPublicKeyFile; }
  // int GetTunnelLocalPortNumber() const { return FTunnelLocalPortNumber; }
  // UnicodeString GetTunnelPortFwd() const { return FTunnelPortFwd; }
  // UnicodeString GetTunnelHostKey() const { return FTunnelHostKey; }
  // bool GetFtpPasvMode() const { return FFtpPasvMode; }
  // bool GetFtpAllowEmptyPassword() const { return FFtpAllowEmptyPassword; }
  // void SetFtpAllowEmptyPassword(bool Value);
  // TAutoSwitch GetFtpForcePasvIp() const { return FFtpForcePasvIp; }
  // TAutoSwitch GetFtpUseMlsd() const { return FFtpUseMlsd; }
  // UnicodeString GetFtpAccount() const { return FFtpAccount; }
  // int GetFtpPingInterval() const { return FFtpPingInterval; }
  // TPingType GetFtpPingType() const { return FFtpPingType; }
  // TFtps GetFtps() const { return FFtps; }
  // TAutoSwitch GetNotUtf() const { return FNotUtf; }
  // UnicodeString GetHostKey() const { return FHostKey; }
  // UnicodeString GetOrigHostName() const { return FOrigHostName; }
  // int GetOrigPortNumber() const { return FOrigPortNumber; }

  // int GetNumberOfRetries() const { return FNumberOfRetries; }
  // void SetNumberOfRetries(int Value) { FNumberOfRetries = Value; }
  // uintptr_t GetSessionVersion() const { return FSessionVersion; }
  // void SetSessionVersion(uintptr_t Value) { FSessionVersion = Value; }

private:
  uintptr_t GetDefaultVersion() { return ::GetVersionNumber219(); }
  TFSProtocol TranslateFSProtocolNumber(int FSProtocol);
  TFSProtocol TranslateFSProtocol(const UnicodeString & ProtocolID);
  TFtps TranslateFtpEncryptionNumber(int FtpEncryption);

private:
  mutable TIEProxyConfig * FIEProxyConfig;

private:
  TProxyMethod GetSystemProxyMethod() const;
  void  PrepareProxyData() const;
  void ParseIEProxyConfig() const;
  void FromURI(const UnicodeString & ProxyURI,
    UnicodeString & ProxyUrl, int & ProxyPort, TProxyMethod & ProxyMethod) const;
  void AdjustHostName(UnicodeString & hostName, const UnicodeString & Prefix);
  void RemoveProtocolPrefix(UnicodeString & hostName);
};
//---------------------------------------------------------------------------
class TStoredSessionList : public TNamedObjectList
{
public:
  explicit TStoredSessionList(bool aReadOnly = false);
  virtual ~TStoredSessionList();
  void Load(const UnicodeString & AKey, bool UseDefaults);
  void Load();
  void Save(bool All, bool Explicit);
  void Saved();
  void Export(const UnicodeString & FileName);
  void Load(THierarchicalStorage * Storage, bool AsModified = false,
    bool UseDefaults = false);
  void Save(THierarchicalStorage * Storage, bool All = false);
  void SelectAll(bool Select);
  void Import(TStoredSessionList * From, bool OnlySelected);
  void RecryptPasswords();
  TSessionData * AtSession(int Index)
    { return static_cast<TSessionData *>(AtObject(Index)); }
  void SelectSessionsToImport(TStoredSessionList * Dest, bool SSHOnly);
  void Cleanup();
  void UpdateStaticUsage();
  int IndexOf(TSessionData * Data);
  TSessionData * FindSame(TSessionData * Data);
  TSessionData * NewSession(const UnicodeString & SessionName, TSessionDataIntf * Session);
  TSessionData * ParseUrl(const UnicodeString & Url, TOptions * Options, bool & DefaultsOnly,
    UnicodeString * FileName = NULL, bool * ProtocolDefined = NULL, UnicodeString * MaskedUrl = NULL);
  TSessionData * GetSession(intptr_t Index) { return static_cast<TSessionData *>(AtObject(Index)); }
  TSessionData * GetDefaultSettings() const { return FDefaultSettings; }
  void SetDefaultSettings(TSessionData * Value);
  TSessionData * GetSessionByName(const UnicodeString & SessionName);

  static void ImportHostKeys(const UnicodeString & TargetKey,
    const UnicodeString & SourceKey, TStoredSessionList * Sessions,
    bool OnlySelected);

private:
  TSessionData * FDefaultSettings;
  bool FReadOnly;
  void DoSave(THierarchicalStorage * Storage, bool All, bool RecryptPasswordOnly);
  void DoSave(bool All, bool Explicit, bool RecryptPasswordOnly);
  void DoSave(THierarchicalStorage * Storage,
    TSessionData * Data, bool All, bool RecryptPasswordOnly,
    TSessionData * FactoryDefaults);
};
//---------------------------------------------------------------------------
bool GetCodePageInfo(UINT CodePage, CPINFOEX & CodePageInfoEx);
unsigned int GetCodePageAsNumber(const UnicodeString & CodePage);
UnicodeString GetCodePageAsString(unsigned int cp);
//---------------------------------------------------------------------------
UnicodeString GetExpandedLogFileName(UnicodeString LogFileName, TSessionDataIntf * SessionData);
//---------------------------------------------------------------------------
#endif
