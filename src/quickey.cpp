
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include "global.h"
#include "runx.h"
#include "autocompletion.h"
#include "sqldb.h"
#include "quickey.h"
#include "utils_win32.h"


PVOID db;

int quickey_init()
{
    const TCHAR *keymap_check=_T("select * from keymap where rowid=1;");
    const TCHAR *keymap_create=_T(" \
create table keymap(        \
key  TEXT PRIMARY KEY ASC,  \
path TEXT,                  \
priority INTEGER            \
);");

    TCHAR dbname[MAX_BUFFER];

    //SHGetSpecialFolderPath(hwnd,path,CSIDL_LOCAL_APPDATA,0);
    GetModuleFileName(NULL,dbname,MAX_BUFFER);
    _tcscat_s(dbname,MAX_BUFFER,_T(".db"));
    db=sqldb_init(dbname);
    if(sqldb_exec(db,keymap_check,NULL,0,NULL,NULL)!=0) {
        sqldb_exec(db,keymap_create,NULL,0,NULL,NULL);
        quickey_createsetting();

        quickey_putsettings();
        quickey_addintercmd();
    }
    quickey_getsettings();
    return 0;
}
int quickey_createsetting()
{
    const TCHAR *setting_drop=_T("drop table setting;");
    const TCHAR *setting_create=_T(" \
create table setting(        \
key   TEXT,                  \
value TEXT                   \
);");
    sqldb_exec(db,setting_drop,NULL,0,NULL,NULL);
    return sqldb_exec(db,setting_create,NULL,0,NULL,NULL);
}
int quickey_clearallsetting()
{
    const TCHAR *setting_delete=_T("delete from setting;");
    return sqldb_exec(db,setting_delete,NULL,0,NULL,NULL);
}
int quickey_deletesetting(const TCHAR *key, const TCHAR *value)
{
    const TCHAR *sql_fmt=_T("delete from setting where key='%s' and value='%s';");
    TCHAR sql[MAX_BUFFER];
    _stprintf_s(sql,MAX_BUFFER,sql_fmt,key,value);
    return sqldb_exec(db,sql,NULL,0,NULL,NULL);
}

int quickey_getsetting(const TCHAR *key,TCHAR value[MAX_BUFFER])
{
    const TCHAR *sql_fmt=_T("select value from setting where key = '%s' ;");
    TCHAR sql[MAX_BUFFER];
    TCHAR sqlvalue[MAX_BUFFER];
    _stprintf_s(sql,MAX_BUFFER,sql_fmt,key);
    int sql_ret=sqldb_exec(db,sql,NULL,1,&sqlvalue,NULL);
    value[0]=_T('\0');
    if(sql_ret==0){
        _tcscpy_s(value,MAX_BUFFER, sqlvalue);
    }
    return sql_ret;
}

int quickey_putsetting(const TCHAR *key, const TCHAR *value)
{
    const TCHAR *sql_fmt_t=_T("insert into setting values('%s','%s');");
    TCHAR sql[MAX_BUFFER];
    _stprintf_s(sql,MAX_BUFFER,sql_fmt_t,key,value);
    return sqldb_exec(db,sql,NULL,0,NULL,NULL);
}

