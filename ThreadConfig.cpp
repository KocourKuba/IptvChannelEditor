#include "pch.h"
#include "ThreadConfig.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void ThreadConfig::SendNotifyParent(UINT message, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	CWnd* parent = (CWnd*)m_parent;
	if (parent->GetSafeHwnd())
		parent->SendMessage(message, wParam, lParam);

}

void ThreadConfig::PostNotifyParent(UINT message, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	CWnd* parent = (CWnd*)m_parent;
	if (parent->GetSafeHwnd())
		parent->PostMessage(message, wParam, lParam);
}
