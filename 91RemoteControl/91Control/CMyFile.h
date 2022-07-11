#pragma once


// CMyFile 对话框

class CMyFile : public CDialogEx
{
	DECLARE_DYNAMIC(CMyFile)

public:
	CMyFile(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CMyFile();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = DIALOG_FILE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	SOCKET m_sockClient;
	CString m_strRemotePath;
	CString m_strControlPath;
	CListCtrl m_lstcFile;
	afx_msg void OnBnClickedBack();
	afx_msg void OnBnClickedUpload();
	afx_msg void OnBnClickedDownload();
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkListFile(NMHDR* pNMHDR, LRESULT* pResult);
};
