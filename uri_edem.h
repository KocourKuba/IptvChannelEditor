#pragma once
#include "epg_technic.h"

class uri_edem : public epg_technic
{
public:

	uri_edem() : epg_technic({L"it999", L"it999" }) { m_use_mapper[0] = false; }

	void parse_uri(const std::wstring& url) override;
	std::wstring get_templated_stream(StreamSubType subType, const TemplateParams& params) const override;
	std::wstring get_playlist_template(const PlaylistTemplateParams& params) const override;

	std::vector<std::tuple<StreamSubType, std::wstring>>& get_supported_stream_type() const override
	{
		static std::vector<std::tuple<StreamSubType, std::wstring>> streams = { {StreamSubType::enHLS, L"HLS"} };
		return streams;
	};
};
