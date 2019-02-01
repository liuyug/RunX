
#include <tchar.h>
#include <windows.h>
#include <shlobj.h>
#include <initguid.h>

#include <setupapi.h>
#include <cfgmgr32.h>

#include "global.h"
#include "utils_win32.h"


TCHAR ERR_MESSAGE[MAX_BUFFER];

TCHAR *trim(TCHAR *str,int from,const TCHAR *chs)
{
    TCHAR *chstr;
    int len=_tcslen(str);
    if(len==0)   return str;
    chstr=str+len-1;
    if(from&RIGHT) {
        while(chstr>=str&&_tcsrchr(chs,*chstr)!=NULL) chstr--;
        *(chstr+1)=_T('\0');
    }
    chstr=str;
    if(from&LEFT) {
        while(*chstr!=_T('\0')&&_tcsrchr(chs,*chstr)!=NULL) chstr++;
        if(str!=chstr) {
            _tcscpy_s(str,sizeof(*str),chstr);
        }
    }
    return str;
}

TCHAR * GetWin32ErrorMessage(unsigned long errcode)
{
    if(errcode==0) errcode=GetLastError();
    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM ,
        NULL,
        errcode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        ERR_MESSAGE,
        MAX_BUFFER,
        NULL
    );
    return ERR_MESSAGE;
}

void ErrLook(HWND hWnd,const TCHAR *title,const TCHAR *cmdString)
{
    TCHAR message[MAX_BUFFER];
    _stprintf_s(message,_T("Error code: %s\n\n%s"),cmdString,GetWin32ErrorMessage(_ttol(cmdString)));
    MessageBox(hWnd,message,title,MB_ICONINFORMATION|MB_OK);
}

/*
 *  Use debugview to check output message.
 */
bool _trace(const TCHAR *format, ...)
{
   TCHAR buffer[MAX_BUFFER];
   va_list argptr;
   va_start(argptr, format);
   _vstprintf_s(buffer, format, argptr);
   va_end(argptr);
   OutputDebugString(buffer);
   return true;
}

SIZE GetFontSize(HFONT hFont)
{
    TCHAR sample[]=_T("abcdefg");
    SIZE  fsize;
    HDC hdc;
    hdc=CreateCompatibleDC(NULL);
    if(hFont==NULL) {
        hFont=(HFONT)GetStockObject(DEFAULT_GUI_FONT);
    }
    SelectObject(hdc,hFont);
    GetTextExtentPoint32(hdc,sample,_tcslen(sample),&fsize);

    DeleteDC(hdc);
    return fsize;
}

int SetWindowTransparent(HWND hWnd,unsigned char nAlpha)
{
    SetWindowLong(hWnd,GWL_EXSTYLE,GetWindowLong(hWnd,GWL_EXSTYLE)|WS_EX_LAYERED);
    return SetLayeredWindowAttributes(hWnd,0,nAlpha,LWA_ALPHA);
}

int NoTaskBarWindow(HWND hWnd)
{
    SetWindowLong(hWnd,GWL_EXSTYLE,GetWindowLong(hWnd,GWL_EXSTYLE)|WS_EX_TOOLWINDOW);
    return 0;
}

void DisableScreenSaver(int disable)
{
    if(disable>0) {
        SystemParametersInfo(SPI_SETSCREENSAVEACTIVE,FALSE,NULL,SPIF_SENDCHANGE);
    } else {
        SystemParametersInfo(SPI_SETSCREENSAVEACTIVE,TRUE,NULL,SPIF_SENDCHANGE);
    }
}

