/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2022): sharky72 (https://github.com/KocourKuba)

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

#pragma once
#include "uri_base.h"

#include "UtilsLib\utils.h"

class IconContainer
{
public:
	IconContainer() = default;
	IconContainer(std::wstring root) : root_path(root) {}
	IconContainer(const IconContainer& src)
	{
		*this = src;
	}

	IconContainer(IconContainer&& src) noexcept
	{
		*this = std::move(src);
	}

public:
	const IconContainer& operator=(const IconContainer& src)
	{
		if (this != &src)
		{
			icon_uri = src.icon_uri;
			root_path = src.root_path;
		}

		return *this;
	}

	IconContainer& operator=(IconContainer&& src) noexcept
	{
		if (this != &src)
		{
			icon_uri = src.icon_uri;
			root_path = std::move(src.root_path);
		}
		return *this;
	}

	void set_root_path(const std::wstring& root) { root_path = root; }
	const std::wstring& get_root_path() const { return root_path; }

	const uri_base& get_icon_uri() const { return icon_uri; }
	uri_base& get_icon_uri() { return icon_uri; }

	std::wstring get_icon_absolute_path() const { return get_icon_uri().get_filesystem_path(root_path); };

	void convert_https()
	{
		if (icon_uri.get_schema() == L"https://")
			icon_uri.set_schema(L"http://");
	}

	void set_icon_uri(const uri_base& val, bool make_http = true)
	{
		icon_uri = val;
		if (make_http)
			convert_https();
	}

	void set_icon_uri(const std::wstring& val, bool make_http = true)
	{
		icon_uri.set_uri(val);
		if (make_http)
			convert_https();
	}

private:
	uri_base icon_uri;
	std::wstring root_path;
};