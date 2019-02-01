#ifndef _RUNIT_H_
#define _RUNIT_H_

extern HWND hInput;
extern HWND hAutoCompletion;
extern HFONT hRunitFont;
extern int  ExecAll;
extern int  ExecConsoleApp;
extern int  AutoStart;
extern int  EnableDropdown;
extern int  DropdownItem;
extern int  EnableCtrlPN;
extern int  EnableCtrlKJ;
extern int  EnableUPDOWN;
extern int  EnableTAB;
extern unsigned int  HK_Mod;
extern unsigned int  HK_Key;
extern ATOM ID_HOTKEY;
extern TCHAR szTitle[256];

#define fontSize	32

void ShowAbout(HWND hWnd);
void ShowHelp(HWND hWnd);
void ShowConfig(HWND hWnd);

inline int max ( int a, int b ) { return (a) > (b) ? (a) : (b); };
inline int min ( int a, int b ) { return (a) < (b) ? (a) : (b); };

#endif // _RUNIT_H_
