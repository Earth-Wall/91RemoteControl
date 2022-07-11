#pragma once


// CScreen 对话框

class CScreen : public CDialogEx
{
	DECLARE_DYNAMIC(CScreen)

public:
	CScreen(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CScreen();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = DIALOG_SCREEN };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bMouse;
	BOOL m_bKeyboard;
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
};
