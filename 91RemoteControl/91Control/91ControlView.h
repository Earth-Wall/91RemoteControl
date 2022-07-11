
// 91ControlView.h: CMy91ControlView 类的接口
//

#pragma once
// TODO: 在此处添加头文件
#include <list>
#include "Pkg.h"
#include "CLock.h"
#include "CScreen.h"
#include "CMyFile.h"
#include "CCmd.h"
using namespace std;

#define  WM_HANDLECMD WM_USER + 1

class CMy91ControlView : public CView
{
protected: // 仅从序列化创建
	CMy91ControlView() noexcept;
	DECLARE_DYNCREATE(CMy91ControlView)

// 特性
public:
	CMy91ControlDoc* GetDocument() const;

// 操作
public:

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 实现
public:
	virtual ~CMy91ControlView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void PostNcDestroy();

	afx_msg void OnScreen();
	afx_msg void OnFile();
	afx_msg void OnCmd();

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

	SOCKET m_sockClient;
	HANDLE hThread;
	bool m_bWorking;
	static DWORD CALLBACK RecvThreadProc(LPVOID lpParam);
	list<pair<PKGHDR, LPBYTE>> m_lstCommand;
	CLock m_lckForLstCmd;
	PC2SSCREENCMD m_pScreenCmd;

	CScreen dlgScreen;
	CMyFile dlgFile;
	CCmd dlgCmd;

	bool m_bMouse;    // 是否控制远程主机鼠标
	bool m_bKeyboard; // 是否控制远程主机键盘

protected:
	afx_msg LRESULT OnHandlecmd(WPARAM wParam, LPARAM lParam);

};

#ifndef _DEBUG  // 91ControlView.cpp 中的调试版本
inline CMy91ControlDoc* CMy91ControlView::GetDocument() const
   { return reinterpret_cast<CMy91ControlDoc*>(m_pDocument); }
#endif