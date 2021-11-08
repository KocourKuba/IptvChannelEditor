#include "StdAfx.h"
#include "uri_itv.h"
#include "utils.h"

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

std::wstring uri_itv::get_templated(StreamSubType subType, const TemplateParams& params) const
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
		}
	}
	else
	{
		url = get_uri();
		if (params.shift_back)
		{
			url += L"&utc={START}&lutc={NOW}";
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
