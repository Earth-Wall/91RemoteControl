// 91Remote.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

//头文件
#include <iostream>
#include <WinSock2.h>
#include <windows.h>
#include "Pkg.h"
using namespace std;

//宏
#define MAX_WORD 65535
#define WAIT_TIME (1000 / 100)

//函数声明
void OnScreen(SOCKET sockClient);
void OnMouse(BYTE* pBuff);
void OnKeyboard(BYTE* pBuff);
void OnDriver(SOCKET sockClient);
void OnFileInfo(char* szPath, SOCKET sockClient);
void OnFileUp(char* szPath, SOCKET sockClient);
void OnFileDown(char* pBuff, SOCKET sockClient);
void OnCmd(BYTE* pBuff, int nBufferLength, SOCKET sockClient);

//全局变量
extern int g_nScreenWith = GetSystemMetrics(SM_CXFULLSCREEN);
extern int g_nScreenHeigh = GetSystemMetrics(SM_CYFULLSCREEN);

int main()
{
    //1.创建socket
    SOCKET sockClient = socket(
        AF_INET,     // ipv4地址簇
        SOCK_STREAM, // 数据流
        IPPROTO_TCP  // tcp协议
    );
    if (sockClient == INVALID_SOCKET)
    {
        printf("socket创建失败\r\n");
        return 0;
    }
    else
    {
        printf("socket创建成功\r\n");
    }

    //4.连接服务器

    //服务器端口和ip
    sockaddr_in siServer;
    siServer.sin_family = AF_INET;
    siServer.sin_port = htons(1354); // htons(host to net short)小尾转大尾
    siServer.sin_addr.S_un.S_addr = inet_addr("192.168.1.2");

    int nRet = connect(sockClient, (sockaddr*)&siServer, sizeof(siServer));
    if (nRet == SOCKET_ERROR)
    {
        printf("服务器连接失败\r\n");
        return 0;
    }
    else
    {
        printf("服务器连接成功\r\n");
    }

    //接收包头和数据
    while (true)
    {
        //接收包头
        PKGHDR hdr;
        int nRet = recv(sockClient, (char*)&hdr, sizeof(hdr), 0);
        
        LPBYTE pBuff = NULL;
        if (hdr.m_nLen > 0)
        {
            //接收数据
            int nRecved = 0;
            pBuff = new BYTE[hdr.m_nLen];

            while (nRecved < hdr.m_nLen)
            {
                nRet = recv(
                    sockClient,
                    (char*)(pBuff + nRecved), hdr.m_nLen - nRecved,
                    0
                );
                nRecved += nRet;
            }
        }

        //解析命令
        switch (hdr.m_cmd)
        {
        case S2C_SCREEN:
        {
            OnScreen(sockClient);
            break;
        }
        case S2C_MOUSE:
        {
            OnMouse(pBuff);
            break;
        }
        case S2C_KEYBOARD:
        {
            OnKeyboard(pBuff);
            break;
        }
        case S2C_FILE_DRIVER:
        {
            OnDriver(sockClient);
            break;
        }
        case S2C_FILE_INFO:
        {
            OnFileInfo((char*)pBuff, sockClient);
            break;
        }
        case S2C_FILE_UP:
        {
            OnFileUp((char*)pBuff, sockClient);
            break;
        }
        case S2C_FILE_DOWN:
        {
            OnFileDown((char*)pBuff, sockClient);
            break;
        }
        case S2C_CMD:
        {
            OnCmd(pBuff, hdr.m_nLen, sockClient);
            break;
        }
        default:
            break;
        }

        if (pBuff != NULL)
        {
            delete[] pBuff;
        }
    }

    //std::cout << "Hello World!\n";
}

