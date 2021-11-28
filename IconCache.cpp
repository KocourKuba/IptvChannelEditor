#include "StdAfx.h"
#include "IconCache.h"
#include "utils.h"
#include "Crc32.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const CImage& CIconCache::get_icon(const std::wstring& path)
{
	static CImage nullImage;

	int hash = crc32_bitwise(path.c_str(), path.size() * sizeof(wchar_t));

	if (auto pair = m_imageMap.find(hash); pair != m_imageMap.end())
		return pair->second->get_image();

	// not found in cache, try to load
	auto container = std::make_unique<ImageContainer>();
	if (utils::LoadImage(path, container->get_image()))
	{
		return m_imageMap.emplace(hash, std::move(container)).first->second->get_image();
	}

	return nullImage;
}
