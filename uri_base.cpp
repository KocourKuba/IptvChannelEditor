#include "StdAfx.h"
#include "uri_base.h"
#include "utils.h"

void uri_base::set_uri(const std::string& url)
{
	clear();

	// https, udp, http, file, plugin_file
	static std::regex re(R"((http?:\/\/)(.*))");
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

std::string uri_base::get_filesystem_path(const std::wstring& root) const
{
	if (is_local())
	{
		std::string rpath = utils::utf16_to_utf8(root) + get_path();
		std::replace(rpath.begin(), rpath.end(), '/', '\\');
		return rpath;
	}

	return get_uri();
}
