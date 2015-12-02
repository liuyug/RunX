
#include <windows.h>

#include "runit.h"
#include "resource.h"
#include "informationbox.h"

INT_PTR CALLBACK InformationBoxProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    const TCHAR *message;
    switch(uMsg) {
    case WM_INITDIALOG:
        message=(const TCHAR*)lParam;
        SetDlgItemText(hwndDlg,IDC_EDIT_INFORMATION,message);
        SendMessage(GetDlgItem(hwndDlg,IDC_EDIT_INFORMATION),WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),0);
        SetWindowText(hwndDlg,szTitle);
        return 1;
    case WM_CLOSE:
        EndDialog(hwndDlg, 0);
        return 1;
    case WM_COMMAND:
        switch(LOWORD(wParam)) {
        case IDOK:
            EndDialog(hwndDlg, 0);
            return 0;

        }
    }
    return 0;
}


int InformationBox(HWND hWnd,const TCHAR * message)
{
    return (int)DialogBoxParam(NULL,MAKEINTRESOURCE(IDD_DIALOG_INFORMATION),
                               hWnd,InformationBoxProc,
                               (LPARAM)message);
}
