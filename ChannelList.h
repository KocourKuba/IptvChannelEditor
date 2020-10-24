#pragma once
#include <vector>
#include <map>
#include <memory>
#include "ChannelCategory.h"
#include "ChannelInfo.h"

class ChannelList
{
public:
	static constexpr auto TV_INFO = "tv_info";
	static constexpr auto TV_CATEGORIES = "tv_categories";
	static constexpr auto TV_CHANNELS = "tv_channels";

	const std::map<int, std::unique_ptr<ChannelCategory>>& get_categories() const { return categories; }
	std::map<int, std::unique_ptr<ChannelCategory>>& get_categories() { return categories; }

	const std::vector<std::unique_ptr<ChannelInfo>>& get_channels() const { return channels; }
	std::vector<std::unique_ptr<ChannelInfo>>& get_channels() { return channels; }

	const std::set<int>& get_edem_channels() const { return edem_channels; }
	std::set<int>& get_edem_channels() { return edem_channels; }

	int GetFreeCategoryID();
public:
	ChannelList() = default;

	ChannelInfo* CreateChannel();
	bool LoadFromFile(const std::wstring& path);
	bool SaveToFile(const std::wstring& path);

protected:
	std::map<int, std::unique_ptr<ChannelCategory>> categories;
	std::vector<std::unique_ptr<ChannelInfo>> channels;
	std::set<int> edem_channels;
};

