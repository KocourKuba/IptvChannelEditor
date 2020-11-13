#include "StdAfx.h"
#include "PlayListEntry.h"
#include "utils.h"

static std::map<directives, std::string> s_ext_directives = {
	{ ext_header,   "#EXTM3U" },
	{ ext_info,     "#EXTINF:" },
	{ ext_group,    "#EXTGRP:" },
	{ ext_playlist, "#PLAYLIST:" },
};

static std::map<std::string, info_tags> s_tags = {
	{ "channel-id",  tag_channel_id  },
	{ "group-title", tag_group_title },
	{ "tvg-id",      tag_tvg_id      },
	{ "tvg-logo",    tag_tvg_logo    },
	{ "tvg-rec",     tag_tvg_rec     },
	{ "tvg-name",    tag_tvg_name    },
	{ "tvg-shift",   tag_tvg_shift   },
};

void m3u_entry::Clear()
{
	duration = 0;
	ext_name = ext_pathname;
	ext_tags.clear();
}

void m3u_entry::Parse(const std::string& str)
{
	// #EXTINF:0 group-title="новости" tvg-id="828" tvg-logo="http://epg.it999.ru/img/828.png" tvg-rec="3",BBC World News
	// #EXTGRP:Общие

	// #EXTM3U
	// #EXT_NAME:<EXT_VALUE>
	// #EXTINF:<DURATION> [<KEY>="<VALUE>"]*,<TITLE>
	// http://example.tv/live.strm

	if (str.empty())
		return;

	if (str.front() != '#')
	{
		// http://example.tv/live.strm
		ext_name = ext_pathname;
		ext_value = str;
	}
	else
	{
		for (const auto& item : s_ext_directives)
		{
			// split by space
			// #EXTINF:<DURATION> [<KEY>="<VALUE>"]*,<TITLE>
			auto extarr = utils::string_split(str);
			if(extarr.empty()) continue;

			if (strncmp(extarr[0].c_str(), item.second.c_str(), item.second.size()) != 0)
				continue;

			// 0 - #EXTINF:<DURATION>
			// 1,2,3... - [<KEY>="<VALUE>"]*,<TITLE>
			// #EXT_NAME:<EXT_VALUE>
			ext_name = item.first;
			if(ext_name == ext_header)
				break;

			// extarr[0] = #EXT_NAME:<EXT_VALUE>
			auto carray = utils::string_split(extarr[0], ':');

			if (ext_name != ext_info)
			{
				// carray[0] = #EXTINF, carray[1] = <EXT_VALUE>
				if(carray.size() > 1)
					ext_value = carray[1];
			}
			else
			{
				// carray[0] = #EXTINF, carray[1] = <DURATION>
				if (carray.size() > 1)
					duration = utils::char_to_int(carray[1]);

				// remove #EXT_NAME:<EXT_VALUE> from string
				// [<KEY>="<VALUE>"]*,<TITLE>
				auto val = utils::string_ltrim(str.substr(extarr[0].size()), " ");
				auto tarray = utils::string_split(val, ',');
				if (!tarray.empty())
				{
					// get last token: <TITLE>
					dir_title = tarray.back();
					auto remain = utils::string_rtrim(val.substr(0, val.size() - dir_title.size()), " ,");
					ParseDirectiveTags(remain);
				}
			}
			break;
		}
	}
}

void m3u_entry::ParseDirectiveTags(const std::string& str)
{
	auto tags_array = utils::regex_split(str, R"(\"\s+)");
	for(const auto& tag : tags_array)
	{
		auto vtag = utils::string_split(tag, '=');
		if(vtag.size() == 2 && !vtag[0].empty())
		{
			if (const auto& pair = s_tags.find(vtag[0]); pair != s_tags.end())
				ext_tags.emplace(pair->second, utils::string_trim(vtag[1], " \"\'"));
		}
	}
}

void PlaylistEntry::Parse(const std::string& str)
{
	m3u_entry::Parse(str);

	if (ext_name == ext_group)
	{
		category = utils::utf8_to_utf16(ext_value);
	}

	if (ext_name == ext_pathname)
	{
		// 1 - domain
		// 2 - access key
		// 3 - channel id

		std::regex re_url(R"([a-z]+:\/\/([0-9a-z\.]+)\/iptv\/([0-9A-Za-z\.]+)\/(\d+)\/index.m3u8)");
		std::smatch m_url;
		if (std::regex_match(ext_value, m_url, re_url))
		{
			domain = m_url[1].str();
			access_key = m_url[2].str();
			int id = utils::char_to_int(m_url[3].str());
			stream_uri.set_uri(utils::URI_TEMPLATE);
			stream_uri.set_Id(id);
		}
		else
		{
			stream_uri.set_uri(ext_value);
		}
	}

	if(const auto& pair = ext_tags.find(tag_group_title); pair != ext_tags.end())
	{
		category = utils::utf8_to_utf16(pair->second);
	}

	if (const auto& pair = ext_tags.find(tag_tvg_logo); pair != ext_tags.end())
	{
		std::regex re_url(R"(\/\/epg.it999.ru\/img\/)");
		set_icon_uri(std::regex_replace(pair->second, re_url, "//epg.it999.ru/img2/"));
	}

	if (const auto& pair = ext_tags.find(tag_tvg_rec); pair != ext_tags.end())
	{
		archive = utils::char_to_int(pair->second);
	}

	if (const auto& pair = ext_tags.find(tag_tvg_id); pair != ext_tags.end())
	{
		tvg_id = utils::char_to_int(pair->second);
	}

	if(!dir_title.empty())
		title = utils::utf8_to_utf16(dir_title);
}

void PlaylistEntry::Clear()
{
	m3u_entry::Clear();

	title.clear();
	category.clear();
	stream_uri.clear();
	set_icon_uri("");
	set_icon(CImage());
}
