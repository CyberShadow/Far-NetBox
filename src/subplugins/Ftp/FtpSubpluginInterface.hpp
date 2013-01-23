#include <SysUtils.hpp>

namespace ftp {

//------------------------------------------------------------------------------
class TGlobalFunctions : public TGlobalFunctionsIntf
{
public:
  explicit TGlobalFunctions();
  virtual ~TGlobalFunctions();

  virtual HINSTANCE GetHandle() const;
  virtual UnicodeString GetCurrentDirectory() const;
};
//------------------------------------------------------------------------------
} // namespace ftp
//------------------------------------------------------------------------------
