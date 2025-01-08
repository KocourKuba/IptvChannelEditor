/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2025): sharky72 (https://github.com/KocourKuba)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#pragma once

const UINT WM_TRAYICON_NOTIFY = (WM_USER + 200);

class CTrayIcon : public CWnd
{
private:
	struct TRAY_ICON_DATA
	{
		HICON hIcon = nullptr;		// Handle to the icon
		CString strToolTip; // tooltip displayed for the tray icon
	};

public:
	CTrayIcon();
	virtual ~CTrayIcon();

public:
	BOOL Create(LPCTSTR lpszCaption, CWnd* pParentWnd, UINT nIconID, UINT uMenuID = 0,
				UINT uDefMenuItemID = 0, BOOL bDefMenuItemByPos = FALSE);

	BOOL ShowBalloonTip(LPCTSTR lpszInfo = nullptr, LPCTSTR lpszInfoTitle = nullptr,
						DWORD dwInfoFlags = NIIF_NONE, UINT uTimeout = 10,
						HICON hBaloonIcon = nullptr);

	void StopAnimation();
	void StartAnimation(UINT uElapse = 500) { CWnd::SetTimer(m_timerID, uElapse, nullptr); }

	BOOL SetAnimationIcons(const UINT* lpIDArray, int nIDCount, const CString* lpStrTipArray = nullptr);
	void SetAnimationIcons(const UINT* lpIDArray, const CString* lpStrTipArray, int nIDCount);


	void SetDefaultValues();

	BOOL SetTooltipText(UINT nTipText);
	BOOL SetTooltipText(LPCTSTR lpszTipText);

	CString GetTooltipText() const { return m_toolTip; }

	BOOL SetCallbackMessage(UINT newCallbackMessage);
	UINT GetCallbackMessage() const { return m_niData.uCallbackMessage; }

	BOOL SetDefaultMenuItem(UINT uItem, BOOL bByPos);
	void GetDefaultMenuItem(UINT& uItem, BOOL& bByPos);

	BOOL SetNotificationWnd(CWnd* pWndNotify);
	CWnd* GetNotificationWnd() const { return CWnd::FromHandle(m_niData.hWnd); }

	HICON GetIcon() const { return m_niData.hIcon; }

	BOOL SetIcon(HICON hIcon);
	BOOL SetIcon(LPCTSTR lpszIconName);
	BOOL SetIcon(UINT resourceID);

	BOOL AddIcon();
	BOOL RemoveIcon();

	BOOL HideIcon();
	BOOL ShowIcon();

	BOOL ToggleShowIcon(BOOL bShow) { return bShow ? ShowIcon() : HideIcon(); }

	void MinToTray(CWnd* pWnd, BOOL showIcon = TRUE);
	void MaxFromTray(CWnd* pWnd, BOOL hideIcon = TRUE);

	BOOL ShellNotify(DWORD dwMessage) { return ::Shell_NotifyIcon(dwMessage, (PNOTIFYICONDATA)&m_niData); }

	// Adds an icon to the system tray.
	void InstallIconPending();

	BOOL CreateMinimizedWnd(CWnd* pWndApp);

	void RemoveAnimationIcons();

	void SetTrayIcon(UINT nIcon, DWORD dwMessage = NIM_ADD) { SetIcon(nIcon); }

	void SetTimer(UINT nIDEvent, UINT uElapse);
	void KillTimer() { StopAnimation(); }

protected:

	LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) override;
	virtual LRESULT OnTrayNotification(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg LRESULT OnTaskbarCreated(WPARAM wParam, LPARAM lParam);

private:
	static BOOL CALLBACK FindTrayWnd(HWND hWnd, LPARAM lParam);

	BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle,
				const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = nullptr) override;

	BOOL CanAnimate() const;
	BOOL GetTrayWindowRect(CRect& rect);
	BOOL SetShellTooltip(LPCTSTR lpszTipText);

protected:
	BOOL m_isDefMenuItemByPos = FALSE;
	BOOL m_isHidden = FALSE;
	BOOL m_isRemoved = FALSE;
	BOOL m_isShowPending = FALSE;

	NOTIFYICONDATA m_niData{};

	UINT m_flags = 0;
	UINT m_iconID = 0;
	UINT m_timerID = 0;
	UINT m_timerIdx = 0;
	UINT m_defMenuItemID = 0;
	HWND m_hwndNotify = nullptr;
	CWnd m_wndMinimize;
	size_t m_maxTipSize = 0;
	CString m_toolTip;
	HICON m_hDefaultIcon = nullptr;
	std::vector<TRAY_ICON_DATA> m_aniTrayIcons;
};
