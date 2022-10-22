#pragma once

class ThreadConfig
{
public:
	void SendNotifyParent(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);
	void PostNotifyParent(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);

	std::stringstream m_data;
	void* m_parent = nullptr;
	HANDLE m_hStop = nullptr;
	std::wstring m_rootPath;
	std::wstring m_url;
	bool m_use_cache = true;
};
