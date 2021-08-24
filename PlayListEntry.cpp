#include "StdAfx.h"
#include "PlayListEntry.h"
#include "m3u_entry.h"
#include "utils.h"

bool PlaylistEntry::Parse(const std::string& str)
{
	m3u_entry m3uEntry(str);

	switch (m3uEntry.get_directive())
	{
		case m3u_entry::ext_pathname:
			stream_uri->parse_uri(str);
			domain = stream_uri->get_domain();
			access_key = stream_uri->get_uid();
			return true;
		case m3u_entry::ext_group:
			category = utils::utf8_to_utf16(m3uEntry.get_dvalue());
			if (category.find(L"зрослые") != std::wstring::npos)
			{
				// Channel for adult
				set_adult(1);
			}
			break;
		case m3u_entry::ext_info:
			if (const auto& pair = m3uEntry.get_tags().find(m3u_entry::tag_group_title); pair != m3uEntry.get_tags().end())
			{
				category = utils::utf8_to_utf16(pair->second);
				if (category.empty())
				{
					category = L"Unset";
				}
				else if (category.find(L"зрослые") != std::wstring::npos)
				{
					// Channel for adult
					set_adult(1);
				}
			}

			if (const auto& pair = m3uEntry.get_tags().find(m3u_entry::tag_tvg_logo); pair != m3uEntry.get_tags().end())
			{
				set_icon_uri(utils::string_replace(pair->second, "//epg.it999.ru/img/", "//epg.it999.ru/img2/"));
			}

			if (const auto& pair = m3uEntry.get_tags().find(m3u_entry::tag_tvg_rec); pair != m3uEntry.get_tags().end())
			{
				set_archive(utils::char_to_int(pair->second));
			}

			if (const auto& pair = m3uEntry.get_tags().find(m3u_entry::tag_tvg_id); pair != m3uEntry.get_tags().end())
			{
				set_tvg_id(utils::char_to_int(pair->second));
			}

			if (!m3uEntry.get_dir_title().empty())
			{
				set_title(utils::utf8_to_utf16(m3uEntry.get_dir_title()));
			}

			break;
		default:
			break;
	}

	return false;
}
