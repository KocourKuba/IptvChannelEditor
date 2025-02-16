/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2025): sharky72 (https://github.com/KocourKuba)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "pch.h"
#include "uri_base.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void uri_base::set_uri(const std::wstring& url)
{
	// https, udp, http, file, plugin_file
	static boost::wregex re(LR"((https?:\/\/|plugin_file:\/\/)(.*))");
	boost::wsmatch m;
	if (boost::regex_match(url, m, re))
	{
		scheme = m[1].str();
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
