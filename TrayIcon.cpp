#include "pch.h"

#include "TrayIcon.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

constexpr auto TRAYICON_CLASS_NAME = _T("CTrayIcon");

const UINT WM_TASKBAR_CREATED = ::RegisterWindowMessage(_T("WmTaskbarCreated"));

BEGIN_MESSAGE_MAP(CTrayIcon, CWnd)
	ON_REGISTERED_MESSAGE(WM_TASKBAR_CREATED, &CTrayIcon::OnTaskbarCreated)
	ON_WM_SETTINGCHANGE()
	ON_WM_TIMER()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTrayIcon

CTrayIcon::CTrayIcon()
{
	SetDefaultValues();

	// Register our class
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(wndcls));

	wndcls.lpszClassName = _T("CTrayIcon");
	wndcls.hInstance = AfxGetInstanceHandle();
	wndcls.hCursor = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
	wndcls.lpfnWndProc = ::DefWindowProc;

	AfxRegisterClass(&wndcls);
}

CTrayIcon::~CTrayIcon()
{
	RemoveIcon();
	DestroyWindow();
	RemoveAnimationIcons();
}

void CTrayIcon::SetDefaultValues()
{
	m_iconID = 0;

	m_timerID = 5000;
	m_timerIdx = 0;

	m_maxTipSize = 64;
	m_toolTip = _T("");

	m_defMenuItemID = 0;
	m_isDefMenuItemByPos = TRUE;

	m_isHidden = TRUE;
	m_isRemoved = TRUE;
	m_isShowPending = FALSE;

	m_hwndNotify = nullptr;
	m_hDefaultIcon = nullptr;

	ZeroMemory(&m_niData, sizeof(NOTIFYICONDATA));

	m_niData.cbSize = sizeof(NOTIFYICONDATA);
	m_niData.hWnd = nullptr;
	m_niData.uID = 0;
	m_niData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	m_niData.uCallbackMessage = WM_TRAYICON_NOTIFY;
	m_niData.hIcon = nullptr;
	m_niData.szTip[0] = 0;

	// must be set guid on Windows 7 or higher
	// just zero it, see http://msdn.microsoft.com/en-us/library/windows/desktop/bb773352(v=vs.85).aspx
	ZeroMemory(&m_niData.guidItem, sizeof(m_niData.guidItem));
	m_niData.hBalloonIcon = nullptr;
}

BOOL CTrayIcon::SetNotificationWnd(CWnd* pWndNotify)
{
	ASSERT(pWndNotify);

	// Make sure notify window is valid
	if (!pWndNotify || !::IsWindow(pWndNotify->GetSafeHwnd()))
	{
		return FALSE;
	}

	m_niData.uFlags = 0;
	m_niData.hWnd = pWndNotify->GetSafeHwnd();

	return ShellNotify(NIM_MODIFY);
}

BOOL CTrayIcon::SetCallbackMessage(UINT newCallbackMessageID)
{
	// Make sure we do not conflict with other messages
	ASSERT(m_niData.uCallbackMessage >= WM_APP);

	m_niData.uFlags = NIF_MESSAGE;
	m_niData.uCallbackMessage = newCallbackMessageID;

	return ShellNotify(NIM_MODIFY);
}

BOOL CTrayIcon::SetIcon(LPCTSTR lpszIconName)
{
	int cx = ::GetSystemMetrics(SM_CXSMICON);
	int cy = ::GetSystemMetrics(SM_CYSMICON);

	m_hDefaultIcon = (HICON)::LoadImage(AfxGetResourceHandle(), lpszIconName, IMAGE_ICON, cx, cy, LR_DEFAULTCOLOR);

	return SetIcon(m_hDefaultIcon);
}

BOOL CTrayIcon::SetIcon(UINT resourceID)
{
	return SetIcon(MAKEINTRESOURCE(resourceID));
}

BOOL CTrayIcon::SetIcon(HICON hIcon)
{
	if (m_niData.hIcon == hIcon)
	{
		return TRUE;
	}

	m_niData.uFlags = NIF_ICON;
	m_niData.hIcon = hIcon;

	if (!m_isHidden)
	{
		return ShellNotify(NIM_MODIFY);
	}

	return TRUE;
}

