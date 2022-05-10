#pragma once
#include "uri_base.h"
#include "UtilsLib/vectormap.h"

class vod_quality
{
public:
	std::wstring title;
	std::wstring url;
};

class vod_episode
{
public:
	std::wstring id;
	std::wstring episode_id;
	std::wstring title;
	std::wstring url;
};

class vod_season
{
public:
	std::wstring id;
	std::wstring season_id;
	std::wstring title;
	utils::vectormap<std::wstring, vod_episode> episodes;
};

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
	std::wstring movie_time; // in minutes
	std::set<std::wstring> genres;
	uri_base poster_url;

	utils::vectormap<std::wstring, vod_season> seasons;
	utils::vectormap<std::wstring, vod_quality> quality;
};

class vod_category
{
public:
	vod_category() = default;
	vod_category(const std::wstring& category_id) : id(category_id) {}
	~vod_category() = default;

public:
	std::wstring id;
	std::wstring name;
	utils::vectormap<std::wstring, std::wstring> genres;
	utils::vectormap<std::wstring, std::shared_ptr<vod_movie>> movies;
};
