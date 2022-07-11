#pragma once
#include <minwindef.h>
#pragma pack(push,1)

//包头
enum PKGCMD
{
    PKGCMD_INITIAL_VALUE,

    S2C_SCREEN,
    S2C_MOUSE,
    S2C_KEYBOARD,
    S2C_FILE_DRIVER,
    S2C_FILE_INFO,
    S2C_FILE_DOWN,
    S2C_FILE_UP,
    S2C_CMD,

    C2S_SCREEN,
    C2S_FILE_DRIVER,
    C2S_FILE_DELETE,
    C2S_FILE_INFO,
    C2S_FILE_DOWN,
    C2S_FILE_UP,
    C2S_CMD
};


typedef struct tagPkgHeader
{
    tagPkgHeader() :m_cmd(PKGCMD_INITIAL_VALUE), m_nLen(0) {}
    PKGCMD m_cmd; // 包头标志
    DWORD m_nLen; // 数据长度
}PKGHDR, * PPKGHDR;

//数据包

//屏幕数据
typedef struct tagC2S_SCREEN_CMD
{
    tagC2S_SCREEN_CMD() :m_dwScreenWith(-1), m_dwScreenHeigh(-1), m_dwDataLen(-1) {}
    DWORD m_dwScreenWith;
    DWORD m_dwScreenHeigh;
    DWORD m_dwDataLen;
    BYTE m_data[1];
}C2SSCREENCMD, * PC2SSCREENCMD;

//鼠标标志
enum MOUSECMD
{
    MOUSECMD_INITIAL_VALUE,

    MOUSE_MOVE,

    L_Button_Down,
    L_Button_Up,
    L_Button_DblClk,

    R_Button_Down,
    R_Button_Up,
    R_Button_DblClk
};

//鼠标数据
typedef struct tagS2C_MOUSE_CMD
{
    tagS2C_MOUSE_CMD() :m_dwType(MOUSECMD_INITIAL_VALUE), m_dwX(-1), m_dwY(-1) {}
    MOUSECMD m_dwType;
    DWORD m_dwX;
    DWORD m_dwY;
}S2CMOUSECMD, * PS2CMOUSECMD;

//键盘标志
enum KEYBOARDCMD
{
    KEYBOARDCMD_INITIAL_VALUE,

    KEYDOWN,
    KEYUP
};

//键盘数据
typedef struct tagS2C_KEYBOARD_CMD
{
    tagS2C_KEYBOARD_CMD() :m_dwType(KEYBOARDCMD_INITIAL_VALUE), m_nChar(-1), m_nRepCnt(-1), m_nFlags(-1) {}
    KEYBOARDCMD m_dwType;
    UINT m_nChar;
    UINT m_nRepCnt;
    UINT m_nFlags;
}S2CKEYBOARDCMD, * PS2CKEYBOARDCMD;

//文件数据
typedef struct tagC2S_DRIVER_CMD
{
    BYTE m_szDrive[MAXBYTE] = { 0 };
}C2SDRIVERCMD, * PC2SDRIVERCMD;

typedef struct tagS2C_PATH_CMD
{
    CHAR m_szFilePath[MAXBYTE] = { 0 };
}S2CPATHCMD, * PS2CPATHCMD;

typedef struct tagC2S_FILE_INFO_CMD
{
    INT m_nItem;
    CHAR m_szFileName[MAXBYTE] = { 0 };
    CHAR m_szFileType[MAXBYTE] = { 0 };
}C2SFILEINFOCMD, * PC2SFILEINFOCMD;

typedef struct tagTRANSFER_INFO
{
    CHAR m_szRemotePath[MAXBYTE] = { 0 };
    CHAR m_szControlPath[MAXBYTE] = { 0 };
    __int64 nFileSize = 0;
}TRANSFERINFO, * PTRANSFERINFO;



//CMD数据
//直接使用char*传一个字符串

#pragma pack(pop)