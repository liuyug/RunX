#ifndef _WIN32X_H_
#define _WIN32X_H_

#define LEFT    0x0001
#define RIGHT   0x0002
#define BOTH    LEFT | RIGHT

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

bool _trace(const TCHAR *format, ...);
#define debug_output                _trace
#ifdef NDEBUG
#define debug                       ((void)0)
#define debug_log(msg)              ((void)0)
#define debug_error(x)              ((void)0)
#define debug_errmsg(desc,errmsg)   ((void)0)
#else
#define debug                       _trace
#define debug_log(msg)              _trace(_T("%s\n"),msg)
#define debug_error(desc)           _trace(_T("%s:%s\n"),__FUNCTION__,desc)
#define debug_errmsg(desc,errmsg)   _trace(_T("%s:%s - %s\n"),__FUNCTION__,desc,errmsg)
#endif


TCHAR *trim(TCHAR *str,int from,const TCHAR *chs);

SIZE GetFontSize(HFONT hFont);
TCHAR * GetWin32ErrorMessage(unsigned long errcode);
void ErrLook(HWND hWnd,const TCHAR *title,const TCHAR *cmdString);

void DisableScreenSaver(int disable);
int RequestPrivilege(const TCHAR *PRIVILEGE_NAME);

int strwildcardcmp(const TCHAR *str,const TCHAR *wildcard,int igcase);

int EjectDisk(const TCHAR * drive);

int GetFilesDialog(HWND hWnd,TCHAR * files, size_t length);
int GetDirectoryDialog(HWND hWnd,TCHAR * dir, size_t length);

void DisableScreenSaver(int disable);
int IsConsoleApp(const TCHAR *path);
void RunScreenSaver(HWND hWnd);
void ShutOffDisplay(HWND hWnd);

#endif // _WIN32X_H_