void OnScreen(SOCKET sockClient)
{
    //从屏幕拷贝数据到内存位图

    HDC hDcScreen = GetDC(NULL);
    HDC hDcMem = CreateCompatibleDC(hDcScreen);
    HBITMAP hBmpMem = CreateCompatibleBitmap(hDcScreen, g_nScreenWith, g_nScreenHeigh);
    SelectObject(hDcMem, hBmpMem);
    BitBlt(
        hDcMem, 0, 0, g_nScreenWith, g_nScreenHeigh,
        hDcScreen, 0, 0, SRCCOPY
    );

    //获取屏幕数据
    DWORD dwBufLen = g_nScreenHeigh * g_nScreenWith * sizeof(COLORREF);
    LPBYTE pBuffBmp = new BYTE[dwBufLen];
    GetBitmapBits(hBmpMem, dwBufLen, pBuffBmp);

    //发送屏幕数据
    PKGHDR hdr;
    hdr.m_cmd = C2S_SCREEN;
    hdr.m_nLen = dwBufLen + sizeof(C2SSCREENCMD);

    PC2SSCREENCMD pSreenData = (PC2SSCREENCMD)new BYTE[hdr.m_nLen];
    pSreenData->m_dwScreenHeigh = g_nScreenHeigh;
    pSreenData->m_dwScreenWith = g_nScreenWith;
    pSreenData->m_dwDataLen = dwBufLen;
    memcpy(pSreenData->m_data, pBuffBmp, pSreenData->m_dwDataLen);

    send(sockClient, (char*)&hdr, sizeof(hdr), 0);
    send(sockClient, (char*)pSreenData, hdr.m_nLen, 0);

    delete[] pBuffBmp;
    delete[](LPBYTE)pSreenData;
}

void OnMouse(BYTE* pBuff)
{
    PS2CMOUSECMD pMouseCmd = (PS2CMOUSECMD)pBuff;

    switch (pMouseCmd->m_dwType)
    {
    case MOUSE_MOVE:
    {
        printf("鼠标移动 X:%d Y:%d", pMouseCmd->m_dwX, pMouseCmd->m_dwY);
        mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, pMouseCmd->m_dwX * MAX_WORD / g_nScreenWith, pMouseCmd->m_dwY * MAX_WORD / g_nScreenHeigh, 0, 0);
        break;
    }
    case L_Button_Down:
    {
        printf("左键按下 X:%d Y:%d", pMouseCmd->m_dwX, pMouseCmd->m_dwY);
        mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTDOWN, pMouseCmd->m_dwX * MAX_WORD / g_nScreenWith, pMouseCmd->m_dwY * MAX_WORD / g_nScreenHeigh, 0, 0);
        break;
    }
    case L_Button_Up:
    {
        printf("左键弹起 X:%d Y:%d", pMouseCmd->m_dwX, pMouseCmd->m_dwY);
        mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTUP, pMouseCmd->m_dwX * MAX_WORD / g_nScreenWith, pMouseCmd->m_dwY * MAX_WORD / g_nScreenHeigh, 0, 0);
        break;
    }
    case L_Button_DblClk:
    {
        printf("左键双击 X:%d Y:%d", pMouseCmd->m_dwX, pMouseCmd->m_dwY);
        mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTDOWN, pMouseCmd->m_dwX * MAX_WORD / g_nScreenWith, pMouseCmd->m_dwY * MAX_WORD / g_nScreenHeigh, 0, 0);
        break;
    }
    case R_Button_Down:
    {
        printf("右键点击 X:%d Y:%d", pMouseCmd->m_dwX, pMouseCmd->m_dwY);
        mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_RIGHTDOWN, pMouseCmd->m_dwX * MAX_WORD / g_nScreenWith, pMouseCmd->m_dwY * MAX_WORD / g_nScreenHeigh, 0, 0);
        break;
    }
    case R_Button_Up:
    {
        printf("右键弹起 X:%d Y:%d", pMouseCmd->m_dwX, pMouseCmd->m_dwY);
        mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_RIGHTUP, pMouseCmd->m_dwX * MAX_WORD / g_nScreenWith, pMouseCmd->m_dwY * MAX_WORD / g_nScreenHeigh, 0, 0);
        break;
    }
    case R_Button_DblClk:
    {
        printf("右键双击 X:%d Y:%d", pMouseCmd->m_dwX, pMouseCmd->m_dwY);
        mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_RIGHTDOWN, pMouseCmd->m_dwX * MAX_WORD / g_nScreenWith, pMouseCmd->m_dwY * MAX_WORD / g_nScreenHeigh, 0, 0);
        break;
    }
    default:
        break;
    }
}

