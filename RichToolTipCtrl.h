////////////////////////////////////////////////////////////////////////////
// File:	RichToolTipCtrl.h
// Version:	1.2
// Created:	06-Oct-2008
//
// Author:	Paul S. Vickery
// E-mail:	developer@vickeryhome.freeserve.co.uk
//
// Class to provide a means of using rich text in a tool-tip, without using
// the newer styles only available in later systems. Based on CToolTipCtrl.
//
// You are free to use or modify this code, with no restrictions, other than
// you continue to acknowledge me as the original author in this source code,
// or any code derived from it.
//
// If you use this code, or use it as a base for your own code, it would be
// nice to hear from you simply so I know it's not been a waste of time!
//
// Copyright (c) 2005--2008 Paul S. Vickery
//
////////////////////////////////////////////////////////////////////////////
// Version History:
//
// Version 1.2 - 06-Oct-2008
// =========================
// - Changed to draw correctly on XP/Vista
//
// Version 1.1 - 22-Mar-2005
// =========================
// - Added static method for escaping plain text to make it RTF safe
// - Modified the positioning to work on multi-monitor systems
// - Removed dependency on RichToolTipCtrlDemo.h
//
// Version 1.0 - 14-Mar-2005
// =========================
// Initial version
//
////////////////////////////////////////////////////////////////////////////
// PLEASE LEAVE THIS HEADER INTACT
////////////////////////////////////////////////////////////////////////////

#pragma once

// RichToolTipCtrl.h : header file
//

#include <richole.h>  // for IRichEditOleCallback

#ifndef _UXTHEME_H_
// to avoid reliance on uxtheme.h, and uxtheme.lib, we'll load
// the theme functions, but only once
typedef HANDLE HTHEME;
#endif // _UXTHEME_H_

typedef HTHEME(_stdcall* PFNOPENTHEMEDATA)(HWND, LPWSTR);
typedef HRESULT(_stdcall* PFNCLOSETHEMEDATA)(HTHEME);
typedef HRESULT(__stdcall* PFNDRAWTHEMEBACKGROUND)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT* pRect, const RECT* pClipRect);
typedef BOOL(__stdcall* PFNISTHEMEBACKGROUNDPARTIALLYTRANSPARENT)(HTHEME hTheme, int iPartId, int iStateId);
typedef HRESULT(__stdcall* PFNDRAWTHEMEPARENTBACKGROUND)(HWND, HDC, RECT*);
typedef BOOL(__stdcall* PFNISAPPTHEMED)(void);

#include <shlwapi.h>  // for DllGetVersion definitions

/////////////////////////////////////////////////////////////////////////////
// CRichToolTipCtrl window

class CRichToolTipCtrl : public CToolTipCtrl
{
	DECLARE_DYNAMIC(CRichToolTipCtrl);

	// Construction
public:
	CRichToolTipCtrl();

	// Attributes
public:

	// Operations
public:

	template <typename Ch, size_t S>
	static constexpr auto any_string(const char(&literal)[S]) -> const std::array<Ch, S>
	{
		std::array<Ch, S> r = {};

		for (size_t i = 0; i < S; i++)
			r[i] = literal[i];

		return r;
	}

	template<typename T>
	static std::basic_string<T> MakeTextRTFSafe(const std::basic_string<T>& text)
	{
		// modify the specified text to make it safe for use in a rich-edit
		// control, by escaping special RTF characters '\', '{' and '}'

		const auto& paragraph = any_string<char>(R"({\par})");

		std::wstring rtf;
		for (auto& it = text.begin(); it != text.end(); ++it)
		{
			if (*it == '\r') continue;;

			if (*it == '\n')
			{
				rtf.append(paragraph.begin(), paragraph.end());
				++it;
				continue;
			}

			if (*it == '\\' || *it == '{' || *it == '}')
			{
				rtf += '\\';
			}

			rtf += *it;
		}

		return rtf;
	}

protected:
	BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) override;

// Implementation
public:
	virtual ~CRichToolTipCtrl() = default;

protected:
	/////////////////////////////////////////////////////////////////////////////
	// CRichToolTipRichEditCtrl window

	class CRichToolTipRichEditCtrl : public CRichEditCtrl
	{
		// Construction
	protected:
		CRichToolTipRichEditCtrl() = default;

		// Attributes
	protected:

		// Operations
	protected:
		int StreamTextIn(LPCTSTR lpszText);

		// Implementation
	protected:
		virtual ~CRichToolTipRichEditCtrl() = default;

		// Generated message map functions
	protected:

		DECLARE_MESSAGE_MAP()

		friend class CRichToolTipCtrl;
	};

