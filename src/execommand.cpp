
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <shlobj.h>

#include "global.h"
#include "runit.h"
#include "execommand.h"
#include "resource.h"
#include "informationbox.h"
#include "utils_win32.h"


void ExecWindowCommand(HWND hWnd,const TCHAR *op,const TCHAR *cmdString)
{
    TCHAR cmd[MAX_BUFFER],param[MAX_BUFFER];
    TCHAR *space,*quote;
    TCHAR message[MAX_BUFFER];
    _tcscpy_s(cmd,MAX_BUFFER,cmdString);
    trim(cmd,LEFT,_T(" \t"));
    // parse command with parameter
    param[0]=_T('\0');
    if(cmd[0]==_T('"')){    // with quote
        quote=_tcschr(cmd+1,_T('"'));
        quote++;    // keep '"'
        if(*quote==_T(' ')){    // split command with delimiter ' '
            *quote++=_T('\0');
            _tcscpy_s(param,MAX_BUFFER,quote);
        }
    }else if((space=_tcschr(cmd,_T(' ')))!=NULL){
        *space++=_T('\0');
        _tcscpy_s(param,MAX_BUFFER,space);
    }
    // console app
    if(ExecConsoleApp>0&&IsConsoleApp(cmd)>0){
        _stprintf_s(message,MAX_BUFFER,_T("/k \"%s\" %s"),cmd,param);
        _tcscpy_s(cmd,MAX_BUFFER,_T("cmd.exe"));
        _tcscpy_s(param,MAX_BUFFER,message);
    }
    debug_output(_T("SHEXEC:%s,%s,%s\n"),op,cmd,param);
    int ret=PtrToInt(ShellExecute(NULL,op,cmd,param,_T(""),SW_SHOWNORMAL));
    if(ret<=32){
        _stprintf_s(message,MAX_BUFFER,_T("%s\n\n%s"),cmdString,GetWin32ErrorMessage(0));
        MessageBox(hWnd,message,szTitle,MB_ICONERROR|MB_OK);
        //debug_errmsg(_T("ShellExecute"));
    }
}
void ExecAppCommand(HWND hWnd,const TCHAR *cmdString)
{
#define SHTDN_REASON_FLAG_PLANNED 0x80000000
    TCHAR shellcmd[MAX_BUFFER];
    if(_tcsicmp(cmdString,_T("/about"))==0){
        ShowAbout(NULL);
    }else if(_tcsicmp(cmdString,_T("/config"))==0){
        ShowConfig(NULL);
    }else if(_tcsnicmp(cmdString,_T("/syserr "),8)==0){
        ErrLook(NULL,szTitle,cmdString+8);
    }else if(_tcsicmp(cmdString,_T("/quit"))==0){
        DestroyWindow(hWnd);
    }else if(_tcsnicmp(cmdString,_T("/runas "),7)==0){
        ExecWindowCommand(NULL,_T("RunAs"),cmdString+7);
    }else if(_tcsicmp(cmdString,_T("/lock"))==0){
        LockWorkStation();
    }else if(_tcsicmp(cmdString,_T("/logoff"))==0){
        ExitWindowsEx(EWX_LOGOFF,SHTDN_REASON_FLAG_PLANNED);
    }else if(_tcsicmp(cmdString,_T("/shutdown"))==0){
        RequestPrivilege(SE_SHUTDOWN_NAME);
        ExitWindowsEx(EWX_POWEROFF|EWX_FORCE,SHTDN_REASON_FLAG_PLANNED);
    }else if(_tcsicmp(cmdString,_T("/restart"))==0){
        RequestPrivilege(SE_SHUTDOWN_NAME);
        ExitWindowsEx(EWX_REBOOT|EWX_FORCE,SHTDN_REASON_FLAG_PLANNED);
    }else if(_tcsnicmp(cmdString,_T("/eject "),7)==0){
        if(EjectDisk(cmdString+7)==0){
            _tcscpy_s(shellcmd,MAX_BUFFER,_T("Please remove your disk or cdrom: "));
            _tcscat_s(shellcmd,MAX_BUFFER,cmdString+7);
            MessageBox(hWnd,shellcmd,szTitle,MB_ICONINFORMATION|MB_OK);
        }else{
            _tcscpy_s(shellcmd,MAX_BUFFER,_T("Can't remove the disk or cdrom: "));
            _tcscat_s(shellcmd,MAX_BUFFER,cmdString+7);
            MessageBox(hWnd,shellcmd,szTitle,MB_ICONWARNING|MB_OK);
        }
    }else if(_tcsicmp(cmdString,_T("/edithosts"))==0){
        _tcscpy_s(shellcmd,MAX_BUFFER,_T("notepad "));
        SHGetSpecialFolderPath(NULL,shellcmd+_tcslen(shellcmd),CSIDL_SYSTEM,0);
        _tcscat_s(shellcmd,MAX_BUFFER,_T("\\drivers\\etc\\hosts"));
        ExecWindowCommand(NULL,_T("RunAs"),shellcmd);
    }else if(_tcsnicmp(cmdString,_T("/url "),5)==0){
        ExecWindowCommand(NULL,_T("open"),cmdString+5);
    }else if(_tcsicmp(cmdString,_T("/shutdisplay"))==0){
        ShutOffDisplay(hWnd);
    }else if(_tcsicmp(cmdString,_T("/runscreensaver"))==0){
        RunScreenSaver(hWnd);
    }else{
        _tcscpy_s(shellcmd,MAX_BUFFER,_T("Can't find command: "));
        _tcscat_s(shellcmd,MAX_BUFFER,cmdString);
        MessageBox(hWnd,shellcmd,szTitle,MB_ICONWARNING|MB_OK);
    }

}

