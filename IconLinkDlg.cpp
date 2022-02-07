// IconLink.cpp : implementation file
//

#include "pch.h"
#include <afxdialogex.h>
#include "IconLinkDlg.h"
#include "IPTVChannelEditor.h"
#include "IconCache.h"


// CIconLink dialog

IMPLEMENT_DYNAMIC(CIconLinkDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CIconLinkDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_GET, &CIconLinkDlg::OnBnClickedButtonGet)
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()

CIconLinkDlg::CIconLinkDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_ICON_LINK, pParent)
	, m_url(_T(""))
{

}

CIconLinkDlg::~CIconLinkDlg()
{
}

BOOL CIconLinkDlg::OnInitDialog()
{
	__super::OnInitDialog();

	OnBnClickedButtonGet();

	return TRUE;
}

void CIconLinkDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_URL, m_url);
	DDX_Control(pDX, IDC_STATIC_ICON, m_wndIcon);
}

// CIconLink message handlers

void CIconLinkDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	CMFCDynamicLayout* layout = GetDynamicLayout();

	if (layout)
	{
		CSize size = layout->GetMinSize();
		CRect rect(0, 0, size.cx, size.cy);
		AdjustWindowRect(&rect, GetStyle(), FALSE);
		lpMMI->ptMinTrackSize.x = rect.Width();
		lpMMI->ptMinTrackSize.y = rect.Height();
	}

	__super::OnGetMinMaxInfo(lpMMI);
}

void CIconLinkDlg::OnBnClickedButtonGet()
{
	UpdateData(TRUE);

	if (m_url.IsEmpty())
	{
		m_wndIcon.SetBitmap(nullptr);
		return;
	}

	const auto& img = GetIconCache().get_icon(m_url.GetString());
	SetImage(img, m_wndIcon);
}
