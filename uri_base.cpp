#include "StdAfx.h"
#include "uri_base.h"
#include "utils.h"

void uri_base::set_uri(const std::string& url)
{
	clear();

	// https, udp, http, file, plugin_file
	static std::regex re(R"((http?:\/\/|plugin_file:\/\/)(.*))");
	std::smatch m;
	if (std::regex_match(url, m, re))
	{
		schema = m[1].str();
		path = m[2].str();
	}
}

std::string uri_base::get_filesystem_path(const std::string& root) const
{
	if (is_local())
	{
		std::string rpath = root + get_path();
		std::replace(rpath.begin(), rpath.end(), '/', '\\');
		return rpath;
	}

	return get_uri();
}

std::wstring uri_base::get_filesystem_path(const std::wstring& root) const
{
	if (is_local())
	{
		std::wstring rpath = root + utils::utf8_to_utf16(get_path());
		std::replace(rpath.begin(), rpath.end(), '/', '\\');
		return rpath;
	}

	return utils::utf8_to_utf16(get_uri());
}
