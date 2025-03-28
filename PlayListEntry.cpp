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
#include "PlayListEntry.h"

#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool PlaylistEntry::Parse(const std::string& str)
{
	if (str.empty()) return false;

	m3uEntry.parse(str);

	bool result = false;
	switch (m3uEntry.get_directive())
	{
		case m3u_entry::directives::ext_header:
		{
			if (!playlist) break;

			if (playlist->m3u_header.get_tags_map().empty())
			{
				playlist->m3u_header = m3uEntry;
			}
			else
			{
				playlist->m3u_header.get_tags_map().merge(m3uEntry.get_tags_map());
			}

			const auto& tags = playlist->m3u_header.get_tags_map();

			if (const auto& root = search_logo(tags); !root.empty())
			{
				playlist->logo_root = root;
			}

			if (auto value = search_catchup(tags); value != CatchupType::cu_not_set)
			{
				playlist->catchup = value;
			}

			playlist->catchup_days = search_archive(tags);
			playlist->catchup_source = search_catchup_source(tags);
			break;
		}
		case m3u_entry::directives::ext_pathname:
		{
			return true;
		}
		case m3u_entry::directives::ext_group:
		{
			if (category.empty())
			{
				category = m3uEntry.get_dvalue();
				check_adult(m3uEntry.get_tags_map(), category);
			}
			break;
		}
		case m3u_entry::directives::ext_info:
		{
			const auto& tags = m3uEntry.get_tags_map();

			// #EXTINF:-1 timeshift="14" catchup-days="14" catchup-type="flussonic" tvg-id="pervy"  group-title="�����" tvg-logo="http://pl.ottglanz.tv:80/icon/2214.png",������ HD
			// #EXTINF:0 tvg-name="ch002" tvg-logo="http://1usd.tv/images/chIcons/2.png" timeshift="4", ������ 1 HD
			// #EXTINF:0 CUID="1" tvg-name="1:458" tvg-id="1:458" arc-time="120" catchup="shift" catchup-days="5"
			//           tvg-logo="http://stb.fox-tv.fun/images/logo_chanel/1RUSTV200.png" group-title="�����",������ �����

			if (!m3uEntry.get_dir_title().empty())
			{
				set_title(utils::utf8_to_utf16(m3uEntry.get_dir_title()));
			}

			search_group(tags);
			search_epg(tags);
			int value = search_archive(tags);
			if (value != 0)
			{
				set_archive_days(value);
			}
			else if (playlist->catchup_days != 0)
			{
				set_archive_days(playlist->catchup_days);
			}

			if (auto logo = search_logo(tags); !logo.empty())
			{
				if (playlist && !playlist->logo_root.empty())
				{
					logo = playlist->logo_root + logo;
				}

				set_icon_uri(utils::utf8_to_utf16(logo));
			}

			if (auto value = search_catchup(tags); value != CatchupType::cu_not_set)
			{
				set_catchup(value);
				playlist->per_channel_catchup = true;
				if (const auto& pair = tags.find(m3u_entry::info_tags::tag_catchup_source); pair != tags.end()) {
					set_catchup_source(pair->second);
				}
			}
			break;
		}
		case m3u_entry::directives::ext_vlcopt:
			break;

		default:
			break;
	}

	return result;
}

void PlaylistEntry::search_id(const std::wstring& search_tag)
{
	static std::map<std::wstring, m3u_entry::info_tags> id_tags = {
		{ L"channel-id", m3u_entry::info_tags::tag_channel_id      },
		{ L"CUID",       m3u_entry::info_tags::tag_cuid            },
		{ L"tvg-chno",   m3u_entry::info_tags::tag_tvg_chno        },
		{ L"tvg-id",     m3u_entry::info_tags::tag_tvg_id          },
		{ L"tvg-name",   m3u_entry::info_tags::tag_tvg_name        },
		{ L"name",       m3u_entry::info_tags::tag_directive_title },
	};

	const m3u_tags& tags = m3uEntry.get_tags_map();
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
		if (!category.empty())
		{
			check_adult(tags, category);
		}
	}
}

