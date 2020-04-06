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

#if (defined(__MINGW__) || defined(__MINGW32__))
#include <wchar.h>
static inline int wcscpy_s(wchar_t *dest, size_t n, const wchar_t *src)
{
    int len;
    len = wcslen(src) + 1;
    if (len < n) {
        wcscpy(dest, src);
        return 0;
    }
    return 1;
}

static inline int wcscat_s(wchar_t *dest, size_t n, const wchar_t *src)
{
    int len;
    len = wcslen(src) + 1;
    if (len < n) {
        wcscat(dest, src);
        return 0;
    }
    return 1;
}

#define _tcscpy_s wcscpy_s
#define _tcscat_s wcscat_s
#define _stprintf_s swprintf
#define _vstprintf_s vswprintf
#define _tcscmp wcscmp

bool _trace(const TCHAR *format, ...);
#define debug                       _trace
#define debug_output                _trace
#define debug_log(msg)              _trace(_T("%s\n"),msg)

bool _error(const TCHAR *format, ...);
#define debug_error                 _error
#define debug_errmsg(desc,errmsg)   _error(_T("ERROR: %s - %s\n"), desc,errmsg)
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
