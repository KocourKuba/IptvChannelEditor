#include "pch.h"
#include "Credentials.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void Credentials::Clear()
{
	login.clear();
	password.clear();
	token.clear();
	subdomain.clear();
	portal.clear();
	comment.clear();
	suffix.clear();
	caption.clear();
	config.clear();
	logo.clear();
	background.clear();
	update_url.clear();
	update_package_url.clear();
	update_name.clear();
	package_name.clear();
	version_id.clear();
	ch_web_path.clear();
	ch_list.clear();
	custom_increment = 0;
	custom_update_name = 0;
	custom_package_name = 0;
	server_id = 0;
	profile_id = 0;
	embed = 0;
	not_valid = false;
}

