// PlaylistParseThread.cpp : implementation file
//

#include "pch.h"
#include "PlaylistParseXMLThread.h"
#include "IPTVChannelEditor.h"
#include "PlayListEntry.h"
#include "ChannelCategory.h"
#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CPlaylistParseXMLThread, CWinThread)

BOOL CPlaylistParseXMLThread::InitInstance()
{
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	auto playlist = std::make_unique<Playlist>();
	if (m_config.m_data)
	{
		utils::vector_to_streambuf<char> buf(*m_config.m_data);
		std::istream stream(&buf);
		if (stream.good())
		{
			// Read the xml file into a vector
			std::vector<char> buffer((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
			buffer.emplace_back('\0');

			// Parse the buffer using the xml file parsing library into doc
			rapidxml::xml_document<> doc;

			try
			{
				doc.parse<0>(buffer.data());
			}
			catch (rapidxml::parse_error& ex)
			{
				ex;
				return false;
			}

			const auto& root_path = GetAppPath(utils::PLUGIN_ROOT);
			auto cat_node = doc.first_node(utils::TV_CATEGORIES)->first_node(utils::TV_CATEGORY);
			// Iterate <tv_category> nodes
			while (cat_node)
			{
				auto category = std::make_shared<ChannelCategory>(cat_node, StreamType::enBase, root_path);
				playlist->categories.emplace(category->get_key(), category);
				cat_node = cat_node->next_sibling();
			}

			auto ch_node = doc.first_node(utils::TV_CHANNELS)->first_node(utils::TV_CHANNEL);
			// Iterate <tv_channel> nodes
			int step = 0;
			int count = 0;
			while (ch_node)
			{
				auto channel = std::make_shared<ChannelInfo>(ch_node, StreamType::enChannels, root_path);
				count++;

				auto entry = std::make_shared<PlaylistEntry>(m_config.m_pluginType, m_config.m_rootPath);
				entry->set_key(channel->get_key());
				entry->set_title(channel->get_title());
				entry->set_icon_uri(channel->get_icon_uri());
				entry->set_epg1_id(channel->get_epg1_id());
				entry->set_archive_days(channel->get_archive_days());
				entry->set_adult(channel->get_adult());
				entry->stream_uri->copy(channel->stream_uri);
				playlist->m_entries.emplace_back(entry);
				ch_node = ch_node->next_sibling();
				m_config.NotifyParent(WM_UPDATE_PROGRESS, step++, count);
			}
		}
	}

	m_config.NotifyParent(WM_END_LOAD_PLAYLIST, (WPARAM)playlist.release());

	CoUninitialize();

	return FALSE;
}
