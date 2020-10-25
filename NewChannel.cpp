// NewChannel.cpp : implementation file
//

#include "StdAfx.h"
#include "EdemChannelEditor.h"
#include "NewChannel.h"

// NewChannel dialog

IMPLEMENT_DYNAMIC(NewChannel, CDialogEx)

BEGIN_MESSAGE_MAP(NewChannel, CDialogEx)
END_MESSAGE_MAP()

NewChannel::NewChannel(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_NEW_CATEGORY, pParent)
{
}

void NewChannel::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_CATEGORY, m_name);
}

BOOL NewChannel::OnInitDialog()
{
	__super::OnInitDialog();

	return TRUE;
}

