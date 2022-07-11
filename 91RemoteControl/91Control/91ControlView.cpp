
// 91ControlView.cpp: CMy91ControlView 类的实现
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "91Control.h"
#endif

#include "91ControlDoc.h"
#include "91ControlView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// TODO: 在此处添加头文件



// CMy91ControlView

IMPLEMENT_DYNCREATE(CMy91ControlView, CView)

BEGIN_MESSAGE_MAP(CMy91ControlView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_COMMAND(MENU_SCREEN, &CMy91ControlView::OnScreen)
	ON_COMMAND(MENU_FILE, &CMy91ControlView::OnFile)
	ON_COMMAND(MENU_CMD, &CMy91ControlView::OnCmd)
	ON_MESSAGE(WM_HANDLECMD, &CMy91ControlView::OnHandlecmd)
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
END_MESSAGE_MAP()

// CMy91ControlView 构造/析构

CMy91ControlView::CMy91ControlView() noexcept
{
	// TODO: 在此处添加构造代码

	m_pScreenCmd = NULL;
	m_bMouse = FALSE;
	m_bKeyboard = FALSE;

	//1.创建socket
	SOCKET sockServer = socket(
		AF_INET,     // ipv4地址簇
		SOCK_STREAM, // 数据流
		IPPROTO_TCP  // tcp协议
	);
	if (sockServer == INVALID_SOCKET)
	{
		AfxMessageBox("socket创建失败");
		return;
	}

	//2.绑定端口和ip
	sockaddr_in siServer;
	siServer.sin_family = AF_INET;
	siServer.sin_port = htons(1354);
	siServer.sin_addr.S_un.S_addr = inet_addr("192.168.1.2");

	int nRet = bind(sockServer, (sockaddr*)&siServer, sizeof(siServer));
	if (nRet == SOCKET_ERROR)
	{
		AfxMessageBox("端口绑定失败");
		return;
	}

	//3.监听
	nRet = listen(sockServer, SOMAXCONN);
	if (nRet == SOCKET_ERROR)
	{
		AfxMessageBox("监听失败");
		return;
	}

	//4.等待连接
	sockaddr_in siClient;
	int nLen = sizeof(siClient);

	m_sockClient = accept(
		sockServer,
		(sockaddr*)&siClient, &nLen // 传出连接服务器的客户端的端口和ip
	);
	if (m_sockClient == INVALID_SOCKET)
	{
		AfxMessageBox("连接建立失败");
		return;
	}
	else
	{
		CString strMsgBox;
		strMsgBox.Format(
			"【客户端接入】ip %s: 端口 %d",
			inet_ntoa(siClient.sin_addr),
			ntohs(siClient.sin_port)
		);

		AfxMessageBox(strMsgBox);
	}

	m_bWorking = TRUE;
	hThread = CreateThread(NULL, 0, RecvThreadProc, this, 0, NULL);
	if (hThread == NULL)
	{
		AfxMessageBox("线程创建失败");
	}
}

DWORD CMy91ControlView::RecvThreadProc(LPVOID lpParam)
{
	CMy91ControlView* pThis = (CMy91ControlView*)lpParam;

	//接收包头和数据
	while (pThis->m_bWorking)
	{
		//接收包头
		PKGHDR hdr;
		int nRet = recv(pThis->m_sockClient, (char*)&hdr, sizeof(hdr), 0);
		if (nRet == SOCKET_ERROR)
		{
			break;
		}

		LPBYTE pBuff = NULL;
		if (hdr.m_nLen > 0)
		{
			//接收数据
			int nRecved = 0;
			pBuff = new BYTE[hdr.m_nLen];

			while (nRecved < hdr.m_nLen)
			{
				nRet = recv(
					pThis->m_sockClient,
					(char*)(pBuff + nRecved), hdr.m_nLen - nRecved,
					0
				);
				if (nRet == SOCKET_ERROR)
				{
					break;
				}
				nRecved += nRet;
			}
		}

		pThis->m_lckForLstCmd.Lock();
		pThis->m_lstCommand.push_back(pair<PKGHDR, LPBYTE>(hdr, pBuff));
		pThis->m_lckForLstCmd.UnLock();

		if (pThis->m_hWnd != NULL)
		{
			pThis->PostMessage(WM_HANDLECMD);
		}
	}

	return 0;
}

CMy91ControlView::~CMy91ControlView()
{
}

BOOL CMy91ControlView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CMy91ControlView 绘图

void CMy91ControlView::OnDraw(CDC* pDC)
{
	CMy91ControlDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码

	if (m_pScreenCmd != NULL)
	{
		CDC dcMem;
		dcMem.CreateCompatibleDC(pDC);
		CBitmap bmp;
		bmp.CreateCompatibleBitmap(pDC, m_pScreenCmd->m_dwScreenWith, m_pScreenCmd->m_dwScreenHeigh);
		bmp.SetBitmapBits(m_pScreenCmd->m_dwDataLen, m_pScreenCmd->m_data);
		dcMem.SelectObject(&bmp);

		pDC->BitBlt(0, 0,
			m_pScreenCmd->m_dwScreenWith,
			m_pScreenCmd->m_dwScreenHeigh,
			&dcMem,
			0, 0,
			SRCCOPY);
	}
}


// CMy91ControlView 打印

BOOL CMy91ControlView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CMy91ControlView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CMy91ControlView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}


