#pragma once
#include "uri_base.h"
#include "UtilsLib\vectormap.h"

class vod_filter
{
public:
	std::wstring id;
	std::wstring title;
};

using vod_filter_storage = utils::vectormap<std::wstring, vod_filter>;

class vod_genre
{
public:
	std::wstring id;
	std::wstring title;
};

using vod_genre_storage = utils::vectormap<std::wstring, vod_genre>;

class vod_quality
{
public:
	std::wstring title;
	std::wstring url;
};

using vod_quality_storage = utils::vectormap<std::wstring, vod_quality>;

class vod_episode
{
public:
	std::wstring id;
	std::wstring episode_id;
	std::wstring title;
	std::wstring url;
	vod_quality_storage quality;
};

using vod_episode_episode = utils::vectormap<std::wstring, vod_episode>;

class vod_season
{
public:
	std::wstring id;
	std::wstring season_id;
	std::wstring title;
	vod_episode_episode episodes;
};

using vod_season_storage = utils::vectormap<std::wstring, vod_season>;

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
	vod_genre_storage genres;
	uri_base poster_url;
	vod_season_storage seasons;
	vod_quality_storage quality;
};

using vod_movie_storage = utils::vectormap<std::wstring, std::shared_ptr<vod_movie>>;

class vod_category
{
public:
	vod_category() = default;
	vod_category(const std::wstring& category_id) : id(category_id) {}
	~vod_category() = default;

public:
	std::wstring id;
	std::wstring name;
	utils::vectormap<std::wstring, vod_filter_storage> filters;
	vod_genre_storage genres;
	vod_movie_storage movies;
};

using vod_category_storage = utils::vectormap<std::wstring, std::shared_ptr<vod_category>>;
