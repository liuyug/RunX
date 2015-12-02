
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include "global.h"
#include "runit.h"
#include "resource.h"
#include "input.h"
#include "quickey.h"
#include "autocompletion.h"
#include "execommand.h"
#include "config.h"
#include "utils_win32.h"

#define version _T("1.2.1")
/*
#ifdef __WIN64__
1                  RT_MANIFEST    "RunIt.exe.ManifestX64"
#else
1                  RT_MANIFEST    "RunIt.exe.Manifest"
#endif
*/

HWND hInput;
HWND hAutoCompletion;
HWND hConfig;
HFONT hRunitFont;
int  ExecAll;
int  ExecConsoleApp;
int  AutoStart;
int  EnableDropdown;
int  DropdownItem;
int  EnableCtrlPN;
int  EnableCtrlKJ;
int  EnableUPDOWN;
int  EnableTAB;

unsigned int  HK_Mod;
unsigned int  HK_Key;


ATOM ID_HOTKEY;
NOTIFYICONDATA nifData;

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);


/*  Make the class name into a global variable  */
const TCHAR * szClassName = _T("RunIt");
TCHAR szTitle[256];



int WINAPI WinMain (HINSTANCE hThisInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpszArgument,
                    int nCmdShow)
{
    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */
#ifdef __WIN64__
    _stprintf_s(szTitle,_T("RunIt x64 v%s"),version);
#else
    _stprintf_s(szTitle,_T("RunIt v%s"),version);
#endif
    debug_output(szTitle);
    ExecAll=1;
    ExecConsoleApp=1;
#ifdef NDEBUG
    AutoStart=1;
#else
    AutoStart=0;
#endif
    EnableDropdown=1;
    DropdownItem=5;
    EnableCtrlPN=1;
    EnableCtrlKJ=1;
    EnableUPDOWN=1;
    EnableTAB=1;

    HK_Mod=0;
#ifdef NDEBUG
    HK_Key=0x91;    // ScrLk
#else
    HK_Key=45;      // Insert
#endif
    quickey_init();

    LOGFONT logFont;
    memset(&logFont,0,sizeof(logFont));
    logFont.lfHeight=-MulDiv(11, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72);
    debug_output(_T("font height:%ld\n"),logFont.lfHeight);
    //logFont.lfWeight=FW_NORMAL;
    logFont.lfWeight=FW_BOLD;
    logFont.lfCharSet=DEFAULT_CHARSET;
    logFont.lfOutPrecision=OUT_DEFAULT_PRECIS;
    logFont.lfClipPrecision=CLIP_DEFAULT_PRECIS;
    logFont.lfQuality=DEFAULT_QUALITY;
    logFont.lfPitchAndFamily=DEFAULT_PITCH|FF_DONTCARE<<4;
    _tcscpy_s(logFont.lfFaceName,_T("Ms Shell Dlg"));
    hRunitFont=CreateFontIndirect(&logFont);

    INITCOMMONCONTROLSEX icce;
    icce.dwSize=sizeof(icce);
    icce.dwICC=ICC_TREEVIEW_CLASSES|ICC_LISTVIEW_CLASSES|ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icce);
    CoInitialize(NULL);

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (hThisInstance, MAKEINTRESOURCE(IDI_ICON_APP));
    wincl.hIconSm = LoadIcon (hThisInstance, MAKEINTRESOURCE(IDI_ICON_APP));
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;

    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
               WS_EX_TOOLWINDOW,                   /* Extended possibilites for variation */
               szClassName,         /* Classname */
               szTitle,       /* Title Text */
               WS_POPUP|WS_BORDER , /* default window */
               CW_USEDEFAULT ,       /* Windows decides the position */
               CW_USEDEFAULT ,       /* where the window ends up on the screen */
               CW_USEDEFAULT ,                 /* The programs width */
               CW_USEDEFAULT ,                 /* and height in pixels */
               HWND_DESKTOP,        /* The window is a child-window to desktop */
               NULL,                /* No menu */
               hThisInstance,       /* Program Instance handler */
               NULL                 /* No Window Creation data */
           );

    /* Make the window visible on the screen */
    nCmdShow=SW_HIDE;
    ShowWindow (hwnd, nCmdShow);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&messages, NULL, 0, 0)) {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }

    /* The program return-value is 0 - The value that PostQuitMessage() gave */

    quickey_close();
    return messages.wParam;
}