int quickey_getsettings()
{
    const TCHAR *sql_fmt=_T("select value from setting where key = '%s' ;");
    TCHAR sql[MAX_BUFFER];
    TCHAR value[MAX_BUFFER];

    _stprintf_s(sql,MAX_BUFFER,sql_fmt,_T("ExecAll"));
    sqldb_exec(db,sql,NULL,1,&value,NULL);
    ExecAll=_ttoi(value);
    _stprintf_s(sql,MAX_BUFFER,sql_fmt,_T("ExecConsoleApp"));
    sqldb_exec(db,sql,NULL,1,&value,NULL);
    ExecConsoleApp=_ttoi(value);
    _stprintf_s(sql,MAX_BUFFER,sql_fmt,_T("AutoStart"));
    sqldb_exec(db,sql,NULL,1,&value,NULL);
    AutoStart=_ttoi(value);
    _stprintf_s(sql,MAX_BUFFER,sql_fmt,_T("EnableDropdown"));
    sqldb_exec(db,sql,NULL,1,&value,NULL);
    EnableDropdown=_ttoi(value);
    _stprintf_s(sql,MAX_BUFFER,sql_fmt,_T("DropdownItem"));
    sqldb_exec(db,sql,NULL,1,&value,NULL);
    DropdownItem=_ttoi(value);
    _stprintf_s(sql,MAX_BUFFER,sql_fmt,_T("EnableCtrlPN"));
    sqldb_exec(db,sql,NULL,1,&value,NULL);
    EnableCtrlPN=_ttoi(value);
    _stprintf_s(sql,MAX_BUFFER,sql_fmt,_T("EnableCtrlKJ"));
    sqldb_exec(db,sql,NULL,1,&value,NULL);
    EnableCtrlKJ=_ttoi(value);
    _stprintf_s(sql,MAX_BUFFER,sql_fmt,_T("EnableUPDOWN"));
    sqldb_exec(db,sql,NULL,1,&value,NULL);
    EnableUPDOWN=_ttoi(value);
    _stprintf_s(sql,MAX_BUFFER,sql_fmt,_T("EnableTAB"));
    sqldb_exec(db,sql,NULL,1,&value,NULL);
    EnableTAB=_ttoi(value);
    _stprintf_s(sql,MAX_BUFFER,sql_fmt,_T("HK_Mod"));
    sqldb_exec(db,sql,NULL,1,&value,NULL);
    HK_Mod=_ttoi(value);
    _stprintf_s(sql,MAX_BUFFER,sql_fmt,_T("HK_Key"));
    sqldb_exec(db,sql,NULL,1,&value,NULL);
    HK_Key=_ttoi(value);
    return 0;
}
int quickey_putsettings()
{
    const TCHAR *insertfmt_i=_T("insert into setting values('%s',%d);");
    //const TCHAR *insertfmt_t=_T("insert into setting values('%s','%s');");
    TCHAR sql[MAX_BUFFER];
    _stprintf_s(sql,MAX_BUFFER,insertfmt_i,_T("ExecAll"),ExecAll);
    sqldb_exec(db,sql,NULL,0,NULL,NULL);
    _stprintf_s(sql,MAX_BUFFER,insertfmt_i,_T("ExecConsoleApp"),ExecConsoleApp);
    sqldb_exec(db,sql,NULL,0,NULL,NULL);
    _stprintf_s(sql,MAX_BUFFER,insertfmt_i,_T("AutoStart"),AutoStart);
    sqldb_exec(db,sql,NULL,0,NULL,NULL);
    _stprintf_s(sql,MAX_BUFFER,insertfmt_i,_T("EnableDropdown"),EnableDropdown);
    sqldb_exec(db,sql,NULL,0,NULL,NULL);
    _stprintf_s(sql,MAX_BUFFER,insertfmt_i,_T("DropdownItem"),DropdownItem);
    sqldb_exec(db,sql,NULL,0,NULL,NULL);
    _stprintf_s(sql,MAX_BUFFER,insertfmt_i,_T("EnableCtrlPN"),EnableCtrlPN);
    sqldb_exec(db,sql,NULL,0,NULL,NULL);
    _stprintf_s(sql,MAX_BUFFER,insertfmt_i,_T("EnableCtrlKJ"),EnableCtrlKJ);
    sqldb_exec(db,sql,NULL,0,NULL,NULL);
    _stprintf_s(sql,MAX_BUFFER,insertfmt_i,_T("EnableUPDOWN"),EnableUPDOWN);
    sqldb_exec(db,sql,NULL,0,NULL,NULL);
    _stprintf_s(sql,MAX_BUFFER,insertfmt_i,_T("EnableTAB"),EnableTAB);
    sqldb_exec(db,sql,NULL,0,NULL,NULL);
    _stprintf_s(sql,MAX_BUFFER,insertfmt_i,_T("HK_Mod"),HK_Mod);
    sqldb_exec(db,sql,NULL,0,NULL,NULL);
    _stprintf_s(sql,MAX_BUFFER,insertfmt_i,_T("HK_Key"),HK_Key);
    sqldb_exec(db,sql,NULL,0,NULL,NULL);
    return 0;
}

