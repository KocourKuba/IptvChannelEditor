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

#pragma once

class m3u_entry
{
public:
	enum class directives
	{
		ext_unknown = -1,
		ext_pathname,
		ext_header,
		ext_group,
		ext_playlist,
		ext_info,
		ext_vlcopt,
	};

	enum class info_tags
	{
		tag_directive_title = -1,
		tag_url_tvg,
		tag_url_logo,
		tag_channel_id,
		tag_cuid,
		tag_group_title,
		tag_tvg_id,
		tag_tvg_chno,
		tag_tvg_logo,
		tag_tvg_rec,
		tag_tvg_name,
		tag_tvg_shift,
		tag_timeshift,
		tag_arc_timeshift,
		tag_arc_time,
		tag_catchup,
		tag_catchup_days,
		tag_catchup_time,
		tag_catchup_type,
		tag_catchup_template,
		tag_catchup_source,
		tag_http_user_agent,
		tag_parent_code,
		tag_censored,
	};

public:
	m3u_entry() = default;
	m3u_entry(const std::string_view& str) { parse(str); };
	virtual ~m3u_entry() = default;

	void clear();
	void parse(const std::string_view& str);

	int get_duration() const { return duration; }

	directives get_directive() const { return ext_name; }

	const std::map<std::string, std::string>& get_ext_tags() const { return ext_tags; }
	std::map<std::string, std::string>& get_ext_tags() { return ext_tags; }

	const std::map<info_tags, std::string>& get_tags_map() const { return tags_map; }
	std::map<info_tags, std::string>& get_tags_map() { return tags_map; }

	std::string get_dvalue() const { return ext_value; }

	std::string get_dir_title() const { return dir_title; }

	static const std::string& get_str_tag(info_tags tag);

protected:
	void parse_directive_tags(std::string_view str);

	int duration = 0;
	directives ext_name = directives::ext_unknown;
	std::string ext_value;
	std::string dir_title;
	std::map<info_tags, std::string> tags_map;
	std::map<std::string, std::string> ext_tags;
};
