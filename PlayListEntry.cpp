#include "pch.h"
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
		case m3u_entry::ext_pathname:
		{
			stream_uri->parse_uri(str);
			result = stream_uri->is_valid();
			if (result && category.empty())
				category = L"Unset";
			break;
		}
		case m3u_entry::ext_group:
			if (!category.empty()) break;

			category = m3uEntry.get_dvalue();
			check_adult(category);
			break;
		case m3u_entry::ext_info:
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

		}
		break;
		default:
			break;
	}

	return result;
}

void PlaylistEntry::search_id(const std::map<m3u_entry::info_tags, std::wstring>& tags)
{
	if (const auto& pair = tags.find(m3u_entry::tag_channel_id); pair != tags.end())
	{
		stream_uri->set_id(pair->second);
	}
	else if (const auto& pair = tags.find(m3u_entry::tag_cuid); pair != tags.end())
	{
		stream_uri->set_id(pair->second);
	}
}

void PlaylistEntry::search_group(const std::map<m3u_entry::info_tags, std::wstring>& tags)
{
	if (const auto& pair = tags.find(m3u_entry::tag_group_title); pair != tags.end())
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
	if (const auto& pair = tags.find(m3u_entry::tag_tvg_rec); pair != tags.end())
	{
		set_archive_days(utils::char_to_int(pair->second));
	}
	if (const auto& pair = tags.find(m3u_entry::tag_catchup_days); pair != tags.end())
	{
		set_archive_days(utils::char_to_int(pair->second));
	}
	if (const auto& pair = tags.find(m3u_entry::tag_catchup_time); pair != tags.end())
	{
		set_archive_days(utils::char_to_int(pair->second) / 86400);
	}
	if (const auto& pair = tags.find(m3u_entry::tag_timeshift); pair != tags.end())
	{
		set_archive_days(utils::char_to_int(pair->second));
	}
}

void PlaylistEntry::search_epg(const std::map<m3u_entry::info_tags, std::wstring>& tags)
{
	// priority -> tvg_id -> tvg_name -> title
	if (const auto& pair = tags.find(m3u_entry::tag_tvg_id); pair != tags.end())
	{
		set_epg1_id(pair->second);
	}
	else if (const auto& pair = tags.find(m3u_entry::tag_tvg_name); pair != tags.end())
	{
		set_epg1_id(pair->second);
	}
	else if (const auto& pair = tags.find(m3u_entry::tag_directive_title); pair != tags.end())
	{
		set_epg1_id(pair->second);
	}
}

void PlaylistEntry::search_logo(const std::map<m3u_entry::info_tags, std::wstring>& tags)
{
	if (const auto& pair = tags.find(m3u_entry::tag_tvg_logo); pair != tags.end())
	{
		if (logo_root.empty())
			set_icon_uri(utils::string_replace<wchar_t>(pair->second, L"//epg.it999.ru/img/", L"//epg.it999.ru/img2/"));
		else
			set_icon_uri(logo_root + pair->second);
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
