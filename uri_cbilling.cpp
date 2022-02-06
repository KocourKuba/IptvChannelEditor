#include "pch.h"
#include "uri_cbilling.h"
#include "PlayListEntry.h"

#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto ACCOUNT_TEMPLATE = L"http://api.iptvx.tv/auth/info";
static constexpr auto ACCOUNT_HEADER_TEMPLATE = L"accept: */*\r\nx-public-key: {:s}";
static constexpr auto PLAYLIST_TEMPLATE = L"http://cbilling.pw/playlist/{:s}_otp_dev{:d}.m3u8";
static constexpr auto URI_TEMPLATE_HLS = L"http://{SUBDOMAIN}/s/{TOKEN}/{ID}.m3u8";
static constexpr auto URI_TEMPLATE_HLS2 = L"http://{SUBDOMAIN}/{ID}/video.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{SUBDOMAIN}/{ID}/mpegts?token={TOKEN}";
static constexpr auto EPG1_TEMPLATE = L"http://epg.ott-play.com/php/show_prog.php?f=cbilling/epg/{:s}.json";
static constexpr auto EPG1_TEMPLATE_JSON = L"http://epg.ott-play.com/cbilling/epg/{:s}.json";
static constexpr auto EPG2_TEMPLATE = L"https://api.iptvx.tv/epg/{:s}";
static constexpr auto EPG2_TEMPLATE_JSON = L"https://api.iptvx.tv/epg/{:s}";

void uri_cbilling::parse_uri(const std::wstring& url)
{
	// http://s01.iptvx.tv:8090/s/82s4fb5785dcf28dgd6ga681a94ba78f/pervyj.m3u8
	static std::wregex re_url_hls(LR"(^https?:\/\/(.+)\/s\/(.+)\/(.+)\.m3u8$)");

	std::wsmatch m;
	if (std::regex_match(url, m, re_url_hls))
	{
		set_template(true);
		set_domain(m[1].str());
		set_token(m[2].str());
		set_id(m[3].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_cbilling::get_templated(StreamSubType subType, const TemplateParams& params) const
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
					AppendArchive(url);
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

	ReplaceVars(url, params);
	return url;
}

std::wstring uri_cbilling::get_access_url(const std::wstring& /*login*/, const std::wstring& /*password*/) const
{
	return ACCOUNT_TEMPLATE;
}

std::wstring uri_cbilling::get_access_info_header() const
{
	return ACCOUNT_HEADER_TEMPLATE;
}


bool uri_cbilling::parse_access_info(const std::vector<BYTE>& json_data, std::list<AccountParams>& params) const
{
	using json = nlohmann::json;

	try
	{
		json js = json::parse(json_data);

		json js_data = js["data"];

		PutAccountParameter("package", js_data, params);
		PutAccountParameter("end_date", js_data, params);
		PutAccountParameter("devices_num", js_data, params);
		PutAccountParameter("server", js_data, params);
		PutAccountParameter("vod", js_data, params);

		return true;
	}
	catch (const json::parse_error&)
	{
		// parse errors are ok, because input may be random bytes
	}
	catch (const json::out_of_range&)
	{
		// out of range errors may happen if provided sizes are excessive
	}
	catch (const json::type_error&)
	{
		// type errors may happen if provided sizes are excessive
	}

	return false;
}

std::wstring uri_cbilling::get_epg1_uri(const std::wstring& id) const
{
	return fmt::format(EPG1_TEMPLATE, id);
}

std::wstring uri_cbilling::get_epg1_uri_json(const std::wstring& id) const
{
	return fmt::format(EPG1_TEMPLATE_JSON, id);
}

std::wstring uri_cbilling::get_epg2_uri(const std::wstring& id) const
{
	return fmt::format(EPG2_TEMPLATE, id);
}

std::wstring uri_cbilling::get_epg2_uri_json(const std::wstring& id) const
{
	return fmt::format(EPG2_TEMPLATE_JSON, id);
}

std::wstring uri_cbilling::get_playlist_template(bool first /*= true*/) const
{
	return PLAYLIST_TEMPLATE;
}
