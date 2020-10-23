// NewCategory.cpp : implementation file
//

#include "StdAfx.h"
#include "EdemChannelEditor.h"
#include "NewCategory.h"

// NewCategory dialog

IMPLEMENT_DYNAMIC(NewCategory, CDialogEx)

BEGIN_MESSAGE_MAP(NewCategory, CDialogEx)
	ON_STN_CLICKED(IDC_STATIC_CATEGORY_ICON, &NewCategory::OnStnClickedStaticCategoryIcon)
END_MESSAGE_MAP()

NewCategory::NewCategory(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_NEW_CATEGORY, pParent)
{

}

void NewCategory::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_CATEGORY, m_name);
	DDX_Control(pDX, IDC_STATIC_CATEGORY_ICON, m_categoryIcon);
}

BOOL NewCategory::OnInitDialog()
{
	__super::OnInitDialog();

	m_iconUrl = _T("icons/shablon.png");
	LoadImage(m_categoryIcon, m_pluginRoot + _T("icons/shablon.png"));
	return TRUE;
}

// NewCategory message handlers

void NewCategory::LoadImage(CStatic& wnd, const CString& fullPath)
{
	HBITMAP hImg = nullptr;
	CImage image;
	if (SUCCEEDED(image.Load(fullPath)))
	{
		CDC* screenDC = GetDC();

		CRect rc;
		wnd.GetClientRect(rc);

		CImage resized;
		resized.Create(rc.Width(), rc.Height(), 32);
		image.StretchBlt(resized.GetDC(), rc, SRCCOPY);

		resized.ReleaseDC();
		ReleaseDC(screenDC);

		hImg = (HBITMAP)resized.Detach();
	}

	HBITMAP hOld = wnd.SetBitmap(hImg);
	if (hOld)
		::DeleteObject(hOld);
}

void NewCategory::OnStnClickedStaticCategoryIcon()
{
	const CString logo_path = m_pluginRoot + CATEGORY_LOGO_PATH;

	CFileDialog dlg(TRUE);
	CString file(logo_path);
	CString filter(_T("PNG file(*.png)#*.png#All Files (*.*)#*.*#"));
	filter.Replace('#', '\0');

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
		LoadImage(m_categoryIcon, logo_path + oFN.lpstrFileTitle);
		CString icon_url(CATEGORY_LOGO_PATH);
		icon_url += oFN.lpstrFileTitle;
		m_iconUrl = icon_url;
		UpdateData(FALSE);
	}
}
