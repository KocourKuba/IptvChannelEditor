#include "StdAfx.h"
#include "AccessInfoPassDlg.h"
#include "IPTVChannelEditor.h"
#include "utils.h"
#include "json.hpp"
#include "PlayListEntry.h"

using json = nlohmann::json;

// CAccessDlg dialog

IMPLEMENT_DYNAMIC(CAccessInfoPassDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CAccessInfoPassDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_GET, &CAccessInfoPassDlg::OnBnClickedBtnGet)
END_MESSAGE_MAP()


CAccessInfoPassDlg::CAccessInfoPassDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_ACCESS_INFO_PASS, pParent)
{
}

void CAccessInfoPassDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_KEY, m_login);
	DDX_Text(pDX, IDC_EDIT_DOMAIN, m_password);
	DDX_Text(pDX, IDC_EDIT_STATUS, m_status);
	DDX_Text(pDX, IDC_EDIT_SUBSCRIPTION, m_subscription);
	DDX_Text(pDX, IDC_EDIT_BALANCE, m_balance);
	DDX_Text(pDX, IDC_EDIT_FORECAST, m_forecast);
	DDX_Control(pDX, IDC_BUTTON_GET, m_wndGet);
}

BOOL CAccessInfoPassDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	static std::wregex re(LR"((\w+)-(\w+))");
	std::wsmatch m;
	std::wstring token(m_token.GetString());
	if (std::regex_match(token, m, re))
	{
		m_login = m[1].str().c_str();
		m_password = m[2].str().c_str();
		UpdateData(FALSE);

		OnBnClickedBtnGet();
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CAccessInfoPassDlg::OnOK()
{
	UpdateData(TRUE);

	m_token.Format(_T("%s-%s"), m_login.GetString(), m_password.GetString());

	__super::OnOK();
}

void CAccessInfoPassDlg::OnBnClickedBtnGet()
{
	UpdateData(TRUE);

	const auto& access_url = fmt::format(_T("http://list.playtv.pro/api/dune-api5m.php?subscr={:s}-{:s}"),
								  m_login.GetString(),
								  m_password.GetString());
	std::vector<BYTE> data;
	if (!utils::DownloadFile(utils::utf16_to_utf8(access_url), data) || data.empty())
		return;

	try
	{
		json js = json::parse(data);
		json js_data = js["data"];

		m_status = js.value("status", "").c_str();
		m_subscription = utils::utf8_to_utf16(js_data.value("abon", "")).c_str();
		m_balance.Format(_T("%hs RUR"), js_data.value("money", "").c_str());
		m_forecast.Format(_T("%hs RUR"), js_data.value("money_need", "").c_str());
	}
	catch (const json::parse_error&)
	{
		// parse errors are ok, because input may be random bytes
	}
	catch (const json::out_of_range&)
	{
		// out of range errors may happen if provided sizes are excessive
	}

	// Short (OTTPplay.es) format
	// #EXTM3U
	// #EXTINF:0 tvg-rec="3",Первый HD
	// #EXTGRP:Общие
	// http://list.playtv.pro/tv_live-m3u8/-token---pass-

	const auto& pl_url = fmt::format(_T("http://list.playtv.pro/tv_live-m3u8/{:s}-{:s}"),
									 m_login.GetString(),
									 m_password.GetString());

	data.clear();
	std::unique_ptr<std::istream> pl_stream;
	if (utils::DownloadFile(utils::utf16_to_utf8(pl_url), data))
	{
		utils::vector_to_streambuf<char> buf(data);
		pl_stream = std::make_unique<std::istream>(&buf);

		if (pl_stream && pl_stream->good())
		{
			int step = 0;
			std::string line;
			auto entry = std::make_unique<PlaylistEntry>(m_streamType, theApp.GetAppPath(utils::PLUGIN_ROOT));
			while (std::getline(*pl_stream, line))
			{
				utils::string_rtrim(line, "\r");
				if (!entry->Parse(line)) continue;

				const auto& access_key = entry->get_access_key();
				const auto& domain = entry->get_domain();
				if (!access_key.empty() && !domain.empty())
				{
					m_accessKey = access_key.c_str();
					m_domain = domain.c_str();
					break;
				}
			}
		}
	}

	UpdateData(FALSE);
}
