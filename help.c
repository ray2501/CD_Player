#include "player.h"

/******************************************************
  * CreateStatusBar() - Create a status bar for our program.
  *
  * Input: Parent Window HWND and give the number of status bar nParts
  * Output: return created StatusBar HWND
  ******************************************************/
HWND CreateStatusBar(HWND hwnd, int nParts)
{
    HANDLE hInstance = (HANDLE) GetWindowLong(hwnd, GWL_HINSTANCE); 
    HWND hWndStatusBar;
    RECT rcClient;
    int *lpParts;
    int i, nWidth;
	
    hWndStatusBar = CreateWindowEx (0, STATUSCLASSNAME, NULL,
				WS_CHILD | WS_VISIBLE | CCS_BOTTOM | SBARS_SIZEGRIP ,
				0, 0, 0, 0, hwnd,
				(HMENU) ID_STATUSBAR,
				hInstance, NULL) ;
					
    // Get the coordinates of the parent window's client area.
    GetClientRect(hwnd, &rcClient);

    // Allocate an array for holding the right edge coordinates.
    lpParts = (int *)malloc(sizeof(int) * nParts);
	
    // Calculate the right edge coordinate for each part, and
    // copy the coordinates to the array.
    nWidth = rcClient.right / nParts;
    for (i = 0; i < nParts; ++i) { 
       lpParts[i] = nWidth;
       nWidth += nWidth;
    }

    // Tell the status bar to create the window parts.
    SendMessage(hWndStatusBar, SB_SETPARTS, (WPARAM) nParts, (LPARAM) lpParts);

    // Free the array then return
    if(lpParts) free(lpParts);
    return hWndStatusBar;
}
