#pragma once
#include "uri_base.h"
#include "utils.h"

class IconContainer
{
public:
	static constexpr auto ICON_URL = "icon_url";

public:
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
	IconContainer* operator=(const IconContainer& src)
	{
		if (this != &src)
		{
			icon_uri = src.icon_uri;
		}

		return this;
	}

	IconContainer* operator=(IconContainer&& src)
	{
		if (this != &src)
		{
			icon_uri = std::move(src.icon_uri);
		}
		return this;
	}

	void set_root_path(const std::wstring& root) { root_path = root; }
	const uri_base& get_icon_uri() const { return icon_uri; }

	std::wstring get_icon_absolute_path() const { return get_icon_uri().get_filesystem_path(root_path); };

	void set_icon_uri(const uri_base& val) { icon_uri = val; }
	void set_icon_uri(const std::string& val) { icon_uri.set_uri(val); }
	void set_icon_uri(const std::wstring& val) { icon_uri.set_uri(utils::utf16_to_utf8(val)); }

private:
	uri_base icon_uri;
	std::wstring root_path;
};