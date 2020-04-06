#include <tchar.h>
#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>
#include <commctrl.h>

#include "global.h"
#include "runx.h"
#include "resource.h"
#include "config.h"
#include "quickey.h"
#include "utils_win32.h"

INT_PTR CALLBACK DlgGeneralWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgDropdownlWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgFilesWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgPathsWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgHotkeyWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

struct PAGEGROUP_TAG {
    const TCHAR *name;
    HWND  page;
    unsigned int id;
    DLGPROC proc;
};
struct PAGEGROUP_TAG PageGroup[]= {
    {_T("General"),NULL,IDD_DIALOG_GENERAL,DlgGeneralWndProc},
    {_T("Dropdown List"),NULL,IDD_DIALOG_DROPDOWNLIST,DlgDropdownlWndProc},
    {_T("Paths"),NULL,IDD_DIALOG_PATHS,DlgPathsWndProc},
    {_T("Files"),NULL,IDD_DIALOG_FILES,DlgFilesWndProc},
    {_T("Hot Key"),NULL,IDD_DIALOG_HOTKEY,DlgHotkeyWndProc},
    {_T(""),NULL,0,NULL}
};

const TCHAR *ColNames[]= {
    _T("Key"),
    _T("Path"),
    NULL,
    _T("Priority"),
    NULL
};
HWND GetPageHandle(unsigned int id)
{
    int iPage=0;
    while(PageGroup[iPage].name!=NULL) {
        if(PageGroup[iPage].id==id)
            return PageGroup[iPage].page;
        iPage++;
    }
    return NULL;
}
HWND GetPageHandle(const TCHAR *name)
{
    int iPage=0;
    while(PageGroup[iPage].name!=NULL) {
        if(_tcscmp(PageGroup[iPage].name,name)==0)
            return PageGroup[iPage].page;
        iPage++;
    }
    return NULL;
}
void DoDropdown()
{
    RECT rect;
    POINT pt;
    GetWindowRect(GetParent(hInput),&rect);
    pt.x=rect.left;
    pt.y=rect.bottom;
    int Width=rect.right-rect.left;
    int Height=(rect.bottom-rect.top)*DropdownItem;
    MoveWindow(hAutoCompletion,pt.x,pt.y,Width,Height,TRUE);
}
void DoRegisterHotKey()
{
    UnregisterHotKey(GetParent(hInput),ID_HOTKEY);
    BOOL ret = RegisterHotKey(GetParent(hInput),ID_HOTKEY,HK_Mod,HK_Key);
    if (ret != TRUE)
        debug_error(GetWin32ErrorMessage(GetLastError()));
}
void DoAutoStart()
{
    long reg_ret;
    const TCHAR *SubkeyRun=_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
    const TCHAR *Name=_T("RunIt");
    TCHAR Value[MAX_BUFFER];
    HKEY hKey;
    reg_ret=RegOpenKeyEx(HKEY_CURRENT_USER,SubkeyRun,0,KEY_ALL_ACCESS,&hKey);
    if(reg_ret!=ERROR_SUCCESS) {
        debug_errmsg(_T("RegOpenKeyEx"),GetWin32ErrorMessage(reg_ret));
        return;
    }
    if(AutoStart>0) {
        GetModuleFileName(NULL,Value,MAX_BUFFER);
		debug_errmsg(_T("GetModuleFileName"), Value);
        reg_ret=RegSetValueEx(hKey,Name,0,REG_SZ,(BYTE*)Value,_tcslen(Value) * sizeof(TCHAR));
    } else {
        reg_ret=RegDeleteValue(hKey,Name);
    }
    if(reg_ret!=ERROR_SUCCESS) {
        debug_errmsg(_T("RegSetValueEx or RegDeleteValue"),GetWin32ErrorMessage(reg_ret));
    }
    RegCloseKey(hKey);
}
void ApplyConfig()
{
    TCHAR value[MAX_BUFFER];
    AutoStart=Button_GetCheck(GetDlgItem(GetPageHandle(IDD_DIALOG_GENERAL),IDC_CHECKBOX_AUTOSTART));
    ExecAll=Button_GetCheck(GetDlgItem(GetPageHandle(IDD_DIALOG_GENERAL),IDC_CHECKBOX_EXECALL));
    ExecConsoleApp=Button_GetCheck(GetDlgItem(GetPageHandle(IDD_DIALOG_GENERAL),IDC_CHECKBOX_EXECONSOLEAPP));

    EnableDropdown=Button_GetCheck(GetDlgItem(GetPageHandle(IDD_DIALOG_DROPDOWNLIST),IDC_CHECKBOX_ENABLEDROPDOWNLIST));
    DropdownItem=GetDlgItemInt(GetPageHandle(IDD_DIALOG_DROPDOWNLIST),IDC_EDIT_DROPDONWITEM,NULL,FALSE);
    EnableCtrlPN=Button_GetCheck(GetDlgItem(GetPageHandle(IDD_DIALOG_DROPDOWNLIST),IDC_CHECKBOX_CTRLPN));
    EnableCtrlKJ=Button_GetCheck(GetDlgItem(GetPageHandle(IDD_DIALOG_DROPDOWNLIST),IDC_CHECKBOX_CTRLKJ));
    EnableUPDOWN=Button_GetCheck(GetDlgItem(GetPageHandle(IDD_DIALOG_DROPDOWNLIST),IDC_CHECKBOX_UPDOWN));
    EnableTAB=Button_GetCheck(GetDlgItem(GetPageHandle(IDD_DIALOG_DROPDOWNLIST),IDC_CHECKBOX_TAB));

    HK_Mod=0;
    if(Button_GetCheck(GetDlgItem(GetPageHandle(IDD_DIALOG_HOTKEY),IDC_CHECKBOX_CTRL))) {
        HK_Mod|=MOD_CONTROL;
    }
    if(Button_GetCheck(GetDlgItem(GetPageHandle(IDD_DIALOG_HOTKEY),IDC_CHECKBOX_ALT))) {
        HK_Mod|=MOD_ALT;
    }
    if(Button_GetCheck(GetDlgItem(GetPageHandle(IDD_DIALOG_HOTKEY),IDC_CHECKBOX_WIN))) {
        HK_Mod|=MOD_WIN;
    }
    if(Button_GetCheck(GetDlgItem(GetPageHandle(IDD_DIALOG_HOTKEY),IDC_CHECKBOX_SHIFT))) {
        HK_Mod|=MOD_SHIFT;
    }
    quickey_clearallsetting();
    quickey_putsettings();
    Edit_GetText(GetDlgItem(GetPageHandle(IDD_DIALOG_PATHS),IDC_EDIT_FILTERS),value,MAX_BUFFER);
    quickey_putsetting(_T("path_filters"),value);
    for(int i=0; i<ListBox_GetCount(GetDlgItem(GetPageHandle(IDD_DIALOG_PATHS),IDC_LIST_PATH)); i++) {
        ListBox_GetText(GetDlgItem(GetPageHandle(IDD_DIALOG_PATHS),IDC_LIST_PATH),i,value);
        quickey_listbox_addpath(value);
    }

    // action
    DoAutoStart();
    DoRegisterHotKey();
    DoDropdown();
}
void UpdateListView(HWND hWnd)
{
    ListView_DeleteAllItems(hWnd);

    quickey_listview_update(_T(""),hWnd);
}
void DeleteSelFiles(HWND hWnd)
{
    int iSelCount=ListView_GetSelectedCount(hWnd);
    int iCount=ListView_GetItemCount(hWnd);
    int iItem=iCount-1;
    TCHAR szItemText[MAX_BUFFER];
    while(iItem>-1 && iSelCount>0) {
        if(LVIS_SELECTED&ListView_GetItemState(hWnd,iItem,LVIS_SELECTED)) {
            ListView_GetItemText(hWnd,iItem,0,szItemText,MAX_BUFFER);
            ListView_DeleteItem(hWnd,iItem);
            iSelCount--;
            quickey_deletefile(szItemText);
        }
        iItem--;
    }
}
void DeleteSelPaths(HWND hWnd)
{
    int selcount=ListBox_GetSelCount(hWnd);
    int *items=new int[selcount];
    ListBox_GetSelItems(hWnd,selcount,items);
    for(int i=selcount-1; i>-1; i--) {
        ListBox_DeleteString(hWnd,items[i]);
    }
}