public:
	class CRichToolTipXPStyle
	{
		// comctl32.dll, and some other DLLs, export a DllGetVersion method
		// so we use that to see which version of comctl32.dll is currently loaded
		static DWORD GetComCtl32Version()
		{
			HINSTANCE hInstDll = LoadLibrary(_T("COMCTL32.DLL"));
			if (hInstDll == nullptr)
				return 0;
			DLLGETVERSIONPROC pfnDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hInstDll, "DllGetVersion");
			if (pfnDllGetVersion == nullptr)
				return 0;
			DLLVERSIONINFO dvi = { sizeof(dvi) };
			pfnDllGetVersion(&dvi);
			FreeLibrary(hInstDll);
			return dvi.dwMajorVersion;
		}
	public:
		CRichToolTipXPStyle()
		{
			m_nIsAppXPStyled = -1;
			m_hThemeDLL = LoadLibrary(_T("uxtheme.dll"));
			if (m_hThemeDLL != nullptr)
			{
				m_pfnOpenThemeData = (PFNOPENTHEMEDATA)GetProcAddress(m_hThemeDLL, "OpenThemeData");
				m_pfnCloseThemeData = (PFNCLOSETHEMEDATA)GetProcAddress(m_hThemeDLL, "CloseThemeData");
				m_pfnDrawThemeBackground = (PFNDRAWTHEMEBACKGROUND)GetProcAddress(m_hThemeDLL, "DrawThemeBackground");
				m_pfnIsAppThemed = (PFNISAPPTHEMED)GetProcAddress(m_hThemeDLL, "IsAppThemed");
				m_pfnDrawThemeParentBackground = (PFNDRAWTHEMEPARENTBACKGROUND)GetProcAddress(m_hThemeDLL, "DrawThemeParentBackground");
				m_pfnIsThemeBackgroundPartiallyTransparent = (PFNISTHEMEBACKGROUNDPARTIALLYTRANSPARENT)GetProcAddress(m_hThemeDLL, "IsThemeBackgroundPartiallyTransparent");
			}
			else
			{
				m_pfnOpenThemeData = nullptr;
				m_pfnCloseThemeData = nullptr;
				m_pfnDrawThemeBackground = nullptr;
				m_pfnIsAppThemed = nullptr;
				m_pfnDrawThemeParentBackground = nullptr;
				m_pfnIsThemeBackgroundPartiallyTransparent = nullptr;
			}
		}

		~CRichToolTipXPStyle()
		{
			if (m_hThemeDLL != nullptr)
				FreeLibrary(m_hThemeDLL);
		}

		HTHEME OpenThemeData(HWND hwnd, LPWSTR pszClassList)
		{
			return (m_pfnOpenThemeData != nullptr) ? m_pfnOpenThemeData(hwnd, pszClassList) : nullptr;
		}

		HRESULT CloseThemeData(HTHEME hTheme)
		{
			return (m_pfnCloseThemeData != nullptr) ? m_pfnCloseThemeData(hTheme) : E_FAIL;
		}

		HRESULT DrawThemeBackground(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT* pRect, const RECT* pClipRect = nullptr)
		{
			return (m_pfnDrawThemeBackground != nullptr) ? m_pfnDrawThemeBackground(hTheme, hdc, iPartId, iStateId, pRect, pClipRect) : E_FAIL;
		}

		HRESULT DrawThemeParentBackground(HWND hwnd, HDC hdc, RECT* prc)
		{
			return (m_pfnDrawThemeParentBackground != nullptr) ? m_pfnDrawThemeParentBackground(hwnd, hdc, prc) : S_OK;
		}

		BOOL IsThemeBackgroundPartiallyTransparent(HTHEME hTheme, int iPartId, int iStateId)
		{
			return (m_pfnIsThemeBackgroundPartiallyTransparent != nullptr) ? m_pfnIsThemeBackgroundPartiallyTransparent(hTheme, iPartId, iStateId) : FALSE;
		}

		BOOL IsAppThemed()
		{
			return (m_pfnIsAppThemed != nullptr) ? m_pfnIsAppThemed() : FALSE;
		}

		BOOL IsAppXPStyled()
		{
			if (m_nIsAppXPStyled == -1)
				// IsAppThemed returns TRUE even if the app has no manifest file
				// The only way to really check is to test the major version of comctl32.dll
				m_nIsAppXPStyled = (IsAppThemed() && GetComCtl32Version() >= 6) ? 1 : 0;
			return (m_nIsAppXPStyled > 0);
		}

		void Reset() { m_nIsAppXPStyled = -1; }
	protected:
		HINSTANCE m_hThemeDLL;  // DLL handle for XP styling library
		PFNOPENTHEMEDATA m_pfnOpenThemeData;
		PFNCLOSETHEMEDATA m_pfnCloseThemeData;
		PFNDRAWTHEMEBACKGROUND m_pfnDrawThemeBackground;
		PFNISAPPTHEMED m_pfnIsAppThemed;
		PFNDRAWTHEMEPARENTBACKGROUND m_pfnDrawThemeParentBackground;
		PFNISTHEMEBACKGROUNDPARTIALLYTRANSPARENT m_pfnIsThemeBackgroundPartiallyTransparent;
		int m_nIsAppXPStyled;
	};

	// Generated message map functions
