// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//

#include "wingui_sswindow.h"

/////////////////////////////////////////////////////////////////////////
// CMainWindow message map and member functions

BEGIN_MESSAGE_MAP(CSSWindow, CWnd)
    ON_WM_CLOSE()
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_PAINT()
    ON_WM_TIMER()
END_MESSAGE_MAP()

//////////
// CSSWindow::CSSWindow
// arguments:	void
// returns:		void
// function:	sets initial rect for window and refresh timer
CSSWindow::CSSWindow()
{
	int nX = rand() % 50;
	int nY = rand() % 50;
	m_Rect.SetRect(0+nX,0+nY,640+nX,480+nY);
	SetMode(MODE_NO_GRAPHICS, MODE_NO_GRAPHICS);

	m_hBOINCIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON));
}

// CMainWindow::SetMode
// arguments:	nMode: the new mode
// returns:		void
// function:	destroys the current window and creates a new window
//				in the new mode
void CSSWindow::SetMode(int nMode, int nPrev)
{
	RECT WindowRect = {0,0,0,0};
	if(nPrev == MODE_DEFAULT) m_nPrevMode = m_nMode;
	else m_nPrevMode = nPrev;
	m_nMode = nMode;

	if(GetSafeHwnd()) {
		if((m_nPrevMode != MODE_FULLSCREEN) && (m_nPrevMode != MODE_BLANK_SCREEN))
			GetWindowRect(&m_Rect);
		DestroyWindow();
	}

	CString strWndClass = AfxRegisterWndClass(0);
	DWORD dwExStyle;
	DWORD dwStyle;

	if (nMode == MODE_FULLSCREEN || nMode == MODE_BLANK_SCREEN) {
		HDC screenDC=::GetDC(NULL);
		WindowRect.left = WindowRect.top = 0;
		WindowRect.right=GetDeviceCaps(screenDC, HORZRES);
		WindowRect.bottom=GetDeviceCaps(screenDC, VERTRES);
		::ReleaseDC(NULL, screenDC);
		dwExStyle=WS_EX_TOPMOST;
		dwStyle=WS_POPUP;
		while(ShowCursor(false) >= 0);
	} else {
		if(m_Rect.IsRectEmpty()) m_Rect.SetRect(CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT);
		WindowRect = m_Rect;
		dwExStyle=WS_EX_APPWINDOW|WS_EX_WINDOWEDGE;
		dwStyle=WS_OVERLAPPEDWINDOW;
		while(ShowCursor(true) < 0);
	}

	CreateEx(dwExStyle, strWndClass, "DEFAULT BOINC Graphics",
		dwStyle|WS_CLIPSIBLINGS|WS_CLIPCHILDREN, WindowRect,
		NULL, 0, NULL);

	if(nMode == MODE_FULLSCREEN || nMode == MODE_WINDOW || nMode == MODE_BLANK_SCREEN) {
		ShowWindow(SW_SHOW);
		if(nMode == MODE_FULLSCREEN || nMode == MODE_BLANK_SCREEN) SetForegroundWindow();
	} else {
		ShowWindow(SW_HIDE);
	}
	SetFocus();
}

// CSSWindow::GetMode
// arguments:	void
// returns:		the current mode of the window
// function:	gets the current mode of the window
int CSSWindow::GetMode()
{
	return m_nMode;
}

// CSSWindow::GetPrevMode
// arguments:	void
// returns:		the previous mode of the window
// function:	gets the previous mode of the window
int CSSWindow::GetPrevMode()
{
	return m_nPrevMode;
}

void CSSWindow::PaintDefault()
{
	PAINTSTRUCT ps;
	CDC* pdc;
	RECT winRect;
	CBrush cb;
	cb.CreateSolidBrush(RGB(0,0,0));

	pdc = BeginPaint(&ps);
	GetClientRect(&winRect);
	pdc->FillRect(&winRect, &cb);
	EndPaint(&ps);
}

