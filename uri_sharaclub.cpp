#include "pch.h"
#include "uri_sharaclub.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto ACCOUNT_TEMPLATE = L"http://list.playtv.pro/api/dune-api5m.php?subscr={:s}-{:s}";
static constexpr auto PLAYLIST_TEMPLATE = L"http://list.playtv.pro/tv_live-m3u8/{:s}-{:s}";
static constexpr auto URI_TEMPLATE_HLS = L"http://{SUBDOMAIN}/live/{TOKEN}/{ID}/video.m3u8";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{SUBDOMAIN}/live/{TOKEN}/{ID}.ts";
static constexpr auto EPG1_TEMPLATE_JSON = L"http://api.sramtv.com/get/?type=epg&ch={:s}";
static constexpr auto EPG2_TEMPLATE_JSON = L"http://api.gazoni1.com/get/?type=epg&ch={:s}";


uri_sharaclub::uri_sharaclub()
{
	epg2 = true;
	epg_params[0]["epg_root"] = "";
}

void uri_sharaclub::parse_uri(const std::wstring& url)
{
	// http://em.gazoni1.com:80/live/s.277258.1d25esee4e77f0419432d2ed8eb0ee525/pervyHD/video.m3u8

	static std::wregex re_url(LR"(^https?:\/\/(.+)\/live\/(.+)\/(.+)\/.+\.m3u8$)");
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

std::wstring uri_sharaclub::get_templated_stream(StreamSubType subType, const TemplateParams& params) const
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

std::wstring uri_sharaclub::get_epg_uri_json(int epg_idx, const std::wstring& id, time_t for_time /*= 0*/) const
{
	return fmt::format(epg_idx == 0 ? EPG1_TEMPLATE_JSON : EPG2_TEMPLATE_JSON, id);
}

std::wstring uri_sharaclub::get_playlist_template(const PlaylistTemplateParams& params) const
{
	return fmt::format(PLAYLIST_TEMPLATE, params.login, params.password);
}

bool uri_sharaclub::parse_access_info(const PlaylistTemplateParams& params, std::list<AccountInfo>& info_list) const
{
	std::vector<BYTE> data;
	if (!utils::DownloadFile(fmt::format(ACCOUNT_TEMPLATE, params.login, params.password), data) || data.empty())
	{
		return false;
	}

	JSON_ALL_TRY
	{
		nlohmann::json parsed_json = nlohmann::json::parse(data);
		if (parsed_json.contains("status"))
		{
			AccountInfo info{ L"state", utils::utf8_to_utf16(parsed_json.value("status", "")) };
			info_list.emplace_back(info);
		}

		nlohmann::json js_data = parsed_json["data"];
		put_account_info("login", js_data, info_list);
		put_account_info("money", js_data, info_list);
		put_account_info("money_need", js_data, info_list);
		put_account_info("abon", js_data, info_list);

		return true;
	}
	JSON_ALL_CATCH;

	return false;
}
