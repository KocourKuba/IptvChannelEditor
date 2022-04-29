#pragma once
#include "vod_movie.h"
#include "UtilsLib/vectormap.h"

class vod_category
{
public:
	vod_category(const std::wstring& category_id) : id(category_id) {}
	~vod_category() = default;

public:
	std::wstring id;
	utils::vectormap<std::wstring, std::shared_ptr<vod_movie>> movies;
};
