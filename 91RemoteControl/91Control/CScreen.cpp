// CScreen.cpp: 实现文件
//

#include "pch.h"
#include "91Control.h"
#include "CScreen.h"
#include "afxdialogex.h"
#include "Pkg.h"

// CScreen 对话框

IMPLEMENT_DYNAMIC(CScreen, CDialogEx)

CScreen::CScreen(CWnd* pParent /*=nullptr*/)
	: CDialogEx(DIALOG_SCREEN, pParent)
	, m_bMouse(FALSE)
	, m_bKeyboard(FALSE)
{

}

CScreen::~CScreen()
{
}

void CScreen::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, CHECK_MOUSE, m_bMouse);
	DDX_Check(pDX, CHECK_KEYBOARD, m_bKeyboard);
}


BEGIN_MESSAGE_MAP(CScreen, CDialogEx)
	ON_BN_CLICKED(IDOK, &CScreen::OnBnClickedOk)
END_MESSAGE_MAP()


// CScreen 消息处理程序


void CScreen::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码

	//更新数据
	UpdateData(TRUE);

	CDialogEx::OnOK();
}


BOOL CScreen::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
