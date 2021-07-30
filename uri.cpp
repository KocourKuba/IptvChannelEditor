#include "StdAfx.h"
#include "uri.h"
#include "utils.h"
#include "Crc32.h"

void uri::set_uri(const std::string& url)
{
	uri::clear();

	// https, udp, http, file, file_plugin
	static std::regex re(R"(([a-z_]+:\/\/)(.*))");
	std::smatch m;
	if (std::regex_match(url, m, re))
	{
		schema = m[1].str();
		path = m[2].str();
	}
}

std::string uri::get_icon_absolute_path(const std::string& root) const
{
	if (is_local())
	{
		std::string rpath = root + get_path();

		static std::regex re_pf(R"(\/)");
		return std::regex_replace(rpath, re_pf, R"(\)");
	}

	return get_uri();
}

std::string uri::get_icon_absolute_path(const std::wstring& root) const
{
	if (is_local())
	{
		std::string rpath = utils::utf16_to_utf8(root) + get_path();

		static std::regex re_pf(R"(\/)");
		return std::regex_replace(rpath, re_pf, R"(\)");
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

std::string uri_stream::get_ts_translated_url() const
{
	// http://ts://{SUBDOMAIN}/iptv/{UID}/204/index.m3u8 -> http://{SUBDOMAIN}/iptv/{UID}/204/index.m3u8
	// http://ts://rtmp.api.rt.com/hls/rtdru.m3u8 -> http://rtmp.api.rt.com/hls/rtdru.m3u8

	static std::regex re_ts(R"(ts:\/\/)");
	return std::regex_replace(get_id_translated_url(), re_ts, "");
}

std::string uri_stream::get_id_translated_url() const
{
	// templated url changed, custom is unchanged
	// http://ts://{SUBDOMAIN}/iptv/{UID}/{ID}/index.m3u8 -> http://ts://{SUBDOMAIN}/iptv/{UID}/204/index.m3u8

	std::regex re_ch(R"(\{ID\})");
	return schema + std::regex_replace(path, re_ch, utils::int_to_char(id));
}

void uri_stream::set_uri(const std::string& url)
{
	// http://ts://{SUBDOMAIN}/iptv/{UID}/205/index.m3u8
	static std::regex re(R"([a-z_]+:\/\/ts:\/\/[a-zA-Z0-9\.\-|\{ID\}]+\/iptv\/[A-Z0-9|\{ID\}]+\/([0-9|\{ID\}]+)\/index.m3u8)");
	std::smatch m;
	if (std::regex_match(url, m, re))
	{
		templated = true;
		id = utils::char_to_int(m[1].str());
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
