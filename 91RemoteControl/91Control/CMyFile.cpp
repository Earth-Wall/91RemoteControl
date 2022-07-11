// CMyFile.cpp: 实现文件
//

#include "pch.h"
#include "91Control.h"
#include "CMyFile.h"
#include "afxdialogex.h"
#include "Pkg.h"
#include "Pkg.h"

// CMyFile 对话框

IMPLEMENT_DYNAMIC(CMyFile, CDialogEx)

CMyFile::CMyFile(CWnd* pParent /*=nullptr*/)
	: CDialogEx(DIALOG_FILE, pParent)
	, m_strRemotePath(_T(""))
	, m_strControlPath(_T(""))
{







}

CMyFile::~CMyFile()
{
}

void CMyFile::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, EDIT_REMOTE_PATH, m_strRemotePath);
	DDX_Text(pDX, MFCEDITBROWSE_CONTROL_PATH, m_strControlPath);
	DDX_Control(pDX, LIST_FILE, m_lstcFile);
}


BEGIN_MESSAGE_MAP(CMyFile, CDialogEx)
	ON_BN_CLICKED(BUTTON_BACK, &CMyFile::OnBnClickedBack)
	ON_BN_CLICKED(BUTTON_UPLOAD, &CMyFile::OnBnClickedUpload)
	ON_BN_CLICKED(BUTTON_DOWNLOAD, &CMyFile::OnBnClickedDownload)
	ON_NOTIFY(NM_DBLCLK, LIST_FILE, &CMyFile::OnDblclkListFile)
END_MESSAGE_MAP()


// CMyFile 消息处理程序


void CMyFile::OnBnClickedBack()
{
	// TODO: 在此添加控件通知处理程序代码

	//获取当前列表项的文件路径字符串
	char* szPaths = m_strRemotePath.GetBuffer();

	//获取字符串的长度
	int i = strlen(szPaths);

	//如果长度大于3，则当前路径不是盘符
	if (i > 3)
	{
		//从后往前循环删除直到遇到\\

		for (i = i - 2; i >= 0; i--)
		{
			if (szPaths[i] == '\\')
			{
				m_strRemotePath.Delete(i + 1, strlen(szPaths));
				break;
			}
		}

		//父项的文件路径
		CString strParentRemotePath;
		strParentRemotePath = m_strRemotePath + "*.*";

		//发送文件路径
		PKGHDR hdr;
		hdr.m_cmd = S2C_FILE_INFO;
		hdr.m_nLen = sizeof(S2CPATHCMD);

		S2CPATHCMD Path;
		strcpy(Path.m_szFilePath, strParentRemotePath.GetBuffer());

		send(m_sockClient, (char*)&hdr, sizeof(hdr), 0);
		send(m_sockClient, (char*)&Path, sizeof(Path), 0);

		UpdateData(FALSE);
	}
	else
	{
		PKGHDR hdr;
		hdr.m_cmd = S2C_FILE_DRIVER;
		hdr.m_nLen = 0;
		send(m_sockClient, (char*)&hdr, sizeof(hdr), 0);
	}
}


void CMyFile::OnBnClickedUpload()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);

	if (!m_strRemotePath.IsEmpty() && !m_strControlPath.IsEmpty())
	{
		char* pControlPath = m_strControlPath.GetBuffer();
		int nMax = 0;
		for (size_t i = 0; i < m_strControlPath.GetLength(); i++)
		{
			if (pControlPath[i] == '\\')
			{
				nMax = i;
			}
		}

		char szName[MAXBYTE] = { 0 };
		for (size_t i = 0,j = nMax+1; pControlPath[j] != '\0'; i++,j++)
		{
			szName[i] = pControlPath[j];
		}

		CString strRemotePath;
		strRemotePath = m_strRemotePath + szName;

		HANDLE hFile = CreateFile(m_strControlPath,
			GENERIC_WRITE | GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			return;
		}

		__int64 nFileSize = 0;
		GetFileSizeEx(hFile, (PLARGE_INTEGER)&nFileSize);

		//发送包头和数据
		PKGHDR hdr;
		hdr.m_cmd = S2C_FILE_UP;
		hdr.m_nLen = sizeof(TRANSFERINFO);

		TRANSFERINFO filePath;
		strcpy(filePath.m_szRemotePath, strRemotePath.GetBuffer());
		strcpy(filePath.m_szControlPath, m_strControlPath.GetBuffer());
		filePath.nFileSize = nFileSize;

		send(m_sockClient, (char*)&hdr, sizeof(hdr), 0);
		send(m_sockClient, (char*)&filePath, sizeof(filePath), 0);

		//发送数据
		while (TRUE)
		{
			char szBuffer[0x10000] = { 0 };
			DWORD nRead = 0;

			ReadFile(
				hFile,                // handle to file
				szBuffer,             // data buffer
				sizeof(szBuffer),  // number of bytes to read
				&nRead, // number of bytes read
				NULL);

			if (nRead == 0)
			{
				break;
			}

			send(m_sockClient, (char*)szBuffer, nRead, 0);

		}

		CloseHandle(hFile);
	}
}


