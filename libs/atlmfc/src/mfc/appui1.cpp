// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"



/////////////////////////////////////////////////////////////////////////////
// CWinApp UI related functions

void CWinApp::EnableModeless(BOOL bEnable)
{
    DoEnableModeless(bEnable);
}

void CWinApp::DoEnableModeless(BOOL bEnable)
{
#ifdef _AFX_NO_OLE_SUPPORT
	UNUSED(bEnable);
#endif
	return;
}

int AFXAPI AfxMessageBox(LPCTSTR lpszText, UINT nType, UINT nIDHelp)
{
	CWinApp* pApp = AfxGetApp();
  return 0;
}

int AFXAPI AfxMessageBox(UINT nIDPrompt, UINT nType, UINT nIDHelp)
{
	CString string;
	if (!string.LoadString(nIDPrompt))
	{
		ASSERT(FALSE);
	}
	if (nIDHelp == (UINT)-1)
		nIDHelp = nIDPrompt;
	return AfxMessageBox(string, nType, nIDHelp);
}

////////////////////////////////////////////////////////////////////////////
// UI related CWnd functions

HWND PASCAL CWnd::GetSafeOwner_(HWND hParent, HWND* pWndTop)
{
	// get window to start with
	HWND hWnd = hParent;

	// a popup window cannot be owned by a child window
	while (hWnd != NULL && (::GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD))
		hWnd = ::GetParent(hWnd);

	// determine toplevel window to disable as well
	HWND hWndTop = hWnd, hWndTemp = hWnd;
	for (;;)
	{
		if (hWndTemp == NULL)
			break;
		else
			hWndTop = hWndTemp;
		hWndTemp = ::GetParent(hWndTop);
	}

	// get last active popup of first non-child that was found
	if (hParent == NULL && hWnd != NULL)
		hWnd = ::GetLastActivePopup(hWnd);

	// disable and store top level parent window if specified
	if (pWndTop != NULL)
	{
		if (hWndTop != NULL && ::IsWindowEnabled(hWndTop) && hWndTop != hWnd)
		{
			*pWndTop = hWndTop;
			::EnableWindow(hWndTop, FALSE);
		}
		else
			*pWndTop = NULL;
	}

	return hWnd;    // return the owner as HWND
}

/////////////////////////////////////////////////////////////////////////////
// UI related CCmdTarget functions

CView* PASCAL CCmdTarget::GetRoutingView_()
{
	CView* pView = AfxGetThreadState()->m_pRoutingView;
	return pView;
}

/////////////////////////////////////////////////////////////////////////////
