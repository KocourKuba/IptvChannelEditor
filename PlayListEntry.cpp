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
#include <regex>
#include "PlayListEntry.h"
#include "UtilsLib\utils.h"
#include "UtilsLib\rapidxml_value.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

bool PlaylistEntry::Parse(const std::wstring& str, const m3u_entry& m3uEntry)
{
	if (str.empty()) return false;

	bool result = false;
	switch (m3uEntry.get_directive())
	{
	case m3u_entry::directives::ext_pathname:
		{
			stream_uri->parse_uri(str);
			result = stream_uri->is_valid();
			if (result && category.empty())
				category = L"Unset";
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
				set_title(m3uEntry.get_dir_title());
			}

			search_id(tags);
			search_group(tags);
			search_archive(tags);
			search_epg(tags);
			search_logo(tags);
			search_catchup(tags);
			break;
		}
		default:
			break;
	}

	if (result)
	{
		// special cases after parsing
		switch (stream_type)
		{
		case PluginType::enGlanz:
		case PluginType::enOneCent:
		case PluginType::enIptvOnline:
			if (get_epg_id(0).front() == 'X')
			{
				const auto& id = get_epg_id(0).substr(1);
				set_epg_id(0, id); // primary EPG
			}
			break;
		case PluginType::enSharavoz:
		case PluginType::enOneOtt:
		case PluginType::enCbilling:
		case PluginType::enShuraTV:
			set_epg_id(0, stream_uri->get_parser().id); // primary EPG
			break;
		case PluginType::enLightIptv:
		case PluginType::enFilmax:
		{
			auto epg_id = get_epg_id(0);
			stream_uri->get_parser().id = epg_id;
			epg_id.erase(std::remove(epg_id.begin(), epg_id.end(), '/'), epg_id.end());
			set_epg_id(0, epg_id);
			break;
		}
		case PluginType::enOttclub:
			set_epg_id(0, stream_uri->get_parser().id); // primary EPG
			set_icon_uri(fmt::format(L"http://{:s}/images/{:s}.png", stream_uri->get_parser().domain, stream_uri->get_parser().id));
			break;
		case PluginType::enVidok:
			set_icon_uri(fmt::format(L"http://ott.st/logos/{:s}.png", stream_uri->get_parser().id));
			break;
		case PluginType::enKineskop:
			set_icon_uri(std::regex_replace(get_icon_uri().get_uri(), std::wregex(LR"(http:\/\/\w{2}\.(.*))"), L"http://$1"));
			break;
		case PluginType::enEdem:
		case PluginType::enFox:
		case PluginType::enSharaTV:
		case PluginType::enMymagic:
		case PluginType::enRusskoeTV:
		default:
			break;
		}

		if (!stream_uri->get_epg_parameters(1).epg_url.empty())
			set_epg_id(1, get_epg_id(0));
	}

	return result;
}

void PlaylistEntry::search_id(const std::map<m3u_entry::info_tags, std::wstring>& tags)
{
	if (const auto& pair = tags.find(m3u_entry::info_tags::tag_channel_id); pair != tags.end())
	{
		stream_uri->get_parser().id = pair->second;
	}
	else if (const auto& pair = tags.find(m3u_entry::info_tags::tag_cuid); pair != tags.end())
	{
		stream_uri->get_parser().id = pair->second;
	}
}

void PlaylistEntry::search_group(const std::map<m3u_entry::info_tags, std::wstring>& tags)
{
	if (const auto& pair = tags.find(m3u_entry::info_tags::tag_group_title); pair != tags.end())
	{
		category = pair->second;
		if (category.empty())
		{
			category = L"Unset";
		}
		else
		{
			check_adult(category);
		}
	}
}

void PlaylistEntry::search_archive(const std::map<m3u_entry::info_tags, std::wstring>& tags)
{
	if (const auto& pair = tags.find(m3u_entry::info_tags::tag_tvg_rec); pair != tags.end())
	{
		set_archive_days(utils::char_to_int(pair->second));
	}
	else if (const auto& pair = tags.find(m3u_entry::info_tags::tag_catchup_days); pair != tags.end())
	{
		set_archive_days(utils::char_to_int(pair->second));
	}
	else if (const auto& pair = tags.find(m3u_entry::info_tags::tag_catchup_time); pair != tags.end())
	{
		set_archive_days(utils::char_to_int(pair->second) / 86400);
	}
	else if (const auto& pair = tags.find(m3u_entry::info_tags::tag_timeshift); pair != tags.end())
	{
		set_archive_days(utils::char_to_int(pair->second));
	}
	else if (const auto& pair = tags.find(m3u_entry::info_tags::tag_arc_timeshift); pair != tags.end())
	{
		set_archive_days(utils::char_to_int(pair->second));
	}
	else if (const auto& pair = tags.find(m3u_entry::info_tags::tag_arc_time); pair != tags.end())
	{
		set_archive_days(utils::char_to_int(pair->second));
	}
}

void PlaylistEntry::search_epg(const std::map<m3u_entry::info_tags, std::wstring>& tags)
{
	// priority -> tvg_id -> tvg_name -> title
	std::wstring tvg_id;
	if (const auto& pair = tags.find(m3u_entry::info_tags::tag_tvg_id); pair != tags.end())
	{
		tvg_id = pair->second;
	}
	else if (const auto& pair = tags.find(m3u_entry::info_tags::tag_tvg_name); pair != tags.end())
	{
		tvg_id = pair->second;
	}
	else if (const auto& pair = tags.find(m3u_entry::info_tags::tag_directive_title); pair != tags.end())
	{
		tvg_id = pair->second;
	}

	set_epg_id(0, tvg_id);
}

void PlaylistEntry::search_logo(const std::map<m3u_entry::info_tags, std::wstring>& tags)
{
	if (const auto& pair = tags.find(m3u_entry::info_tags::tag_tvg_logo); pair != tags.end())
	{
		if (logo_root.empty())
			set_icon_uri(utils::string_replace<wchar_t>(pair->second, L"//epg.it999.ru/img/", L"//epg.it999.ru/img2/"));
		else
			set_icon_uri(logo_root + pair->second);
	}
}

void PlaylistEntry::search_catchup(const std::map<m3u_entry::info_tags, std::wstring>& tags)
{
	if (const auto& pair = tags.find(m3u_entry::info_tags::tag_catchup); pair != tags.end())
	{
		set_catchup(pair->second);
	}
}

void PlaylistEntry::check_adult(const std::wstring& category)
{
	std::wstring lowcase(category);
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