void CTrayIcon::SetTimer(UINT nIDEvent, UINT uElapse)
{
	m_timerID = nIDEvent;
	StartAnimation(uElapse);
}

void CTrayIcon::RemoveAnimationIcons()
{
	for (auto& ti : m_aniTrayIcons)
	{
		::DestroyIcon(ti.hIcon);
		ti.hIcon = nullptr;
	}
	m_aniTrayIcons.clear();
}

BOOL CTrayIcon::SetAnimationIcons(const UINT* lpIDArray, int nIDCount, const CString* lpStrTipArray /*= nullptr*/)
{
	RemoveAnimationIcons();

	// system icon size
	int cx = ::GetSystemMetrics(SM_CXSMICON);
	int cy = ::GetSystemMetrics(SM_CYSMICON);

	for (int iIcon = 0; iIcon < nIDCount; ++iIcon)
	{
		TRAY_ICON_DATA tdata;

		if (lpStrTipArray != nullptr)
		{
			tdata.strToolTip = lpStrTipArray[iIcon];
		}

		tdata.hIcon = (HICON)::LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(lpIDArray[iIcon]), IMAGE_ICON, cx, cy, LR_DEFAULTCOLOR);

		if (tdata.hIcon == nullptr)
		{
			return FALSE;
		}

		m_aniTrayIcons.emplace_back(tdata);
	}

	return TRUE;
}

BOOL CTrayIcon::SetTooltipText(LPCTSTR lpszTipText)
{
	m_toolTip = lpszTipText;

	if (m_toolTip.GetLength() >= (int)m_maxTipSize)
	{
		m_toolTip = m_toolTip.Left((int)m_maxTipSize - 1);
	}

	return SetShellTooltip(m_toolTip);
}

BOOL CTrayIcon::SetShellTooltip(LPCTSTR lpszTipText)
{
	ASSERT(AfxIsValidString(lpszTipText));
	ASSERT(_tcslen(lpszTipText) < m_maxTipSize);

	m_niData.uFlags = NIF_TIP;
	_tcsnccpy_s(m_niData.szTip, _countof(m_niData.szTip), lpszTipText, m_maxTipSize - 1);

	if (!m_isHidden)
	{
		return ShellNotify(NIM_MODIFY);
	}

	return TRUE;
}

BOOL CTrayIcon::SetTooltipText(UINT nTipText)
{
	CString strTipText;
	strTipText.LoadString(nTipText);

	return SetTooltipText(strTipText);
}

BOOL CTrayIcon::AddIcon()
{
	if (!m_isRemoved)
	{
		RemoveIcon();
	}

	m_niData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	if (!ShellNotify(NIM_ADD))
	{
		m_isShowPending = TRUE;
		return FALSE;
	}

	m_isRemoved = m_isHidden = FALSE;

	return TRUE;
}

BOOL CTrayIcon::RemoveIcon()
{
	m_isShowPending = FALSE;

	if (m_isRemoved)
	{
		return m_isRemoved;
	}

	m_niData.uFlags = 0;
	if (!ShellNotify(NIM_DELETE))
	{
		return FALSE;
	}

	m_isRemoved = m_isHidden = TRUE;

	return m_isRemoved;
}

BOOL CTrayIcon::HideIcon()
{
	if (m_isRemoved || m_isHidden)
	{
		return TRUE;
	}

	m_niData.uFlags = NIF_STATE;
	m_niData.dwState = NIS_HIDDEN;
	m_niData.dwStateMask = NIS_HIDDEN;

	m_isHidden = ShellNotify(NIM_MODIFY);

	return m_isHidden;
}

BOOL CTrayIcon::ShowIcon()
{
	if (m_isRemoved)
		return AddIcon();

	if (!m_isHidden)
		return TRUE;

	m_niData.uFlags = NIF_STATE;
	m_niData.dwState = 0;
	m_niData.dwStateMask = NIS_HIDDEN;

	if (!ShellNotify(NIM_MODIFY))
		return FALSE;

	m_isRemoved = m_isHidden = FALSE;
	return TRUE;
}

