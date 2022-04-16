#include "pch.h"
#include "uri_cbilling.h"
#include "PlayListEntry.h"

#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto ACCOUNT_TEMPLATE = L"http://protected-api.com/auth/info";
static constexpr auto ACCOUNT_HEADER_TEMPLATE = L"accept: */*\r\nx-public-key: {:s}";
static constexpr auto PLAYLIST_TEMPLATE = L"http://247on.cc/playlist/{:s}_otp_dev{:d}.m3u8";
static constexpr auto URI_TEMPLATE_HLS = L"http://{SUBDOMAIN}/s/{TOKEN}/{ID}.m3u8";
static constexpr auto URI_TEMPLATE_HLS2 = L"http://{SUBDOMAIN}/{ID}/video.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{SUBDOMAIN}/{ID}/mpegts?token={TOKEN}";
static constexpr auto EPG1_TEMPLATE_JSON = L"http://protected-api.com/epg/{:s}/?date=";

uri_cbilling::uri_cbilling()
{
	epg_params[0]["epg_root"] = "";
}

void uri_cbilling::parse_uri(const std::wstring& url)
{
	// http://s01.iptvx.tv:8090/s/82s4fb5785dcf28dgd6ga681a94ba78f/pervyj.m3u8
	static std::wregex re_url_hls(LR"(^https?:\/\/(.+)\/s\/(.+)\/(.+)\.m3u8$)");

	std::wsmatch m;
	if (std::regex_match(url, m, re_url_hls))
	{
		templated = true;
		domain = std::move(m[1].str());
		token = std::move(m[2].str());
		id = std::move(m[3].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_cbilling::get_templated_stream(StreamSubType subType, const TemplateParams& params) const
{
	std::wstring url;

	if (is_template())
	{
		auto& new_params = const_cast<TemplateParams&>(params);
		std::wstring no_port(params.domain);
		if (auto pos = no_port.find(':'); pos != std::wstring::npos)
		{
			no_port = no_port.substr(0, pos);
		}

		switch (subType)
		{
			case StreamSubType::enHLS:
				url = URI_TEMPLATE_HLS;
				if (params.shift_back)
				{
					append_archive(url);
				}
				break;
			case StreamSubType::enHLS2:
				url = URI_TEMPLATE_HLS2;
				new_params.domain = no_port;
				if (params.shift_back)
				{
					utils::string_replace_inplace(url, L"video.m3u8", L"video-{START}-10800.m3u8");
				}
				break;
			case StreamSubType::enMPEGTS:
				url = URI_TEMPLATE_MPEG;
				new_params.domain = std::move(no_port);
				if (params.shift_back)
				{
					utils::string_replace_inplace(url, L"mpegts", L"archive-{START}-10800.ts");
				}
				break;
		}
	}
	else
	{
		url = get_uri();
	}

	replace_vars(url, params);
	return url;
}

std::wstring uri_cbilling::get_access_info_header() const
{
	return ACCOUNT_HEADER_TEMPLATE;
}

bool uri_cbilling::parse_access_info(const PlaylistTemplateParams& params, std::list<AccountInfo>& info_list) const
{
	auto& header = fmt::format(ACCOUNT_HEADER_TEMPLATE, params.password);
	std::vector<BYTE> data;
	if (!utils::DownloadFile(ACCOUNT_TEMPLATE, data, &header) || data.empty())
	{
		return false;
	}

	JSON_ALL_TRY
	{
		nlohmann::json parsed_json = nlohmann::json::parse(data);
		nlohmann::json js_data = parsed_json["data"];

		put_account_info("package", js_data, info_list);
		put_account_info("end_date", js_data, info_list);
		put_account_info("devices_num", js_data, info_list);
		put_account_info("server", js_data, info_list);
		put_account_info("vod", js_data, info_list);

		return true;
	}
	JSON_ALL_CATCH;

	return false;
}

std::wstring uri_cbilling::get_epg_uri_json(int epg_idx, const std::wstring& id, time_t for_time /*= 0*/) const
{
	COleDateTime dt(for_time ? for_time : COleDateTime::GetCurrentTime());
	return fmt::format(EPG1_TEMPLATE_JSON, id, dt.GetYear(), dt.GetMonth(), dt.GetDay());
}

std::wstring uri_cbilling::get_playlist_template(const PlaylistTemplateParams& params) const
{
	return fmt::format(PLAYLIST_TEMPLATE, params.password, params.device);
}