// CMy91ControlView 诊断

#ifdef _DEBUG
void CMy91ControlView::AssertValid() const
{
	CView::AssertValid();
}

void CMy91ControlView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMy91ControlDoc* CMy91ControlView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMy91ControlDoc)));
	return (CMy91ControlDoc*)m_pDocument;
}
#endif //_DEBUG


// CMy91ControlView 消息处理程序


void CMy91ControlView::OnScreen()
{
	// TODO: 在此添加命令处理程序代码

	//弹出对话框，选择是否操控鼠标键盘，之后在单文档客户区显示远程屏幕
	dlgScreen.m_bMouse = m_bMouse;
	dlgScreen.m_bKeyboard = m_bKeyboard;
	dlgScreen.DoModal();

	m_bMouse = dlgScreen.m_bMouse;
	m_bKeyboard = dlgScreen.m_bKeyboard;

	SetTimer(S2C_SCREEN, 1000, NULL);
}


void CMy91ControlView::OnFile()
{
	// TODO: 在此添加命令处理程序代码

	//弹出对话框，直接操作，操作结束关闭对话框
	dlgFile.m_sockClient = m_sockClient;
	dlgFile.DoModal();
}


void CMy91ControlView::OnCmd()
{
	// TODO: 在此添加命令处理程序代码

	//弹出对话框，直接操作，操作结束关闭对话框
	dlgCmd.m_sockClient = m_sockClient;
	dlgCmd.DoModal();
}


afx_msg LRESULT CMy91ControlView::OnHandlecmd(WPARAM wParam, LPARAM lParam)
{
	m_lckForLstCmd.Lock();
	auto parCmd = m_lstCommand.front();
	m_lstCommand.pop_front();
	m_lckForLstCmd.UnLock();

	switch (parCmd.first.m_cmd)
	{
	case C2S_SCREEN:
	{
		if (m_pScreenCmd != NULL)
		{
			delete[]m_pScreenCmd;
			m_pScreenCmd = NULL;
		}

		m_pScreenCmd = (PC2SSCREENCMD)parCmd.second;
		InvalidateRect(NULL);
		break;
	}
	case C2S_FILE_DRIVER:
	{
		PC2SDRIVERCMD pDriver = (PC2SDRIVERCMD)parCmd.second;
		char* szDrive = (char*)pDriver->m_szDrive;
		for (size_t i = 0; *(szDrive + i) != 0;)
		{
			dlgFile.m_lstcFile.InsertItem(0, szDrive + i);
			i += (lstrlen(szDrive) + 1);
		}

		delete[](LPBYTE)pDriver;
		dlgFile.UpdateData(FALSE);

		//设置列表的列宽度
		dlgFile.m_lstcFile.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
		dlgFile.m_lstcFile.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
		break;
	}
	case C2S_FILE_DELETE:
	{
		dlgFile.m_lstcFile.DeleteAllItems();
		dlgFile.UpdateData(FALSE);
		break;
	}
	case C2S_FILE_INFO:
	{
		PC2SFILEINFOCMD pFileInfo = (PC2SFILEINFOCMD)parCmd.second;
		int nSubItem = 1;
		dlgFile.m_lstcFile.InsertItem(pFileInfo->m_nItem, pFileInfo->m_szFileName);
		dlgFile.m_lstcFile.SetItemText(pFileInfo->m_nItem, nSubItem++, pFileInfo->m_szFileType);
		dlgFile.UpdateData(FALSE);

		//设置列表的列宽度
		dlgFile.m_lstcFile.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
		dlgFile.m_lstcFile.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
		break;
	}
	case C2S_CMD:
	{
		dlgCmd.m_strResult.Append((char*)parCmd.second);
		dlgCmd.UpdateData(FALSE);
		break;
	}
	default:
		break;
	}

	return 0;
}