BOOL CTrayIcon::SetDefaultMenuItem(UINT uItem, BOOL bByPos)
{
	if ((m_defMenuItemID == uItem) && (m_isDefMenuItemByPos == bByPos))
	{
		return TRUE;
	}

	m_defMenuItemID = uItem;
	m_isDefMenuItemByPos = bByPos;

	CMenu menu;
	if (!menu.LoadMenu(m_niData.uID))
	{
		return FALSE;
	}

	// see if we can access the submenu
	CMenu* pSubMenu = menu.GetSubMenu(0);
	if (!pSubMenu)
	{
		return FALSE;
	}

	// check to see if we can set the submenu for the popup.
	return ::SetMenuDefaultItem(pSubMenu->m_hMenu, m_defMenuItemID, m_isDefMenuItemByPos);
}

void CTrayIcon::GetDefaultMenuItem(UINT& uItem, BOOL& bByPos)
{
	uItem = m_defMenuItemID;
	bByPos = m_isDefMenuItemByPos;
}

/////////////////////////////////////////////////////////////////////////////
// CXTPTrayIcon message handlers

void CTrayIcon::OnTimer(UINT_PTR nIDEvent)
{
	// Update the tray icon and tooltip text.
	if (nIDEvent == m_timerID)
	{
		if (m_timerIdx < m_aniTrayIcons.size())
		{
			const auto& ti = m_aniTrayIcons[m_timerIdx];
			SetShellTooltip(ti.strToolTip.IsEmpty() ? m_toolTip.GetString() : ti.strToolTip);

			if (ti.hIcon)
			{
				SetIcon(ti.hIcon);
			}
		}

		m_timerIdx = (m_timerIdx + 1) % (int)m_aniTrayIcons.size();
	}
}

BOOL CTrayIcon::Create(LPCTSTR lpszCaption, CWnd* pParentWnd, UINT nIconID, UINT uMenuID /*= 0*/,
						  UINT uDefMenuItemID /*= 0*/, BOOL bDefMenuItemByPos /*= FALSE*/)
{
	m_iconID = nIconID;
	m_toolTip = lpszCaption;
	m_hwndNotify = pParentWnd->GetSafeHwnd();

	m_maxTipSize = _countof(m_niData.szTip) - 1;

	// Set the tray icon and tooltip text
	SetIcon(m_iconID);
	SetTooltipText(m_toolTip);

	// Create an invisible window
	CWnd::CreateEx(0, TRAYICON_CLASS_NAME, lpszCaption, WS_POPUP, 0, 0, 0, 0, nullptr, nullptr);

	m_niData.uID = uMenuID;
	m_niData.hWnd = m_hWnd;
	m_defMenuItemID = uDefMenuItemID;
	m_isDefMenuItemByPos = bDefMenuItemByPos;

	m_flags = NIF_MESSAGE | NIF_ICON | NIF_TIP;

	return AddIcon();
}

BOOL CTrayIcon::ShowBalloonTip(LPCTSTR lpszInfo, LPCTSTR lpszInfoTitle /*= nullptr*/,
								  DWORD dwInfoFlags /*= NIIF_NONE*/, UINT uTimeout /*= 10*/,
								  HICON hBaloonIcon /*= nullptr*/)
{
	if (!lpszInfo)
		return FALSE;

	// The balloon tooltip text can be up to 255 chars long!
	ASSERT(AfxIsValidString(lpszInfo));
	ASSERT(_tcslen(lpszInfo) < 256);

	// The balloon title text can be up to 63 chars long!
	if (lpszInfoTitle)
	{
		ASSERT(AfxIsValidString(lpszInfoTitle));
		ASSERT(_tcslen(lpszInfoTitle) < 64);
	}

	// The timeout must be between 10 and 30 seconds.
	ASSERT(uTimeout >= 10 && uTimeout <= 30);

	m_niData.uFlags |= NIF_INFO;

	_tcsncpy_s(m_niData.szInfo, _countof(m_niData.szInfo), lpszInfo, 255);

	if (lpszInfoTitle)
	{
		_tcsncpy_s(m_niData.szInfoTitle, _countof(m_niData.szInfoTitle), lpszInfoTitle, 63);
	}
	else
	{
		m_niData.szInfoTitle[0] = _T('\0');
	}

	m_niData.uTimeout = uTimeout * 1000;
	m_niData.dwInfoFlags = dwInfoFlags;

	if (hBaloonIcon)
	{
		m_niData.hBalloonIcon = hBaloonIcon;
	}

	BOOL bResult = ShellNotify(NIM_MODIFY);

	// Clear text string
	m_niData.szInfo[0] = _T('\0');

	return bResult;
}

