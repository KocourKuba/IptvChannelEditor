#include "pch.h"
#include "uri_base.h"
#include "utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void uri_base::set_uri(const std::wstring& url)
{
	clear();

	// https, udp, http, file, plugin_file
	static std::wregex re(LR"((https?:\/\/|plugin_file:\/\/)(.*))");
	std::wsmatch m;
	if (std::regex_match(url, m, re))
	{
		schema = m[1].str();
		path = m[2].str();
	}
}

std::wstring uri_base::get_filesystem_path(const std::wstring& root) const
{
	if (is_local())
	{
		std::wstring rpath = root + get_path();
		std::replace(rpath.begin(), rpath.end(), '/', '\\');
		return rpath;
	}

	return get_uri();
}