// CMainWindow::DefWindowProc
// arguments:	message: message received
//				wParam: message's wparam
//				lParam: message's lparam
// returns:		dependent on message
// function:	handles messages as a screensaver would
LRESULT CSSWindow::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	CPoint mousePos;
	switch(message) {
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			if(m_nMode == MODE_FULLSCREEN || m_nMode == MODE_BLANK_SCREEN)
				SetMode(m_nPrevMode, MODE_DEFAULT);
			return 0;
		case WM_MOUSEMOVE:
			if(m_nMode == MODE_FULLSCREEN || m_nMode == MODE_BLANK_SCREEN) {
				GetCursorPos(&mousePos);
				if(mousePos != m_MousePos) SetMode(m_nPrevMode, MODE_DEFAULT);
			}
			return 0;
	}

	if(m_uSetMsg == message) {
		SetMode(wParam, lParam);
		return 0;
	}

	if(m_uGetMsg == message) {
		return MAKELONG(GetMode(), GetPrevMode());
	}

	return CWnd::DefWindowProc(message, wParam, lParam);
}

//////////
// CSSWindow::OnClose
// arguments:	void
// returns:		void
// function:	hides the window
void CSSWindow::OnClose()
{
	SetMode(MODE_NO_GRAPHICS, MODE_NO_GRAPHICS);
}

//////////
// CSSWindow::OnCreate
// arguments:	lpcs: a pointer to the create structure
// returns:		0 if successful, otherwise -1
// function:	gets mouse position for screensaver and starts timer
int CSSWindow::OnCreate(LPCREATESTRUCT lpcs)
{
    if (CWnd::OnCreate(lpcs) == -1) {
		return -1;
	}
	m_uSetMsg = RegisterWindowMessage("BOINC_APP_SET");
	m_uGetMsg = RegisterWindowMessage("BOINC_APP_GET");

	GetCursorPos(&m_MousePos);
	SetTimer(1, 100, NULL);

	m_nPosX = m_nPosY = 0;
	m_nDX = m_nDY = 5;

	UtilGetRegKey("Blank", m_bBlankScreen);
	UtilGetRegKey("Blank Time", m_uBlankTime);
	m_uBlankTime *= 10;
	m_uBlankTime += time(0);

    return 0;
}

//////////
// CSSWindow::OnDestroy
// arguments:	void
// returns:		void
// function:	kills timer
void CSSWindow::OnDestroy()
{
	KillTimer(1);
}

//////////
// CSSWindow::OnPaint
// arguments:	null
// returns:		null
// function:	clears the window and draws the bouncing icon if necessary
void CSSWindow::OnPaint()
{
	PAINTSTRUCT ps;
	CDC* pdc;
	RECT winRect;
	CBrush cb;
	cb.CreateSolidBrush(RGB(0,0,0));

	pdc = BeginPaint(&ps);
	GetClientRect(&winRect);
	pdc->FillRect(&winRect, &cb);

	if (m_nMode == MODE_FULLSCREEN || m_nMode == MODE_WINDOW) {
		pdc->DrawIcon(m_nPosX, m_nPosY, m_hBOINCIcon);
		m_nPosX += m_nDX;
		m_nPosY += m_nDY;
		if (m_nPosX <= winRect.left || (m_nPosX+32) >= winRect.right) m_nDX *= -1;
		if (m_nPosY <= winRect.top || (m_nPosY+32) >= winRect.bottom) m_nDY *= -1;
		if (m_nPosX < winRect.left) m_nPosX = winRect.left;
		if ((m_nPosX+32) > winRect.right) m_nPosX = winRect.right-32;
		if (m_nPosY < winRect.top) m_nPosY = winRect.top;
		if ((m_nPosY+32) > winRect.bottom) m_nPosY = winRect.bottom-32;
	}

	EndPaint(&ps);
}

//////////
// CSSWindow::BlankScreen
// arguments:	null
// returns:		null
// function:	returns true if we should go into blank screen mode
bool CSSWindow::BlankScreen()
{
	return	(m_bBlankScreen) &&
			(m_nMode == MODE_FULLSCREEN) &&
			(time(0) >= m_uBlankTime);
}

//////////
// CSSWindow::OnTimer
// arguments:	null
// returns:		null
// function:	redraw the window if needed
void CSSWindow::OnTimer()
{
	if (m_nMode == MODE_FULLSCREEN || m_nMode == MODE_WINDOW) {
		Invalidate();
		OnPaint();
	}
}