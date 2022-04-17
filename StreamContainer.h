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
#include "Config.h"

/// <summary>
/// Container for stream interface
/// </summary>
class StreamContainer
{
public:
	StreamContainer() = delete;
	StreamContainer(StreamType type);
	~StreamContainer() = default;

	static std::unique_ptr<uri_stream> get_instance(StreamType type);

	void set_type(StreamType type);

	const StreamContainer& operator=(const StreamContainer& src)
	{
		if (this != &src)
		{
			*stream_uri = *src.stream_uri;
			stream_type = src.stream_type;
		}

		return *this;
	}

	std::unique_ptr<uri_stream> stream_uri;
	StreamType stream_type;
};
