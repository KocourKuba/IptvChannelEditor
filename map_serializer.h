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

class serializable_map : public std::map<int, std::pair<std::string, std::string>>
{
private:
	size_t offset = 0;

	void write(std::stringstream& ss, const int& key) const
	{
		ss.write((const char*)&key, sizeof(int));
	}

	void write(std::stringstream& ss, const std::string& str) const
	{
		uint32_t size = (uint32_t)str.size();
		ss.write((const char*)&size, sizeof(uint32_t));
		ss.write(str.c_str(), size);
	}

	void write(std::stringstream& ss, const std::pair<std::string, std::string>& pair) const
	{
		write(ss, pair.first);
		write(ss, pair.second);
	}

	void read(std::vector<char>& buffer, int& key)
	{
		key = *((int*)(buffer.data() + offset));
		offset += sizeof(int);
	}

	void read(std::vector<char>& buffer, std::string& str)
	{
		uint32_t size = *((uint32_t*)(buffer.data() + offset));
		offset += sizeof(uint32_t);
		std::string data(buffer.data() + offset, buffer.data() + offset + size);
		str.swap(data);
		offset += size;
	}

	void read(std::vector<char>& buffer, std::pair<std::string, std::string>& pair)
	{
		read(buffer, pair.first);
		read(buffer, pair.second);
	}

public:

	std::vector<char> serialize()
	{
		std::stringstream ss;
		for (const auto& i : *this)
		{
			write(ss, i.first);
			write(ss, i.second);
		}

		std::vector<char> buffer((std::istreambuf_iterator<char>(ss)), std::istreambuf_iterator<char>());

		return buffer;
	}

	void deserialize(std::vector<char>& buffer)
	{
		offset = 0;
		while (offset < buffer.size())
		{
			int key = 0;
			read(buffer, key);
			std::pair<std::string, std::string> value;
			read(buffer, value);
			(*this)[key] = value;
		}
	}
};
