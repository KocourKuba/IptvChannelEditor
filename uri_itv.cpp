#include "pch.h"
#include "uri_itv.h"

#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto ACCOUNT_TEMPLATE = L"http://api.itv.live/data/{:s}";
static constexpr auto PLAYLIST_TEMPLATE = L"http://itv.ooo/p/{:s}/hls.m3u8";
static constexpr auto URI_TEMPLATE_HLS = L"http://{SUBDOMAIN}/{ID}/video.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{SUBDOMAIN}/{ID}/mpegts?token={TOKEN}";
static constexpr auto URI_TEMPLATE_ARCH_HLS = L"http://{SUBDOMAIN}/{ID}/archive-{START}-10800.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_ARCH_MPEG = L"http://{SUBDOMAIN}/{ID}/archive-{START}-10800.ts?token={TOKEN}";
static constexpr auto EPG1_TEMPLATE_JSON = L"http://api.itv.live/epg/{:s}";

void uri_itv::parse_uri(const std::wstring& url)
{
	// http://cloud15.05cdn.wf/ch378/video.m3u8?token=5bdbc7125f6ed805a5fd238b9f885d1a3c67a6594
	// http://cloud15.05cdn.wf/ch378/mpegts?token=5bdbc7125f6ed805a5fd238b9f885d1a3c67a6594
	static std::wregex re_url_hls(LR"(^https?:\/\/(.+)\/(.+)\/[^\?]+\?token=(.+)$)");
	std::wsmatch m;
	if (std::regex_match(url, m, re_url_hls))
	{
		templated = true;
		domain = std::move(m[1].str());
		id = std::move(m[2].str());
		token = std::move(m[3].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_itv::get_templated_stream(StreamSubType subType, const TemplateParams& params) const
{
	std::wstring url;

	if (is_template())
	{
		switch (subType)
		{
			case StreamSubType::enHLS:
				url = params.shift_back ? URI_TEMPLATE_ARCH_HLS : URI_TEMPLATE_HLS;
				break;
			case StreamSubType::enMPEGTS:
				url = params.shift_back ? URI_TEMPLATE_ARCH_MPEG : URI_TEMPLATE_MPEG;
				break;
			default:
				break;
		}
	}
	else
	{
		url = get_uri();
		if (params.shift_back)
		{
			append_archive(url);
		}
	}

	replace_vars(url, params);

	return url;
}

std::wstring uri_itv::get_epg_uri_json(bool first, const std::wstring& id, time_t for_time /*= 0*/) const
{
	return fmt::format(EPG1_TEMPLATE_JSON, id);
}

std::wstring uri_itv::get_playlist_template(const PlaylistTemplateParams& params) const
{
	return fmt::format(PLAYLIST_TEMPLATE, params.password);
}

bool uri_itv::parse_access_info(const PlaylistTemplateParams& params, std::list<AccountInfo>& info_list) const
{
	std::vector<BYTE> data;
	if (!utils::DownloadFile(fmt::format(ACCOUNT_TEMPLATE, params.password), data) || data.empty())
	{
		return false;
	}

	JSON_ALL_TRY
	{
		nlohmann::json parsed_json = nlohmann::json::parse(data);
		nlohmann::json js_data = parsed_json["user_info"];

		put_account_info("login", js_data, info_list);
		put_account_info("pay_system", js_data, info_list);
		put_account_info("cash", js_data, info_list);

		std::wstring subscription;
		if (!parsed_json.contains("package_info"))
		{
			subscription = L"No packages";
		}
		else
		{
			nlohmann::json pkg_data = parsed_json["package_info"];
			for (const auto& item : pkg_data)
			{
				if (!subscription.empty())
					subscription += L", ";

				subscription += fmt::format(L"{:s}", utils::utf8_to_utf16(item.value("name", "")));
			}
		}

		AccountInfo info{ L"package_info", subscription };
		info_list.emplace_back(info);

		return true;
	}
	JSON_ALL_CATCH;

	return false;
}

const nlohmann::json& uri_itv::get_epg_root(bool first, const nlohmann::json& epg_data) const
{
	return epg_data["res"];
}

std::string uri_itv::get_epg_name(bool first, const nlohmann::json& val) const
{
	return get_json_value("title", val);
}

std::string uri_itv::get_epg_desc(bool first, const nlohmann::json& val) const
{
	return get_json_value("desc", val);
}

time_t uri_itv::get_epg_time_start(bool first, const nlohmann::json& val) const
{
	return get_json_int_value("startTime", val);
}

time_t uri_itv::get_epg_time_end(bool first, const nlohmann::json& val) const
{
	return get_json_int_value("stopTime", val);
}