int EnablePriviledge(HANDLE hToken,TOKEN_PRIVILEGES & TokenPriv,const TCHAR *PRIVILEGE_NAME)
{
    if (!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES |TOKEN_QUERY, &hToken)) {
        return 1;
    }
    ZeroMemory(&TokenPriv, sizeof(TOKEN_PRIVILEGES));
    if (!LookupPrivilegeValue(NULL, PRIVILEGE_NAME, &TokenPriv.Privileges[0].Luid)) {
        return 2;
    }
    TokenPriv.PrivilegeCount = 1;
    TokenPriv.Privileges[0].Attributes |= SE_PRIVILEGE_ENABLED;
    if(!AdjustTokenPrivileges(hToken, false, &TokenPriv, 0, NULL, 0)) {
        return 3;
    }
    return 0;
}
int RequestPrivilege(const TCHAR *PRIVILEGE_NAME)
{
    HANDLE           hToken=NULL;
    TOKEN_PRIVILEGES TokenPriv;
    return EnablePriviledge(hToken,TokenPriv,PRIVILEGE_NAME);
}
void RevertPriviledge(HANDLE hToken,TOKEN_PRIVILEGES & TokenPriv)
{
    TokenPriv.Privileges[0].Attributes &= (~SE_PRIVILEGE_ENABLED);
    AdjustTokenPrivileges(hToken, false, &TokenPriv, 0, NULL, 0);
}
/** \brief compare two characters
 *
 * \param ch1 the first char
 * \param ch2 the second char
 * \param igcase ignore case-sensitive
 * \return equal return 1, otherwise 0
 *
 */

int chrcmp(TCHAR ch1,TCHAR ch2,int igcase)
{
    if(igcase>0) {
        if(ch1<_T('[')) ch1+=32;    // make a lower characters
        if(ch2<_T('[')) ch2+=32;    // same above
    }
    return ch1==ch2;
}
/** \brief Verrify string with wildcard characters
 *
 * \param  str input string
 * \param  wildcard * ?
 * \param  igcase ignore case-sensitive
 * \return 0 is ok, 1 is failed
 *
 */
int strwildcardcmp(const TCHAR *str,const TCHAR *wildcard,int igcase)
{
    const TCHAR *w=wildcard;
    const TCHAR *s=str;
    const TCHAR *lastchar;
    while(*w!=_T('\0')) {
        if(*w==_T('*')) {
            w++;
            if(*w==_T('\0'))    return 0;   // end with * , match always success.
#if 0
            // only match first, should use below code
            while(!chrcmp(*w,*s,igcase)&&*s!=_T('\0'))   s++;
            if(*s==_T('\0'))    return 1;   // don't find *w in string
            s++;
#else
            lastchar=w;
            while(1) {
                if(chrcmp(*lastchar,*s,igcase)) {   // match after *
                    if(*lastchar==_T('\0'))  return 0; // match till end, success
                    lastchar++;
                    s++;
                } else if(*lastchar==_T('\0')) { // match one char
                    s=s-(lastchar-w)+1;
                    lastchar=w;
                } else if(*s==_T('\0')) {   // don't match and reach end
                    return 1;
                } else if(*lastchar==_T('?')||*lastchar==_T('*')) { // match to next
                    w=lastchar-1;   // add 1 below
                    break;
                } else if(w==lastchar) { // still match *
                    s++;
                } else { // don't match char after *
                    s=s-(lastchar-w)+1;
                    lastchar=w;
                }
            }
#endif
        } else if (*w==_T('?')) {    // match one characters
            s++;
        } else if (chrcmp(*w,*s,igcase)) { // one by one compare
            s++;
        } else { // don't match, return
            return 1;
        }
        w++;    // next characters
    }
    if(*s==_T('\0'))    return 0;
    return 1;
}

