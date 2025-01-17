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

#include "pch.h"
#include "IconCache.h"
#include "IPTVChannelEditor.h"
#include "AccountSettings.h"
#include "Constants.h"

#include "UtilsLib\xxhash.hpp"
#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const CImage& CIconCache::get_icon(const std::wstring& path, bool force /*= false*/)
{
	int hash = xxh::xxhash<32>(path);

	if(!force)
	{
		if (auto pair = m_imageMap.find(hash); pair != m_imageMap.end())
			return pair->second->get_image();
	}

	// not found in cache, try to load
	const auto& unset_url = fmt::format(L"{:s}{:s}{:s}", GetConfig().get_string(true, REG_SAVE_IMAGE_PATH), utils::CHANNELS_LOGO_PATH, L"channel_unset.png");
	auto container = std::make_unique<ImageContainer>();
	if (!LoadImageFromUrl(path, container->get_image()))
	{
		CImage nullImage;
		LoadImageFromUrl(unset_url, nullImage);
		container->set_image(nullImage);
	}

	m_imageMap[hash] = std::move(container);

	return m_imageMap.at(hash)->get_image();
}
