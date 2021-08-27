#include "StdAfx.h"
#include "IconCache.h"
#include "utils.h"

const CImage& CIconCache::get_icon(const std::wstring& name, const std::wstring& path)
{
	static CImage nullImage;

	if (auto pair = m_imageMap.find(name); pair != m_imageMap.end())
		return pair->second->get_image();

	// not found in cache, try to load
	auto container = std::make_unique<ImageContainer>();
	if (utils::LoadImage(path, container->get_image()))
	{
		return m_imageMap.emplace(name, std::move(container)).first->second->get_image();
	}

	return nullImage;
}
