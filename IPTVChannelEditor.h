/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2022): sharky72 (https://github.com/KocourKuba)

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

#ifndef __AFXWIN_H__
#error "include 'StdAfx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "Config.h"

// CEdemChannelEditorApp:
// See EdemChannelEditor.cpp for the implementation of this class
//

class CIPTVChannelEditorApp : public CWinAppEx
{
	typedef struct
	{
		CString csLang;
		CString csPath;
		CString csSuffix;
	} LangStruct;

public:
	CIPTVChannelEditorApp();

	// Overrides
public:
	BOOL InitInstance() override;

	void FillLangMap();

	DECLARE_MESSAGE_MAP()

public:
	std::map<WORD, LangStruct> m_LangMap;
};

class CCommandLineInfoEx : public CCommandLineInfo
{
public:
	CCommandLineInfoEx() = default;
	void ParseParam(LPCTSTR szParam, BOOL bFlag, BOOL bLast) override;

public:
	bool m_bDev = false;
	bool m_bMakeAll = false;
	bool m_bPortable = false;
	bool m_bRestoreReg = false;
	bool m_bNoEmbed = false;
	bool m_bNoCustom = false;
	bool m_bCleanupReg = false;
};

BOOL LoadImageFromUrl(const std::wstring& fullPath, CImage& image);
void SetImageControl(const CImage& image, CStatic& wnd);

std::wstring GetAppPath(LPCWSTR szSubFolder = nullptr);

void ConvertAccounts();

bool PackPlugin(const StreamType plugin_type,
				bool showMessage,
				bool make_web_update = false,
				std::wstring output_path = L"",
				bool noEmbed = false,
				bool noCustom = false);

void SaveWindowPos(HWND hWnd, LPCTSTR name);
void RestoreWindowPos(HWND hWnd, LPCTSTR name);
int RequestToUpdateServer(const std::wstring& command);

std::wstring load_string_resource(unsigned int id);
uintmax_t calc_folder_size(const std::wstring& path);

std::wstring GetPluginShortNameW(const StreamType plugin_type, bool bCamel = false);
std::string  GetPluginShortNameA(const StreamType plugin_type, bool bCamel = false);

extern CIPTVChannelEditorApp theApp;
