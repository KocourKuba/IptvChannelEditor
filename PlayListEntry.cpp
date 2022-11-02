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

#include "pch.h"
#include "PlayListEntry.h"
#include "UtilsLib\utils.h"
#include "UtilsLib\rapidxml_value.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static std::map<std::wstring, m3u_entry::info_tags> id_tags = {
	{ L"channel-id", m3u_entry::info_tags::tag_channel_id      },
	{ L"CUID",       m3u_entry::info_tags::tag_cuid            },
	{ L"tvg-chno",   m3u_entry::info_tags::tag_tvg_chno        },
	{ L"tvg-id",     m3u_entry::info_tags::tag_tvg_id          },
	{ L"tvg-name",   m3u_entry::info_tags::tag_tvg_name        },
	{ L"name",       m3u_entry::info_tags::tag_directive_title },
};

static std::array<m3u_entry::info_tags, 3> epg_search_tags =
{
	m3u_entry::info_tags::tag_tvg_id,
	m3u_entry::info_tags::tag_tvg_name,
	m3u_entry::info_tags::tag_directive_title,
};

static std::array<m3u_entry::info_tags, 6> archive_search_tags =
{
	m3u_entry::info_tags::tag_catchup_days,
	m3u_entry::info_tags::tag_catchup_time,
	m3u_entry::info_tags::tag_timeshift,
	m3u_entry::info_tags::tag_arc_timeshift,
	m3u_entry::info_tags::tag_arc_time,
	m3u_entry::info_tags::tag_tvg_rec,
};

bool PlaylistEntry::Parse(const std::string& str)
{
	if (str.empty()) return false;

	m3uEntry.parse(str);

	bool result = false;
	switch (m3uEntry.get_directive())
	{
		case m3u_entry::directives::ext_header:
		{
			const auto& tags = m3uEntry.get_tags();
			if (const auto& pair = tags.find(m3u_entry::info_tags::tag_url_logo); pair != tags.end())
			{
				if (playlist)
					playlist->logo_root = pair->second;
			}
			break;
		}
		case m3u_entry::directives::ext_pathname:
		{
			parent_plugin->parse_stream_uri(utils::utf8_to_utf16(str), this);
			result = is_valid();
			if (result && category.empty())
			{
				set_category(load_string_resource(IDS_STRING_UNSET));
			}
			break;
		}
		case m3u_entry::directives::ext_group:
		{
			if (category.empty())
			{
				category = m3uEntry.get_dvalue();
				check_adult(category);
			}
			break;
		}
		case m3u_entry::directives::ext_info:
		{
			const auto& tags = m3uEntry.get_tags();

			// #EXTINF:-1 timeshift="14" catchup-days="14" catchup-type="flussonic" tvg-id="pervy"  group-title="Общие" tvg-logo="http://pl.ottglanz.tv:80/icon/2214.png",Первый HD
			// #EXTINF:0 tvg-name="ch002" tvg-logo="http://1usd.tv/images/chIcons/2.png" timeshift="4", Россия 1 HD
			// #EXTINF:0 CUID="1" tvg-name="1:458" tvg-id="1:458" arc-time="120" catchup="shift" catchup-days="5"
			//           tvg-logo="http://stb.fox-tv.fun/images/logo_chanel/1RUSTV200.png" group-title="Общие",Первый канал

			if (!m3uEntry.get_dir_title().empty())
			{
				set_title(utils::utf8_to_utf16(m3uEntry.get_dir_title()));
			}

			search_group(tags);
			search_archive(tags);
			search_epg(tags);
			search_logo(tags);
			search_catchup(tags);
			break;
		}
		case m3u_entry::directives::ext_vlcopt:
			break;

		default:
			break;
	}

	if (result)
	{
		// special cases after parsing
		switch (parent_plugin->get_plugin_type())
		{
		case PluginType::enIptvOnline:
			if (get_epg_id(0).front() == 'X')
			{
				set_epg_id(0, get_epg_id(0).substr(1));
			}
			break;
		case PluginType::enSharavoz:
		case PluginType::enOneOtt:
		case PluginType::enOttclub:
			set_epg_id(0, get_id());
			break;
		default:
			break;
		}

		switch (parent_plugin->get_plugin_type())
		{
			case PluginType::enOttclub:
				set_icon_uri(fmt::format(L"http://{:s}/images/{:s}.png", get_domain(), get_id()));
				break;
			case PluginType::enKineskop:
				set_icon_uri(boost::regex_replace(get_icon_uri().get_uri(), boost::wregex(LR"(http:\/\/\w{2}\.(.*))"), L"http://$1"));
				break;
			default:
				break;
		}

		if (!parent_plugin->get_epg_parameter(1).epg_url.empty())
		{
			set_epg_id(1, get_epg_id(0));
		}
	}

	return result;
}

