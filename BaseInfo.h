#pragma once
#include "StreamContainer.h"
#include "IconContainer.h"

enum class BaseType { enUndefined, enChannel, enCategory, enPlEntry };

class BaseInfo
	: public StreamContainer
	, public IconContainer
{
public:
	BaseInfo() = default;
	BaseInfo(BaseType type) : base_type(type) {};

public:
	virtual int get_id() const { return get_stream_uri().get_Id(); }
	virtual void set_id(int val) { get_stream_uri().set_Id(val); }

	const std::wstring& get_title() const { return title; }
	void set_title(const std::wstring& val) { title = val; }

	int get_tvg_id() const { return tvg_id; }
	void set_tvg_id(int val) { tvg_id = val; }

	int get_epg_id() const { return epg_id; }
	void set_epg_id(int val) { epg_id = val; }

	int get_adult() const { return adult; }
	void set_adult(int val) { adult = val; }

	int get_archive() const { return archive; }
	void set_archive(int val) { archive = val; }

	void swap_id(BaseInfo& src) { int tmp = src.get_id(); src.set_id(get_id()); set_id(tmp); }

	BaseType get_type() const { return base_type; }

protected:
	BaseType base_type = BaseType::enUndefined;

private:
	std::wstring title;
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
