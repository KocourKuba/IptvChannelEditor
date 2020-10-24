// NewCategory.cpp : implementation file
//

#include "StdAfx.h"
#include "EdemChannelEditor.h"
#include "NewCategory.h"

// NewCategory dialog

static constexpr auto CATEGORY_LOGO_URL = "icons/";
static constexpr auto CHANNEL_LOGO_URL = "icons/channels/";
static constexpr auto ICON_TEMPLATE = L"icons/channels/channel_unset.png";

#ifdef _DEBUG
static constexpr auto ICON_TEMPLATE_PATH = L"..\\edem_plugin\\icons\\channels\\channel_unset.png";
static constexpr auto CATEGORY_LOGO_PATH = L"..\\edem_plugin\\icons\\";
static constexpr auto CHANNEL_LOGO_PATH  = L"..\\edem_plugin\\icons\\channels\\";
#else
static constexpr auto ICON_TEMPLATE_PATH = L".\\edem_plugin\\icons\\channels\\channel_unset.png";
static constexpr auto CATEGORY_LOGO_PATH = L".\\edem_plugin\\icons\\";
static constexpr auto CHANNEL_LOGO_PATH  = L".\\edem_plugin\\icons\\channels\\";
#endif // _DEBUG

IMPLEMENT_DYNAMIC(NewCategory, CDialogEx)

BEGIN_MESSAGE_MAP(NewCategory, CDialogEx)
	ON_STN_CLICKED(IDC_STATIC_CATEGORY_ICON, &NewCategory::OnStnClickedStaticCategoryIcon)
END_MESSAGE_MAP()

NewCategory::NewCategory(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_NEW_CATEGORY, pParent)
	, m_iconUrl(ICON_TEMPLATE)
{
}

void NewCategory::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_CATEGORY, m_name);
	DDX_Control(pDX, IDC_STATIC_CATEGORY_ICON, m_wndIcon);
}

BOOL NewCategory::OnInitDialog()
{
	__super::OnInitDialog();

	theApp.LoadImage(m_wndIcon, theApp.GetAppPath() + ICON_TEMPLATE_PATH);
	return TRUE;
}

// NewCategory message handlers

void NewCategory::OnStnClickedStaticCategoryIcon()
{
	CString file = theApp.GetAppPath();
	file += m_type ? CATEGORY_LOGO_PATH : CHANNEL_LOGO_PATH;

	CString filter(_T("PNG file(*.png)#*.png#All Files (*.*)#*.*#"));
	filter.Replace('#', '\0');

	CFileDialog dlg(TRUE);
	OPENFILENAME& oFN = dlg.GetOFN();
	oFN.lpstrFilter = filter.GetString();
	oFN.nMaxFile = MAX_PATH;
	oFN.nFilterIndex = 0;
	oFN.lpstrFile = file.GetBuffer(MAX_PATH);
	oFN.lpstrTitle = _T("Load channel logo");
	oFN.Flags |= OFN_EXPLORER | OFN_NOREADONLYRETURN | OFN_ENABLESIZING | OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_PATHMUSTEXIST;
	oFN.Flags |= OFN_FILEMUSTEXIST | OFN_NONETWORKBUTTON;

	INT_PTR nResult = dlg.DoModal();
	file.ReleaseBuffer();

	if (nResult == IDOK)
	{
		m_iconUrl = m_type ? CATEGORY_LOGO_URL : CHANNEL_LOGO_URL;
		m_iconUrl += oFN.lpstrFileTitle;

		CString path(m_type ? CATEGORY_LOGO_PATH : CHANNEL_LOGO_PATH);
		path += oFN.lpstrFileTitle;
		theApp.LoadImage(m_wndIcon, path);

		UpdateData(FALSE);
	}
}
