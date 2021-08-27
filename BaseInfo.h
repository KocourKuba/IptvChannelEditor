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
	virtual const std::string& get_id() const { return stream_uri->get_id(); }
	virtual void set_id(const std::string& val) { stream_uri->set_id(val); }

	const int get_key() const { return key; }
	void set_key(const int val) { key = val; }

	const std::wstring& get_title() const { return title; }
	void set_title(const std::wstring& val) { title = val; }

	int get_epg1_id() const { return epg_id; }
	void set_epg1_id(int val) { epg_id = val; }

	int get_epg2_id() const { return tvg_id; }
	void set_epg2_id(int val) { tvg_id = val; }

	int get_adult() const { return adult; }
	void set_adult(int val) { adult = val; }

	int get_archive() const { return archive; }
	void set_archive(int val) { archive = val; }

	void swap_id(BaseInfo& src)
	{
		std::string tmp = src.get_id();
		src.set_id(get_id());
		set_id(tmp);
	}

	bool is_type(InfoType type) const { return base_type == type; }

protected:
	InfoType base_type = InfoType::enUndefined;

private:
	std::wstring title;
	int key = 0;
	int tvg_id = 0; // TVGuide id http://www.teleguide.info/kanal%d.html
	int epg_id = 0; // ott-play epg http://epg.ott-play.com/edem/epg/%d.json
	int time_shift_hours = 0;
	int adult = 0;
	int archive = 0;
	bool disabled = false;
	bool favorite = false;
	std::set<int> categories;
	bool changed = false;
};
