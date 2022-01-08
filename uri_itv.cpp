#include "pch.h"
#include "uri_itv.h"

#include "UtilsLib\json.hpp"
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
static constexpr auto EPG1_TEMPLATE = L"http://api.itv.live/epg/{:s}";
static constexpr auto EPG1_TEMPLATE_JSON = EPG1_TEMPLATE;

void uri_itv::parse_uri(const std::wstring& url)
{
	// http://cloud15.05cdn.wf/ch378/video.m3u8?token=5bdbc7125f6ed805a5fd238b9f885d1a3c67a6594
	// http://cloud15.05cdn.wf/ch378/mpegts?token=5bdbc7125f6ed805a5fd238b9f885d1a3c67a6594
	static std::wregex re_url_hls(LR"(^https?:\/\/(.+)\/(.+)\/[^\?]+\?token=(.+)$)");
	std::wsmatch m;
	if (std::regex_match(url, m, re_url_hls))
	{
		set_template(true);
		set_domain(m[1].str());
		set_id(m[2].str());
		set_token(m[3].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_itv::get_templated(StreamSubType subType, TemplateParams& params) const
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
			AppendArchive(url);
		}
	}

	ReplaceVars(url, params);

	return url;
}

std::wstring uri_itv::get_access_url(const std::wstring& login, const std::wstring& password) const
{
	return fmt::format(ACCOUNT_TEMPLATE, password);
}

std::wstring uri_itv::get_epg1_uri(const std::wstring& id) const
{
	return fmt::format(EPG1_TEMPLATE, id);
}

std::wstring uri_itv::get_epg1_uri_json(const std::wstring& id) const
{
	return fmt::format(EPG1_TEMPLATE_JSON, id);
}

std::wstring uri_itv::get_playlist_template(bool first /*= true*/) const
{
	return PLAYLIST_TEMPLATE;
}

bool uri_itv::parse_access_info(const std::vector<BYTE>& json_data, std::map<std::wstring, std::wstring>& params) const
{
	using json = nlohmann::json;

	try
	{
		json js = json::parse(json_data);

		json js_data = js["user_info"];
		for (auto& x : js_data.items())
		{
			if (x.value().is_number_integer())
				params.emplace(utils::utf8_to_utf16(x.key()), std::to_wstring(x.value().get<int>()));
			if (x.value().is_number_float())
				params.emplace(utils::utf8_to_utf16(x.key()), std::to_wstring(x.value().get<float>()));
			else if (x.value().is_string())
				params.emplace(utils::utf8_to_utf16(x.key()), utils::utf8_to_utf16(x.value().get<std::string>()));
		}

		std::wstring subscription;
		if (!js.contains("package_info"))
		{
			subscription = L"No packages";
		}
		else
		{
			json pkg_data = js["package_info"];
			for (const auto& item : pkg_data)
			{
				if (!subscription.empty())
					subscription += L", ";

				subscription += fmt::format(L"{:s}", utils::utf8_to_utf16(item.value("name", "")));
			}
		}

		params.emplace(L"package_info", subscription);

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

	return false;
}
