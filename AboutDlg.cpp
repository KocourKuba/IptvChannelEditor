#include "StdAfx.h"
#include "resource.h"		// main symbols
#include "AboutDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

IStream* CreateStreamOnResource(UINT id)
{
	IStream* ipStream = nullptr;
	HGLOBAL hgblResourceData = nullptr;
	do
	{
		HRSRC hrsrc = FindResource(nullptr, MAKEINTRESOURCE(id), _T("PNG"));
		if (hrsrc == nullptr) break;

		DWORD dwResourceSize = SizeofResource(nullptr, hrsrc);
		HGLOBAL hglbImage = LoadResource(nullptr, hrsrc);
		if (hglbImage == nullptr) break;

		LPVOID pvSourceResourceData = LockResource(hglbImage);
		if (pvSourceResourceData == nullptr) break;

		hgblResourceData = GlobalAlloc(GMEM_MOVEABLE, dwResourceSize);
		if (hgblResourceData == nullptr) break;

		LPVOID pvResourceData = GlobalLock(hgblResourceData);

		if (pvResourceData == nullptr) break;

		CopyMemory(pvResourceData, pvSourceResourceData, dwResourceSize);
		GlobalUnlock(hgblResourceData);
		if (FAILED(CreateStreamOnHGlobal(hgblResourceData, TRUE, &ipStream))) break;

		return ipStream;
	} while (false);

	if (hgblResourceData == nullptr)
		::GlobalFree(hgblResourceData);

	return nullptr;
}

CAboutDlg::CAboutDlg() : CDialog(IDD_ABOUTBOX)
{
}

BOOL CAboutDlg::OnInitDialog()
{
	__super::OnInitDialog();

	m_version.Format(_T("Version %d.%d"), MAJOR, MINOR);

	IStream* pStream = CreateStreamOnResource(IDB_PNG_QR);
	if (pStream != nullptr)
	{
		CImage img;
		img.Load(pStream);
		img.SetHasAlphaChannel(true);
		pStream->Release();

		HBITMAP hOld = m_QR.SetBitmap(img.Detach());
		if (hOld)
			::DeleteObject(hOld);
	}

	UpdateData(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_QR, m_QR);
	DDX_Text(pDX, IDC_STATIC_VERSION, m_version);
}

void CAboutDlg::OnOK()
{
	__super::OnOK();
}