INT_PTR CALLBACK DlgGeneralWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message) {
    case WM_INITDIALOG:
        Button_SetCheck(GetDlgItem(hwnd,IDC_CHECKBOX_EXECALL),ExecAll>0?BST_CHECKED:BST_UNCHECKED);
        Button_SetCheck(GetDlgItem(hwnd,IDC_CHECKBOX_EXECONSOLEAPP),ExecConsoleApp>0?BST_CHECKED:BST_UNCHECKED);
        Button_SetCheck(GetDlgItem(hwnd,IDC_CHECKBOX_AUTOSTART),AutoStart>0?BST_CHECKED:BST_UNCHECKED);
        return 1;
    default:
        break;
    }
    return 0;
}
INT_PTR CALLBACK DlgDropdownlWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message) {
    case WM_INITDIALOG:
        Button_SetCheck(GetDlgItem(hwnd,IDC_CHECKBOX_ENABLEDROPDOWNLIST),EnableDropdown>0?BST_CHECKED:BST_UNCHECKED);
        SetDlgItemInt(hwnd,IDC_EDIT_DROPDONWITEM,DropdownItem,FALSE);
        Button_SetCheck(GetDlgItem(hwnd,IDC_CHECKBOX_CTRLKJ),EnableCtrlKJ>0?BST_CHECKED:BST_UNCHECKED);
        Button_SetCheck(GetDlgItem(hwnd,IDC_CHECKBOX_CTRLPN),EnableCtrlPN>0?BST_CHECKED:BST_UNCHECKED);
        Button_SetCheck(GetDlgItem(hwnd,IDC_CHECKBOX_UPDOWN),EnableUPDOWN>0?BST_CHECKED:BST_UNCHECKED);
        Button_SetCheck(GetDlgItem(hwnd,IDC_CHECKBOX_TAB),EnableTAB>0?BST_CHECKED:BST_UNCHECKED);
        return 1;
    default:
        break;
    }
    return 0;
}
WNDPROC oldHotkeyWndProc;
LRESULT CALLBACK HotkeyWindowWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    TCHAR key[MAX_BUFFER];
    switch (message) {
    case WM_SETFOCUS:
        HideCaret(hwnd);
        return 0;
    case WM_KEYDOWN:
        switch (wParam) {
        case VK_CONTROL:
        case VK_MENU:
        case VK_SHIFT:
        case VK_LWIN:
        case VK_RWIN:
            return 0;
        default:
            HK_Key=wParam;
            GetKeyNameText(lParam,key,MAX_BUFFER);
            SetWindowText(hwnd,key);
            _stprintf_s(key,MAX_BUFFER,_T("%d"),HK_Key);
            SetWindowText(GetDlgItem(GetParent(hwnd),IDC_EDIT_HOTKEY_DEC),key);
        }
        return 0;
    case WM_KEYUP:
        return 0;
    case WM_CHAR:
        return 0;
    }
    return CallWindowProc(oldHotkeyWndProc, hwnd, message,wParam, lParam);
}
INT_PTR CALLBACK DlgHotkeyWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    switch(message) {
    case WM_INITDIALOG:
        TCHAR key[MAX_BUFFER];
        oldHotkeyWndProc=SubclassWindow(GetDlgItem(hwnd,IDC_EDIT_HOTKEY),HotkeyWindowWndProc);

        GetKeyNameText(MapVirtualKey(HK_Key,MAPVK_VK_TO_VSC)<<16,key,MAX_BUFFER);

        SetWindowText(GetDlgItem(hwnd,IDC_EDIT_HOTKEY),key);
        _stprintf_s(key,MAX_BUFFER,_T("%d"),HK_Key);
        SetWindowText(GetDlgItem(hwnd,IDC_EDIT_HOTKEY_DEC),key);
        Button_SetCheck(GetDlgItem(hwnd,IDC_CHECKBOX_CTRL),HK_Mod&MOD_CONTROL?BST_CHECKED:BST_UNCHECKED);
        Button_SetCheck(GetDlgItem(hwnd,IDC_CHECKBOX_ALT),HK_Mod&MOD_ALT?BST_CHECKED:BST_UNCHECKED);
        Button_SetCheck(GetDlgItem(hwnd,IDC_CHECKBOX_WIN),HK_Mod&MOD_WIN?BST_CHECKED:BST_UNCHECKED);
        Button_SetCheck(GetDlgItem(hwnd,IDC_CHECKBOX_SHIFT),HK_Mod&MOD_SHIFT?BST_CHECKED:BST_UNCHECKED);

        return 1;
    default:
        break;
    }
    return 0;
}
INT_PTR CALLBACK DlgFilesWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HWND hControl;
    switch(message) {
    case WM_INITDIALOG:
        hControl=GetDlgItem(hwnd,IDC_LIST_FILES);
        ListView_SetExtendedListViewStyle(hControl,LVS_EX_FULLROWSELECT);
        LVCOLUMN lvcol;
        int iCol;
        iCol=0;
        memset(&lvcol,0,sizeof(lvcol));
        lvcol.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT;
        lvcol.fmt=LVCFMT_LEFT;
        lvcol.cx=100;
        lvcol.pszText=(TCHAR *)ColNames[iCol];
        lvcol.cchTextMax=_tcslen(lvcol.pszText);
        ListView_InsertColumn(hControl,iCol++,&lvcol);
        lvcol.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT;
        lvcol.fmt=LVCFMT_LEFT;
        lvcol.cx=400+50;
        lvcol.pszText=(TCHAR *)ColNames[iCol];
        lvcol.cchTextMax=_tcslen(lvcol.pszText);
        ListView_InsertColumn(hControl,iCol++,&lvcol);

//        lvcol.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT;
//        lvcol.fmt=LVCFMT_LEFT;
//        lvcol.cx=50;
//        lvcol.pszText=(TCHAR *)ColNames[iCol];
//        lvcol.cchTextMax=_tcslen(lvcol.pszText);
//        ListView_InsertColumn(hControl,iCol++,&lvcol);

        UpdateListView(hControl);
        return 1;
    case WM_COMMAND:
        switch(LOWORD(wParam)) {
        case IDC_BUTTON_CLEARALL:
            quickey_clearall();
            ListView_DeleteAllItems(GetDlgItem(hwnd,IDC_LIST_FILES));
            return 1;
        case IDC_BUTTON_DELETEFILE:
            DeleteSelFiles(GetDlgItem(hwnd,IDC_LIST_FILES));
            return 1;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return 0;
}
INT_PTR CALLBACK DlgPathsWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    TCHAR path[MAX_BUFFER];
    TCHAR key[MAX_BUFFER];
    HCURSOR hCursor;
    switch(message) {
    case WM_INITDIALOG:
        quickey_getsetting(_T("path_filters"),path);
        debug(_T("path: %s"), path);
        if(_tcslen(path)>0) {
            SetDlgItemText(hwnd,IDC_EDIT_FILTERS,path);
        } else {
            SetDlgItemText(hwnd,IDC_EDIT_FILTERS,_T("*.lnk;*.exe"));
        }
        quickey_listbox_updatepath(GetDlgItem(hwnd,IDC_LIST_PATH));
#if 0
        if(ListBox_GetCount(GetDlgItem(hwnd,IDC_LIST_PATH))==0) {
            SHGetSpecialFolderPath(NULL,path,CSIDL_WINDOWS,0);
            ListBox_AddString(GetDlgItem(hwnd,IDC_LIST_PATH),path);
            SHGetSpecialFolderPath(NULL,path,CSIDL_SYSTEM,0);
            ListBox_AddString(GetDlgItem(hwnd,IDC_LIST_PATH),path);
        }
#endif
        return 1;
    case WM_COMMAND:
        switch(LOWORD(wParam)) {
        case IDC_BUTTON_ADDPATH:
            GetDlgItemText(hwnd,IDC_EDIT_PATH,path,MAX_BUFFER);
            ListBox_AddString(GetDlgItem(hwnd,IDC_LIST_PATH),path);
            return 1;
        case IDC_BUTTON_ADDENVPATH:
            GetEnvironmentVariable(_T("PATH"),path,MAX_BUFFER);
            TCHAR *b,*e;
            b=e=path;
            while(*e!=_T('\0')) {
                if(*e==_T(';')) {
                    *e=_T('\0');
                    if(_tcslen(b)>2)
                        ListBox_AddString(GetDlgItem(hwnd,IDC_LIST_PATH),b);
                    b=e+1;
                }
                e++;
            }
            if(_tcslen(b)>2)
                ListBox_AddString(GetDlgItem(hwnd,IDC_LIST_PATH),b);
            return 1;
        case IDC_BUTTON_DELPATH:
            DeleteSelPaths(GetDlgItem(hwnd,IDC_LIST_PATH));
            return 1;
        case IDC_BUTTON_BROWSE:
            GetDirectoryDialog(hwnd, path, MAX_BUFFER);
            Edit_SetText(GetDlgItem(hwnd,IDC_EDIT_PATH),path);
            return 1;
        case IDC_BUTTON_UPDATEFILES:
            hCursor=LoadCursor(NULL,IDC_WAIT);
            SetCursor(hCursor);
            TCHAR filters[MAX_BUFFER];
            GetDlgItemText(hwnd,IDC_EDIT_FILTERS,filters,MAX_BUFFER);
            TCHAR * filter;
            filter=filters;
            while(*filter!=_T('\0')) {
                if(*filter==_T(';')) {
                    *filter=_T('\0');
                }
                filter++;
            }
            filter++;
            *filter=_T('\0');
            for(int i=0; i<ListBox_GetCount(GetDlgItem(hwnd,IDC_LIST_PATH)); i++) {
                ListBox_GetText(GetDlgItem(hwnd,IDC_LIST_PATH),i,path);
                quickey_addfilesfrompath(path,filters,0);
            }
            UpdateListView(GetDlgItem(GetPageHandle(IDD_DIALOG_FILES),IDC_LIST_FILES));
            hCursor=LoadCursor(NULL,IDC_ARROW);
            SetCursor(hCursor);
            return 0;
        case IDC_BUTTON_ADDSTARTMENU:
            hCursor=LoadCursor(NULL,IDC_WAIT);
            SetCursor(hCursor);

            SHGetSpecialFolderPath(NULL,path,CSIDL_APPDATA,0);
            _tcscat_s(path,MAX_BUFFER,_T("\\Microsoft\\Internet Explorer\\Quick Launch"));
            quickey_addfilesfrompath(path,_T("*.lnk"),1);

            SHGetSpecialFolderPath(NULL,path,CSIDL_STARTMENU,0);
            quickey_addfilesfrompath(path,_T("*.lnk"),1);

            SHGetSpecialFolderPath(NULL,path,CSIDL_COMMON_STARTMENU,0);
            quickey_addfilesfrompath(path,_T("*.lnk"),1);

            UpdateListView(GetDlgItem(GetPageHandle(IDD_DIALOG_FILES),IDC_LIST_FILES));
            hCursor=LoadCursor(NULL,IDC_ARROW);
            SetCursor(hCursor);
            return 1;
        case IDC_BUTTON_ADDINTCMD:
            hCursor=LoadCursor(NULL,IDC_WAIT);
            SetCursor(hCursor);
            quickey_addintercmd();
            UpdateListView(GetDlgItem(GetPageHandle(IDD_DIALOG_FILES),IDC_LIST_FILES));
            hCursor=LoadCursor(NULL,IDC_ARROW);
            SetCursor(hCursor);
            return 1;
        case IDC_BUTTON_BROWSEFILE:
			path[0] = _T('"');
            GetFilesDialog(hwnd, path + 1, MAX_BUFFER);
			_tcscat_s(path,MAX_BUFFER, _T("\""));
            Edit_SetText(GetDlgItem(hwnd,IDC_EDIT_SINGLEFILE),path);
            quickey_genkey(path,key);
            Edit_SetText(GetDlgItem(hwnd,IDC_EDIT_SINGLEKEY),key);
            return 1;
        case IDC_BUTTON_ADDSINGLEFILE:
            hCursor=LoadCursor(NULL,IDC_WAIT);
            SetCursor(hCursor);
            Edit_GetText(GetDlgItem(hwnd,IDC_EDIT_SINGLEFILE),path,MAX_BUFFER);
            Edit_GetText(GetDlgItem(hwnd,IDC_EDIT_SINGLEKEY),key,MAX_BUFFER);
            quickey_addfile(path,key);
            UpdateListView(GetDlgItem(GetPageHandle(IDD_DIALOG_FILES),IDC_LIST_FILES));
            hCursor=LoadCursor(NULL,IDC_ARROW);
            SetCursor(hCursor);
            return 1;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return 0;
}
INT_PTR CALLBACK ConfigWindowWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message) {
    case WM_INITDIALOG:
        HWND hTreeView;


        SendMessage(hwnd,WM_SETICON,ICON_SMALL,(LPARAM)LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_ICON_APP)));
        SendMessage(hwnd,WM_SETICON,ICON_BIG,(LPARAM)LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_ICON_APP)));
        hTreeView=GetDlgItem(hwnd,IDC_TREE_SETTINGS);
        POINT pt;
        RECT  rcTree;
        GetWindowRect(hTreeView,&rcTree);
        pt.x=rcTree.right;
        pt.y=rcTree.top;
        ScreenToClient(hwnd,&pt);
        pt.x+=LOWORD(GetDialogBaseUnits());
        int iPage;
        iPage=0;
        while(PageGroup[iPage].name!=NULL) {
            PageGroup[iPage].page=CreateDialog(NULL,MAKEINTRESOURCE(PageGroup[iPage].id),
                                               hwnd,PageGroup[iPage].proc);
            SetWindowPos(PageGroup[iPage].page,NULL,pt.x,pt.y,0,0,
                         SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOSIZE|SWP_NOZORDER);
            iPage++;
        }
        // tree initialize
        HTREEITEM tv_root,tv_child;
        TVINSERTSTRUCT tvis;

        iPage=0;
        memset(&tvis,0,sizeof(tvis));
        tvis.hParent=NULL;
        tvis.hInsertAfter=TVI_ROOT;
        tvis.itemex.mask=TVIF_TEXT|TVIF_PARAM|TVIF_CHILDREN;
        tvis.itemex.pszText=(TCHAR *)PageGroup[iPage].name;
        tvis.itemex.cchTextMax=_tcslen(tvis.itemex.pszText);
        tvis.itemex.cChildren=0;
        tvis.itemex.lParam=(LPARAM)PageGroup[iPage].page;
        tv_root=TreeView_InsertItem(hTreeView,&tvis);
        ShowWindow((HWND)tvis.itemex.lParam,SW_SHOW);
        TreeView_SelectItem(hTreeView,tv_root);

        iPage++;
        tvis.hParent=NULL;
        tvis.hInsertAfter=TVI_ROOT;
        tvis.itemex.mask=TVIF_TEXT|TVIF_PARAM|TVIF_CHILDREN;
        tvis.itemex.pszText=(TCHAR *)PageGroup[iPage].name;
        tvis.itemex.cchTextMax=_tcslen(tvis.itemex.pszText);
        tvis.itemex.cChildren=1;
        tvis.itemex.lParam=(LPARAM)PageGroup[iPage].page;
        tv_root=TreeView_InsertItem(hTreeView,&tvis);
        iPage++;
        tvis.hParent=tv_root;
        tvis.hInsertAfter=TVI_LAST;
        tvis.itemex.pszText=(TCHAR *)PageGroup[iPage].name;
        tvis.itemex.cchTextMax=_tcslen(tvis.itemex.pszText);
        tvis.itemex.cChildren=0;
        tvis.itemex.lParam=(LPARAM)PageGroup[iPage].page;
        tv_child=TreeView_InsertItem(hTreeView,&tvis);
        iPage++;
        tvis.hParent=tv_root;
        tvis.hInsertAfter=TVI_LAST;
        tvis.itemex.pszText=(TCHAR *)PageGroup[iPage].name;
        tvis.itemex.cchTextMax=_tcslen(tvis.itemex.pszText);
        tvis.itemex.cChildren=0;
        tvis.itemex.lParam=(LPARAM)PageGroup[iPage].page;
        tv_child=TreeView_InsertItem(hTreeView,&tvis);
        iPage++;
        // hotkey
        tvis.hParent=NULL;
        tvis.hInsertAfter=TVI_ROOT;
        tvis.itemex.pszText=(TCHAR *)PageGroup[iPage].name;
        tvis.itemex.cchTextMax=_tcslen(tvis.itemex.pszText);
        tvis.itemex.cChildren=0;
        tvis.itemex.lParam=(LPARAM)PageGroup[iPage].page;
        tv_child=TreeView_InsertItem(hTreeView,&tvis);
        iPage++;

        return 1;
    case WM_CLOSE:
        EndDialog(hwnd, 0);
        return 1;
    case WM_NOTIFY:
        switch (wParam) {
        case IDC_TREE_SETTINGS :
            NMTREEVIEW * nmtv;
            HWND page;
            nmtv=(NMTREEVIEW*)lParam;
            switch(nmtv->hdr.code) {
            case TVN_SELCHANGED:
                page=(HWND)nmtv->itemOld.lParam;
                if(page!=NULL)
                    ShowWindow(page,SW_HIDE);
                page=(HWND)nmtv->itemNew.lParam;
                if(page!=NULL) {
                    ShowWindow(page,SW_SHOW);
                    SetFocus(page);
                }
                return 1;
            }
            break;
        }
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam)) {
        case IDOK:
            ApplyConfig();
            SendMessage(hwnd,WM_CLOSE,0,0);
            return 1;
        case IDC_BUTTON_CANCEL:
            SendMessage(hwnd,WM_CLOSE,0,0);
            return 1;
        case IDC_BUTTON_APPLY:
            ApplyConfig();
            return 1;
        default:
            break;
        }
    default :
        break;
    }
    return 0;
}
HWND CreateConfigWindow(HWND hwnd)
{
    return CreateDialog(NULL,MAKEINTRESOURCE(IDD_DIALOG_SETTINGS),NULL,ConfigWindowWndProc);
}
