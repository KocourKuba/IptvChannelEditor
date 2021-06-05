#include "StdAfx.h"
#include "uri.h"
#include "utils.h"
#include "Crc32.h"

void uri::set_uri(const std::wstring& url)
{
	uri::clear();

	// https, udp, http, file, file_plugin
	std::wregex re(LR"(([a-z_]+:\/\/)(.*))");
	std::wsmatch m;
	if (std::regex_match(url, m, re))
	{
		schema = m[1].str();
		path = m[2].str();
	}
}

std::wstring uri::get_icon_relative_path(LPCWSTR szRoot /*= nullptr*/) const
{
	if (is_local())
	{
		std::wstring rpath = szRoot ? szRoot : L"";
		rpath += get_path();

		std::wregex re_pf(LR"(\/)");
		return std::regex_replace(rpath, re_pf, LR"(\)");
	}

	return get_uri();
}

void uri_stream::clear()
{
	uri::clear();
	id = 0;
	templated = false;
	schema.clear();
	path.clear();
}

std::wstring uri_stream::get_ts_translated_url() const
{
	// http://ts://{SUBDOMAIN}/iptv/{UID}/204/index.m3u8 -> http://{SUBDOMAIN}/iptv/{UID}/204/index.m3u8
	// http://ts://rtmp.api.rt.com/hls/rtdru.m3u8 -> http://rtmp.api.rt.com/hls/rtdru.m3u8

	std::wregex re_ts(LR"(ts:\/\/)");
	return std::regex_replace(get_id_translated_url(), re_ts, L"");
}

std::wstring uri_stream::get_id_translated_url() const
{
	// templated url changed, custom is unchanged
	// http://ts://{SUBDOMAIN}/iptv/{UID}/{ID}/index.m3u8 -> http://ts://{SUBDOMAIN}/iptv/{UID}/204/index.m3u8

	std::wregex re_ch(LR"(\{ID\})");
	return schema + std::regex_replace(path, re_ch, utils::int_to_wchar(id));
}

void uri_stream::set_uri(const std::wstring& url)
{
	// http://ts://{SUBDOMAIN}/iptv/{UID}/205/index.m3u8
	std::wregex re(LR"([a-z_]+:\/\/ts:\/\/[a-zA-Z0-9\.\-|\{ID\}]+\/iptv\/[A-Z0-9|\{ID\}]+\/([0-9|\{ID\}]+)\/index.m3u8)");
	std::wsmatch m;
	if (std::regex_match(url, m, re))
	{
		templated = true;
		id = utils::wchar_to_int(m[1].str());
		// replace to http://ts://{SUBDOMAIN}/iptv/{UID}/{ID}/index.m3u8
		uri::set_uri(utils::URI_TEMPLATE);
	}
	else
	{
		// http://ts://rtmp.api.rt.com/hls/rtdru.m3u8
		templated = false;
		uri::set_uri(url);
	}

	hash = crc32_bitwise(url.c_str(), url.size());
}
