//------------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Common.h"
#include "Exceptions.h"
#include "TextsCore.h"
//------------------------------------------------------------------------------
#pragma package(smart_init)
//------------------------------------------------------------------------------
bool ExceptionMessage(const Exception * E, UnicodeString & Message)
{
  bool Result = true;
  if (dynamic_cast<const EAbort *>(E) != NULL)
  {
    Result = false;
  }
  else if (dynamic_cast<const EAccessViolation*>(E) != NULL)
  {
    Message = LoadStr(ACCESS_VIOLATION_ERROR);
  }
  else if (E && E->Message.IsEmpty())
  {
    Result = false;
  }
  else if (E)
  {
    Message = E->Message;
  }
  return Result;
}
//------------------------------------------------------------------------------
TStrings * ExceptionToMoreMessages(Exception * E)
{
  TStrings * Result = NULL;
  UnicodeString Message;
  if (ExceptionMessage(E, Message))
  {
    Result = new TStringList();
    Result->Add(Message);
    ExtException * ExtE = dynamic_cast<ExtException *>(E);
    if (ExtE != NULL)
    {
      Result->AddStrings(ExtE->GetMoreMessages());
    }
  }
  return Result;
}
//------------------------------------------------------------------------------
ExtException::ExtException(Exception * E) :
  Exception(L""),
  FMoreMessages(NULL),
  FHelpKeyword()
{
  CALLSTACK;
  // TRACEFMT("[%s]", UnicodeString((E != NULL) ? E->Message : UnicodeString(L"<null>")).c_str());
  AddMoreMessages(E);
  // TRACE("/");
}
//------------------------------------------------------------------------------
ExtException::ExtException(Exception * E, const UnicodeString & Msg):
  Exception(Msg),
  FMoreMessages(NULL),
  FHelpKeyword()
{
  AddMoreMessages(E);
}
//------------------------------------------------------------------------------
ExtException::ExtException(ExtException * E, const UnicodeString & Msg):
  Exception(Msg),
  FMoreMessages(NULL),
  FHelpKeyword()
{
  CALLSTACK;
  // TRACEFMT("[%s] [%s]", UnicodeString((E != NULL) ? E->Message : UnicodeString(L"<null>")).c_str(), Msg.c_str());
  AddMoreMessages(E);
  // TRACE("/");
}
//------------------------------------------------------------------------------
ExtException::ExtException(Exception * E, int Ident) :
  Exception(E, Ident),
  FMoreMessages(NULL),
  FHelpKeyword()
{
  CALLSTACK;
}
//------------------------------------------------------------------------------
ExtException::ExtException(const UnicodeString & Msg, Exception * E) :
  Exception(L""),
  FMoreMessages(NULL),
  FHelpKeyword()
{
  CALLSTACK;
  // TRACEFMT("[%s] [%s]", Msg.c_str(), UnicodeString((E != NULL) ? E->Message : UnicodeString(L"<null>")).c_str());
  // "copy exception"
  AddMoreMessages(E);
  // and append message to the end to more messages
  if (!Msg.IsEmpty())
  {
    if (Message.IsEmpty())
    {
      Message = Msg;
    }
    else
    {
      if (FMoreMessages == NULL)
      {
        FMoreMessages = new TStringList();
      }
      FMoreMessages->Append(Msg);
    }
  }
  // TRACE("/");
}
//------------------------------------------------------------------------------
ExtException::ExtException(const UnicodeString & Msg, const UnicodeString & MoreMessages,
    const UnicodeString & HelpKeyword) :
  Exception(Msg),
  FMoreMessages(NULL),
  FHelpKeyword(HelpKeyword)
{
  CALLSTACK;
  // TRACEFMT("[%s] [%s]", Msg.c_str(), MoreMessages.c_str());
  if (!MoreMessages.IsEmpty())
  {
    FMoreMessages = new TStringList();
    FMoreMessages->SetText(MoreMessages);
  }
  // TRACE("/");
}
//------------------------------------------------------------------------------
ExtException::ExtException(const UnicodeString & Msg, TStrings * MoreMessages,
  bool Own, const UnicodeString & HelpKeyword) :
  Exception(Msg),
  FMoreMessages(NULL),
  FHelpKeyword(HelpKeyword)
{
  CALLSTACK;
  // TRACEFMT("[%s] [%s]", Msg.c_str(), UnicodeString((MoreMessages != NULL) ? MoreMessages->Text : UnicodeString(L"<null>")).c_str());
  if (Own)
  {
    FMoreMessages = MoreMessages;
  }
  else
  {
    FMoreMessages = new TStringList();
    FMoreMessages->Assign(MoreMessages);
  }
  // TRACE("/");
}
//------------------------------------------------------------------------------
void ExtException::AddMoreMessages(const Exception * E)
{
  CALLSTACK;
  if (E != NULL)
  {
    if (FMoreMessages == NULL)
    {
      FMoreMessages = new TStringList();
    }

    const ExtException * ExtE = dynamic_cast<const ExtException *>(E);
    if (ExtE != NULL)
    {
      if (!ExtE->GetHelpKeyword().IsEmpty())
      {
        // we have to yet decide what to do now
        assert(GetHelpKeyword().IsEmpty());

        FHelpKeyword = ExtE->GetHelpKeyword();
      }

      if (ExtE->GetMoreMessages() != NULL)
      {
        // TRACEFMT("1 [%s]", ExtE->GetMoreMessages()->Text.get().c_str());
        FMoreMessages->Assign(ExtE->GetMoreMessages());
      }
    }

    UnicodeString Msg;
    ExceptionMessage(E, Msg);

    // new exception does not have own message, this is in fact duplication of
    // the exception data, but the exception class may being changed
    // TRACEFMT("2 [%s]", Msg.c_str());
    if (Message.IsEmpty())
    {
      Message = Msg;
    }
    else if (!Msg.IsEmpty())
    {
      FMoreMessages->Insert(0, Msg);
    }

    if (FMoreMessages->GetCount() == 0)
    {
      delete FMoreMessages;
      FMoreMessages = NULL;
    }
  }
  // TRACE("/");
}
//------------------------------------------------------------------------------
ExtException::~ExtException()
{
  delete FMoreMessages;
  FMoreMessages = NULL;
}
//------------------------------------------------------------------------------
ExtException * ExtException::Clone()
{
  return new ExtException(this, L"");
}
//------------------------------------------------------------------------------
UnicodeString LastSysErrorMessage()
{
  CALLSTACK;
  int LastError = GetLastError();
  UnicodeString Result;
  if (LastError != 0)
  {
    Result = FORMAT(L"System Error.  Code: %d.\r\n%s", LastError, SysErrorMessage(LastError).c_str());
  }
  return Result;
}
//------------------------------------------------------------------------------
EOSExtException::EOSExtException(const UnicodeString & Msg) :
  ExtException(Msg, LastSysErrorMessage())
{
  CALLSTACK;
}
//------------------------------------------------------------------------------
EFatal::EFatal(Exception * E, const UnicodeString & Msg) :
  ExtException(Msg, E),
  FReopenQueried(false)
{
  EFatal * F = dynamic_cast<EFatal *>(E);
  if (F != NULL)
  {
    FReopenQueried = F->GetReopenQueried();
  }
}
//------------------------------------------------------------------------------
ExtException * EFatal::Clone()
{
  return new EFatal(this, L"");
}
//------------------------------------------------------------------------------
ExtException * ESshTerminate::Clone()
{
  return new ESshTerminate(this, L"", Operation);
}
//------------------------------------------------------------------------------
ECallbackGuardAbort::ECallbackGuardAbort() : EAbort(L"callback abort")
{
}
//------------------------------------------------------------------------------
Exception * CloneException(Exception * E)
{
  CALLSTACK;
  ExtException * Ext = dynamic_cast<ExtException *>(E);
  if (Ext != NULL)
  {
    // TRACE("1");
    return Ext->Clone();
  }
  else if (dynamic_cast<ECallbackGuardAbort *>(E) != NULL)
  {
    // TRACE("2");
    return new ECallbackGuardAbort();
  }
  else if (dynamic_cast<EAbort *>(E) != NULL)
  {
    // TRACE("3");
    return new EAbort(E->Message);
  }
  else
  {
    // TRACE("4");
    return new Exception(E->Message);
  }
}
//------------------------------------------------------------------------------
void RethrowException(Exception * E)
{
  CALLSTACK;
  if (dynamic_cast<EFatal *>(E) != NULL)
  {
    // TRACE("1");
    throw EFatal(E, L"");
  }
  else if (dynamic_cast<ECallbackGuardAbort *>(E) != NULL)
  {
    // TRACE("2");
    throw ECallbackGuardAbort();
  }
  else if (dynamic_cast<EAbort *>(E) != NULL)
  {
    // TRACE("3");
    throw EAbort(E->Message);
  }
  else
  {
    // TRACE("4");
    throw ExtException(E, L"");
  }
}
