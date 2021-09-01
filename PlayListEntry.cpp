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
			access_key = stream_uri->get_token();
			return true;
		case m3u_entry::ext_group:
			category = utils::utf8_to_utf16(m3uEntry.get_dvalue());
			if (category.find(L"зрослые") != std::wstring::npos
				|| category.find(L"Adult") != std::wstring::npos
				|| category.find(L"XXX") != std::wstring::npos)
			{
				// Channel for adult
				set_adult(1);
			}
			break;
		case m3u_entry::ext_info:
		{
			const auto& tags = m3uEntry.get_tags();
			if (const auto& pair = tags.find(m3u_entry::tag_group_title); pair != tags.end())
			{
				category = utils::utf8_to_utf16(pair->second);
				if (category.empty())
				{
					category = L"Unset";
				}
				else if (category.find(L"зрослые") != std::wstring::npos
						 || category.find(L"Adult") != std::wstring::npos
						 || category.find(L"XXX") != std::wstring::npos)
				{
					// Channel for adult
					set_adult(1);
				}
			}

			if (const auto& pair = tags.find(m3u_entry::tag_tvg_logo); pair != tags.end())
			{
				set_icon_uri(utils::string_replace(pair->second, "//epg.it999.ru/img/", "//epg.it999.ru/img2/"));
			}

			if (const auto& pair = tags.find(m3u_entry::tag_tvg_rec); pair != tags.end())
			{
				set_archive_days(utils::char_to_int(pair->second));
			}

			if (const auto& pair = tags.find(m3u_entry::tag_tvg_id); pair != tags.end())
			{
				set_epg1_id(pair->second);
				set_epg2_id(pair->second);
			}

			if (!m3uEntry.get_dir_title().empty())
			{
				set_title(utils::utf8_to_utf16(m3uEntry.get_dir_title()));
			}
		}
		break;
		default:
			break;
	}

	return false;
}
