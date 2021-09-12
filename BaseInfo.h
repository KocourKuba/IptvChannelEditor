#pragma once
#include "StreamContainer.h"
#include "IconContainer.h"

enum class InfoType { enUndefined, enChannel, enCategory, enPlEntry };

class BaseInfo
	: public StreamContainer
	, public IconContainer
{
public:
	BaseInfo() = default;
	BaseInfo(InfoType type, StreamType streamType, const std::wstring& root_path)
		: StreamContainer(streamType)
		, IconContainer(root_path)
		, base_type(type) {};

public:
	const int get_key() const { return key; }
	void set_key(const int val) { key = val; }

	const std::wstring& get_title() const { return title; }
	void set_title(const std::wstring& val) { title = val; }

	std::string get_epg1_id() const { return epg_id1; }
	void set_epg1_id(const std::string& val) { epg_id1 = val; }

	std::string get_epg2_id() const { return epg_id2; }
	void set_epg2_id(const std::string& val) { epg_id2 = val; }

	int get_adult() const { return adult; }
	void set_adult(int val) { adult = val; }

	bool is_archive() const { return archive_days > 0; }

	int get_archive_days() const { return archive_days; }
	void set_archive_days(int val) { archive_days = val; }

	const std::string& get_archive_source() const { return archive_source; }
	void set_archive_source(const std::string& val) { archive_source = val; }

	void swap_id(BaseInfo& src)
	{
		std::string tmp = src.stream_uri->get_id();
		src.stream_uri->set_id(stream_uri->get_id());
		src.stream_uri->set_id(tmp);
	}

	bool is_type(InfoType type) const { return base_type == type; }

protected:
	InfoType base_type = InfoType::enUndefined;

private:
	std::wstring title;
	int key = 0;
	std::string epg_id1; // primary epg source ott-play epg http://epg.ott-play.com/edem/epg/%d.json
	std::string epg_id2; // secondary epg source TVGuide id http://www.teleguide.info/kanal%d.html
	std::string archive_source;
	int time_shift_hours = 0;
	int adult = 0;
	int archive_days = 0;
	bool disabled = false;
	bool favorite = false;
	std::set<int> categories;
};
