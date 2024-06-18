/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2024): sharky72 (https://github.com/KocourKuba)

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

#include "pch.h"
#include <afxdialogex.h>
#include "IPTVChannelEditor.h"
#include "PathsSettingsPage.h"
#include "AccountSettings.h"
#include "Constants.h"


// CPathsSettingsPage dialog

IMPLEMENT_DYNAMIC(CPathsSettingsPage, CTooltipPropertyPage)

BEGIN_MESSAGE_MAP(CPathsSettingsPage, CTooltipPropertyPage)
END_MESSAGE_MAP()

CPathsSettingsPage::CPathsSettingsPage() : CTooltipPropertyPage(IDD_PATHS_SETTINGS_PAGE)
{
}

void CPathsSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_MFCEDITBROWSE_PLAYER, m_player);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_PLAYER, m_wndPlayer);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_PROBE, m_probe);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_PROBE, m_wndProbe);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_CH_LIST_PATH, m_wndListsPath);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_CH_LIST_PATH, m_lists_path);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_PLUGINS_PATH, m_wndPluginsPath);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_PLUGINS_PATH, m_plugins_path);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_PLUGINS_WEB_UPDATE_PATH, m_wndPluginsWebUpdatePath);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_PLUGINS_WEB_UPDATE_PATH, m_plugins_web_update_path);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_PLUGINS_SETTINGS_PATH, m_wndPluginSettingsPath);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_PLUGINS_SETTINGS_PATH, m_plugins_settings_path);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_PLUGINS_IMAGE_PATH, m_wndPluginImagePath);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_PLUGINS_IMAGE_PATH, m_plugins_image_path);
}

BOOL CPathsSettingsPage::OnInitDialog()
{
	__super::OnInitDialog();

	m_player = GetConfig().get_string(true, REG_PLAYER).c_str();
	m_probe = GetConfig().get_string(true, REG_FFPROBE).c_str();
	m_lists_path = GetConfig().get_string(true, REG_LISTS_PATH).c_str();
	m_plugins_path = GetConfig().get_string(true, REG_OUTPUT_PATH).c_str();
	m_plugins_web_update_path = GetConfig().get_string(true, REG_WEB_UPDATE_PATH).c_str();
	m_plugins_settings_path = GetConfig().get_string(true, REG_SAVE_SETTINGS_PATH).c_str();
	m_plugins_image_path = GetConfig().get_string(true, REG_SAVE_IMAGE_PATH).c_str();

	m_wndListsPath.EnableFolderBrowseButton(m_lists_path.GetString(), BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE);
	m_wndPluginsPath.EnableFolderBrowseButton(m_plugins_path.GetString(), BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE);
	m_wndPluginsWebUpdatePath.EnableFolderBrowseButton(m_plugins_web_update_path.GetString(), BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE);
	m_wndPluginSettingsPath.EnableFolderBrowseButton(m_plugins_settings_path.GetString(), BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE);
	m_wndPluginImagePath.EnableFolderBrowseButton(m_plugins_image_path.GetString(), BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE);

	CString filter(_T("EXE file(*.exe)|*.exe|All Files (*.*)|*.*||"));
	m_wndPlayer.EnableFileBrowseButton(nullptr, filter.GetString(), OFN_EXPLORER | OFN_ENABLESIZING | OFN_LONGNAMES | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST);
	m_wndProbe.EnableFileBrowseButton(nullptr, filter.GetString(), OFN_EXPLORER | OFN_ENABLESIZING | OFN_LONGNAMES | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST);

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPathsSettingsPage::OnApply()
{
	UpdateData(TRUE);

	if (m_lists_path.IsEmpty())
	{
		m_lists_path = _T(".\\");
		m_lists_path += utils::CHANNELS_LIST_PATH;
	}

	if (m_lists_path.Right(1) != '\\')
		m_lists_path += '\\';

	const auto& list_path = GetConfig().get_string(true, REG_LISTS_PATH);
	if (m_lists_path != list_path.c_str()) //-V1051
	{
		auto res = AfxMessageBox(IDS_STRING_COPY_LISTS, MB_ICONEXCLAMATION | MB_YESNO);
		if (res == IDYES)
		{
			std::error_code err;
			std::filesystem::copy(list_path, m_lists_path.GetString(),
								  std::filesystem::copy_options::overwrite_existing| std::filesystem::copy_options::recursive, err);
			if (err.value() != 0)
			{
				const auto& msg = fmt::format(load_string_resource(IDS_STRING_ERR_COPY), err.value(), list_path, m_lists_path.GetString());
				AfxMessageBox(msg.c_str(), MB_OK | MB_ICONSTOP);
			}
		}
	}

	if (m_plugins_path.IsEmpty())
		m_plugins_path = _T(".\\");

	if (m_plugins_path.Right(1) != '\\')
		m_plugins_path += '\\';

	if (m_plugins_web_update_path.IsEmpty())
		m_plugins_web_update_path = _T(".\\");

	if (m_plugins_web_update_path.Right(1) != '\\')
		m_plugins_web_update_path += '\\';

	if (m_plugins_settings_path.IsEmpty())
		m_plugins_settings_path = _T(".\\Settings\\");

	if (m_plugins_settings_path.Right(1) != '\\')
		m_plugins_settings_path += '\\';

	const auto& settings_path = GetConfig().get_string(true, REG_SAVE_SETTINGS_PATH);
	if (m_plugins_settings_path != settings_path.c_str()) //-V1051
	{
		auto res = AfxMessageBox(IDS_STRING_COPY_CONFIGS, MB_ICONEXCLAMATION | MB_YESNO);
		if (res == IDYES)
		{
			std::error_code err;
			std::filesystem::copy(settings_path, m_plugins_settings_path.GetString(),
								  std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive, err);
			if (err.value() != 0)
			{
				const auto& msg = fmt::format(load_string_resource(IDS_STRING_ERR_COPY), err.value(), settings_path, m_plugins_settings_path.GetString());
				AfxMessageBox(msg.c_str(), MB_ICONERROR | MB_OK);
			}
		}
	}

	GetConfig().set_string(true, REG_PLAYER, m_player.GetString());
	GetConfig().set_string(true, REG_FFPROBE, m_probe.GetString());
	GetConfig().set_string(true, REG_LISTS_PATH, m_lists_path.GetString());
	GetConfig().set_string(true, REG_OUTPUT_PATH, m_plugins_path.GetString());
	GetConfig().set_string(true, REG_WEB_UPDATE_PATH, m_plugins_web_update_path.GetString());
	GetConfig().set_string(true, REG_SAVE_SETTINGS_PATH, m_plugins_settings_path.GetString());

	return __super::OnApply();
}