int LoadMedia(const TCHAR * drive)
{
    TCHAR  VolumeName[] = _T("\\\\.\\?:");
    VolumeName[4]=drive[0];
    HANDLE hDrv = CreateFile(VolumeName,
                             GENERIC_READ|GENERIC_WRITE,
                             FILE_SHARE_READ | // share mode
                             FILE_SHARE_WRITE,
                             NULL,             // default security attributes
                             OPEN_EXISTING,    // disposition
                             0,                // file attributes
                             NULL);

    if(hDrv==INVALID_HANDLE_VALUE) {
        debug_errmsg(VolumeName,GetWin32ErrorMessage(0));
        return 1;
    }
    DWORD count;
    int Result;

    Result=DeviceIoControl(hDrv,IOCTL_STORAGE_LOAD_MEDIA, NULL,0,NULL,0,&count, NULL);
    if (!Result) {
        debug_errmsg(VolumeName,GetWin32ErrorMessage(0));
        return 3;
    }
#if 0
    Result=DeviceIoControl(hDrv,FSCTL_UNLOCK_VOLUME , NULL,0,NULL,0,&count, NULL);
    if (!Result) {
        debug_errmsg(VolumeName);
        return 5;
    }
#endif
    CloseHandle(hDrv);
    return 0;
}
int EjectMedia(const TCHAR * drive)
{
    TCHAR  VolumeName[] = _T("\\\\.\\?:");
    VolumeName[4]=drive[0];
    HANDLE hDrv = CreateFile(VolumeName,
                             GENERIC_READ|GENERIC_WRITE,
                             FILE_SHARE_READ | // share mode
                             FILE_SHARE_WRITE,
                             NULL,             // default security attributes
                             OPEN_EXISTING,    // disposition
                             0,                // file attributes
                             NULL);

    if(hDrv==INVALID_HANDLE_VALUE) {
        debug_errmsg(VolumeName,GetWin32ErrorMessage(0));
        return 1;
    }
    DWORD count;
    int  Result;
    Result=DeviceIoControl(hDrv,FSCTL_LOCK_VOLUME, NULL,0,NULL,0,&count, NULL);
    if (!Result) {
        debug_errmsg(VolumeName,GetWin32ErrorMessage(0));
        return 2;
    }
    Result=DeviceIoControl(hDrv,FSCTL_DISMOUNT_VOLUME, NULL,0,NULL,0,&count, NULL);
    if (!Result) {
        debug_errmsg(VolumeName,GetWin32ErrorMessage(0));
        return 3;
    }
    // from winioctl.h
    typedef struct _PREVENT_MEDIA_REMOVAL {
        BOOLEAN PreventMediaRemoval;
    } PREVENT_MEDIA_REMOVAL,*PPREVENT_MEDIA_REMOVAL;

    PREVENT_MEDIA_REMOVAL prevent;
    prevent.PreventMediaRemoval=false;
    Result=DeviceIoControl(hDrv,IOCTL_STORAGE_MEDIA_REMOVAL,&prevent,sizeof(prevent),NULL,0,&count, NULL);
    if (!Result) {
        debug_errmsg(VolumeName,GetWin32ErrorMessage(0));
    }
    // eject media, such as eject cdrom
    Result=DeviceIoControl(hDrv,IOCTL_STORAGE_EJECT_MEDIA, NULL,0,NULL,0,&count, NULL);
    if (!Result) {
        debug_errmsg(VolumeName,GetWin32ErrorMessage(0));
    }

#if 0
    bResult=DeviceIoControl(hDrv,FSCTL_UNLOCK_VOLUME , NULL,0,NULL,0,&count, NULL);
    if (!bResult) {
        debug_errmsg(VolumeName,GetWin32ErrorMessage(0));
        return 5;
    }
#endif
    CloseHandle(hDrv);
    return 0;
}