protected:
	CRichToolTipRichEditCtrl m_edit;
	CSize m_sizeEditMin;
	CRect m_lastRc;
	CSize CalculateMinimiumRichEditSize();

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnShow(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()

	// Interface Map
public:
	BEGIN_INTERFACE_PART(RichEditOleCallback, IRichEditOleCallback)
		INIT_INTERFACE_PART(CRichToolTipCtrl, RichEditOleCallback)
		STDMETHOD(GetNewStorage) (LPSTORAGE*);
		STDMETHOD(GetInPlaceContext) (LPOLEINPLACEFRAME*, LPOLEINPLACEUIWINDOW*, LPOLEINPLACEFRAMEINFO);
		STDMETHOD(ShowContainerUI) (BOOL);
		STDMETHOD(QueryInsertObject) (LPCLSID, LPSTORAGE, LONG);
		STDMETHOD(DeleteObject) (LPOLEOBJECT);
		STDMETHOD(QueryAcceptData) (LPDATAOBJECT, CLIPFORMAT*, DWORD, BOOL, HGLOBAL);
		STDMETHOD(ContextSensitiveHelp) (BOOL);
		STDMETHOD(GetClipboardData) (CHARRANGE*, DWORD, LPDATAOBJECT*);
		STDMETHOD(GetDragDropEffect) (BOOL, DWORD, LPDWORD);
		STDMETHOD(GetContextMenu) (WORD, LPOLEOBJECT, CHARRANGE*, HMENU*);
	END_INTERFACE_PART(RichEditOleCallback)

	DECLARE_INTERFACE_MAP()
};

inline STDMETHODIMP_(ULONG) CRichToolTipCtrl::XRichEditOleCallback::AddRef()
{
	METHOD_PROLOGUE_EX_(CRichToolTipCtrl, RichEditOleCallback)
		return (ULONG)pThis->InternalAddRef();
}

inline STDMETHODIMP_(ULONG) CRichToolTipCtrl::XRichEditOleCallback::Release()
{
	METHOD_PROLOGUE_EX_(CRichToolTipCtrl, RichEditOleCallback)
		return (ULONG)pThis->InternalRelease();
}

inline STDMETHODIMP CRichToolTipCtrl::XRichEditOleCallback::QueryInterface(REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE_EX_(CRichToolTipCtrl, RichEditOleCallback)
		return (HRESULT)pThis->InternalQueryInterface(&iid, ppvObj);
}

inline STDMETHODIMP CRichToolTipCtrl::XRichEditOleCallback::GetInPlaceContext(
	LPOLEINPLACEFRAME* /*lplpFrame*/, LPOLEINPLACEUIWINDOW* /*lplpDoc*/,
	LPOLEINPLACEFRAMEINFO /*lpFrameInfo*/)
{
	return S_OK;
}

inline STDMETHODIMP CRichToolTipCtrl::XRichEditOleCallback::ShowContainerUI(BOOL /*fShow*/)
{
	return S_OK;
}

inline STDMETHODIMP CRichToolTipCtrl::XRichEditOleCallback::QueryInsertObject(LPCLSID /*lpclsid*/, LPSTORAGE /*pstg*/, LONG /*cp*/)
{
	return S_OK;
}

inline STDMETHODIMP CRichToolTipCtrl::XRichEditOleCallback::DeleteObject(LPOLEOBJECT /*lpoleobj*/)
{
	return S_OK;
}

inline STDMETHODIMP CRichToolTipCtrl::XRichEditOleCallback::QueryAcceptData(
	LPDATAOBJECT /*lpdataobj*/, CLIPFORMAT* /*lpcfFormat*/, DWORD /*reco*/,
	BOOL /*fReally*/, HGLOBAL /*hMetaPict*/)
{
	return S_OK;
}

inline STDMETHODIMP CRichToolTipCtrl::XRichEditOleCallback::ContextSensitiveHelp(BOOL /*fEnterMode*/)
{
	return E_NOTIMPL;
}

inline STDMETHODIMP CRichToolTipCtrl::XRichEditOleCallback::GetClipboardData(CHARRANGE* /*lpchrg*/, DWORD /*reco*/, LPDATAOBJECT* /*lplpdataobj*/)
{
	return S_OK;
}

inline STDMETHODIMP CRichToolTipCtrl::XRichEditOleCallback::GetDragDropEffect(BOOL /*fDrag*/, DWORD /*grfKeyState*/, LPDWORD /*pdwEffect*/)
{
	return S_OK;
}

inline STDMETHODIMP CRichToolTipCtrl::XRichEditOleCallback::GetContextMenu(WORD /*seltype*/, LPOLEOBJECT /*lpoleobj*/, CHARRANGE* /*lpchrg*/, HMENU* /*lphmenu*/)
{
	return S_OK;
}
