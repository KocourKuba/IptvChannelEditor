#pragma once
#include "vod_episode.h"
#include "uri_base.h"

class vod_movie
{
public:
	std::wstring id;
	std::wstring title;
	std::wstring year;
	std::wstring rating;
	std::wstring age;
	std::wstring country;
	std::wstring director;
	std::wstring casting;
	std::wstring description;
	std::wstring url;
	std::set<std::wstring> genres;
	uri_base poster_url;
	int length = -1; // in minutes

	std::vector<vod_episode> episodes;
};

