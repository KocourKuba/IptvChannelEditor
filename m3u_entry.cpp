#include "pch.h"
#include "m3u_entry.h"

#include "UtilsLib\utils.h"
#include "UtilsLib\rapidxml_value.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static std::map<std::string, m3u_entry::directives> s_ext_directives = {
	{ "#EXTM3U"   , m3u_entry::ext_header   },
	{ "#EXTINF"   , m3u_entry::ext_info     },
	{ "#EXTGRP"   , m3u_entry::ext_group    },
	{ "#PLAYLIST" , m3u_entry::ext_playlist },
};

static std::map<std::string, m3u_entry::info_tags> s_tags = {
	{ "url-tvg",        m3u_entry::tag_url_tvg        },
	{ "url-logo",       m3u_entry::tag_url_logo       },
	{ "channel-id",     m3u_entry::tag_channel_id     },
	{ "CUID",           m3u_entry::tag_cuid           },
	{ "group-title",    m3u_entry::tag_group_title    },
	{ "tvg-id",         m3u_entry::tag_tvg_id         },
	{ "tvg-logo",       m3u_entry::tag_tvg_logo       },
	{ "tvg-rec",        m3u_entry::tag_tvg_rec        },
	{ "tvg-name",       m3u_entry::tag_tvg_name       },
	{ "tvg-shift",      m3u_entry::tag_tvg_shift      },
	{ "timeshift",      m3u_entry::tag_timeshift      },
	{ "catchup",        m3u_entry::tag_catchup        },
	{ "catchup-days",   m3u_entry::tag_catchup_days   },
	{ "catchup-type",   m3u_entry::tag_catchup_type   },
	{ "catchup-source", m3u_entry::tag_catchup_source },
};

void m3u_entry::clear()
{
	duration = 0;
	ext_name = ext_pathname;
	ext_tags.clear();
}

void m3u_entry::parse(const std::string& str)
{
	// #EXTINF:0 group-title="новости" tvg-id="828" tvg-logo="http://epg.it999.ru/img/828.png" tvg-rec="3",BBC World News
	// #EXTGRP:Общие

	// #EXTM3U
	// #EXT_NAME:<EXT_VALUE>
	// #EXTINF:<DURATION>,<TITLE>
	// #EXTINF:<DURATION> [<KEY>="<VALUE>"]*,<TITLE>
	// http://example.tv/live.strm

	static std::regex re_dir(R"((#[A-Z0-9-]+)[:\s]?(.*))");
	static std::regex re_info(R"((-?\d+)\s*(.+=\".+"\s*)*,\s*(.+))");

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
		{
			// #EXTM3U url-tvg="http://iptv-content.webhop.net/guide.xml" url-logo="http://iptv-content.webhop.net/220x132/"
			if (!m_dir[2].str().empty())
				parse_directive_tags(m_dir[2].str());
			break;
		}
		case ext_info:
		{
			std::smatch m;
			const auto& value = m_dir[2].str();
			if (std::regex_match(value, m, re_info))
			{
				duration = rapidxml::char_to_int(m[1].str());
				dir_title = m[3].str();
				// put title to directive for tvg parsing
				ext_tags.emplace(tag_directive_title, dir_title);

				if (m[2].matched)
				{
					parse_directive_tags(m[2].str());
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

void m3u_entry::parse_directive_tags(const std::string& str)
{
	auto tags_array = utils::regex_split(str, R"(\"\s+)");
	for (const auto& tag : tags_array)
	{
		auto vtag = utils::string_split(tag, '=');
		if (vtag.size() == 2 && !vtag[0].empty())
		{
			if (const auto& pair = s_tags.find(vtag[0]); pair != s_tags.end())
				ext_tags.emplace(pair->second, utils::string_trim(vtag[1], " \"\'"));
		}
	}
}
