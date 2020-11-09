// NewCategory.cpp : implementation file
//

#include "StdAfx.h"
#include "EdemChannelEditor.h"
#include "NewCategoryDlg.h"
#include "utils.h"

// CNewCategoryDlg dialog

#ifdef _DEBUG
static constexpr auto PLUGIN_ROOT        = L"..\\edem_plugin\\";
static constexpr auto CATEGORY_LOGO_PATH = L"..\\edem_plugin\\icons\\";
#else
static constexpr auto PLUGIN_ROOT        = L".\\edem_plugin\\";
static constexpr auto CATEGORY_LOGO_PATH = L".\\edem_plugin\\icons\\";
#endif // _DEBUG

IMPLEMENT_DYNAMIC(CNewCategoryDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CNewCategoryDlg, CDialogEx)
	ON_STN_CLICKED(IDC_STATIC_CATEGORY_ICON, &CNewCategoryDlg::OnStnClickedStaticCategoryIcon)
END_MESSAGE_MAP()

CNewCategoryDlg::CNewCategoryDlg(BOOL bNew /*= TRUE*/, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_NEW_CATEGORY, pParent)
	, m_iconUri(utils::ICON_TEMPLATE)
	, m_bNew(bNew)
{
}

void CNewCategoryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_CATEGORY, m_name);
	DDX_Control(pDX, IDC_STATIC_CATEGORY_ICON, m_wndIcon);
}

BOOL CNewCategoryDlg::OnInitDialog()
{
	__super::OnInitDialog();

	if (m_bNew)
	{
		SetWindowText(_T("New Category"));
	}
	else
	{
		SetWindowText(_T("Edit Category"));
	}

	std::wstring fullPath = m_iconUri.get_icon_relative_path(theApp.GetAppPath(PLUGIN_ROOT));
	CImage img;
	if (theApp.LoadImage(fullPath.c_str(), img))
		theApp.SetImage(img, m_wndIcon);

	return TRUE;
}

// CNewCategoryDlg message handlers

void CNewCategoryDlg::OnStnClickedStaticCategoryIcon()
{
	static constexpr auto CATEGORY_LOGO_URL = "icons/";

	UpdateData(TRUE);

	CString path = theApp.GetAppPath(CATEGORY_LOGO_PATH);
	CString file = m_iconUri.get_icon_relative_path(theApp.GetAppPath(PLUGIN_ROOT)).c_str();
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
		CString newPath = file.Left(file.GetLength() - _tcslen(oFN.lpstrFileTitle));
		if (path.CompareNoCase(newPath) != 0)
		{
			path += oFN.lpstrFileTitle;
			CopyFile(file, path, FALSE);
			CImage img;
			if (theApp.LoadImage(path, img))
				theApp.SetImage(img, m_wndIcon);
		}

		std::string icon_path = CATEGORY_LOGO_URL;
		icon_path += CStringA(oFN.lpstrFileTitle).GetString();
		m_iconUri.set_schema(uri::PLUGIN_SCHEME);
		m_iconUri.set_path(icon_path);

		UpdateData(FALSE);
	}
}
