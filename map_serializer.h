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
		size_t size = str.size();
		ss.write((const char*)&size, sizeof(size_t));
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
		size_t size = *((size_t*)(buffer.data() + offset));
		offset += sizeof(size_t);
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
