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
#include "uri_base.h"
#include "UtilsLib\vectormap.h"

struct vod_item_def
{
	std::wstring id;
	std::wstring title;
};

using vod_filter_def = vod_item_def;
using vod_genre_def = vod_item_def;

using vod_filter_storage = utils::vectormap<std::wstring, vod_filter_def>;
using vod_genre_storage = utils::vectormap<std::wstring, vod_genre_def>;

struct vod_variant_def : public vod_item_def
{
	std::wstring url;
};

using vod_variants_storage = utils::vectormap<std::wstring, vod_variant_def>;

struct vod_episode_def : public vod_variant_def
{
	std::wstring number;
	vod_variants_storage qualities;
	vod_variants_storage audios;
};

using vod_episodes_storage = utils::vectormap<std::wstring, vod_episode_def>;

struct vod_season_def : public vod_item_def
{
	std::wstring number;
	std::wstring year;
	vod_episodes_storage episodes;
};

using vod_season_storage = utils::vectormap<std::wstring, vod_season_def>;

struct vod_movie_def : public vod_episode_def
{
	std::wstring title_orig;
	std::wstring year;
	std::wstring rating;
	std::wstring age;
	std::wstring country;
	std::wstring director;
	std::wstring casting;
	std::wstring description;
	std::wstring category;
	bool is_series = false;
	int movie_time = 0; // in minutes
	vod_genre_storage genres;
	uri_base poster_url;
	vod_season_storage seasons;

	std::map<std::wstring, std::wstring*, std::less<>> parser_mapper = {
		{L"id"          , &id},
		{L"title"       , &title},
		{L"title_orig"  , &title_orig},
		{L"year"        , &year},
		{L"rating"      , &rating},
		{L"age"         , &age},
		{L"country"     , &country},
		{L"director"    , &director},
		{L"casting"     , &casting},
		{L"description" , &description},
		{L"category"    , &category},
	};
};

using vod_movie_storage = utils::vectormap<std::wstring, std::shared_ptr<vod_movie_def>>;

struct vod_category
{
	vod_category() = default;
	vod_category(const std::wstring& category_id) : id(category_id) {}

	std::wstring id;
	std::wstring name;
	utils::vectormap<std::wstring, vod_filter_storage> filters;
	vod_genre_storage genres;
	vod_genre_storage years;
	vod_movie_storage movies;
};

using vod_category_storage = utils::vectormap<std::wstring, std::shared_ptr<vod_category>>;