void CTrayIcon::StopAnimation()
{
	CWnd::KillTimer(m_timerID);

	SetShellTooltip(m_toolTip);
	SetIcon(m_iconID);
}

LRESULT CTrayIcon::OnTrayNotification(WPARAM wParam, LPARAM lParam)
{
	// Return quickly if its not for this tray icon
	if (m_hwndNotify == nullptr || wParam != m_niData.uID)
	{
		return 0L;
	}

	// is our notification window has already handled this message?
	if (::SendMessage(m_hwndNotify, WM_TRAYICON_NOTIFY, wParam, lParam))
	{
		return 1;
	}

	switch (LOWORD(lParam))
	{
		case NIN_BALLOONSHOW: break;
		case NIN_BALLOONHIDE: break;
		case NIN_BALLOONTIMEOUT: break;
		case NIN_BALLOONUSERCLICK: break;

		case NIN_SELECT: // fall thru
		case NIN_KEYSELECT:
		case WM_CONTEXTMENU:
		case WM_RBUTTONUP:
		{
			CMenu menu;
			if (!menu.LoadMenu(m_niData.uID))
			{
				return 0;
			}

			CMenu* pSubMenu = menu.GetSubMenu(0);
			if (pSubMenu == nullptr)
			{
				return 0;
			}

			::SetMenuDefaultItem(pSubMenu->m_hMenu, m_defMenuItemID, m_isDefMenuItemByPos);

			CPoint pos;
			GetCursorPos(&pos);
			::SetForegroundWindow(m_hwndNotify);
			::TrackPopupMenu(pSubMenu->m_hMenu, 0, pos.x, pos.y, 0, m_hwndNotify, nullptr);
			::PostMessage(m_hwndNotify, WM_NULL, 0, 0);

			menu.DestroyMenu();

			BOOL result = ShellNotify(NIM_SETFOCUS);
			UNUSED_ALWAYS(result);
		}
		break;

		case WM_LBUTTONDBLCLK:
		{
			// double click received, the default action is to execute default menu item
			::SetForegroundWindow(m_hwndNotify);

			UINT uItem;
			if (!m_isDefMenuItemByPos)
			{
				uItem = m_defMenuItemID;
			}
			else
			{
				CMenu menu;
				if (!menu.LoadMenu(m_niData.uID))
				{
					return 0;
				}

				CMenu* pSubMenu = menu.GetSubMenu(0);
				if (pSubMenu == nullptr)
				{
					return 0;
				}

				uItem = pSubMenu->GetMenuItemID(m_defMenuItemID);

				menu.DestroyMenu();
			}

			::SendMessage(m_hwndNotify, WM_COMMAND, uItem, 0);
		}
	}

	return 1;
}

LRESULT CTrayIcon::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == m_niData.uCallbackMessage)
	{
		return OnTrayNotification(wParam, lParam);
	}

	return CWnd::WindowProc(message, wParam, lParam);
}

// Called when the taskbar is created (even after explorer crashes and restarts
LRESULT CTrayIcon::OnTaskbarCreated(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	InstallIconPending();
	return 0L;
}

void CTrayIcon::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CWnd::OnSettingChange(uFlags, lpszSection);

	if (m_isShowPending && uFlags == SPI_SETWORKAREA)
	{
		InstallIconPending();
	}
}

void CTrayIcon::InstallIconPending()
{
	if (m_isRemoved || m_isHidden)
	{
		return;
	}

	m_niData.uFlags = m_flags;
	m_isShowPending = !ShellNotify(NIM_ADD);
}

BOOL CALLBACK CTrayIcon::FindTrayWnd(HWND hWnd, LPARAM lParam)
{
	TCHAR szClassName[256] = { 0 };
	if (::GetClassName(hWnd, szClassName, _countof(szClassName)) == 0)
	{
		return FALSE;
	}

	// Is it the main system tray?
	if (_tcsicmp(szClassName, _T("TrayNotifyWnd")) == 0)
	{
		CRect* pRect = (CRect*)lParam;
		::GetWindowRect(hWnd, pRect);
		return TRUE;
	}

	// May be System Clock?
	if (_tcsicmp(szClassName, _T("TrayClockWClass")) == 0)
	{
		CRect* pRect = (CRect*)lParam;
		CRect rectClock;
		::GetWindowRect(hWnd, rectClock);

		// if clock has too line - adjust
		if (rectClock.bottom < pRect->bottom - 5)
		{
			pRect->top = rectClock.bottom;
		}
		else
		{
			pRect->right = rectClock.left;
		}

		return FALSE;
	}

	return TRUE;
}

