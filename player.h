#include <tchar.h>
#include <windows.h>
#include <windowsX.h>
#include <commctrl.h>

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL OnCreate(HWND hwnd, LPCREATESTRUCT pCreateStruct);
void OnCommand(HWND hwnd, UINT id, HWND hwndCtl, UINT codeNotify);
void OnHScroll(HWND hwnd, HWND hwndCtl, UINT code, UINT pos);
void OnSize(HWND hwnd, UINT state, int cx, int cy);
void OnTimer(HWND hwnd, UINT id);
void OnClose(HWND hwnd);
void OnDestroy(HWND hwnd);
HWND CreateStatusBar(HWND hwnd, int nParts);

#define CD_NOCD       -1
#define CD_INIT           0
#define CD_PLAY         1
#define CD_PAUSE       2
#define CD_STOP          3

#define MY_TIMER     100
#define CD_TIMER      101
#define IDR_TOOLBAR1 107

#define ID_TOOLBAR   201
#define ID_TRACKBAR  202
#define ID_LISTBOX   203
#define ID_STATUSBAR 204

#define IDM_OPEN         1000
#define IDM_CLOSE	     1001
#define IDM_EJECT	     1002
#define IDM_EXIT          1004
#define IDM_VOLUME   1010
#define IDM_ABOUT      1020

#define IDM_BUTTON1  2001
#define IDM_BUTTON2  2002
#define IDM_BUTTON3  2003
#define IDM_BUTTON4  2004
#define IDM_BUTTON5  2005
