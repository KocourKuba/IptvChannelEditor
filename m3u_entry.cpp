#include "pch.h"
#include "m3u_entry.h"

#include "UtilsLib\utils.h"
#include "UtilsLib\rapidxml_value.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static std::map<std::wstring_view, m3u_entry::directives> s_ext_directives = {
	{ L"#EXTM3U"   , m3u_entry::directives::ext_header   },
	{ L"#EXTINF"   , m3u_entry::directives::ext_info     },
	{ L"#EXTGRP"   , m3u_entry::directives::ext_group    },
	{ L"#PLAYLIST" , m3u_entry::directives::ext_playlist },
};

static std::map<std::wstring_view, m3u_entry::info_tags> s_tags = {
	{ L"url-tvg",        m3u_entry::info_tags::tag_url_tvg        },
	{ L"url-logo",       m3u_entry::info_tags::tag_url_logo       },
	{ L"channel-id",     m3u_entry::info_tags::tag_channel_id     },
	{ L"CUID",           m3u_entry::info_tags::tag_cuid           },
	{ L"group-title",    m3u_entry::info_tags::tag_group_title    },
	{ L"tvg-id",         m3u_entry::info_tags::tag_tvg_id         },
	{ L"tvg-logo",       m3u_entry::info_tags::tag_tvg_logo       },
	{ L"tvg-rec",        m3u_entry::info_tags::tag_tvg_rec        },
	{ L"tvg-name",       m3u_entry::info_tags::tag_tvg_name       },
	{ L"tvg-shift",      m3u_entry::info_tags::tag_tvg_shift      },
	{ L"timeshift",      m3u_entry::info_tags::tag_timeshift      },
	{ L"catchup",        m3u_entry::info_tags::tag_catchup        },
	{ L"catchup-days",   m3u_entry::info_tags::tag_catchup_days   },
	{ L"catchup-time",   m3u_entry::info_tags::tag_catchup_time   },
	{ L"catchup-type",   m3u_entry::info_tags::tag_catchup_type   },
	{ L"catchup-source", m3u_entry::info_tags::tag_catchup_source },
};

using wsvmatch = std::match_results<std::wstring_view::const_iterator>;

std::wstring_view wmatch_view(const wsvmatch::value_type& sm)
{
	if (sm.first == sm.second)
		return std::wstring_view();

	return sm.matched ? std::wstring_view(std::addressof(*sm.first), std::distance(sm.first, sm.second)) : std::wstring_view();
}

void m3u_entry::clear()
{
	duration = 0;
	ext_name = directives::ext_pathname;
	ext_tags.clear();
}

void m3u_entry::parse(const std::wstring_view& str)
{
	// #EXTINF:0 group-title="новости" tvg-id="828" tvg-logo="http://epg.it999.ru/img/828.png" tvg-rec="3",BBC World News
	// #EXTGRP:Общие

	// #EXTM3U
	// #EXT_NAME:<EXT_VALUE>
	// #EXTINF:<DURATION>,<TITLE>
	// #EXTINF:<DURATION> [<KEY>="<VALUE>"]*,<TITLE>
	// http://example.tv/live.strm

	static std::wregex re_dir(LR"((#[A-Z0-9-]+)[:\s]?(.*))");
	static std::wregex re_info(LR"((-?\d+)\s*(.+=\".+"\s*)*,\s*(.+))");

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
	wsvmatch m_dir;
	if (!std::regex_match(str.begin(), str.end(), m_dir, re_dir))
		return;

	auto pair = s_ext_directives.find(wmatch_view(m_dir[1]));
	if (pair == s_ext_directives.end())
		return;

	ext_name = pair->second;

	switch (ext_name)
	{
		case directives::ext_header:
		{
			// #EXTM3U url-tvg="http://iptv-content.webhop.net/guide.xml" url-logo="http://iptv-content.webhop.net/220x132/"
			auto hdr = wmatch_view(m_dir[2]);
			if (!hdr.empty())
				parse_directive_tags(hdr);
			break;
		}
		case directives::ext_info:
		{
			wsvmatch m;
			auto value = wmatch_view(m_dir[2]);
			if (std::regex_match(value.begin(), value.end(), m, re_info))
			{
				duration = utils::char_to_int(m[1].str());
				dir_title = m[3].str();
				// put title to directive for tvg parsing
				ext_tags.emplace(info_tags::tag_directive_title, dir_title);

				if (m[2].matched)
				{
					parse_directive_tags(wmatch_view(m[2]));
				}
			}
			break;
		}
		default:
		{
			// carray[0] = #EXTINF, carray[1] = <EXT_VALUE>
			if (m_dir[2].matched)
				ext_value = wmatch_view(m_dir[2]);
			break;
		}
	}
}

void m3u_entry::parse_directive_tags(std::wstring_view str)
{
	static std::wregex re(LR"((?:[^\s\"]+|\"[^\"]*\")+)");
	static std::wregex re_pair(LR"((.*)=(?=[\"\'])(.+))");

	//std::wsmatch m;
	wsvmatch m;
	while(std::regex_search(str.begin(), str.end(), m, re))
	{
		auto pair = wmatch_view(m[0]);
		if (pair.empty())
			continue;

		wsvmatch m_pair;
		if (std::regex_match(pair.begin(), pair.end(), m_pair, re_pair))
		{
			auto tag = wmatch_view(m_pair[1]);
			utils::string_trim(tag, L" ");
			if (!tag.empty())
			{
				auto value = wmatch_view(m_pair[2]);
				utils::string_trim(value, L" \"\'");
				if (!value.empty())
				{
					const auto& pair = s_tags.find(tag);
					if (pair != s_tags.end())
					{
						ext_tags.emplace(pair->second, value);
					}
				}
			}
		}
		str.remove_prefix(m.position() + m.length());
	}
}
