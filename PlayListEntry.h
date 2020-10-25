#pragma once
#include <string>

class PlaylistEntry
{
public:
	PlaylistEntry() = default;

	int id = 0;
	int archive = 0;
	bool notexist = false;
	std::wstring name;
	std::wstring category;
	std::string url;

	void Clear()
	{
		id = 0;
		name.clear();
		category.clear();
		url.clear();
	}
};