void OnKeyboard(BYTE* pBuff)
{
    PS2CKEYBOARDCMD pKeyboardCmd = (PS2CKEYBOARDCMD)pBuff;
    switch (pKeyboardCmd->m_dwType)
    {
    case KEYDOWN:
    {
        printf("按键按下 %c", pKeyboardCmd->m_nChar);
        keybd_event(pKeyboardCmd->m_nChar, 0, 0, 0);
        break;
    }
    case KEYUP:
    {
        printf("按键弹起 %c", pKeyboardCmd->m_nChar);
        break;
    }
    default:
        break;
    }
}

void OnDriver(SOCKET sockClient)
{
    //清空列表
    PKGHDR hdr1;
    hdr1.m_cmd = C2S_FILE_DELETE;
    hdr1.m_nLen = 0;
    send(sockClient, (char*)&hdr1, sizeof(hdr1), 0);

    //获取系统盘符
    char szDrive[MAXBYTE] = {};
    GetLogicalDriveStrings(sizeof(szDrive), szDrive);

    //发送硬盘数据
    PKGHDR hdr;
    hdr.m_cmd = C2S_FILE_DRIVER;
    hdr.m_nLen = sizeof(C2SDRIVERCMD);

    C2SDRIVERCMD DriverData;
    memcpy(DriverData.m_szDrive, szDrive, sizeof(szDrive));
    send(sockClient, (char*)&hdr, sizeof(hdr), 0);
    send(sockClient, (char*)&DriverData, sizeof(DriverData), 0);
}

void OnFileInfo(char* szPath, SOCKET sockClient)
{
    WIN32_FIND_DATA FileInfo = { 0 };

    HANDLE hFileHanlde = FindFirstFile(szPath, &FileInfo);

    if (hFileHanlde != INVALID_HANDLE_VALUE)
    {
        //清空列表
        PKGHDR hdr;
        hdr.m_cmd = C2S_FILE_DELETE;
        hdr.m_nLen = 0;
        send(sockClient, (char*)&hdr, sizeof(hdr), 0);

        INT nItem = 0;

        while (TRUE)
        {
            INT nSubItem = 1;
            if (strcmp(FileInfo.cFileName, ".") == 0 || strcmp(FileInfo.cFileName, "..") == 0)
            {
                if (FindNextFile(hFileHanlde, &FileInfo) == 0)
                {
                    break;
                }
                continue;
            }

            C2SFILEINFOCMD s2cfileInfo;
            s2cfileInfo.m_nItem = nItem;
            //设置文件名
            strcpy(s2cfileInfo.m_szFileName, FileInfo.cFileName);

            //设置文件类型信息
            SHFILEINFO sfi = { 0 };
            SHGetFileInfo(FileInfo.cFileName, 0, &sfi, sizeof(sfi), SHGFI_USEFILEATTRIBUTES | SHGFI_TYPENAME);

            if (FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                strcpy(s2cfileInfo.m_szFileType, "文件夹");
            }
            else
            {
                strcpy(s2cfileInfo.m_szFileType, sfi.szTypeName);
            }

            //发送文件数据
            PKGHDR hdr;
            hdr.m_cmd = C2S_FILE_INFO;
            hdr.m_nLen = sizeof(s2cfileInfo);

            send(sockClient, (char*)&hdr, sizeof(hdr), 0);
            send(sockClient, (char*)&s2cfileInfo, sizeof(s2cfileInfo), 0);

            nItem++;

            if (FindNextFile(hFileHanlde, &FileInfo) == 0)
            {
                break;
            }
        }
    }
}

