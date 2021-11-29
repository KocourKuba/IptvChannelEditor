// IconLink.cpp : implementation file
//

#include "pch.h"
#include <afxdialogex.h>
#include "IconLinkDlg.h"
#include "resource.h"
#include "IconCache.h"
#include "utils.h"


// CIconLink dialog

IMPLEMENT_DYNAMIC(CIconLinkDlg, CDialogEx)

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


BEGIN_MESSAGE_MAP(CIconLinkDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_GET, &CIconLinkDlg::OnBnClickedButtonGet)
END_MESSAGE_MAP()


// CIconLink message handlers


void CIconLinkDlg::OnBnClickedButtonGet()
{
	UpdateData(TRUE);

	if (m_url.IsEmpty())
	{
		m_wndIcon.SetBitmap(nullptr);
		return;
	}

	const auto& img = GetIconCache().get_icon(m_url.GetString());
	utils::SetImage(img, m_wndIcon);
}
