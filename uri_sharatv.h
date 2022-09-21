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

class uri_sharatv : public uri_stream
{
	public:uri_sharatv()
	{
		short_name = "sharatv";
	}

	void load_default() override
	{
		title = "Shara TV";
		name = "shara.tv";
		access_type = AccountAccessType::enLoginPass;

		provider_url = "https://shara-tv.org/";
		playlist_template = "http://tvfor.pro/g/{LOGIN}:{PASSWORD}/1/playlist.m3u";
		uri_parse_template = R"(^https?:\/\/(?<domain>.+)\/(?<id>.+)\/(?<token>.+)$)";

		streams_config[0].uri_template = "http://{DOMAIN}/{ID}/{TOKEN}";
		streams_config[0].uri_arc_template = "{CU_SUBST}={START}&lutc={NOW}";

		epg_params[0].epg_url = "http://epg.drm-play.ml/shara-tv/epg/{ID}.json";
	}
};