void CalcMainWindowPos(HWND hwnd,int margin)
{
    RECT rect;
    SIZE clientSize,screenSize,winSize;
    GetWindowRect(hInput,&rect);
    clientSize.cx=rect.right-rect.left;
    clientSize.cy=rect.bottom-rect.top;
    screenSize.cx=GetSystemMetrics(SM_CXSCREEN);
    screenSize.cy=GetSystemMetrics(SM_CYSCREEN);
    winSize.cx=clientSize.cx+margin*2+GetSystemMetrics(SM_CXBORDER)*2;
    winSize.cy=clientSize.cy+margin*2+GetSystemMetrics(SM_CYBORDER)*2;
    SetWindowPos(hwnd,HWND_BOTTOM,
                 (screenSize.cx-winSize.cx)/2,
                 (screenSize.cy-winSize.cy)/2,
                 winSize.cx,
                 winSize.cy,
                 SWP_FRAMECHANGED|SWP_NOZORDER);

}
int ShowTrayMenu(HWND hWnd)
{
    HMENU hPopupMenu;
    POINT pt;
    hPopupMenu=LoadMenu(NULL,MAKEINTRESOURCE(IDR_MENU_TRAY));
//    if(SetMenuItemBitmaps(hPopupMenu,IDM_ABOUT,MF_BYCOMMAND,
//                          (HBITMAP)LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_ICON_APP),IMAGE_ICON,0,0,LR_DEFAULTCOLOR),NULL)==0){
//        debug_errmsg(_T("SetMenuItemBitmaps"),GetWin32ErrorMessage(0));
//    }
    GetCursorPos(&pt);
    SetForegroundWindow(hWnd);
    TrackPopupMenu(GetSubMenu(hPopupMenu,0),TPM_LEFTALIGN,
                   pt.x,pt.y,0,hWnd,NULL);
    DestroyMenu(hPopupMenu);
    return 0;
}
void ShowAbout(HWND hWnd)
{
    TCHAR message[MAX_BUFFER];
    _stprintf_s(message,_T("%s\nCopyright (C) 2012 Liu Yugang <liuyug@gmail.com>"),szTitle);
    MessageBox(hWnd,message,szTitle,MB_ICONINFORMATION|MB_OK);
}
void ShowConfig(HWND hWnd)
{
    ShowWindow(hConfig,SW_RESTORE);
}

