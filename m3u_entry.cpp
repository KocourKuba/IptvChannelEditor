/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2023): sharky72 (https://github.com/KocourKuba)

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
#include "m3u_entry.h"

#include "UtilsLib\utils.h"
#include "UtilsLib\rapidxml_value.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static std::map<std::string_view, m3u_entry::directives> s_ext_directives = {
	{ "#EXTM3U"    , m3u_entry::directives::ext_header   },
	{ "#EXTINF"    , m3u_entry::directives::ext_info     },
	{ "#EXTGRP"    , m3u_entry::directives::ext_group    },
	{ "#PLAYLIST"  , m3u_entry::directives::ext_playlist },
	{ "#EXTVLCOPT" , m3u_entry::directives::ext_vlcopt   },
};

static std::map<std::string_view, m3u_entry::info_tags> str_tags = {
	{ "url-tvg",          m3u_entry::info_tags::tag_url_tvg          },
	{ "url-logo",         m3u_entry::info_tags::tag_url_logo         },
	{ "channel-id",       m3u_entry::info_tags::tag_channel_id       },
	{ "CUID",             m3u_entry::info_tags::tag_cuid             },
	{ "group-title",      m3u_entry::info_tags::tag_group_title      },
	{ "tvg-id",           m3u_entry::info_tags::tag_tvg_id           },
	{ "tvg-chno",         m3u_entry::info_tags::tag_tvg_chno         },
	{ "tvg-logo",         m3u_entry::info_tags::tag_tvg_logo         },
	{ "tvg-rec",          m3u_entry::info_tags::tag_tvg_rec          },
	{ "tvg-name",         m3u_entry::info_tags::tag_tvg_name         },
	{ "tvg-shift",        m3u_entry::info_tags::tag_tvg_shift        },
	{ "timeshift",        m3u_entry::info_tags::tag_timeshift        },
	{ "arc-timeshift",    m3u_entry::info_tags::tag_arc_timeshift    },
	{ "arc-time",         m3u_entry::info_tags::tag_arc_time         },
	{ "catchup",          m3u_entry::info_tags::tag_catchup          },
	{ "catchup-days",     m3u_entry::info_tags::tag_catchup_days     },
	{ "catchup-time",     m3u_entry::info_tags::tag_catchup_time     },
	{ "catchup-type",     m3u_entry::info_tags::tag_catchup_type     },
	{ "catchup-template", m3u_entry::info_tags::tag_catchup_template },
	{ "catchup-source",   m3u_entry::info_tags::tag_catchup_source   },
	{ "http-user-agent",  m3u_entry::info_tags::tag_http_user_agent  },
	{ "parent-code",      m3u_entry::info_tags::tag_parent_code      },
	{ "censored",         m3u_entry::info_tags::tag_censored         },
};

static std::map<m3u_entry::info_tags, std::string> tags_str = {
	{ m3u_entry::info_tags::tag_url_tvg,          "url-tvg"           },
	{ m3u_entry::info_tags::tag_url_logo,         "url-logo"          },
	{ m3u_entry::info_tags::tag_channel_id,       "channel-id"        },
	{ m3u_entry::info_tags::tag_cuid,             "CUID"              },
	{ m3u_entry::info_tags::tag_group_title,      "group-title"       },
	{ m3u_entry::info_tags::tag_tvg_id,           "tvg-id"            },
	{ m3u_entry::info_tags::tag_tvg_chno,         "tvg-chno"          },
	{ m3u_entry::info_tags::tag_tvg_logo,         "tvg-logo"          },
	{ m3u_entry::info_tags::tag_tvg_rec,          "tvg-rec"           },
	{ m3u_entry::info_tags::tag_tvg_name,         "tvg-name"          },
	{ m3u_entry::info_tags::tag_tvg_shift,        "tvg-shift"         },
	{ m3u_entry::info_tags::tag_timeshift,        "timeshift"         },
	{ m3u_entry::info_tags::tag_arc_timeshift,    "arc-timeshift"     },
	{ m3u_entry::info_tags::tag_arc_time,         "arc-time"          },
	{ m3u_entry::info_tags::tag_catchup,          "catchup"           },
	{ m3u_entry::info_tags::tag_catchup_days,     "catchup-days"      },
	{ m3u_entry::info_tags::tag_catchup_time,     "catchup-time"      },
	{ m3u_entry::info_tags::tag_catchup_type,     "catchup-type"      },
	{ m3u_entry::info_tags::tag_catchup_template, "catchup-template"  },
	{ m3u_entry::info_tags::tag_catchup_source,   "catchup-source"    },
	{ m3u_entry::info_tags::tag_http_user_agent,  "http-user-agent"   },
	{ m3u_entry::info_tags::tag_parent_code,      "parent-code"       },
	{ m3u_entry::info_tags::tag_censored,         "censored"          },
};

