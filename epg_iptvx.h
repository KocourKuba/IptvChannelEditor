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
#include "uri_stream.h"

class epg_iptvx : public uri_stream
{
public:

	epg_iptvx()
	{
		secondary_epg = true;

		auto& params1 = epg_params[0];
		params1.epg_url = L"http://epg.iptvx.one/api/id/{ID}.json";
		params1.epg_root = "ch_programme";
		params1.epg_name = "title";
		params1.epg_desc = "description";
		params1.epg_start = "start";
		params1.epg_end = "";
		params1.epg_time_format = "%d-%m-%Y %H:%M";
		params1.epg_tz = 3600 * 3; // iptvx.one uses moscow time (GMT+3)

		auto& params2 = epg_params[1];
		params2.epg_use_mapper = true;
		params2.epg_url = L"http://technic.cf/epg-iptvxone/epg_day?id={ID}&day={DATE}";
		params2.epg_mapper_url = L"http://technic.cf/epg-iptvxone/channels";
		params2.epg_date_format = L"{:04d}.{:02d}.{:02d}";
		params2.epg_root = "data";
		params2.epg_name = "title";
		params2.epg_desc = "description";
		params2.epg_start = "begin";
		params2.epg_end = "end";
	}

	virtual ~epg_iptvx() = default;
};
