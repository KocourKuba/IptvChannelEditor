/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2022): sharky72 (https://github.com/KocourKuba)

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
		tag_tvg_logo,
		tag_tvg_rec,
		tag_tvg_name,
		tag_tvg_shift,
		tag_timeshift,
		tag_catchup,
		tag_catchup_days,
		tag_catchup_time,
		tag_catchup_type,
		tag_catchup_source,
	};

public:
	m3u_entry() = default;
	m3u_entry(const std::wstring_view& str) { parse(str); };
	virtual ~m3u_entry() = default;

	void clear();
	void parse(const std::wstring_view& str);

	int get_duration() const { return duration; }
	void set_duration(int val) { duration = val; }

	directives get_directive() const { return ext_name; }
	void set_directive(directives val) { ext_name = val; }

	const std::map<info_tags, std::wstring>& get_tags() const { return ext_tags; }
	void set_tags(const std::map<info_tags, std::wstring>& val) { ext_tags = val; }

	const std::wstring& get_dvalue() const { return ext_value; }
	void set_dvalue(const std::wstring& val) { ext_value = val; }

	const std::wstring& get_dir_title() const { return dir_title; }
	void set_dir_title(const std::wstring& val) { dir_title = val; }

protected:
	void parse_directive_tags(std::wstring_view str);

private:
	int duration = 0;
	directives ext_name = directives::ext_unknown;
	std::wstring ext_value;
	std::wstring dir_title;
	std::map<info_tags, std::wstring> ext_tags;
};