BOOL CTrayIcon::GetTrayWindowRect(CRect& rect)
{
	HWND hWndTray = ::FindWindow(_T("Shell_TrayWnd"), nullptr);
	if (hWndTray)
	{
		::GetWindowRect(hWndTray, &rect);
		::EnumChildWindows(hWndTray, FindTrayWnd, (LPARAM)&rect);
		return TRUE;
	}

	APPBARDATA ad;
	::ZeroMemory(&ad, sizeof(APPBARDATA));
	ad.cbSize = sizeof(ad);

	if (::SHAppBarMessage(ABM_GETTASKBARPOS, &ad))
	{
		switch (ad.uEdge)
		{
			case ABE_LEFT:
			case ABE_RIGHT:
				// We want to minimize to the bottom of the taskbar
				rect.top = ad.rc.bottom - 100;
				rect.bottom = ad.rc.bottom - 16;
				rect.left = ad.rc.left;
				rect.right = ad.rc.right;
				break;

			case ABE_TOP:
			case ABE_BOTTOM:
				// We want to minimize to the right of the taskbar
				rect.top = ad.rc.top;
				rect.bottom = ad.rc.bottom;
				rect.left = ad.rc.right - 100;
				rect.right = ad.rc.right - 16;
				break;
		}

		return TRUE;
	}

	if (::SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0))
	{
		rect.left = rect.right - 150;
		rect.top = rect.bottom - 30;
		return TRUE;
	}

	return FALSE;
}

BOOL CTrayIcon::CanAnimate() const
{
	ANIMATIONINFO ai;
	::ZeroMemory(&ai, sizeof(ANIMATIONINFO));
	ai.cbSize = sizeof(ANIMATIONINFO);

	::SystemParametersInfo(SPI_GETANIMATION, sizeof(ANIMATIONINFO), &ai, 0);

	return ai.iMinAnimate;
}

BOOL CTrayIcon::CreateMinimizedWnd(CWnd* pWndApp)
{
	// Create the minimize window
	if (!::IsWindow(m_wndMinimize.m_hWnd)
		&& !m_wndMinimize.CreateEx(0, AfxRegisterWndClass(0), _T(""), WS_POPUP,
								   CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr))
	{
		return FALSE;
	}

	pWndApp->SetParent(&m_wndMinimize);
	return TRUE;
}

void CTrayIcon::MinToTray(CWnd* pWndApp, BOOL showIcon /*= TRUE*/)
{
	if (CanAnimate())
	{
		CRect rectFrom, rectTo;

		pWndApp->GetWindowRect(rectFrom);
		GetTrayWindowRect(rectTo);

		::DrawAnimatedRects(pWndApp->m_hWnd, IDANI_CAPTION, rectFrom, rectTo);
	}

	CreateMinimizedWnd(pWndApp);
	pWndApp->ShowWindow(SW_HIDE);

	if (showIcon)
		ShowIcon();
}

void CTrayIcon::MaxFromTray(CWnd* pWndApp, BOOL hideIcon /*= TRUE*/)
{
	if (CanAnimate())
	{
		CRect rectFrom;
		GetTrayWindowRect(rectFrom);

		CRect rectTo;
		pWndApp->GetWindowRect(rectTo);

		pWndApp->SetParent(nullptr);
		::DrawAnimatedRects(pWndApp->m_hWnd, IDANI_CAPTION, rectFrom, rectTo);
	}
	else
	{
		pWndApp->SetParent(nullptr);
	}

	pWndApp->SetWindowPos(nullptr, 0, 0, 0, 0,
						  SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);

	// Move focus away and back again to ensure taskbar icon is recreated
	if (::IsWindow(m_wndMinimize.m_hWnd))
	{
		m_wndMinimize.SetActiveWindow();
	}

	pWndApp->SetActiveWindow();
	pWndApp->SetForegroundWindow();

	if (hideIcon)
		HideIcon();
}
