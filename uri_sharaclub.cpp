#include "StdAfx.h"
#include "uri_sharaclub.h"

static constexpr auto ACCOUNT_TEMPLATE = "http://list.playtv.pro/api/dune-api5m.php?subscr={:s}-{:s}";
static constexpr auto PLAYLIST_TEMPLATE = "http://list.playtv.pro/tv_live-m3u8/{:s}-{:s}";
static constexpr auto URI_TEMPLATE_HLS = "http://{SUBDOMAIN}/live/{TOKEN}/{ID}/video.m3u8";
static constexpr auto URI_TEMPLATE_MPEG = "http://{SUBDOMAIN}/live/{TOKEN}/{ID}.ts";
static constexpr auto EPG1_TEMPLATE = "http://api.sramtv.com/get/?type=epg&ch={:s}&date=&date={:4d}-{:02d}-{:02d}";
static constexpr auto EPG2_TEMPLATE = "http://api.gazoni1.com/get/?type=epg&ch={:s}&date={:4d}-{:02d}-{:02d}";


void uri_sharaclub::parse_uri(const std::string& url)
{
	// http://em.gazoni1.com:80/live/s.277258.1d25esee4e77f0419432d2ed8eb0ee525/pervyHD/video.m3u8

	static std::regex re_url(R"(^https?:\/\/(.+)\/live\/(.+)\/(.+)\/.+\.m3u8$)");
	std::smatch m;
	if (std::regex_match(url, m, re_url))
	{
		set_template(true);
		domain = m[1].str();
		token = m[2].str();
		id = m[3].str();
		return;
	}

	uri_stream::parse_uri(url);
}

std::string uri_sharaclub::get_templated(StreamSubType subType, const TemplateParams& params) const
{
	std::string url;

	if (!is_template())
	{
		url = get_uri();
	}
	else
	{
		switch (subType)
		{
			case StreamSubType::enHLS:
				url = URI_TEMPLATE_HLS;
				break;
			case StreamSubType::enMPEGTS:
				url = URI_TEMPLATE_MPEG;
				break;
		}

		// http://{SUBDOMAIN}/live/{TOKEN}/{ID}/video.m3u8
		// http://{SUBDOMAIN}/live/{TOKEN}/{ID}.ts
		utils::string_replace_inplace(url, "{SUBDOMAIN}", params.domain);
		utils::string_replace_inplace(url, "{TOKEN}", params.token);
		utils::string_replace_inplace(url, "{ID}", get_id());
	}

	if (params.shift_back)
	{
		url += fmt::format("?utc={:d}&lutc={:d}", params.shift_back, _time32(nullptr));
	}

	return url;
}

std::string uri_sharaclub::get_epg1_uri(const std::string& id) const
{
	COleDateTime dt = COleDateTime::GetCurrentTime();
	return fmt::format(EPG1_TEMPLATE, id, dt.GetYear(), dt.GetMonth(), dt.GetDay());
}

std::string uri_sharaclub::get_epg2_uri(const std::string& id) const
{
	COleDateTime dt = COleDateTime::GetCurrentTime();
	return fmt::format(EPG2_TEMPLATE, id, dt.GetYear(), dt.GetMonth(), dt.GetDay());
}

std::string uri_sharaclub::get_playlist_url(const std::string& login, const std::string& password) const
{
	return fmt::format(PLAYLIST_TEMPLATE, login.c_str(), password.c_str());
}

std::string uri_sharaclub::get_access_url(const std::string& login, const std::string& password) const
{
	return fmt::format(ACCOUNT_TEMPLATE, login.c_str(), password.c_str());
}
