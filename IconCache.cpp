#include "pch.h"
#include "IconCache.h"
#include "IPTVChannelEditor.h"

#include "UtilsLib\Crc32.h"
#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const CImage& CIconCache::get_icon(const std::wstring& path)
{
	static CImage nullImage;
	if (!nullImage)
	{
		LoadImage(GetAppPath(utils::CATEGORIES_LOGO_PATH) + L"channel_unset.png", nullImage);
	}

	int hash = crc32_bitwise(path.c_str(), path.size() * sizeof(wchar_t));

	if (auto pair = m_imageMap.find(hash); pair != m_imageMap.end())
		return pair->second->get_image();

	// not found in cache, try to load
	auto container = std::make_unique<ImageContainer>();
	if (LoadImage(path, container->get_image()))
	{
		m_imageMap[hash] = std::move(container);
		return m_imageMap.at(hash)->get_image();
	}

	return nullImage;
}
