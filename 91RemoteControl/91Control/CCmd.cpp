// CCmd.cpp: 实现文件
//

#include "pch.h"
#include "91Control.h"
#include "CCmd.h"
#include "afxdialogex.h"
#include "Pkg.h"


// CCmd 对话框

IMPLEMENT_DYNAMIC(CCmd, CDialogEx)

CCmd::CCmd(CWnd* pParent /*=nullptr*/)
	: CDialogEx(DIALOG_CMD, pParent)
	, m_strCmdText(_T(""))
	, m_strResult(_T(""))
{

}

CCmd::~CCmd()
{
}

void CCmd::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, EDIT_CMD_TEXT, m_strCmdText);
	DDX_Text(pDX, EDIT_RESULT, m_strResult);
	//  DDX_Control(pDX, EDIT_RESULT, m_edtResult);
}


BEGIN_MESSAGE_MAP(CCmd, CDialogEx)
	ON_BN_CLICKED(BUTTON_SEND, &CCmd::OnBnClickedSend)
	ON_BN_CLICKED(BUTTON_CLEAR, &CCmd::OnBnClickedClear)
END_MESSAGE_MAP()


// CCmd 消息处理程序


void CCmd::OnBnClickedSend()
{
	// TODO: 在此添加控件通知处理程序代码

	UpdateData(TRUE);

	m_strCmdText += "\r\n";

	PKGHDR hdr;
	hdr.m_cmd = S2C_CMD;
	hdr.m_nLen = m_strCmdText.GetLength() + 1;

	char* pCmdData = (char*)new BYTE[hdr.m_nLen];
	memcpy(pCmdData, m_strCmdText.GetBuffer(), hdr.m_nLen);

	send(m_sockClient, (char*)&hdr, sizeof(hdr), 0);
	send(m_sockClient, (char*)pCmdData, hdr.m_nLen, 0);
}


void CCmd::OnBnClickedClear()
{
	// TODO: 在此添加控件通知处理程序代码

	m_strResult = "";
	UpdateData(FALSE);
}