std::string_view match_view(const boost::cmatch::value_type& sm)
{
	if (sm.first == sm.second)
		return {};

	return sm.matched ? std::string_view(std::addressof(*sm.first), std::distance(sm.first, sm.second)) : std::string_view();
}

void m3u_entry::clear()
{
	duration = 0;
	ext_name = directives::ext_pathname;
	ext_tags.clear();
}

void m3u_entry::parse(const std::string_view& str)
{
	// #EXTINF:0 group-title="новости" tvg-id="828" tvg-logo="http://epg.it999.ru/img/828.png" tvg-rec="3",BBC World News
	// #EXTGRP:Общие

	// #EXTM3U
	// #EXT_NAME:<EXT_VALUE>
	// #EXTINF:<DURATION>,<TITLE>
	// #EXTINF:<DURATION> [<KEY>="<VALUE>"]*,<TITLE>
	// http://example.tv/live.strm

	static boost::regex re_dir(R"((#[A-Z0-9-]+)[:\s]?(.*))");
	static boost::regex re_info(R"((-?\d+)\s*(.+=\".+"\s*)*,\s*(.+))");

	if (str.empty())
		return;

	if (str.front() != '#')
	{
		// http://example.tv/live.strm
		ext_name = directives::ext_pathname;
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
	boost::cmatch m_dir;
	try
	{
		if (!boost::regex_match(str._Unchecked_begin(), str._Unchecked_end(), m_dir, re_dir))
			return;
	}
	catch (boost::regex_error& ex)
	{
		return;
	}

	auto pair = s_ext_directives.find(match_view(m_dir[1]));
	if (pair == s_ext_directives.end())
		return;

	ext_name = pair->second;

	switch (ext_name)
	{
		case directives::ext_header:
		case directives::ext_vlcopt:
		{
			// #EXTM3U url-tvg="http://iptv-content.webhop.net/guide.xml" url-logo="http://iptv-content.webhop.net/220x132/"
			auto hdr = match_view(m_dir[2]);
			if (!hdr.empty())
				parse_directive_tags(hdr);
			break;
		}
		case directives::ext_info:
		{
			boost::cmatch m;
			auto value = match_view(m_dir[2]);
			try
			{
				if (!boost::regex_match(value._Unchecked_begin(), value._Unchecked_end(), m, re_info)) break;

				duration = utils::char_to_int(m[1].str());
				dir_title = m[3].str();
				// put title to directive for tvg parsing
				tags_map.emplace(info_tags::tag_directive_title, dir_title);

				if (m[2].matched)
				{
					parse_directive_tags(match_view(m[2]));
				}
			}
			catch (boost::regex_error& ex)
			{
			}
			break;
		}
		default:
		{
			// carray[0] = #EXTINF, carray[1] = <EXT_VALUE>
			if (m_dir[2].matched)
				ext_value = match_view(m_dir[2]);
			break;
		}
	}
}

const std::string& m3u_entry::get_str_tag(info_tags tag)
{
	static std::string empty_tag;
	const auto& pair = tags_str.find(tag);

	return (pair == tags_str.end()) ? empty_tag : pair->second;
}

void m3u_entry::parse_directive_tags(std::string_view str)
{
	static boost::regex re(R"(([^=" ]+)=("(?:\\\"|[^"])*"|(?:\\\"|[^=" ])+))");

	boost::cmatch m;
	try
	{
		while (boost::regex_search(str._Unchecked_begin(), str._Unchecked_end(), m, re))
		{
			auto tag = match_view(m[1]);
			if (!tag.empty())
			{
				auto value = match_view(m[2]);
				utils::string_trim(value, " \"\'");
				if (!value.empty())
				{
					ext_tags.emplace(tag, value);
					if (const auto& pair = str_tags.find(tag); pair != str_tags.end())
					{
						tags_map.emplace(pair->second, value);
					}
				}
			}

			str.remove_prefix(m.position() + m.length());
		}
	}
	catch (boost::regex_error& ex)
	{
	}
}
