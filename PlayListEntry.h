#pragma once
#include "uri.h"
#include "ColoringProperty.h"
#include "IconContainer.h"
#include "InfoContainer.h"

enum directives
{
	ext_unknown = -1,
	ext_pathname,
	ext_header,
	ext_group,
	ext_playlist,
	ext_info,
};

enum info_tags
{
	tag_unknown = -1,
	tag_channel_id,
	tag_group_title,
	tag_tvg_id,
	tag_tvg_logo,
	tag_tvg_rec,
	tag_tvg_name,
	tag_tvg_shift,
};

class m3u_entry
{
public:
	m3u_entry() = default;
	virtual ~m3u_entry() = default;

	virtual void Clear();
	virtual void Parse(const std::string& str);

	int get_duration() const { return duration; }
	void set_duration(int val) { duration = val; }

	directives get_directive() const { return ext_name; }
	void set_directive(directives val) { ext_name = val; }

	const std::map<info_tags, std::string>& get_tags() const { return ext_tags; }
	void set_tags(const std::map<info_tags, std::string>& val) { ext_tags = val; }

	const std::string& get_dvalue() const { return ext_value; }
	void set_dvalue(const std::string& val) { ext_value = val; }

	const std::string& get_title() const { return dir_title; }
	void set_title(const std::string& val) { dir_title = val; }

protected:
	void ParseDirectiveTags(const std::string& str);

protected:
	int duration = 0;
	directives ext_name = ext_unknown;
	std::string ext_value;
	std::string dir_title;
	std::map<info_tags, std::string> ext_tags;
};

class PlaylistEntry
	: public m3u_entry
	, public IconContainer
	, public InfoContainer
	, public ColoringProperty
{
public:
	PlaylistEntry() = default;
	PlaylistEntry(const PlaylistEntry& src) = default;
	PlaylistEntry(const std::string& str) { Parse(str); }

	void Parse(const std::string& str) override;
	void Clear() override;

	int get_channel_id() const { return stream_uri.get_Id(); }
	int get_channel_length() const { return channel_len; }
	int get_tvg_id() { return tvg_id; };
	bool is_archive() const { return archive != 0; }

	const std::wstring& get_title() const { return title; }
	const std::wstring& get_category() const { return category; }
	const uri_stream& get_stream_uri() const { return stream_uri; }

	const std::string& get_domain() const { return domain; }
	const std::string& get_access_key() const { return access_key; }

protected:
	int archive = 0;
	int tvg_id = 0;
	int channel_len = 0;
	std::wstring title;
	std::wstring category;
	uri_stream stream_uri;
	std::string domain;
	std::string access_key;
};