void OnFileUp(char* pBuff, SOCKET sockClient)
{
    PTRANSFERINFO pTransferpath = (PTRANSFERINFO)pBuff;

    HANDLE hFile = CreateFile(pTransferpath->m_szRemotePath,
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

    LPBYTE pBuff2 = NULL;
    int nRecved = 0;
    pBuff2 = new BYTE[pTransferpath->nFileSize];

    while (nRecved < pTransferpath->nFileSize)
    {
        int nRet = recv(
            sockClient,
            (char*)(pBuff2 + nRecved), pTransferpath->nFileSize - nRecved,
            0
        );
        nRecved += nRet;
    }

    DWORD nWrite = 0;
    WriteFile(hFile,
        pBuff2,
        pTransferpath->nFileSize,
        &nWrite,
        NULL);

    CloseHandle(hFile);
    delete[]pBuff2;
}

void OnFileDown(char* pBuff, SOCKET sockClient)
{
    PTRANSFERINFO pTransferpath = (PTRANSFERINFO)pBuff;

    HANDLE hFile = CreateFile(pTransferpath->m_szRemotePath,
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

    //发送大小
    PKGHDR hdr;
    hdr.m_cmd = C2S_FILE_DOWN;
    hdr.m_nLen = nFileSize;
    send(sockClient, (char*)&hdr, sizeof(hdr), 0);

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

        send(sockClient, (char*)szBuffer, nRead, 0);

    }
    CloseHandle(hFile);
}

void OnCmd(BYTE* pBuff, int nBufferLength, SOCKET sockClient)
{
    char* pCmdData = (char*)pBuff;
    printf("%s", pCmdData);

    HANDLE hRead;
    HANDLE hWrite;
    HANDLE hCmdWrite;
    HANDLE hCmdRead;
    HANDLE hProcess = NULL;

    //创建管道
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    BOOL bRet = CreatePipe(&hRead, &hCmdWrite, &sa, 0);
    if (!bRet)
    {
        printf("管道创建失败");
        return;
    }

    bRet = CreatePipe(&hCmdRead, &hWrite, &sa, 0);
    if (!bRet)
    {
        printf("管道创建失败");
        return;
    }

    //启动子进程
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdInput = hCmdRead;
    si.hStdOutput = hCmdWrite;
    si.hStdError = hCmdWrite;

    ZeroMemory(&pi, sizeof(pi));


        
    if (!CreateProcess(NULL, // No module name (use command line). 
        (LPSTR)"cmd.exe", // Command line. 
        NULL,             // Process handle not inheritable. 
        NULL,             // Thread handle not inheritable. 
        TRUE,            // Set handle inheritance to FALSE. 
        CREATE_NO_WINDOW,                // 无窗口
        NULL,             // Use parent's environment block. 
        NULL,             // Use parent's starting directory. 
        &si,              // Pointer to STARTUPINFO structure.
        &pi))             // Pointer to PROCESS_INFORMATION
    {
        printf("子进程创建失败");
        return;
    }

    DWORD dwBytesWrited = 0;
    WriteFile(hWrite, pCmdData, nBufferLength, &dwBytesWrited, NULL);

    for (size_t i = 0; i < WAIT_TIME; i++)
    {
        Sleep(100);
        DWORD dwBytesAvail = 0;
        if (PeekNamedPipe(hRead, NULL, 0, NULL, &dwBytesAvail, NULL)
            && dwBytesAvail > 0)
        {
            char szReceive[0x1000] = { 0 };

            DWORD dwBytesReaded = 0;
            ReadFile(hRead,
                szReceive,
                0x1000,
                &dwBytesReaded,
                NULL);

            PKGHDR hdr;
            hdr.m_cmd = C2S_CMD;
            hdr.m_nLen = dwBytesReaded;

            send(sockClient, (char*)&hdr, sizeof(hdr), 0);
            send(sockClient, szReceive, hdr.m_nLen, 0);

            DWORD dwBytesWrited2 = 0;
            WriteFile(hWrite, "EXIT\r\n", sizeof("EXIT\r\n"), &dwBytesWrited2, NULL);

            return;
        }

    }

    PKGHDR hdr;
    hdr.m_cmd = C2S_CMD;
    hdr.m_nLen = sizeof("失败\r\n");

    send(sockClient, (char*)&hdr, sizeof(hdr), 0);
    send(sockClient, (char*)"失败\r\n", hdr.m_nLen, 0);

    DWORD dwBytesWrited3 = 0;
    WriteFile(hWrite, "EXIT\r\n", sizeof("EXIT\r\n"), &dwBytesWrited3, NULL);
}