/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int margin=5;
    RECT rect;
    POINT pt;
    TCHAR cmdString[MAX_BUFFER];
    switch (message) {                /* handle the messages */
    case WM_CREATE:
        //SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | ~WS_EX_APPWINDOW);
        hInput=CreateInputWindow(hwnd,margin);
        CalcMainWindowPos(hwnd,margin);
        SetFocus(hInput);
        hAutoCompletion=CreateAutoCompletionWindow(hwnd);
        hConfig=CreateConfigWindow(hwnd);

        ID_HOTKEY=GlobalAddAtom(szTitle);
        RegisterHotKey(hwnd,ID_HOTKEY,HK_Mod,HK_Key);

        memset(&nifData,0,sizeof(NOTIFYICONDATA));
        nifData.cbSize=sizeof(NOTIFYICONDATA);
        nifData.hWnd=hwnd;
        nifData.uID=ID_HOTKEY;
        nifData.uCallbackMessage=WM_RUNIT_TRAYNOTIFY;
        nifData.hIcon=LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON_APP));
        _tcscpy_s(nifData.szTip,szTitle);
        nifData.uFlags=NIF_ICON|NIF_TIP|NIF_MESSAGE;
        Shell_NotifyIcon(NIM_ADD,&nifData);
        break;
    case WM_DESTROY:
        Shell_NotifyIcon(NIM_DELETE,&nifData);
        GlobalDeleteAtom(ID_HOTKEY);
        PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
        break;
    case WM_KEYDOWN:
        if(wParam==VK_ESCAPE) {
            ShowWindow(hwnd,SW_HIDE);
            if(EnableDropdown>0) {
                ShowWindow(hAutoCompletion,SW_HIDE);
            }
        }
        break;
    case WM_HOTKEY:
        if(wParam==ID_HOTKEY) {
            if(IsWindowVisible(hwnd)) {
                if(EnableDropdown>0) {
                    SetForegroundWindow(hAutoCompletion);
                }
                SetForegroundWindow(hwnd);
            } else {
                if(EnableDropdown>0) {
                    ShowWindow(hAutoCompletion,SW_RESTORE);
                }
                ShowWindow(hwnd,SW_RESTORE);
            }
        }
        break;
    case WM_NCHITTEST:  // move window
        return HTCAPTION;
    case WM_MOVE:
        if(EnableDropdown>0) {
            GetWindowRect(hwnd,&rect);
            pt.x=rect.left;
            pt.y=rect.bottom;
            GetWindowRect(hAutoCompletion,&rect);
            MoveWindow(hAutoCompletion,pt.x,pt.y,rect.right-rect.left,rect.bottom-rect.top,TRUE);
        }
        break;
    case WM_SETFOCUS:
        SetFocus(hInput);
        break;
    case WM_RUNIT_TRAYNOTIFY:
        switch(lParam) {
        case WM_RBUTTONUP:
            ShowTrayMenu(hwnd);
            return 0;
        }
        break;
    case WM_RUNIT_RUNCMD:
        // hide autocomplete window
        const TCHAR * pszInputKey;
        pszInputKey=(TCHAR *)lParam;
        if(EnableDropdown>0) {
            SendMessage(hAutoCompletion,WM_RUNIT_ACNOTIFY_CLEAR,0,0);
            ShowWindow(hAutoCompletion,SW_HIDE);
            // find real path from db
            quickey_getexec(pszInputKey,&cmdString);
            if(cmdString[0]==_T('\0')) {    // not found
                if(ExecAll>0) {
#ifdef SEARCHPATH
                    if(pszInputKey[0]==_T('\\')) {  // network share
                        _tcscpy_s(cmdString,pszInputKey);
                    } else if(SearchPath(NULL,pszInputKey,_T(".exe"),MAX_BUFFER,cmdString,NULL)==0) {
                        debug_errmsg(_T("SearchPath"),GetWin32ErrorMessage(0));
                        _stprintf_s(cmdString,_T("Can't find file: \"%s\""),pszInputKey);
                        MessageBox(hwnd,cmdString,szTitle,MB_ICONWARNING|MB_OK);
                        ShowWindow(hAutoCompletion,SW_RESTORE);
                        return 0;
                    };
#else
                    _tcscpy_s(cmdString,pszInputKey);
#endif
                } else {
                    _stprintf_s(cmdString,_T("Can't find file: \"%s\""),pszInputKey);
                    MessageBox(hwnd,cmdString,szTitle,MB_ICONWARNING|MB_OK);
                    ShowWindow(hAutoCompletion,SW_RESTORE);
                    return 0;
                }
            }
        } else {
#ifdef SEARCHPATH
            SearchPath(NULL,pszInputKey,_T(".exe"),MAX_BUFFER,cmdString,NULL);
#else
            _tcscpy_s(cmdString,pszInputKey);
#endif
        }
        ShowWindow(hwnd,SW_HIDE);
        debug_output(_T("cmd:%s\n"),cmdString);
        SetWindowText(hInput,_T(""));
        ExecCommand(hwnd,cmdString,0);
        return 0;
    case WM_COMMAND:
        switch(LOWORD(wParam)) {
        case IDM_QUIT:
            DestroyWindow(hwnd);
            return 0;
        case IDM_ABOUT:
            ShowAbout(NULL);
            return 0;
        case IDM_SETTINGS:
            ShowConfig(hwnd);
            return 0;
        }
        break;
    default:                      /* for messages that we don't deal with */
        return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}