void ExecNetShare(HWND hWnd,const TCHAR *cmdString)
{
    TCHAR szNetShare[MAX_BUFFER];
    _tcscpy_s(szNetShare,MAX_BUFFER,cmdString);
    NETRESOURCE ns;
    memset(&ns,0,sizeof(ns));
    ns.dwType=RESOURCETYPE_ANY;
    ns.lpRemoteName=szNetShare;
    if(WNetAddConnection2(&ns,NULL,NULL,CONNECT_INTERACTIVE)!=NO_ERROR) {
        debug_errmsg(_T("WNetAddConnection2"),GetWin32ErrorMessage(0));
        _stprintf_s(szNetShare,MAX_BUFFER,_T("%s\n\n%s"),cmdString,GetWin32ErrorMessage(0));
        MessageBox(hWnd,szNetShare,szTitle,MB_ICONERROR|MB_OK);
        return;
    };

    int ret=PtrToInt(ShellExecute(NULL,_T("open"),cmdString,_T(""),_T(""),SW_SHOWNORMAL));
    if(ret<=32){
        _stprintf_s(szNetShare,MAX_BUFFER,_T("%s\n\n%s"),cmdString,GetWin32ErrorMessage(0));
        MessageBox(hWnd,szNetShare,szTitle,MB_ICONERROR|MB_OK);
    }
}
void ExecCommand(HWND hWnd,const TCHAR *cmdString,int admin)
{
    if(cmdString[0]==_T('\0')){
//        MessageBox(hWnd,_T("Can't find file: nil"),szTitle,MB_ICONWARNING|MB_OK);
    }else if(cmdString[0]==_T('/')){    // application internal command
        return ExecAppCommand(hWnd,cmdString);
    }else if(cmdString[0]==_T('\\')){    // \\x.x.x.x\share
        return ExecNetShare(hWnd,cmdString);
    }else{
        if(admin>0){
            return ExecWindowCommand(hWnd,_T("RunAs"),cmdString);
        }else{
            return ExecWindowCommand(hWnd,_T("open"),cmdString);
        }
    }
}
