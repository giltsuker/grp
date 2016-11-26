// MouseSensetiveDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CGWork.h"
#include "MouseSensetiveDialog.h"
#include "afxdialogex.h"


// MouseSensetiveDialog dialog

IMPLEMENT_DYNAMIC(MouseSensetiveDialog, CDialogEx)

MouseSensetiveDialog::MouseSensetiveDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(MouseSensetiveDialog::IDD, pParent)
	, m_mouse_sensetivity(1)
{

}

MouseSensetiveDialog::~MouseSensetiveDialog()
{
}

void MouseSensetiveDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_MOUSESESNSETIVE, m_mouse_sensetivity);
	DDV_MinMaxDouble(pDX, m_mouse_sensetivity, 0, m_mouse_sensetivity + 1);

}


BEGIN_MESSAGE_MAP(MouseSensetiveDialog, CDialogEx)
END_MESSAGE_MAP()


// MouseSensetiveDialog message handlers
