#ifndef _QUICKEY_H_
#define _QUICKEY_H_


typedef void*    HQUICKEY;
int quickey_init();
int quickey_clearall();
int quickey_close();

int quickey_createsetting();
int quickey_putsetting(const TCHAR *key, const TCHAR *value);
int quickey_getsetting(const TCHAR *key,TCHAR value[MAX_BUFFER]);
int quickey_deletesetting(const TCHAR *key, const TCHAR *value);
int quickey_clearallsetting();


int quickey_putsettings();
int quickey_getsettings();
void quickey_addintercmd();

const TCHAR * quickey_genkey(const TCHAR *path,TCHAR key[MAX_BUFFER]);
int quickey_addfile(const TCHAR *path, const TCHAR *key = NULL);
int quickey_deletefile(const TCHAR *key);
int quickey_addfilesfrompath(const TCHAR *path,const TCHAR *filters,int recursive);
int quickey_listbox_updatepath(HWND hListBox);
int quickey_listbox_addpath(const TCHAR *path);
int quickey_listbox_deletepath(const TCHAR *path);

int quickey_getexec(const TCHAR *key,TCHAR path[][MAX_BUFFER]);


int quickey_listbox_search(const TCHAR *filter,HWND hListBox);
int quickey_listview_update(const TCHAR *filter,HWND hListView);

#endif // _QUICKEY_H_
