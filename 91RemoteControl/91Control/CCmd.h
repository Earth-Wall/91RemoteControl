#pragma once


// CCmd 对话框

class CCmd : public CDialogEx
{
	DECLARE_DYNAMIC(CCmd)

public:
	CCmd(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CCmd();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = DIALOG_CMD };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	SOCKET m_sockClient;
	CString m_strCmdText;
	CString m_strResult;
	afx_msg void OnBnClickedSend();
	afx_msg void OnBnClickedClear();
};
