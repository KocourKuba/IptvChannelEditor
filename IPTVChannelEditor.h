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

#ifndef __AFXWIN_H__
#error "include 'StdAfx.h' before including this file for PCH"
#endif

#include "PluginDefines.h"

#include "7zpp\SevenZipWrapper.h"

// CEdemChannelEditorApp:
// See EdemChannelEditor.cpp for the implementation of this class
//

class CIPTVChannelEditorApp : public CWinAppEx
{
	using LangStruct = struct
	{
		HMODULE hLib;
		CString csLang;
	};

public:
	CIPTVChannelEditorApp();

	// Overrides
public:
	BOOL InitInstance() override;

	std::wstring CheckAndCreateDirs(const std::wstring& settings, const std::wstring& default_path);

	int CheckPluginConsistency(bool isDev);

	void FillLangMap();

	bool PackPlugin(const PluginType plugin_type,
					bool showMessage,
					bool make_web_update = false,
					std::wstring output_path = L"",
					bool noEmbed = false,
					bool noCustom = false);


	DECLARE_MESSAGE_MAP()

public:
	bool m_bDev = false;
	static std::wstring DEV_PATH;
	static std::wstring PACK_DLL_PATH;
	std::map<WORD, LangStruct> m_LangMap;
	SevenZip::SevenZipWrapper m_archiver;
};

class CCommandLineInfoEx : public CCommandLineInfo
{
public:
	void ParseParam(LPCTSTR szParam, BOOL bFlag, BOOL bLast) override;

public:
#ifdef _DEBUG
	bool m_bDev = true;
#else
	bool m_bDev = false;
#endif // _DEBUG
	bool m_bMake = false;
	bool m_bMakeAll = false;
	bool m_bPortable = false;
	bool m_bRestoreReg = false;
	bool m_bNoEmbed = false;
	bool m_bNoCustom = false;
	bool m_bCleanupReg = false;
};

BOOL LoadImageFromUrl(const std::wstring& fullPath, CImage& image);
void SetImageControl(const CImage& image, CStatic& wnd);

std::wstring GetAppPath(LPCWSTR szSubFolder = nullptr, bool no_end_slash = false);

void ConvertAccounts();

void SaveWindowPos(HWND hWnd, LPCTSTR name);
bool LoadPngImage(UINT id, CImage& img);
void SetButtonImage(UINT imgId, CButton& btn);
void SetButtonImage(UINT imgId, CButton* btn);
void RestoreWindowPos(HWND hWnd, LPCTSTR name);
int RequestToUpdateServer(const std::wstring& command, bool isThread = true);

std::wstring load_string_resource(unsigned int id);
std::wstring load_string_resource(unsigned int cp, unsigned int id);
std::string load_string_resource_a(unsigned int id);
std::string load_string_resource_a(unsigned int cp, unsigned int id);

uintmax_t calc_folder_size(const std::wstring& path);

std::wstring GetPluginTypeNameW(const PluginType plugin_type, bool bCamel = false);
std::string  GetPluginTypeNameA(const PluginType plugin_type, bool bCamel = false);

void LogProtocol(const std::string& str);
void LogProtocol(const std::wstring& str);

extern CIPTVChannelEditorApp theApp;
extern std::string g_szServerPath;