int PlaylistEntry::search_archive(const m3u_tags& tags)
{
	// priority -> catchup_days -> catchup_time -> tag_timeshift ... -> tvg_rec
	static std::array<m3u_entry::info_tags, 6> archive_search_tags =
	{
		m3u_entry::info_tags::tag_catchup_days,
		m3u_entry::info_tags::tag_catchup_time,
		m3u_entry::info_tags::tag_timeshift,
		m3u_entry::info_tags::tag_arc_timeshift,
		m3u_entry::info_tags::tag_arc_time,
		m3u_entry::info_tags::tag_tvg_rec,
	};

	int day = 0;
	for (const auto& tag : archive_search_tags)
	{
		if (const auto& pair = tags.find(tag); pair != tags.end() && !pair->second.empty())
		{
			day = utils::char_to_int(pair->second);
			if (tag == m3u_entry::info_tags::tag_catchup_time)
				day /= 86400;
			break;
		}
	}

	return day;
}

void PlaylistEntry::search_epg(const m3u_tags& tags)
{
	// priority -> tvg_id -> tvg_name -> title
	static std::array<m3u_entry::info_tags, 3> epg_search_tags =
	{
		m3u_entry::info_tags::tag_tvg_id,
		m3u_entry::info_tags::tag_tvg_name,
		m3u_entry::info_tags::tag_directive_title,
	};

	for (const auto& tag : epg_search_tags)
	{
		if (const auto& pair = tags.find(tag); pair != tags.end() && !pair->second.empty())
		{
			set_epg_id(0, utils::utf8_to_utf16(pair->second));
			break;
		}
	}
}

std::string PlaylistEntry::search_logo(const m3u_tags& tags)
{
	static std::array<m3u_entry::info_tags, 2> logo_search_tags =
	{
		m3u_entry::info_tags::tag_url_logo,
		m3u_entry::info_tags::tag_tvg_logo,
	};

	for (const auto& tag : logo_search_tags)
	{
		if (const auto& pair = tags.find(tag); pair != tags.end() && !pair->second.empty())
		{
			return pair->second;
		}
	}

	return "";
}

std::string PlaylistEntry::search_catchup_source(const m3u_tags& tags)
{
	static std::array<m3u_entry::info_tags, 2> catchup_source_tags =
	{
		m3u_entry::info_tags::tag_catchup_source,
		m3u_entry::info_tags::tag_catchup_template,
	};

	for (const auto& tag : catchup_source_tags)
	{
		if (const auto& pair = tags.find(tag); pair != tags.end())
		{
			return pair->second;
		}
	}

	return "";
}

CatchupType PlaylistEntry::search_catchup(const m3u_tags& tags)
{
	static std::array<m3u_entry::info_tags, 2> catchup_tags =
	{
		m3u_entry::info_tags::tag_catchup,
		m3u_entry::info_tags::tag_catchup_type,
	};

	for (const auto& tag : catchup_tags)
	{
		if (const auto& pair = tags.find(tag); pair != tags.end() && !pair->second.empty())
		{
			if (pair->second == "append")
				return CatchupType::cu_append;

			if (pair->second == "shift")
				return CatchupType::cu_shift;

			if (pair->second == "flussonic")
				return CatchupType::cu_flussonic;

			if (pair->second == "default")
				return CatchupType::cu_default;
		}
	}

	return CatchupType::cu_not_set;
}

void PlaylistEntry::check_adult(const m3u_tags& tags, const std::string& category)
{
	static std::array<m3u_entry::info_tags, 2> adult_tags =
	{
		m3u_entry::info_tags::tag_parent_code,
		m3u_entry::info_tags::tag_censored,
	};

	for (const auto& tag : adult_tags)
	{
		if (const auto& pair = tags.find(tag); pair != tags.end() && !pair->second.empty())
		{
			set_adult(1);
			return;
		}
	}

	std::wstring lowcase = utils::wstring_tolower_l(utils::utf8_to_utf16(category));
	if (lowcase.find(L"������") != std::wstring::npos
		|| lowcase.find(L"adult") != std::wstring::npos
		|| lowcase.find(L"18+") != std::wstring::npos
		|| lowcase.find(L"xxx") != std::wstring::npos)
	{
		// Channel for adult
		set_adult(1);
	}
}
