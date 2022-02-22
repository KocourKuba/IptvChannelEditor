#include "pch.h"
#include "uri_oneott.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// http://{DOMAIN}/PinApi/{LOGIN}/{PASSWORD}
static constexpr auto ACCOUNT_TEMPLATE = L"http://list.1ott.net/PinApi/{:s}/{:s}";
static constexpr auto PLAYLIST_TEMPLATE = L"http://list.1ott.net/api/{:s}/high/ottplay.m3u8";
static constexpr auto URI_TEMPLATE_HLS = L"http://{SUBDOMAIN}/~{TOKEN}/{ID}/hls/pl.m3u8";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{SUBDOMAIN}/~{TOKEN}/{ID}";
static constexpr auto EPG1_TEMPLATE_JSON = L"http://epg.propg.net/{:s}/epg2/{:4d}-{:02d}-{:02d}";
static constexpr auto EPG2_TEMPLATE_JSON = L"http://epg.ott-play.com/1ott/epg/{:s}.json";


void uri_oneott::parse_uri(const std::wstring& url)
{
	//http://rr2.1ott.net/~109dab8c798d546s8dc9c41b3c3af80d59a/35985/hls/pl.m3u8

	static std::wregex re_url(LR"(^https?:\/\/(.+)\/~(.+)\/(.+)\/hls\/.+\.m3u8$)");
	std::wsmatch m;
	if (std::regex_match(url, m, re_url))
	{
		templated = true;
		domain = std::move(m[1].str());
		token = std::move(m[2].str());
		id = std::move(m[3].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_oneott::get_templated_stream(StreamSubType subType, const TemplateParams& params) const
{
	auto& url = get_uri();

	if (is_template())
	{
		switch (subType)
		{
			case StreamSubType::enHLS:
				url = URI_TEMPLATE_HLS;
				break;
			case StreamSubType::enMPEGTS:
				url = URI_TEMPLATE_MPEG;
				break;
			default:
				break;
		}
	}

	if (params.shift_back)
	{
		append_archive(url);
	}

	replace_vars(url, params);

	return url;
}

std::wstring uri_oneott::get_epg_uri_json(bool first, const std::wstring& id) const
{
	COleDateTime dt = COleDateTime::GetCurrentTime();
	return fmt::format(first ? EPG1_TEMPLATE_JSON : EPG2_TEMPLATE_JSON, id, dt.GetYear(), dt.GetMonth(), dt.GetDay());
}

std::wstring uri_oneott::get_playlist_template(const PlaylistTemplateParams& params) const
{
	return fmt::format(PLAYLIST_TEMPLATE, params.token);
}

bool uri_oneott::parse_access_info(const PlaylistTemplateParams& params, std::list<AccountInfo>& info_list) const
{
	std::vector<BYTE> data;
	if (!utils::DownloadFile(fmt::format(ACCOUNT_TEMPLATE, params.login, params.password), data) || data.empty())
	{
		return false;
	}

	JSON_TRY
	{
		nlohmann::json parsed_json = nlohmann::json::parse(data);
		if (parsed_json.contains("token"))
		{
			const auto& token = utils::utf8_to_utf16(parsed_json.value("token", ""));
			AccountInfo info{ L"token", token };
			info_list.emplace_back(info);

			PlaylistTemplateParams param;
			param.token = token;

			AccountInfo url{ L"url", get_playlist_template(param) };
			info_list.emplace_back(url);
		}

		return true;
	}
	JSON_CATCH;

	return false;
}
