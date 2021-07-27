#include "StdAfx.h"
#include "PlayListEntry.h"
#include "utils.h"

static std::map<std::string, directives> s_ext_directives = {
	{ "#EXTM3U"   , ext_header   },
	{ "#EXTINF"   , ext_info     },
	{ "#EXTGRP"   , ext_group    },
	{ "#PLAYLIST" , ext_playlist },
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
	// #EXTINF:<DURATION>,<TITLE>
	// #EXTINF:<DURATION> [<KEY>="<VALUE>"]*,<TITLE>
	// http://example.tv/live.strm

	if (str.empty())
		return;

	if (str.front() != '#')
	{
		// http://example.tv/live.strm
		ext_name = ext_pathname;
		ext_value = str;
		return;
	}

	// #EXTINF:<DURATION>,<TITLE>
	// #EXTINF:<DURATION> [<KEY>="<VALUE>"]*,<TITLE>

	// 0 - #EXTINF:<DURATION>
	// 1,2,3... - [<KEY>="<VALUE>"]*,<TITLE>
	// #EXT_NAME:<EXT_VALUE>
	// extarr[0] = #EXT_NAME
	// extarr[1] = <EXT_VALUE>
	std::regex re_dir(R"((#[A-Z0-9-]+)[:\s]?(.*))");
	std::smatch m_dir;
	if (!std::regex_match(str, m_dir, re_dir))
		return;

	auto pair = s_ext_directives.find(m_dir[1].str());
	if (pair == s_ext_directives.end())
		return;

	ext_name = pair->second;

	switch (ext_name)
	{
		case ext_header:
			return;
		case ext_info:
		{
			std::regex re(R"((-?\d+)\s*(.+=\".+"\s*)*,\s*(.+))");
			std::smatch m;
			const auto& value = m_dir[2].str();
			if (std::regex_match(value, m, re))
			{
				duration = utils::char_to_int(m[1].str());
				dir_title = m[3].str();
				if(m[2].matched)
				{
					ParseDirectiveTags(m[2].str());
				}
			}
			break;
		}
		default:
		{
			// carray[0] = #EXTINF, carray[1] = <EXT_VALUE>
			if (m_dir[2].matched)
				ext_value = m_dir[2];
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
			auto& uri = get_stream_uri();
			uri.set_uri(utils::URI_TEMPLATE);
			uri.set_Id(id);
		}
		else
		{
			get_stream_uri().set_uri(utils::utf8_to_utf16(ext_value));
		}
	}

	if(const auto& pair = ext_tags.find(tag_group_title); pair != ext_tags.end())
	{
		category = utils::utf8_to_utf16(pair->second);
		if (category.empty())
			category = L"Unset";
	}

	if (const auto& pair = ext_tags.find(tag_tvg_logo); pair != ext_tags.end())
	{
		std::wregex re_url(LR"(\/\/epg.it999.ru\/img\/)");
		set_icon_uri(std::regex_replace(utils::utf8_to_utf16(pair->second), re_url, L"//epg.it999.ru/img2/"));
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
	get_stream_uri().clear();
	set_icon_uri(L"");
	set_icon(CImage());
}
