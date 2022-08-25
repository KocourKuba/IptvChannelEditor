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
#include "StreamContainer.h"
#include "IconContainer.h"

enum class InfoType { enUndefined, enChannel, enCategory, enPlEntry, enPlCategory };

class BaseInfo
	: public StreamContainer
	, public IconContainer
{
public:
	BaseInfo() = default;
	BaseInfo(const BaseInfo& src) : StreamContainer(src.stream_type)
	{
		*this = src;
	}

	BaseInfo(InfoType type, StreamType streamType, std::wstring root_path)
		: StreamContainer(streamType)
		, IconContainer(root_path)
		, base_type(type) {};

public:
	const int get_key() const { return key; }
	void set_key(const int val) { key = val; }

	const std::wstring& get_title() const { return title; }
	void set_title(const std::wstring& val) { title = val; }

	std::wstring get_epg_id(int idx = 0) const { return epg_id[idx]; }
	void set_epg_id(int idx, const std::wstring& val) { epg_id[idx] = val; }

	int get_adult() const { return adult; }
	void set_adult(int val) { adult = val; }

	bool is_archive() const { return archive_days > 0; }

	int get_archive_days() const { return archive_days; }
	void set_archive_days(int val) { archive_days = val; }

	int get_time_shift_hours() const { return time_shift_hours; }
	void set_time_shift_hours(int val) { time_shift_hours = val; }

	const std::wstring& get_catchup() const { return catchup; }
	void set_catchup(const std::wstring& val) { catchup = val; }

	void swap_id(BaseInfo& src)
	{
		auto tmp = src.stream_uri->get_id();
		src.stream_uri->set_id(stream_uri->get_id());
		src.stream_uri->set_id(tmp);
	}

	bool is_type(InfoType type) const { return base_type == type; }

	const BaseInfo& operator=(const BaseInfo& src)
	{
		if (this != &src)
		{
			StreamContainer::operator=(src);
			IconContainer::operator=(src);
			base_type = src.base_type;
			title = src.title;
			key = src.key;
			epg_id = src.epg_id;
			time_shift_hours = src.time_shift_hours;
			adult = src.adult;
			archive_days = src.archive_days;
		}

		return *this;
	}

protected:
	InfoType base_type = InfoType::enUndefined;

private:
	std::wstring title;
	std::wstring catchup;
	int key = 0;
	std::array<std::wstring, 2> epg_id; // epg id
	int time_shift_hours = 0;
	int adult = 0;
	int archive_days = 0;
};