void CMy91ControlView::PostNcDestroy()
{
	// TODO: 在此添加专用代码和/或调用基类

	m_bWorking = FALSE;
	WaitForSingleObject(hThread, INFINITE);
	closesocket(m_sockClient);

	CView::PostNcDestroy();
}


void CMy91ControlView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	switch (nIDEvent)
	{
	case S2C_SCREEN:
	{
		PKGHDR hdr;
		hdr.m_cmd = S2C_SCREEN;
		hdr.m_nLen = 0;
		send(m_sockClient, (char*)&hdr, sizeof(hdr), 0);
	}
	default:
		break;
	}

	CView::OnTimer(nIDEvent);
}


void CMy91ControlView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_bMouse)
	{
		CString strDebug;
		strDebug.Format("鼠标移动 X:%d Y%d\r\n", point.x, point.y);
		OutputDebugString(strDebug);

		PKGHDR hdr;
		hdr.m_cmd = S2C_MOUSE;
		hdr.m_nLen = sizeof(S2CMOUSECMD);

		PS2CMOUSECMD pMouseData = (PS2CMOUSECMD)new BYTE[hdr.m_nLen];
		pMouseData->m_dwType = MOUSE_MOVE;
		pMouseData->m_dwX = point.x;
		pMouseData->m_dwY = point.y;

		send(m_sockClient, (char*)&hdr, sizeof(hdr), 0);
		send(m_sockClient, (char*)pMouseData, hdr.m_nLen, 0);
	}

	CView::OnMouseMove(nFlags, point);
}


void CMy91ControlView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (m_bMouse)
	{
		CString strDebug;
		strDebug.Format("鼠标左键双击 X:%d Y%d\r\n", point.x, point.y);
		OutputDebugString(strDebug);

		PKGHDR hdr;
		hdr.m_cmd = S2C_MOUSE;
		hdr.m_nLen = sizeof(S2CMOUSECMD);

		PS2CMOUSECMD pMouseData = (PS2CMOUSECMD)new BYTE[hdr.m_nLen];
		pMouseData->m_dwType = L_Button_DblClk;
		pMouseData->m_dwX = point.x;
		pMouseData->m_dwY = point.y;

		send(m_sockClient, (char*)&hdr, sizeof(hdr), 0);
		send(m_sockClient, (char*)pMouseData, hdr.m_nLen, 0);
	}

	CView::OnLButtonDblClk(nFlags, point);
}


void CMy91ControlView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (m_bMouse)
	{
		CString strDebug;
		strDebug.Format("鼠标左键按下 X:%d Y%d\r\n", point.x, point.y);
		OutputDebugString(strDebug);

		PKGHDR hdr;
		hdr.m_cmd = S2C_MOUSE;
		hdr.m_nLen = sizeof(S2CMOUSECMD);

		PS2CMOUSECMD pMouseData = (PS2CMOUSECMD)new BYTE[hdr.m_nLen];
		pMouseData->m_dwType = L_Button_Down;
		pMouseData->m_dwX = point.x;
		pMouseData->m_dwY = point.y;

		send(m_sockClient, (char*)&hdr, sizeof(hdr), 0);
		send(m_sockClient, (char*)pMouseData, hdr.m_nLen, 0);
	}

	CView::OnLButtonDown(nFlags, point);
}


void CMy91ControlView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (m_bMouse)
	{
		CString strDebug;
		strDebug.Format("鼠标左键弹起 X:%d Y%d\r\n", point.x, point.y);
		OutputDebugString(strDebug);

		PKGHDR hdr;
		hdr.m_cmd = S2C_MOUSE;
		hdr.m_nLen = sizeof(S2CMOUSECMD);

		PS2CMOUSECMD pMouseData = (PS2CMOUSECMD)new BYTE[hdr.m_nLen];
		pMouseData->m_dwType = L_Button_Up;
		pMouseData->m_dwX = point.x;
		pMouseData->m_dwY = point.y;

		send(m_sockClient, (char*)&hdr, sizeof(hdr), 0);
		send(m_sockClient, (char*)pMouseData, hdr.m_nLen, 0);
	}

	CView::OnLButtonUp(nFlags, point);
}