DEVINST GetDrivesDevInst(const TCHAR *drive)
{
    TCHAR VolumeName[]=_T("?:");
    TCHAR DosDevicePath[MAX_PATH];
    VolumeName[0]=drive[0];
    QueryDosDevice(VolumeName,DosDevicePath,MAX_PATH);

    GUID* guid;
    // GUID_DEVINTERFACE_CDROM
    if (_tcsstr(DosDevicePath,_T("\\Floppy")) != NULL) {
        guid = (GUID*)&GUID_DEVINTERFACE_FLOPPY;
    } else {
        guid = (GUID*)&GUID_DEVINTERFACE_DISK;
    }

    TCHAR DevicePath[] = _T("\\\\.\\?:");
    DevicePath[4]=drive[0];
    STORAGE_DEVICE_NUMBER DriveDeviceNumber;
    HANDLE hDrive;
    hDrive= CreateFile(DevicePath,
                       GENERIC_READ,                // Read only to the drive
                       FILE_SHARE_READ | // share mode
                       FILE_SHARE_WRITE,
                       NULL,             // default security attributes
                       OPEN_EXISTING,    // disposition
                       0,                // file attributes
                       NULL);
    if ( hDrive==INVALID_HANDLE_VALUE ) {
        debug_errmsg(_T("CreateFile"),GetWin32ErrorMessage(0));
        return 0;
    }
    // get its device number

    DWORD dwBytesReturned = 0;
    if (!DeviceIoControl(hDrive, IOCTL_STORAGE_GET_DEVICE_NUMBER,
                         NULL, 0, &DriveDeviceNumber, sizeof(DriveDeviceNumber), &dwBytesReturned, NULL) ) {
        debug_errmsg(_T("DeviceIoControl"),GetWin32ErrorMessage(0));
        return 0;
    }
    CloseHandle(hDrive);

    // Get device interface info set handle for all devices attached to system
    HDEVINFO hDevInfo = SetupDiGetClassDevs(guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (hDevInfo == INVALID_HANDLE_VALUE)	{
        return 0;
    }

    // Retrieve a context structure for a device interface of a device information set
    DWORD dwIndex = 0;

    BYTE Buf[1024];
    PSP_DEVICE_INTERFACE_DETAIL_DATA pspdidd = (PSP_DEVICE_INTERFACE_DETAIL_DATA)Buf;
    SP_DEVICE_INTERFACE_DATA         spdid;
    SP_DEVINFO_DATA                  spdd;
    DWORD                            dwSize;

    spdid.cbSize = sizeof(spdid);

    while (SetupDiEnumDeviceInterfaces(hDevInfo, NULL, guid, dwIndex, &spdid))	{
        dwSize = 0;
        SetupDiGetDeviceInterfaceDetail(hDevInfo, &spdid, NULL, 0, &dwSize, NULL); // check the buffer size

        if ( dwSize!=0 && dwSize<=sizeof(Buf) ) {

            pspdidd->cbSize = sizeof(*pspdidd); // 5 Bytes!

            ZeroMemory(&spdd, sizeof(spdd));
            spdd.cbSize = sizeof(spdd);

            if ( SetupDiGetDeviceInterfaceDetail(hDevInfo, &spdid, pspdidd, dwSize, &dwSize, &spdd) ) {

                // in case you are interested in the USB serial number:
                // the device id string contains the serial number if the device has one,
                // otherwise a generated id that contains the '&' char...
#if 0
                DEVINST DevInstParent = 0;
                CM_Get_Parent(&DevInstParent, spdd.DevInst, 0);
                TCHAR szDeviceIdString[MAX_PATH];
                CM_Get_Device_ID(DevInstParent, szDeviceIdString, MAX_PATH, 0);
                debug_output(_T("DeviceId=%s\n"), szDeviceIdString);
#endif
                // open the disk or cdrom or floppy
                hDrive = CreateFile(pspdidd->DevicePath, 0,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    NULL, OPEN_EXISTING, 0, NULL);
                if ( hDrive != INVALID_HANDLE_VALUE ) {
                    // get its device number
                    STORAGE_DEVICE_NUMBER sdn;
                    if ( DeviceIoControl(hDrive, IOCTL_STORAGE_GET_DEVICE_NUMBER,
                                         NULL, 0, &sdn, sizeof(sdn), &dwBytesReturned, NULL) ) {
                        if ( DriveDeviceNumber.DeviceNumber == sdn.DeviceNumber ) {  // match the given device number with the one of the current device
                            CloseHandle(hDrive);
                            SetupDiDestroyDeviceInfoList(hDevInfo);
                            return spdd.DevInst;
                        }
                    }
                    CloseHandle(hDrive);
                }
            }
        }
        dwIndex++;
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);

    return 0;
}
int EjectStorage(const TCHAR * drive)
{
    // get the device instance handle of the storage volume by means of a SetupDi enum and matching the device number
    DEVINST DevInst = GetDrivesDevInst(drive);
    if ( DevInst == 0 ) {
        return 1;
    }
    // get drives's parent, e.g. the USB bridge, the SATA port, an IDE channel with two drives!
    DEVINST DevInstParent = 0;
    if(CM_Get_Parent(&DevInstParent, DevInst, 0)!=CR_SUCCESS) {
        debug_errmsg(_T("CM_Get_Parent"),GetWin32ErrorMessage(0));
        return 2;
    }
    PNP_VETO_TYPE VetoType = PNP_VetoTypeUnknown;
    TCHAR VetoName[MAX_PATH];
    if(CM_Request_Device_Eject(DevInstParent, &VetoType, VetoName, MAX_PATH, 0)!=CR_SUCCESS||
            VetoType!=PNP_VetoTypeUnknown
      ) {
        debug_errmsg(_T("CM_Request_Device_Eject"),GetWin32ErrorMessage(0));
        return 3;
    }
    return 0;
}

/** \brief eject disk.
 *
 * \param drive disk drive name, such as "u:" or "u"
 * \return  0 if success, 1 if failed
 *
 */

int EjectDisk(const TCHAR * drive)
{
    TCHAR RootPathName[]=_T("?:\\");
    int   nDriveType;
    RootPathName[0]=drive[0];
    nDriveType=GetDriveType(RootPathName);
    if(nDriveType==DRIVE_CDROM) {
        return EjectMedia(drive);
    } else if(nDriveType==DRIVE_REMOVABLE) {
        return EjectStorage(drive);
    }
    return 1;
}

int GetFilesDialog(HWND hWnd,TCHAR *files, size_t length)
{
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = files;
    // Set lpstrFile[0] to '\0' so that GetOpenFileName does not
    // use the contents of szFile to initialize itself.
    ofn.lpstrFile[0] = _T('\0');
	// don't get array size??
    // ofn.nMaxFile = sizeof(files);
	ofn.nMaxFile = length;
    ofn.lpstrFilter = _T("All files (*.*)\0*.*\0Exe files (*.exe)\0*.exe\0\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT |OFN_EXPLORER ;


    // Display the Open dialog box.
    if (GetOpenFileName(&ofn)==TRUE) {
        return 0;
    }
    files[0]=_T('\0');
    files[1]=_T('\0');
    return 1;
}
int GetDirectoryDialog(HWND hWnd,TCHAR *dir, size_t length)
{
    BROWSEINFO bi;
    memset(&bi,0,sizeof(bi));
    bi.hwndOwner=hWnd;
    bi.pszDisplayName=dir;
    bi.ulFlags=BIF_BROWSEFORCOMPUTER|BIF_DONTGOBELOWDOMAIN|BIF_RETURNONLYFSDIRS;
    ITEMIDLIST *idList;
    idList=SHBrowseForFolder(&bi);
    if(idList>0){
		TCHAR path[MAX_PATH];
        SHGetPathFromIDList(idList, path);
        CoTaskMemFree(idList);
		_tcscpy_s(dir, length, path);
        return 0;
    }
    dir[0]=_T('\0');
    return 1;
}

int IsConsoleApp(const TCHAR *path)
{
    IMAGE_DOS_HEADER dos_header;
    IMAGE_NT_HEADERS nt_header;
    unsigned long    readbyte;
    int ret=0;

    HANDLE hFile=CreateFile(path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if(hFile==INVALID_HANDLE_VALUE){
        debug_error(path);
        debug_errmsg(_T("CreateFile"),GetWin32ErrorMessage(0));
        return -1;
    }
    if(!ReadFile(hFile,&dos_header,sizeof(dos_header),&readbyte,NULL)){
        debug_errmsg(_T("ReadFile"),GetWin32ErrorMessage(0));
        CloseHandle(hFile);
        return -1;
    }
    if(readbyte==0) return -2;
    SetFilePointer(hFile,dos_header.e_lfanew,NULL,FILE_BEGIN);
    if(!ReadFile(hFile,&nt_header,sizeof(nt_header),&readbyte,NULL)){
        debug_errmsg(_T("ReadFile"),GetWin32ErrorMessage(0));
        CloseHandle(hFile);
        return -3;
    }
    if(readbyte==0) {
        CloseHandle(hFile);
        return -4;
    }
    if(memcmp(&nt_header.Signature,"PE\0\0",4)!=0){
        CloseHandle(hFile);
        return -5;
    }
    if(nt_header.OptionalHeader.Subsystem==IMAGE_SUBSYSTEM_WINDOWS_CUI){
        ret=1;
    }
    CloseHandle(hFile);
    return ret;
}

void RunScreenSaver(HWND hWnd)
{
    SendMessage(hWnd, WM_SYSCOMMAND, SC_SCREENSAVE, 0);
}
void ShutOffDisplay(HWND hWnd)
{
    SendMessage(hWnd, WM_SYSCOMMAND, SC_MONITORPOWER, 2);
    LockWorkStation();
}
