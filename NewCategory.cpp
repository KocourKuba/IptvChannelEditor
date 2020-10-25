// NewCategory.cpp : implementation file
//

#include "StdAfx.h"
#include "EdemChannelEditor.h"
#include "NewCategory.h"

// NewCategory dialog

static constexpr auto CATEGORY_LOGO_URL = "icons/";
static constexpr auto ICON_TEMPLATE = L"icons/channel_unset.png";

#ifdef _DEBUG
static constexpr auto PLUGIN_ROOT        = L"..\\edem_plugin\\";
static constexpr auto CATEGORY_LOGO_PATH = L"..\\edem_plugin\\icons\\";
#else
static constexpr auto PLUGIN_ROOT        = L".\\edem_plugin\\";
static constexpr auto CATEGORY_LOGO_PATH = L".\\edem_plugin\\icons\\";
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

	theApp.LoadImage(m_wndIcon, theApp.GetAppPath() + PLUGIN_ROOT + m_iconUrl);
	return TRUE;
}

// NewCategory message handlers

void NewCategory::OnStnClickedStaticCategoryIcon()
{
	UpdateData(TRUE);

	CString path = theApp.GetAppPath() + CATEGORY_LOGO_PATH;
	CString file = theApp.GetAppPath() + PLUGIN_ROOT + m_iconUrl;
	file.Replace('/', '\\');

	CString filter(_T("PNG file(*.png)#*.png#All Files (*.*)#*.*#"));
	filter.Replace('#', '\0');

	CFileDialog dlg(TRUE);
	OPENFILENAME& oFN = dlg.GetOFN();
	oFN.lpstrFilter = filter.GetString();
	oFN.nMaxFile = MAX_PATH;
	oFN.nFilterIndex = 0;
	oFN.lpstrFile = file.GetBuffer(MAX_PATH);
	oFN.lpstrTitle = _T("Load logotype image");
	oFN.lpstrInitialDir = path.GetString();
	oFN.Flags |= OFN_EXPLORER | OFN_NOREADONLYRETURN | OFN_ENABLESIZING | OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_PATHMUSTEXIST;
	oFN.Flags |= OFN_FILEMUSTEXIST | OFN_NONETWORKBUTTON | OFN_NOCHANGEDIR;

	INT_PTR nResult = dlg.DoModal();
	file.ReleaseBuffer();

	if (nResult == IDOK)
	{
		theApp.LoadImage(m_wndIcon, file);

		m_iconUrl = CATEGORY_LOGO_URL;
		m_iconUrl += oFN.lpstrFileTitle;

		UpdateData(FALSE);
	}
}
