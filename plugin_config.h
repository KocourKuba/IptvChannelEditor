/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2024): sharky72 (https://github.com/KocourKuba)

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
#include <variant>
#include "PluginDefines.h"
#include "IPTVChannelEditor.h"
#include "UtilsLib\json_wrapper.h"
#include "UtilsLib\inet_utils.h"

#define ENUM_TO_STRING(ENUM_TYPE, ...)                                                                                 \
template<typename ENUM_TYPE, typename StringType>                                                                      \
inline StringType enum_to_string(const ENUM_TYPE e)                                                                    \
{                                                                                                                      \
	static_assert(std::is_enum<ENUM_TYPE>::value, #ENUM_TYPE " must be an enum!");                                     \
	static const std::pair<ENUM_TYPE, StringType> m[] = __VA_ARGS__;                                                   \
	auto it = std::find_if(std::begin(m), std::end(m), [&e](const std::pair<ENUM_TYPE, StringType>& ej_pair)           \
	{                                                                                                                  \
		return ej_pair.first == e;                                                                                     \
	});                                                                                                                \
	return (it != std::end(m)) ? it->second : StringType();                                                            \
}

namespace s_enum
{
enum class StreamType
{
	enHLS = 0,
	enMPEGTS,
};

NLOHMANN_JSON_SERIALIZE_ENUM(StreamType,
{
	{ StreamType::enHLS,    "hls"  },
	{ StreamType::enMPEGTS, "mpeg" }
})

enum class CatchupType {
	cu_shift,
	cu_append,
	cu_flussonic,
	cu_default,
	cu_not_set,
};

NLOHMANN_JSON_SERIALIZE_ENUM(CatchupType,
{
	{ CatchupType::cu_shift,     "shift"     },
	{ CatchupType::cu_append,    "append"    },
	{ CatchupType::cu_flussonic, "flussonic" }
})

enum class AccountAccessType
{
	enUnknown = -1,
	enOtt,
	enPin,
	enLoginPass,
	enNone,
};

NLOHMANN_JSON_SERIALIZE_ENUM(AccountAccessType,
{
	{ AccountAccessType::enUnknown,   "unknown" },
	{ AccountAccessType::enOtt,       "ottkey"  },
	{ AccountAccessType::enPin,       "pin"     },
	{ AccountAccessType::enLoginPass, "login"   },
	{ AccountAccessType::enNone,      "none"    }
})

}
using namespace s_enum;

namespace epg_enum
{

enum class EpgPresets
{
	enDRM = 0,
	enIptvxOne,
	enCbilling,
	enItvLive,
	enPropgNet,
	enTVClub,
	enVidok,
	enMyEPGServer,
	enCustom,
	enLast,
};

ENUM_TO_STRING(EpgPresets,
{
	{ EpgPresets::enDRM,         L"DRM"         },
	{ EpgPresets::enIptvxOne,    L"IptvxOne"    },
	{ EpgPresets::enCbilling,    L"Cbilling"    },
	{ EpgPresets::enItvLive,     L"ItvLive"     },
	{ EpgPresets::enPropgNet,    L"PropgNet"    },
	{ EpgPresets::enTVClub,      L"TVClub"      },
	{ EpgPresets::enVidok,       L"Vidok"       },
	{ EpgPresets::enMyEPGServer, L"MyEPGServer" },
	{ EpgPresets::enCustom,      L"Custom"      },
	{ EpgPresets::enLast,        L"Last"        }
})

}

using namespace epg_enum;

namespace vod_enum
{

enum class VodEngine
{
	enNone = 0,
	enM3U,
	enJson,
	enXC,
	enLast,
};

NLOHMANN_JSON_SERIALIZE_ENUM(VodEngine,
{
	{ VodEngine::enNone,   "None"        },
	{ VodEngine::enM3U,    "M3U"         },
	{ VodEngine::enJson,   "Json"        },
	{ VodEngine::enXC,     "XtreamCodes" },
})

ENUM_TO_STRING(VodEngine,
{
	{ VodEngine::enNone,   L"None"        },
	{ VodEngine::enM3U,    L"M3U"         },
	{ VodEngine::enJson,   L"Json"        },
	{ VodEngine::enXC,     L"XtreamCodes" },
	{ VodEngine::enLast,   L"Last"        },
})

}

using namespace vod_enum;

struct TemplateParams
{
	StreamType streamSubtype = StreamType::enHLS;
	std::wstring ott_key;
	std::wstring subdomain;
	std::wstring port;
	std::wstring token;
	std::wstring s_token;
	std::wstring login;
	std::wstring password;
	std::wstring host;
	std::wstring catchup_source;
	std::wstring catchup_template;
	std::wstring server_id;
	std::wstring device_id;
	std::wstring profile_id;
	std::wstring command;
	std::wstring command_param;
	int shift_back = 0;
	int playlist_idx = 0;
	int server_idx = 0;
	int device_idx = 0;
	int profile_idx = 0;
	int quality_idx = 0;
	int domain_idx = 0;
};

/// <summary>
/// Playlist template parameter
/// </summary>
class PlaylistTemplateInfo
{
public:
	PlaylistTemplateInfo() = default;

	PlaylistTemplateInfo(const std::string& _name) : name(_name) {}

	PlaylistTemplateInfo(UINT ID)
	{
		set_name(load_string_resource(ID));
	}

	std::wstring get_id() const { return L""; }

	std::wstring get_name() const { return utils::utf8_to_utf16(name); }
	void set_name(const std::wstring& val) { name = utils::utf16_to_utf8(val); }
	void set_name(UINT ID) { name = utils::utf16_to_utf8(load_string_resource(ID)); }

	std::wstring get_pl_template() const { return utils::utf8_to_utf16(pl_template); }
	void set_pl_template(const std::wstring& val) { pl_template = utils::utf16_to_utf8(val); }

	std::wstring get_pl_parse_regex() const { return utils::utf8_to_utf16(pl_parse_regex); }
	void set_pl_parse_regex(const std::wstring& val) { pl_parse_regex = utils::utf16_to_utf8(val); }

	std::wstring get_parse_regex() const { return utils::utf8_to_utf16(parse_regex); }
	void set_parse_regex(const std::wstring& val) { parse_regex = utils::utf16_to_utf8(val); }

	std::wstring get_url_prefix() const { return utils::utf8_to_utf16(url_prefix); }
	void set_url_prefix(const std::wstring& val) { url_prefix = utils::utf16_to_utf8(val); }

	std::wstring get_url_params() const { return utils::utf8_to_utf16(url_params); }
	void set_url_params(const std::wstring& val) { url_params = utils::utf16_to_utf8(val); }

	std::wstring get_tag_id_match() const { return utils::utf8_to_utf16(tag_id_match); }
	void set_tag_id_match(const std::wstring& val) { tag_id_match = utils::utf16_to_utf8(val); }

	bool get_epg_id_from_id() const { return epg_id_from_id; }
	void set_epg_id_from_id(bool val) { epg_id_from_id = val; }

	bool get_square_icons() const { return square_icons; }
	void set_square_icons(bool val) { square_icons = val; }

	friend void to_json(nlohmann::json& j, const PlaylistTemplateInfo& c)
	{
		SERIALIZE_STRUCT(j, c, name);
		SERIALIZE_STRUCT(j, c, pl_template);
		SERIALIZE_STRUCT(j, c, pl_parse_regex);
		SERIALIZE_STRUCT(j, c, parse_regex);
		SERIALIZE_STRUCT(j, c, url_prefix);
		SERIALIZE_STRUCT(j, c, url_params);
		SERIALIZE_STRUCT(j, c, tag_id_match);
		SERIALIZE_STRUCT(j, c, epg_id_from_id); //-V601
		SERIALIZE_STRUCT(j, c, square_icons); //-V601
	}

	friend void from_json(const nlohmann::json& j, PlaylistTemplateInfo& c)
	{
		DESERIALIZE_STRUCT(j, c, name);
		DESERIALIZE_STRUCT(j, c, pl_template);
		DESERIALIZE_STRUCT(j, c, pl_parse_regex);
		DESERIALIZE_STRUCT(j, c, parse_regex);
		DESERIALIZE_STRUCT(j, c, url_prefix);
		DESERIALIZE_STRUCT(j, c, url_params);
		DESERIALIZE_STRUCT(j, c, tag_id_match);
		DESERIALIZE_STRUCT(j, c, epg_id_from_id);
		DESERIALIZE_STRUCT(j, c, square_icons);
	}

	std::string name;
	std::string pl_template;
	std::string pl_parse_regex;
	std::string parse_regex;
	std::string url_prefix;
	std::string url_params;
	std::string tag_id_match;
	bool epg_id_from_id = false;
	bool square_icons = false;
	bool is_custom = false;
};

/// <summary>
/// Parameters to parse EPG
/// </summary>
struct EpgParameters
{
	std::string epg_param; // not changed! hardcoded
	std::string epg_domain;
	std::string epg_url;
	std::string epg_root;
	std::string epg_name;
	std::string epg_desc;
	std::string epg_start;
	std::string epg_end;
	std::string epg_date_format;
	std::string epg_time_format;
	size_t epg_timezone = 0;
	bool epg_use_duration = false;

	bool compare_preset(const EpgParameters& src) const
	{
		return (epg_root == src.epg_root
				&& epg_name == src.epg_name
				&& epg_desc == src.epg_desc
				&& epg_start == src.epg_start
				&& epg_end == src.epg_end
				&& epg_time_format == src.epg_time_format
				&& epg_timezone == src.epg_timezone
				&& epg_use_duration == src.epg_use_duration
				);
	}

	std::wstring get_epg_domain() const { return utils::utf8_to_utf16(epg_domain); }
	void set_epg_domain(const std::wstring& val) { epg_domain = utils::utf16_to_utf8(val); }

	std::wstring get_epg_url() const { return utils::utf8_to_utf16(epg_url); }
	void set_epg_url(const std::wstring& val) { epg_url = utils::utf16_to_utf8(val); }

	std::wstring get_epg_root() const { return utils::utf8_to_utf16(epg_root); }
	void set_epg_root(const std::wstring& val) { epg_root = utils::utf16_to_utf8(val); }

	std::wstring get_epg_name() const { return utils::utf8_to_utf16(epg_name); }
	void set_epg_name(const std::wstring& val) { epg_name = utils::utf16_to_utf8(val); }

	std::wstring get_epg_desc() const { return utils::utf8_to_utf16(epg_desc); }
	void set_epg_desc(const std::wstring& val) { epg_desc = utils::utf16_to_utf8(val); }

	std::wstring get_epg_start() const { return utils::utf8_to_utf16(epg_start); }
	void set_epg_start(const std::wstring& val) { epg_start = utils::utf16_to_utf8(val); }

	std::wstring get_epg_end() const { return utils::utf8_to_utf16(epg_end); }
	void set_epg_end(const std::wstring& val) { epg_end = utils::utf16_to_utf8(val); }

	std::wstring get_epg_date_format() const { return utils::utf8_to_utf16(epg_date_format); }
	void set_epg_date_format(const std::wstring& val) { epg_date_format = utils::utf16_to_utf8(val); }

	std::wstring get_epg_time_format() const { return utils::utf8_to_utf16(epg_time_format); }
	void set_epg_time_format(const std::wstring& val) { epg_time_format = utils::utf16_to_utf8(val); }

	friend void to_json(nlohmann::json& j, const EpgParameters& c)
	{
		SERIALIZE_STRUCT(j, c, epg_param);
		SERIALIZE_STRUCT(j, c, epg_domain);
		SERIALIZE_STRUCT(j, c, epg_url);
		SERIALIZE_STRUCT(j, c, epg_root);
		SERIALIZE_STRUCT(j, c, epg_name);
		SERIALIZE_STRUCT(j, c, epg_desc);
		SERIALIZE_STRUCT(j, c, epg_start);
		SERIALIZE_STRUCT(j, c, epg_end);
		SERIALIZE_STRUCT(j, c, epg_date_format);
		SERIALIZE_STRUCT(j, c, epg_time_format);
		SERIALIZE_STRUCT(j, c, epg_timezone);
		SERIALIZE_STRUCT(j, c, epg_use_duration); //-V601
	}

	friend void from_json(const nlohmann::json& j, EpgParameters& c)
	{
		DESERIALIZE_STRUCT(j, c, epg_param);
		DESERIALIZE_STRUCT(j, c, epg_domain);
		DESERIALIZE_STRUCT(j, c, epg_url);
		DESERIALIZE_STRUCT(j, c, epg_root);
		DESERIALIZE_STRUCT(j, c, epg_name);
		DESERIALIZE_STRUCT(j, c, epg_desc);
		DESERIALIZE_STRUCT(j, c, epg_start);
		DESERIALIZE_STRUCT(j, c, epg_end);
		DESERIALIZE_STRUCT(j, c, epg_date_format);
		DESERIALIZE_STRUCT(j, c, epg_time_format);
		DESERIALIZE_STRUCT(j, c, epg_timezone);
		DESERIALIZE_STRUCT(j, c, epg_use_duration);
	}
};

/// <summary>
/// Catchup parameters to generate online and archive streams
/// </summary>
struct StreamParameters
{
	StreamType stream_type = StreamType::enHLS;
	CatchupType cu_type = CatchupType::cu_shift;
	int cu_duration = 10800;

	std::string uri_template;
	std::string uri_arc_template;
	std::string uri_custom_arc_template;
	std::string dune_params;

	std::wstring get_stream_template() const { return utils::utf8_to_utf16(uri_template); }
	std::wstring get_stream_arc_template() const { return utils::utf8_to_utf16(uri_arc_template); }
	std::wstring get_custom_stream_arc_template() const { return utils::utf8_to_utf16(uri_custom_arc_template); }
	std::wstring get_dune_params() const { return utils::utf8_to_utf16(dune_params); }

	void set_uri_template(const std::wstring& value) { uri_template = utils::utf16_to_utf8(value); }
	void set_uri_arc_template(const std::wstring& value) { uri_arc_template = utils::utf16_to_utf8(value); }
	void set_uri_custom_arc_template(const std::wstring& value) { uri_custom_arc_template = utils::utf16_to_utf8(value); }
	void set_dune_params(const std::wstring& value) { dune_params = utils::utf16_to_utf8(value); }

	friend void to_json(nlohmann::json& j, const StreamParameters& c)
	{
		SERIALIZE_STRUCT(j, c, stream_type);
		SERIALIZE_STRUCT(j, c, uri_template);
		SERIALIZE_STRUCT(j, c, uri_arc_template);
		SERIALIZE_STRUCT(j, c, uri_custom_arc_template);
		SERIALIZE_STRUCT(j, c, cu_type);
		SERIALIZE_STRUCT(j, c, cu_duration);
		SERIALIZE_STRUCT(j, c, dune_params);
	}

	friend void from_json(const nlohmann::json& j, StreamParameters& c)
	{
		DESERIALIZE_STRUCT(j, c, stream_type);
		DESERIALIZE_STRUCT(j, c, uri_template);
		DESERIALIZE_STRUCT(j, c, uri_arc_template);
		DESERIALIZE_STRUCT(j, c, uri_custom_arc_template);
		DESERIALIZE_STRUCT(j, c, cu_type);
		DESERIALIZE_STRUCT(j, c, cu_duration);
		DESERIALIZE_STRUCT(j, c, dune_params);
	}
};

enum class DynamicParamsType
{
	enUnknown = -1,
	enServers,
	enDevices,
	enQuality,
	enProfiles,
	enFiles,
	enLinks,
	enManifest,
	enPlaylistTV,
	enPlaylistVOD,
	enDomains,
};

struct DynamicParamsInfo
{
public:
	DynamicParamsInfo() = default;
	DynamicParamsInfo(const std::string& _id, const std::string& _name) : id(_id), name(_name) {}
	DynamicParamsInfo(const std::wstring& _id, const std::wstring& _name) : id(utils::utf16_to_utf8(_id)), name(utils::utf16_to_utf8(_name)) {}

	std::wstring get_id() const { return utils::utf8_to_utf16(id); }
	void set_id(const std::wstring& val) { id = utils::utf16_to_utf8(val); }

	std::wstring get_name() const { return utils::utf8_to_utf16(name); }
	void set_name(const std::wstring& val) { name = utils::utf16_to_utf8(val); }

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(DynamicParamsInfo, id, name);

	std::string id;
	std::string name;
};

class plugin_config
{
public:
	plugin_config();

protected:
	/// <summary>
	/// load default settings
	/// </summary>
	void load_default();

	/// <summary>
	/// fill EPG parsing preset
	/// </summary>
	void FillEpgPresets() const;

public:
	/// <summary>
	/// copy info
	/// </summary>
	void set_config(const PluginType type, const plugin_config& src)
	{
		if (this != &src)
		{
			load_default();
			*this = src;
			set_plugin_type(type);
		}
	}

	bool download_url(const std::wstring& url,
					  std::stringstream& vData,
					  int cache_ttl = 0,
					  std::vector<std::string>* pHeaders = nullptr,
					  bool verb_post = false,
					  const char* post_data = nullptr);

	const std::wstring& get_download_error() { return m_dl.GetLastErrorMessage(); }

	/// <summary>
	/// save plugin parameters to file
	/// </summary>
	bool save_plugin_parameters(const std::wstring& filename, bool use_full_path = false);

	/// <summary>
	/// load plugin parameters to file
	/// </summary>
	void load_plugin_parameters(const std::wstring& filename = L"");

	/// <summary>
	/// set playlist epg url
	/// </summary>
	void set_custom_epg_urls(const std::vector<DynamicParamsInfo>& urls) { custom_epg_urls = urls; }

	/// <summary>
	/// get playlist epg url
	/// </summary>
	const std::vector<DynamicParamsInfo>& get_custom_epg_urls() { return custom_epg_urls; }

	/// <summary>
	/// set playlist epg url
	/// </summary>
	void set_internal_epg_urls(const std::vector<DynamicParamsInfo>& urls) { internal_epg_urls = urls; }

	/// <summary>
	/// get playlist epg url
	/// </summary>
	const std::vector<DynamicParamsInfo>& get_internal_epg_urls() { return internal_epg_urls; }

	/// <summary>
	/// get playlist epg url
	/// </summary>
	std::wstring get_internal_epg_url(const std::wstring& source_id)
	{
		const auto& it = std::find_if(internal_epg_urls.begin(), internal_epg_urls.end(), [source_id](const auto& item)
									  {
										  return item.get_id() == source_id;
									  });
		return it != internal_epg_urls.end() ? it->get_name() : L"";
	}

	/// <summary>
	/// get prefilled EPG parsing preset
	/// </summary>
	EpgParameters get_epg_preset(EpgPresets idx) const;

	/// <summary>
	/// set prefilled EPG parsing preset for selected epg type
	/// </summary>
	void set_epg_preset(size_t epg_idx, EpgPresets idx);

	/// <summary>
	/// get EPG parsing preset index for selected epg type
	/// </summary>
	size_t get_epg_preset_idx(size_t epg_idx) const;

	/// <summary>
	/// set path used to send logs to developer
	/// </summary>
	void set_dev_path(const std::string& path) { dev_code = path; }

	/// <summary>
	/// configure internal plugin settings
	/// </summary>
	virtual void configure_plugin();

	/// <summary>
	/// configure provider plugin settings
	/// </summary>
	virtual void configure_provider_plugin() {}

	/// <summary>
	/// plugin type
	/// </summary>
	PluginType get_plugin_type() const { return plugin_type; }
	void set_plugin_type(const PluginType type) { plugin_type = type; }

	/// <summary>
	/// plugin type name
	/// </summary>
	std::wstring get_type_name() const { return utils::utf8_to_utf16(type_name); }
	const std::string& get_type_name_a() const { return type_name; }

	/// <summary>
	/// plugin class name
	/// </summary>
	const std::string& get_class_name() const { return class_name; }
	void set_class_name(const std::wstring& val) { class_name = utils::utf16_to_utf8(val); }

	/// <summary>
	/// returns link to provider api url
	/// </summary>
	/// <returns>wstring</returns>
	std::wstring get_provider_api_url() const { return utils::utf8_to_utf16(provider_api_url); }
	void set_provider_api_url(const std::wstring& val) { provider_api_url = utils::utf16_to_utf8(val); }

	/// <summary>
	/// property plugin title
	/// </summary>
	std::wstring get_title() const { return utils::utf8_to_utf16(title); }
	void set_title(const std::wstring& val) { title = utils::utf16_to_utf8(val); }

	/// <summary>
	/// property user-agent
	/// </summary>
	std::wstring get_user_agent() const { return utils::utf8_to_utf16(user_agent); }
	void set_user_agent(const std::wstring& val) { user_agent = utils::utf16_to_utf8(val); }

	/// <summary>
	/// property plugin name
	/// </summary>
	const std::string& get_name() const { return name; }
	void set_name(const std::wstring& val) { name = utils::utf16_to_utf8(val); }

	/// <summary>
	/// property plugin access type
	/// </summary>
	const AccountAccessType get_access_type() const { return access_type; }
	void set_access_type(AccountAccessType val) { access_type = val; }

	/// <summary>
	/// property link to provider account
	/// </summary>
	std::wstring get_provider_url() const { return utils::utf8_to_utf16(provider_url); }
	void set_provider_url(const std::wstring& val) { provider_url = utils::utf16_to_utf8(val); }

	/// <summary>
	/// selected playlist template index
	/// </summary>
	size_t get_playlist_idx() const { return playlist_template_index; }
	void set_playlist_idx(size_t idx) { playlist_template_index = idx; }

	/// <summary>
	/// property playlist templates
	/// </summary>
	const std::vector<PlaylistTemplateInfo>& get_playlist_infos() const { return playlist_templates; }
	void set_playlist_infos(const std::vector<PlaylistTemplateInfo>& val) { playlist_templates = val; }

	const PlaylistTemplateInfo& get_playlist_info(size_t idx) const;
	PlaylistTemplateInfo& get_playlist_info(size_t idx);

	const PlaylistTemplateInfo& get_current_playlist_info() const { return get_playlist_info(get_playlist_idx()); };

	/// <summary>
	/// vod engine type
	/// </summary>
	/// <returns>bool</returns>
	VodEngine get_vod_engine() const { return vod_engine; }
	void set_vod_engine(VodEngine val) { vod_engine = val; }

	/// <summary>
	/// property vod filter
	/// </summary>
	bool get_vod_filter() const { return vod_filter; }
	void set_vod_filter(bool val) { vod_filter = val; }

	/// <summary>
	/// property vod templates
	/// </summary>
	const std::vector<PlaylistTemplateInfo>& get_vod_infos() const { return vod_templates; }
	void set_vod_infos(const std::vector<PlaylistTemplateInfo>& val) { vod_templates = val; }

	/// <summary>
	/// selected vod template index
	/// </summary>
	size_t get_vod_info_idx() const { return vod_template_index; }
	void set_vod_info_idx(size_t idx) { vod_template_index = idx; }

	const PlaylistTemplateInfo& get_vod_info(size_t idx) const;
	PlaylistTemplateInfo& get_vod_info(size_t idx);

	const PlaylistTemplateInfo& get_current_vod_info() const { return get_vod_info(get_vod_info_idx()); };

	/// <summary>
	/// property enable show balance info
	/// </summary>
	bool get_balance_support() const { return balance_support; }
	void set_balance_support(bool val) { balance_support = val; }

	/// <summary>
	/// property token requested from provider
	/// </summary>
	bool get_requested_token() const { return requested_token; }
	void set_requested_token(bool val) { requested_token = val; }

	/// <summary>
	/// property supported streams HLS,MPEGTS etc.
	/// </summary>
	/// <returns>vector&</returns>
	const std::array<StreamParameters, 2>& get_supported_streams() const { return streams_config; }

	/// <summary>
	/// property index of supported stream
	/// </summary>
	const StreamParameters& get_supported_stream(size_t idx) const { return streams_config[idx]; }
	StreamParameters& get_supported_stream(size_t idx) { return streams_config[idx]; }

	void set_supported_stream(size_t idx, const StreamParameters& val) { streams_config[idx] = val; }

	/// <summary>
	/// return supported stream
	/// </summary>
	/// <returns>const StreamParameters&</returns>
	const std::array<EpgParameters, 2>& get_epg_parameters() const { return epg_params; }
	std::array<EpgParameters, 2>& get_epg_parameters() { return epg_params; }

	/// <summary>
	/// property epg parameter by index
	/// </summary>
	EpgParameters& get_epg_parameter(int idx) { return epg_params[idx]; };

	/// <summary>
	/// property epg domain
	/// </summary>
	std::wstring get_epg_domain(int idx) { return epg_params[idx].get_epg_domain(); };
	void set_epg_domain(int idx, const std::wstring& val) { epg_params[idx].set_epg_domain(val); };

	/// <summary>
	/// returns json root for epg iteration
	/// </summary>
	/// <param name="epg_idx">index of epg, primary/secondary</param>
	/// <param name="epg_data">downloaded json</param>
	/// <returns>json entry pointed to epg list</returns>
	nlohmann::json get_epg_root(int epg_idx, const nlohmann::json& epg_data) const;

	/// <summary>
	/// property servers list static, not loaded from provider settings
	/// </summary>
	bool get_static_servers() const { return static_servers; }
	void set_static_servers(bool val) { static_servers = val; }

	/// <summary>
	/// property devices list static, not loaded from provider settings
	/// </summary>
	bool get_static_devices() const { return static_devices; }
	void set_static_devices(bool val) { static_devices = val; }

	/// <summary>
	/// is qualities list static, not loaded from provider settings
	/// </summary>
	bool get_static_qualities() const { return static_qualities; }
	void set_static_qualities(bool val) { static_qualities = val; }

	/// <summary>
	/// property profiles list static, not loaded from provider settings
	/// </summary>
	bool get_static_profiles() const { return static_profiles; }
	void set_static_profiles(bool val) { static_profiles = val; }

	/// <summary>
	/// property domain list static, not loaded from provider settings
	/// </summary>
	bool get_static_domains() const { return static_domains; }
	void set_static_domains(bool val) { static_domains = val; }

	/// <summary>
	/// property list external bin files (puts in /bin folder)
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	virtual const std::vector<DynamicParamsInfo>& get_files_list() { return files_list; }
	virtual void set_files_list(const std::vector<DynamicParamsInfo>& info) { files_list = info; }

	/// <summary>
	/// property list for plugin manifest
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	virtual const std::vector<DynamicParamsInfo>& get_manifest_list() { return manifest_list; }
	virtual void set_manifest_list(const std::vector<DynamicParamsInfo>& info) { manifest_list = info; }

	/// <summary>
	/// clear servers list
	/// </summary>
	void clear_servers_list() { servers_list.clear(); }

	/// returns list of servers
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	virtual void fill_servers_list(TemplateParams* params = nullptr) {}

	/// <summary>
	/// set server
	/// </summary>
	/// <param name="params">Template parameters.</param>
	virtual bool set_server(TemplateParams& /*params*/) { return true; }

	/// <summary>
	/// property list of servers
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	virtual const std::vector<DynamicParamsInfo>& get_servers_list() { return servers_list; }
	virtual void set_servers_list(const std::vector<DynamicParamsInfo>& info) { servers_list = info; }

	/// <summary>
	/// clear device list
	/// </summary>
	void clear_device_list() { devices_list.clear(); }

	/// <summary>
	/// returns list of device variants
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	/// <returns>vector<QualityInfo></returns>
	virtual void fill_devices_list(TemplateParams* params = nullptr) {}

	/// <summary>
	/// set device
	/// </summary>
	/// <param name="params">Template parameters.</param>
	virtual bool set_device(TemplateParams& /*params*/) { return true; }

	/// <summary>
	/// property list of quality variants
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	virtual const std::vector<DynamicParamsInfo>& get_devices_list() { return devices_list; }
	virtual void set_devices_list(const std::vector<DynamicParamsInfo>& info) { devices_list = info; }

	/// <summary>
	/// clear qualities list
	/// </summary>
	void clear_qualities_list() { qualities_list.clear(); }

	/// <summary>
	/// fill list of quality variants
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	/// <returns>vector<QualityInfo></returns>
	virtual void fill_qualities_list(TemplateParams* params = nullptr) {}

	/// <summary>
	/// set quality
	/// </summary>
	/// <param name="params">Template parameters.</param>
	virtual bool set_quality(TemplateParams& /*params*/) { return true; }

	/// <summary>
	/// property list of quality variants
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	virtual const std::vector<DynamicParamsInfo>& get_qualities_list() { return qualities_list; }
	virtual void set_qualities_list(const std::vector<DynamicParamsInfo>& info) { qualities_list = info; }

	/// <summary>
	/// clear profile list
	/// </summary>
	void clear_profiles_list() { profiles_list.clear(); }

	/// <summary>
	/// fill list of profiles variants
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	/// <returns>vector<QualityInfo></returns>
	virtual void fill_profiles_list(TemplateParams* params = nullptr) {}

	/// <summary>
	/// set profile
	/// </summary>
	/// <param name="params">Template parameters.</param>
	virtual bool set_profile(TemplateParams& /*params*/) { return true; }

	/// <summary>
	/// property list of profiles
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	virtual const std::vector<DynamicParamsInfo>& get_profiles_list() { return profiles_list; }
	virtual void set_profiles_list(const std::vector<DynamicParamsInfo>& info) { profiles_list = info; }

	/// <summary>
	/// set profile
	/// </summary>
	/// <param name="params">Template parameters.</param>
	virtual bool set_domain(TemplateParams& /*params*/) { return true; }

	/// <summary>
	/// fill list of domains variants
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	/// <returns>vector<QualityInfo></returns>
	virtual void fill_domains_list(TemplateParams* params = nullptr) {}

	/// <summary>
	/// property list of domains
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	virtual const std::vector<DynamicParamsInfo>& get_domains_list() { return domains_list; }
	virtual void set_domains_list(const std::vector<DynamicParamsInfo>& info) { domains_list = info; }

	static void to_json(nlohmann::json& j, const plugin_config& c);
	static void from_json(const nlohmann::json& j, plugin_config& c);

protected:

	static std::array<EpgParameters, (size_t)EpgPresets::enCustom> known_presets;

	utils::CUrlDownload m_dl;

	// non configurable parameters
	PluginType plugin_type = PluginType::enCustom;
	std::string type_name;
	std::vector<DynamicParamsInfo> internal_epg_urls;

	// configurable parameters

	// name of specialized php class for plugin
	std::string class_name;

	// plugin access type
	AccountAccessType access_type = AccountAccessType::enNone;
	// plugin title
	std::string title;
	// plugin user-agent
	std::string user_agent;
	// plugin internal name (used by Dune)
	std::string name;
	// url to provider account
	std::string provider_url;
	// developer url
	std::string dev_code;

	// provider api url
	std::string provider_api_url;

	// vod engine
	VodEngine vod_engine = VodEngine::enNone;

	// show balance info in plugin
	bool balance_support = false;
	// use channels logo are squared, plugin UI settings
	bool vod_filter = false;
	// use token generated or received from provider
	bool requested_token = false;
	// flag for php plugin if uri does not contains parsed 'id' for channel
	bool static_servers = false;
	bool static_qualities = false;
	bool static_devices = false;
	bool static_profiles = false;
	bool static_domains = true;
	// selected playlist template
	size_t playlist_template_index = 0;
	// available playlist templates
	std::vector<PlaylistTemplateInfo> playlist_templates;
	// selected vod template
	size_t vod_template_index = 0;
	// available vod templates
	std::vector<PlaylistTemplateInfo> vod_templates;
	// setting for parsing uri streams
	std::array<StreamParameters, 2> streams_config;
	// setting for parsing json EPG
	std::array<EpgParameters, 2> epg_params;
	std::vector<DynamicParamsInfo> files_list;
	std::vector<DynamicParamsInfo> manifest_list;
	std::vector<DynamicParamsInfo> servers_list;
	std::vector<DynamicParamsInfo> qualities_list;
	std::vector<DynamicParamsInfo> devices_list;
	std::vector<DynamicParamsInfo> profiles_list;
	std::vector<DynamicParamsInfo> domains_list;
	std::array<EpgParameters, 4> epg_presets;
	std::vector<DynamicParamsInfo> custom_epg_urls;
};
