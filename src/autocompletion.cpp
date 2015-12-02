
#include <tchar.h>
#include <windows.h>
#include <windowsx.h>

#include "global.h"
#include "runit.h"
#include "quickey.h"
#include "autocompletion.h"

WNDPROC oldAutoCompletionWndProc;

void AC_SelectNextItem(HWND hEdit,int skip)
{
    int selIndex=ListBox_GetCurSel(hAutoCompletion);
    if(skip!=0){
        selIndex=max(selIndex+skip,0);
        ListBox_SetCurSel(hAutoCompletion,selIndex);
        selIndex=min(ListBox_GetCount(hAutoCompletion)-1,selIndex);
    }
    int txtlen;
    TCHAR cmdString[MAX_BUFFER];
    if(ListBox_GetText(hAutoCompletion,selIndex,cmdString)!=LB_ERR){
        Edit_SetText(hEdit,cmdString);
        txtlen=Edit_GetTextLength(hEdit);
        // move caret to end position
        Edit_SetSel(hEdit,txtlen,txtlen);
    }
    return ;
}

LRESULT CALLBACK AutoCompletionWindowWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    TCHAR input[MAX_BUFFER];
    switch (message) {
    case WM_SETFOCUS:
        SetFocus(hInput);
        break;
    case WM_RUNIT_ACNOTIFY_TEXT:
        while(ListBox_DeleteString(hwnd,0)>0);
        _tcscpy_s(input,(TCHAR *)lParam);
        if(input[0]!=('\0')) {
            quickey_listbox_search(input,hwnd);
            ListBox_SetCurSel(hwnd,0);
        }
        return 0;
    case WM_RUNIT_ACNOTIFY_CLEAR:
        while(ListBox_DeleteString(hwnd,0)>0);
        return 0;
    default:
        break;
    }
    return CallWindowProc(oldAutoCompletionWndProc, hwnd, message,wParam, lParam);
}

HWND CreateAutoCompletionWindow(HWND parent)
{
    RECT rect;
    GetWindowRect(parent,&rect);
    int Width=rect.right-rect.left;
    int Height=(rect.bottom-rect.top)*DropdownItem;

    HWND hWnd=CreateWindow(_T("LISTBOX"),_T(""),LBS_NOTIFY|LBS_HASSTRINGS|WS_POPUP|WS_BORDER|WS_VSCROLL ,
                             rect.left,rect.bottom,Width,Height,
                             parent,NULL,NULL,NULL);

    SetWindowFont(hWnd,hRunitFont,FALSE);
    oldAutoCompletionWndProc=SubclassWindow(hWnd,AutoCompletionWindowWndProc);
    return hWnd;
}