int quickey_clearall()
{
    const TCHAR *table_delete=_T("drop table keymap;");
    const TCHAR *table_create=_T(" \
create table keymap(        \
key  TEXT PRIMARY KEY ASC,  \
path TEXT,                  \
priority INTEGER            \
);");
    sqldb_exec(db,table_delete,NULL,0,NULL,NULL);
    sqldb_exec(db,table_create,NULL,0,NULL,NULL);
    return 0;
}
void quickey_addintercmd()
{
const TCHAR *InternalCmds[]= {
    _T("/about"),
    _T("/config"),
    _T("/quit"),
    _T("/lock"),
    _T("/logoff"),
    _T("/shutdown"),
    _T("/restart"),
    _T("/edithosts"),
    _T("/shutdisplay"),
    _T("/runscreensaver"),
    _T("/url <link>"),
    _T("/syserr <error code>"),
    _T("/runas <application>"),
    _T("/eject <disk>"),
    NULL
};
    int i=0;
    while(InternalCmds[i]!=NULL) {
        quickey_addfile(InternalCmds[i], NULL);
        i++;
    }
}
const TCHAR * quickey_genkey(const TCHAR *path,TCHAR key[MAX_BUFFER])
{
    TCHAR * ext;
    const TCHAR * basename;
    if(key[0]==_T('/')){
        _tcscpy_s(key,MAX_BUFFER,path);
        ext=_tcschr(key,_T(' '));
        if(ext!=NULL) *ext=_T('\0');
        return key;
    }

    basename=_tcsrchr(path,_T('\\'));
    if(basename==NULL){
        basename=path;
    }else{
        basename++;     // skip '\'
    }
    _tcscpy_s(key,MAX_BUFFER,basename);
//    ext=_tcsrchr(key,_T('"'));
//    if(ext!=NULL) {
//        *ext=_T('\0');
//    }
    ext=_tcsrchr(key,_T('.'));
    if(ext!=NULL) {
        *ext=_T('\0');
        ext++;
    }
    return key;
}

int quickey_addfile(const TCHAR *path,const TCHAR *key)
{
    const TCHAR *insertfmt=_T("insert into keymap values('%s','%s',%d);");
    TCHAR sql_insert[MAX_BUFFER];
    TCHAR key2[MAX_BUFFER];
    if(key==NULL) {
        quickey_genkey(path,key2);
    } else {
        _tcscpy_s(key2,MAX_BUFFER,key);
    }
    _stprintf_s(sql_insert,MAX_BUFFER,insertfmt,key2,path,100);
    sqldb_exec(db,sql_insert,NULL,0,NULL,NULL);
    return 0;
}
int quickey_deletefile(const TCHAR *key)
{
    const TCHAR *szSQLFormat=_T("delete from keymap where key = '%s';");
    TCHAR sql[MAX_BUFFER];
    _stprintf_s(sql,MAX_BUFFER,szSQLFormat,key);
    sqldb_exec(db,sql,NULL,0,NULL,NULL);
    return 0;

}
int quickey_filters(const TCHAR *path,const TCHAR *filters)
{
    const TCHAR * filter;
    filter=filters;
    while(*filter!=_T('\0')){
        if(strwildcardcmp(path,filter,1)==0)
            return 0;
        filter=filter+_tcslen(filter)+1;
    }
    return 1;
}
int quickey_addfilesfrompath(const TCHAR *path,const TCHAR *filters,int recursive)
{
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;
    TCHAR childpath[MAX_PATH];
    _stprintf_s(childpath,MAX_BUFFER,_T("%s\\*"),path);
    hFind = FindFirstFile(childpath, &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        debug_output(_T("%s"),childpath);
        debug_errmsg(_T("FindFirstFile"),GetWin32ErrorMessage(0));
        return 1;
    } else {
        do {
            if(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) {
                if(recursive>0&&
                        _tcscmp(FindFileData.cFileName,_T("."))!=0&&
                        _tcscmp(FindFileData.cFileName,_T(".."))!=0
                  ) {   // scan child path
                    _stprintf_s(childpath,MAX_BUFFER,_T("%s\\%s"),path,FindFileData.cFileName);
                    //debug_output(_T("+ %s\n"),childpath);
                    quickey_addfilesfrompath(childpath,filters,recursive);
                }
            } else if(quickey_filters(FindFileData.cFileName,filters)==0) {
                // muse use quote "", or sql statement will raise error
                _stprintf_s(childpath,MAX_BUFFER,_T("\"%s\\%s\""),path,FindFileData.cFileName);
                debug_output(_T("+ %s\n"),childpath);
                quickey_addfile(childpath);
            } else {
                debug_output(_T("- %s\\%s\n"),path,FindFileData.cFileName);
            }
        }while(FindNextFile(hFind, &FindFileData) != 0);
        if(ERROR_NO_MORE_FILES!=GetLastError()) {
            debug_errmsg(_T("FindNextFile"),GetWin32ErrorMessage(0));
        }
    }
    FindClose(hFind);
    return 0;
}


