#pragma once
#include "ImageContainer.h"

class CIconCache
{
public:
	static CIconCache* Instance()
	{
		static CIconCache* _instance = new CIconCache();
		return _instance;
	}

#ifdef _DEBUG
	static void DestroyInstance()
	{
		delete Instance();
	}
#endif // _DEBUG

	const CImage& get_icon(const std::wstring& path);

protected:
	CIconCache() = default;
	virtual ~CIconCache() = default;

private:
	CIconCache(const CIconCache& source) = delete;
	std::map<int, std::unique_ptr<ImageContainer>> m_imageMap;
};

inline CIconCache& GetIconCache() { return *CIconCache::Instance(); }
