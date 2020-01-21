
#include <stdio.h>
#include <time.h>
#include <tchar.h>
#include <windows.h>
#include <windowsx.h>

#include "global.h"
#include "resource.h"
#include "runit.h"
#include "input.h"
#include "autocompletion.h"
#include "quickey.h"
#include "utils_win32.h"

#define EM_SETCUEBANNER 0x1501

const TCHAR *Tips[]={
    _T("RunIt  -  TIP: Ctrl + U to erase characters before caret"),
    _T("RunIt  -  TIP: '/' to call internal command"),
    NULL
};

extern HWND hAutoCompletion;
WNDPROC oldInputWndProc;

const TCHAR * GetRandomTip()
{
	int max_tips = sizeof(Tips) / sizeof(TCHAR *) - 1;
	int u = rand() * max_tips / (RAND_MAX + 1);
    debug_output(_T("Random tip: %d\n"),u);
    return Tips[u];
}
LRESULT CALLBACK InputWindowWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    TCHAR cmdString[MAX_BUFFER];
    switch (message) {
    case WM_KEYDOWN:
        switch (wParam) {
        case VK_ESCAPE:
            SendMessage(GetParent(hwnd),message,wParam,lParam);
            SendMessage(hwnd,EM_SETCUEBANNER,TRUE,(LPARAM)GetRandomTip());
            return 1;
        case VK_CONTROL:
            GetKeyNameText(lParam,cmdString,MAX_BUFFER);
            debug_output(_T("cmd:%s\n"),cmdString);
            return 1;
        case 'U':
            if(HIBYTE(GetKeyState(VK_CONTROL))){
                SendMessage(hAutoCompletion,WM_RUNIT_ACNOTIFY_CLEAR,0,0);
				Edit_GetText(hwnd,cmdString,MAX_BUFFER);
                GetKeyNameText(lParam,cmdString+_tcslen(cmdString)+1,MAX_BUFFER);
                debug_output(_T("cmd:%s Ctrl + %s\n"),cmdString,cmdString+_tcslen(cmdString)+1);
                // select from 0 to current caret
                SendMessage(hwnd,EM_SETSEL,0,HIWORD(SendMessage(hwnd,EM_GETSEL,0,0)));
                // clear selecting area
                SendMessage(hwnd,WM_CLEAR,0,0);
                SendMessage(hwnd,EM_SETCUEBANNER,TRUE,(LPARAM)GetRandomTip());
            }
            return 1;
        case VK_RETURN:
			// select first match item automatically
			AC_SelectNextItem(hwnd,0);
            Edit_GetText(hwnd,cmdString,MAX_BUFFER);
            SendMessage(GetParent(hwnd),WM_RUNIT_RUNCMD,0,(LPARAM)cmdString);
            SendMessage(hwnd,EM_SETCUEBANNER,TRUE,(LPARAM)GetRandomTip());
            return 1;
        case 'P':
            if(EnableDropdown>0&&EnableCtrlPN>0&&HIBYTE(GetKeyState(VK_CONTROL))){
                AC_SelectNextItem(hwnd,-1);
            }
            return 1;
        case 'N':
            if(EnableDropdown>0&&EnableCtrlPN>0&&HIBYTE(GetKeyState(VK_CONTROL))){
                AC_SelectNextItem(hwnd,+1);
            }
            return 1;
        case 'K':
            if(EnableDropdown>0&&EnableCtrlKJ>0&&HIBYTE(GetKeyState(VK_CONTROL))){
                AC_SelectNextItem(hwnd,-1);
            }
            return 1;
        case 'J':
            if(EnableDropdown>0&&EnableCtrlKJ>0&&HIBYTE(GetKeyState(VK_CONTROL))){
                AC_SelectNextItem(hwnd,+1);
            }
            return 1;
        case VK_UP:
            if(EnableDropdown>0&&EnableUPDOWN>0){
                AC_SelectNextItem(hwnd,-1);
            }
            return 1;
        case VK_DOWN:
            if(EnableDropdown>0&&EnableUPDOWN>0){
                AC_SelectNextItem(hwnd,+1);
            }
            return 1;
        default:
            break;
        }
        break;
    case WM_CHAR:
        switch(wParam) {
        case 0x0D:  // CR
        case 0x0A:  // LF
            break;
        case 0x09:  // tab
            if(EnableDropdown>0&&EnableTAB>0){
                AC_SelectNextItem(hwnd,0);
            }
            return 1;
        default:
            if(wParam>31||wParam==0x08){  // only received display characters and 0x08 (BS)
                // debug_output(_T("char :0x%2x\n"),(wParam));
                CallWindowProc(oldInputWndProc, hwnd, message,wParam, lParam);
                Edit_GetText(hwnd,cmdString,MAX_BUFFER);
                // debug_output(_T("cmd : %s\n"),(cmdString));
                if(EnableDropdown>0){
                    SendMessage(hAutoCompletion,WM_RUNIT_ACNOTIFY_TEXT,_tcslen(cmdString),(LPARAM)cmdString);
                }
                return 1;
            }
        }
    default:
        break;
    }
    return CallWindowProc(oldInputWndProc, hwnd, message,wParam, lParam);
}

HWND CreateInputWindow(HWND parent,int margin)
{

    int Width=GetSystemMetrics(SM_CXSCREEN)/2;
	LOGFONT lf;
	GetObject(hRunitFont, sizeof(lf), &lf);
    int Height=-lf.lfHeight + margin * 2 + 2 /* border */;
    debug_output(_T("input size: %dx%d\n"),Width,Height);
    HWND hWnd=CreateWindow(_T("EDIT"),_T(""),WS_CHILD|WS_VISIBLE|WS_BORDER ,
                             margin,margin,Width,Height,
                             parent,NULL,NULL,NULL);


    SendMessage(hWnd,EM_SETCUEBANNER,TRUE,(LPARAM)GetRandomTip());

    SetWindowFont(hWnd,hRunitFont,FALSE);
    oldInputWndProc=SubclassWindow(hWnd,InputWindowWndProc);
    return hWnd;
}
