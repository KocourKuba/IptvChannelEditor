#pragma once
#include <map>
#include <memory>
#include <string>
#include "ImageContainer.h"

class CIconCache
{
public:
	static CIconCache& Instance()
	{
		static CIconCache _instance;
		return _instance;
	}

	const CImage& get_icon(const std::wstring& name, const std::wstring& path);

protected:
	CIconCache() = default;
	virtual ~CIconCache() = default;

private:
	CIconCache(const CIconCache& source) = delete;
	std::map<int, std::unique_ptr<ImageContainer>> m_imageMap;
};

inline CIconCache& GetIconCache() { return CIconCache::Instance(); }