void PlaylistEntry::search_id(const std::wstring& search_tag)
{
	const m3u_tags& tags = m3uEntry.get_tags();
	if (const auto& pair = id_tags.find(search_tag); pair != id_tags.end())
	{
		if (const auto& tag_pair = tags.find(pair->second); tag_pair != tags.end())
		{
			if (!tag_pair->second.empty())
				set_id(utils::utf8_to_utf16(tag_pair->second));
		}
	}
}

void PlaylistEntry::search_group(const m3u_tags& tags)
{
	if (const auto& pair = tags.find(m3u_entry::info_tags::tag_group_title); pair != tags.end())
	{
		category = pair->second;
		if (category.empty())
		{
			set_category(load_string_resource(IDS_STRING_UNSET));
		}
		else
		{
			check_adult(category);
		}
	}
}

void PlaylistEntry::search_archive(const m3u_tags& tags)
{
	// priority -> catchup_days -> catchup_time -> tag_timeshift ... -> tvg_rec
	for (const auto& tag : archive_search_tags)
	{
		const auto& pair = tags.find(tag);
		if (pair != tags.end() && !pair->second.empty())
		{
			int day = utils::char_to_int(pair->second);
			if (tag == m3u_entry::info_tags::tag_catchup_time)
				day /= 86400;
			set_archive_days(day);
			break;
		}
	}
}

void PlaylistEntry::search_epg(const m3u_tags& tags)
{
	// priority -> tvg_id -> tvg_name -> title
	for (const auto& tag : epg_search_tags)
	{
		const auto& pair = tags.find(tag);
		if (pair != tags.end() && !pair->second.empty())
		{
			set_epg_id(0, utils::utf8_to_utf16(pair->second));
			break;
		}
	}
}

void PlaylistEntry::search_logo(const m3u_tags& tags)
{
	if (const auto& pair = tags.find(m3u_entry::info_tags::tag_tvg_logo); pair != tags.end())
	{
		std::string icon_uri;

		if (playlist)
		{
			if (playlist->logo_root.empty())
				icon_uri = utils::string_replace<char>(pair->second, "//epg.it999.ru/img/", "//epg.it999.ru/img2/");
			else
				icon_uri = playlist->logo_root + pair->second;
		}

		set_icon_uri(utils::utf8_to_utf16(icon_uri));
	}
}

void PlaylistEntry::search_catchup(const m3u_tags& tags)
{
	if (const auto& pair = tags.find(m3u_entry::info_tags::tag_catchup); pair != tags.end())
	{
		set_catchup_id(utils::utf8_to_utf16(pair->second));
	}
}

void PlaylistEntry::check_adult(const std::string& category)
{
	std::wstring lowcase(utils::utf8_to_utf16(category));
	utils::wstring_tolower(lowcase);
	if (lowcase.find(L"зрослы") != std::wstring::npos
		|| lowcase.find(L"adult") != std::wstring::npos
		|| lowcase.find(L"18+") != std::wstring::npos
		|| lowcase.find(L"xxx") != std::wstring::npos)
	{
		// Channel for adult
		set_adult(1);
	}
}