void CMyFile::OnBnClickedDownload()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	int nItem = m_lstcFile.GetSelectionMark();

	if (!m_strRemotePath.IsEmpty() && !m_strControlPath.IsEmpty() && nItem != -1)
	{
		CString strFileName = m_lstcFile.GetItemText(nItem, 0);
		CString strFileType = m_lstcFile.GetItemText(nItem, 1);
		if (strFileType != "文件夹")
		{
			CString strRemotePath;
			strRemotePath = m_strRemotePath + strFileName;

			char* pControlPath = m_strControlPath.GetBuffer();
			char szControlPath[MAXBYTE] = { 0 };
			memcpy(szControlPath, pControlPath, m_strControlPath.GetLength());
			for (int i = m_strControlPath.GetLength()-1; i >0; i--)
			{
				if (szControlPath[i] != '\\')
				{
					szControlPath[i] = '\0';
				}
				else
				{
					break;
				}
			}
			CString strControlPath;
			strControlPath = szControlPath;
			strControlPath = strControlPath + strFileName;

			HANDLE hFile = CreateFile(strControlPath,
				GENERIC_WRITE | GENERIC_READ,
				FILE_SHARE_READ,
				NULL,
				CREATE_NEW,
				FILE_ATTRIBUTE_NORMAL,
				NULL);

			if (hFile == INVALID_HANDLE_VALUE)
			{
				return;
			}

			//发送包头和数据
			PKGHDR hdr;
			hdr.m_cmd = S2C_FILE_DOWN;
			hdr.m_nLen = sizeof(TRANSFERINFO);

			TRANSFERINFO  Transferpath = { 0 };
			memcpy(Transferpath.m_szControlPath, strControlPath, strControlPath.GetLength());
			memcpy(Transferpath.m_szRemotePath, strRemotePath, strRemotePath.GetLength());

			send(m_sockClient, (char*)&hdr, sizeof(hdr), 0);
			send(m_sockClient, (char*)&Transferpath, sizeof(Transferpath), 0);

			PKGHDR hdr2;
			int nRet = recv(m_sockClient, (char*)&hdr, sizeof(hdr), 0);

			LPBYTE pBuff2 = NULL;
			int nRecved = 0;
			pBuff2 = new BYTE[hdr2.m_nLen];

			while (nRecved < hdr2.m_nLen)
			{
				int nRet = recv(
					m_sockClient,
					(char*)(pBuff2 + nRecved), hdr2.m_nLen - nRecved,
					0
				);
				nRecved += nRet;
			}

			DWORD nWrite = 0;
			WriteFile(hFile,
				pBuff2,
				hdr2.m_nLen,
				&nWrite,
				NULL);

			CloseHandle(hFile);
			delete[]pBuff2;
		}
	}
}


BOOL CMyFile::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	PKGHDR hdr;
	hdr.m_cmd = S2C_FILE_DRIVER;
	hdr.m_nLen = 0;
	send(m_sockClient, (char*)&hdr, sizeof(hdr), 0);

	int nCol = 0;
	m_lstcFile.InsertColumn(nCol++, "名称", LVCFMT_CENTER);
	m_lstcFile.InsertColumn(nCol++, "类型", LVCFMT_CENTER);

	//设置列表的列宽度
	m_lstcFile.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
	m_lstcFile.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CMyFile::OnDblclkListFile(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CString strFileName = m_lstcFile.GetItemText(pNMItemActivate->iItem, 0);
	CString strFileType = m_lstcFile.GetItemText(pNMItemActivate->iItem, 1);
	if (strFileType == "文件夹" || strFileType == "")
	{
		if (strFileType == "文件夹")
		{
			m_strRemotePath = m_strRemotePath + strFileName + "\\";;
		}
		else
		{
			m_strRemotePath = strFileName;
		}

		CString strCurrentRemotePath;
		strCurrentRemotePath = m_strRemotePath + "*.*";

		//发送文件路径
		PKGHDR hdr;
		hdr.m_cmd = S2C_FILE_INFO;
		hdr.m_nLen = sizeof(S2CPATHCMD);
		S2CPATHCMD Path;
		strcpy(Path.m_szFilePath, strCurrentRemotePath.GetBuffer());
		send(m_sockClient, (char*)&hdr, sizeof(hdr), 0);
		send(m_sockClient, (char*)&Path, sizeof(Path), 0);

		UpdateData(FALSE);
	}

	*pResult = 0;
}
