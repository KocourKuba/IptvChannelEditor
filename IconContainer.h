#pragma once
#include "uri_base.h"

#include "UtilsLib\utils.h"

class IconContainer
{
public:
	static constexpr auto ICON_URL = "icon_url";

public:
	IconContainer() = default;
	IconContainer(const std::wstring& root) : root_path(root) {}
	IconContainer(const IconContainer& src)
	{
		*this = src;
	}

	IconContainer(IconContainer&& src)
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

	IconContainer& operator=(IconContainer&& src)
	{
		if (this != &src)
		{
			icon_uri = std::move(src.icon_uri);
			root_path = std::move(src.root_path);
		}
		return *this;
	}

	void set_root_path(const std::wstring& root) { root_path = root; }
	const std::wstring& get_root_path() const { return root_path; }

	const uri_base& get_icon_uri() const { return icon_uri; }

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

	void set_icon_uri(const std::wstring& val) { icon_uri.set_uri(val); }

private:
	uri_base icon_uri;
	std::wstring root_path;
};