int quickey_listbox_addpath(const TCHAR *path)
{
    return quickey_putsetting(_T("search_path"),path);
}
int quickey_listbox_deletepath(const TCHAR *path)
{
    return quickey_deletesetting(_T("search_path"),path);
}
int CALLBACK quickey_listboxcallback(int cols,TCHAR record[][MAX_BUFFER],void * param)
{
    ListBox_AddString((HWND)param,record[0]);
    return 0;
}

int quickey_listbox_updatepath(HWND hListBox)
{
    const TCHAR *sql=_T("select value from setting where key='search_path'");
    TCHAR record[2][MAX_BUFFER];
    return sqldb_exec(db,sql,quickey_listboxcallback,2,record,hListBox);
    return 0;
}

int quickey_listbox_search(const TCHAR *filter,HWND hListBox)
{
    const TCHAR *sql_fmt=_T("select key from keymap where key like '%%%s%%' order by key limit 10;");
    TCHAR sql[MAX_BUFFER];
    TCHAR record[2][MAX_BUFFER];
    if(filter[0]==_T('\\')){    // no search with the begin of '\'
        debug_output(_T("no search : %s\n"),filter);
        return 0;
    }

    _stprintf_s(sql,MAX_BUFFER,sql_fmt,filter);

    sqldb_exec(db,sql,quickey_listboxcallback,2,record,hListBox);
    return 0;
}

int CALLBACK quickey_listviewcallback(int cols,TCHAR record[][MAX_BUFFER],PVOID param)
{
    LVITEM lvi;
    int index;
    memset(&lvi,0,sizeof(lvi));
    lvi.mask=LVIF_TEXT;
    lvi.iItem=ListView_GetItemCount((HWND)param)+1;
    lvi.pszText=record[0];
    lvi.cchTextMax=_tcslen(lvi.pszText);
    index=ListView_InsertItem((HWND)param,&lvi);
    ListView_SetItemText((HWND)param,index,1,record[1]);
//    ListView_SetItemText((HWND)param,index,2,record[2]);
    return 0;
}
int quickey_listview_update(const TCHAR *filter,HWND hListView)
{
    const TCHAR *sql_fmt=_T("select key,path,priority from keymap where key like '%s%%' order by key;");
    TCHAR sql[MAX_BUFFER];
    TCHAR record[3][MAX_BUFFER];
    _stprintf_s(sql,MAX_BUFFER,sql_fmt,filter);

    sqldb_exec(db,sql,quickey_listviewcallback,3,record,hListView);
    return 0;
}

int quickey_getexec(const TCHAR *key,TCHAR path[][MAX_BUFFER])
{
    const TCHAR *sql_fmt=_T("select path from keymap where key like '%s' ;");
    TCHAR sql[MAX_BUFFER];
    if(key[0]==_T('/') || key[0]==_T('\\')){    // no search with the begin of '\' or '/'
        _tcscpy_s(path[0],MAX_BUFFER,key);
        return 0;
    }

    _stprintf_s(sql,MAX_BUFFER,sql_fmt,key);
    path[0][0]=_T('\0');
    sqldb_exec(db,sql,NULL,1,path,NULL);
    debug_output(_T("quickey path:%s\n"),*path);
    return 0;
}
int quickey_close()
{
    sqldb_close(db);
    return 0;
}