void CMy91ControlView::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (m_bMouse)
	{
		CString strDebug;
		strDebug.Format("鼠标右键双击 X:%d Y%d\r\n", point.x, point.y);
		OutputDebugString(strDebug);

		PKGHDR hdr;
		hdr.m_cmd = S2C_MOUSE;
		hdr.m_nLen = sizeof(S2CMOUSECMD);

		PS2CMOUSECMD pMouseData = (PS2CMOUSECMD)new BYTE[hdr.m_nLen];
		pMouseData->m_dwType = R_Button_DblClk;
		pMouseData->m_dwX = point.x;
		pMouseData->m_dwY = point.y;

		send(m_sockClient, (char*)&hdr, sizeof(hdr), 0);
		send(m_sockClient, (char*)pMouseData, hdr.m_nLen, 0);
	}

	CView::OnRButtonDblClk(nFlags, point);
}


void CMy91ControlView::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (m_bMouse)
	{
		CString strDebug;
		strDebug.Format("鼠标右键按下 X:%d Y%d\r\n", point.x, point.y);
		OutputDebugString(strDebug);

		PKGHDR hdr;
		hdr.m_cmd = S2C_MOUSE;
		hdr.m_nLen = sizeof(S2CMOUSECMD);

		PS2CMOUSECMD pMouseData = (PS2CMOUSECMD)new BYTE[hdr.m_nLen];
		pMouseData->m_dwType = R_Button_Down;
		pMouseData->m_dwX = point.x;
		pMouseData->m_dwY = point.y;

		send(m_sockClient, (char*)&hdr, sizeof(hdr), 0);
		send(m_sockClient, (char*)pMouseData, hdr.m_nLen, 0);
	}

	CView::OnRButtonDown(nFlags, point);
}


void CMy91ControlView::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (m_bMouse)
	{
		CString strDebug;
		strDebug.Format("鼠标右键弹起 X:%d Y%d\r\n", point.x, point.y);
		OutputDebugString(strDebug);

		PKGHDR hdr;
		hdr.m_cmd = S2C_MOUSE;
		hdr.m_nLen = sizeof(S2CMOUSECMD);

		PS2CMOUSECMD pMouseData = (PS2CMOUSECMD)new BYTE[hdr.m_nLen];
		pMouseData->m_dwType = R_Button_Up;
		pMouseData->m_dwX = point.x;
		pMouseData->m_dwY = point.y;

		send(m_sockClient, (char*)&hdr, sizeof(hdr), 0);
		send(m_sockClient, (char*)pMouseData, hdr.m_nLen, 0);
	}

	CView::OnRButtonUp(nFlags, point);
}


void CMy91ControlView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (m_bKeyboard)
	{
		CString strDebug;
		strDebug.Format("按键按下 %c\r\n", nChar);
		OutputDebugString(strDebug);

		PKGHDR hdr;
		hdr.m_cmd = S2C_KEYBOARD;
		hdr.m_nLen = sizeof(S2CMOUSECMD);

		PS2CKEYBOARDCMD pKeyboardData = (PS2CKEYBOARDCMD)new BYTE[hdr.m_nLen];
		pKeyboardData->m_dwType = KEYDOWN;
		pKeyboardData->m_nChar = nChar;
		pKeyboardData->m_nRepCnt = nRepCnt;

		send(m_sockClient, (char*)&hdr, sizeof(hdr), 0);
		send(m_sockClient, (char*)pKeyboardData, hdr.m_nLen, 0);
	}

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CMy91ControlView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (m_bKeyboard)
	{
		CString strDebug;
		strDebug.Format("按键弹起 %c\r\n", nChar);
		OutputDebugString(strDebug);

		PKGHDR hdr;
		hdr.m_cmd = S2C_KEYBOARD;
		hdr.m_nLen = sizeof(S2CMOUSECMD);

		PS2CKEYBOARDCMD pKeyboardData = (PS2CKEYBOARDCMD)new BYTE[hdr.m_nLen];
		pKeyboardData->m_dwType = KEYUP;
		pKeyboardData->m_nChar = nChar;
		pKeyboardData->m_nRepCnt = nRepCnt;
		pKeyboardData->m_nFlags = nFlags;

		send(m_sockClient, (char*)&hdr, sizeof(hdr), 0);
		send(m_sockClient, (char*)pKeyboardData, hdr.m_nLen, 0);
	}

	CView::OnKeyUp(nChar, nRepCnt, nFlags);
}
