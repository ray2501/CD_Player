/*!
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY.
 *
 * Everyone is permitted to copy, modify and distribute verbatim copies of this program and its source code.
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU Library General
 * Public Licence as published by the Free Software Foundation; either version 3 of the Licence, or (at your option)
 * any later version.
 *
 * Copyright (c) 2007 Danny Raynor
 */
#include "player.h"
#include "misc.h"

const TCHAR *szAppName = _T("CDWndClass");
HINSTANCE hInst;
static HWND hTool;
static HWND hTrack;
static HWND hList;
static HWND hStatus;
int playMode = CD_NOCD;  // Use this to identify play mode -> No CD, Initial, Play, Pause and Stop
int trackNumbers = 0; // Record track numbers
int currentTrack = 0; //Record current track
int trackSeconds = 0; // Record track length
int currentSeconds = 0; // Record current play time

// Define Toolbar Buttion
TBBUTTON tbb[] = {
	{0, IDM_BUTTON1, TBSTATE_ENABLED, BTNS_BUTTON},
	{1, IDM_BUTTON2, TBSTATE_ENABLED, BTNS_BUTTON},
	{2, IDM_BUTTON3, TBSTATE_ENABLED, BTNS_BUTTON},
	{3, IDM_BUTTON4, TBSTATE_ENABLED, BTNS_BUTTON},
	{4, IDM_BUTTON5, TBSTATE_ENABLED, BTNS_BUTTON}
};

/******************************************************
 * WinMain() - Main function, proram main entry
 ******************************************************/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wc;
	HWND hwnd;
	MSG Msg;

	hInst = hInstance; 

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, _T("MYICON"));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName = _T("MYMENU");
	wc.lpszClassName = szAppName;
	wc.hIconSm = LoadIcon(hInstance, _T("MYICON"));

	if(!RegisterClassEx(&wc))
	{
		MessageBox(NULL, _T("Register Window Fail!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, szAppName, _T("CD Player"), WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, 315, 435, NULL, NULL, hInstance, NULL);

	if(hwnd == NULL)
	{
		MessageBox(NULL, _T("Create Window Fail!"), _T("Error!"),
				MB_ICONEXCLAMATION | MB_OK);

		return 0;
	}

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while(GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return Msg.wParam;
}

/******************************************************
 * WndProc() - Windows Message Procedure.  
 ******************************************************/
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
		HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
		HANDLE_MSG(hwnd, WM_HSCROLL, OnHScroll);
		HANDLE_MSG(hwnd, WM_SIZE, OnSize);
		HANDLE_MSG(hwnd, WM_TIMER, OnTimer);
		HANDLE_MSG(hwnd, WM_CLOSE, OnClose);
		HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy); 
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

/******************************************************
 * OnCreate() - Handle WM_CREATE message.  
 *
 * This function initial sub windows, and set playMode = -1 (no-CD)
 ******************************************************/
BOOL OnCreate(HWND hwnd, LPCREATESTRUCT pCreateStruct)
{
	INITCOMMONCONTROLSEX cc;
	cc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	cc.dwICC = ICC_BAR_CLASSES;	

	InitCommonControlsEx(&cc);

	// Create our Toolbar
	hTool = CreateToolbarEx(
			hwnd, WS_CHILD | WS_VISIBLE, ID_TOOLBAR,
			5, hInst, IDR_TOOLBAR1,tbb, 5, 32, 32, 32, 32,
			sizeof(TBBUTTON));

	// Create our Trackbar
	hTrack = CreateWindowEx( 
			0, TRACKBAR_CLASS, "Time",
			WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_BOTTOM | TBS_NOTICKS | TBS_ENABLESELRANGE,
			0, 45, 304, 40, hwnd, (HMENU) ID_TRACKBAR, hInst, NULL); 	

	// Create our Listbox
	hList = CreateWindowEx( 
			0,  _T("listbox"),  _T("TrackList"),
			WS_CHILD | WS_VISIBLE | LBS_STANDARD,  // style 
			0, 85, 304, 275, hwnd, (HMENU) ID_LISTBOX, hInst, NULL);  

	// Create our Status bar
	hStatus = CreateStatusBar(hwnd, 2);

	// Set Timer
	SetTimer(hwnd, MY_TIMER, 1000, NULL);  

	// Set playMode in error (No CD) mode
	playMode = CD_NOCD;

	return TRUE;
}

/******************************************************
 * OnCommand() - Handle WM_COMMAND message.  
 ******************************************************/
void OnCommand(HWND hwnd, UINT id, HWND hwndCtl, UINT codeNotify)
{
	int count;
	int tracks;
	MCIERROR error;
	TCHAR szTrack[64];
	TCHAR szReturn[1024];	  
	TCHAR szTracks[1024];
	TCHAR szCommand[1024];  
	TCHAR szSeconds[256];

	switch(codeNotify) {
		case LBN_SELCHANGE:
			count = (int)SendMessage(hList, LB_GETCURSEL, 0, 0); 	
			currentTrack = count + 1; //Because Listbox is zero-based
			currentSeconds = 0;	

			wsprintf(szCommand, "status cdaudio length track %d", currentTrack);			
			error = mciSendString(szCommand, szReturn, sizeof(szReturn)/sizeof(TCHAR), hwnd);		
			trackSeconds = timestring2Int(szReturn);

			SendMessage(hTrack, TBM_SETRANGE,  (WPARAM) TRUE, (LPARAM) MAKELONG(0, trackSeconds)); 
			SendMessage(hTrack, TBM_SETPOS,  (WPARAM) TRUE,  (LPARAM) currentSeconds);		

			if(playMode==CD_PLAY) {
				wsprintf(szCommand, "status cdaudio position track %d", currentTrack);
				error = mciSendString(szCommand, szReturn, sizeof(szReturn)/sizeof(TCHAR), hwnd);

				if(currentTrack==trackNumbers) {
					wsprintf(szCommand, "play cdaudio from %d", currentTrack);
				} else {
					wsprintf(szCommand, "play cdaudio from %d to %d", currentTrack , currentTrack + 1);
				}	  
				error = mciSendString(szCommand, szReturn, sizeof(szReturn)/sizeof(TCHAR), hwnd);
			}		
			break;	
	}  

	switch(id) {
		case IDM_CLOSE:
			// Close the door, then try to open the audio CD t oplay.
			// So don't add break in here.
			error = mciSendString(_T("set cdaudio door closed"), szReturn, sizeof(szReturn)/sizeof(TCHAR), hwnd);

		case IDM_OPEN:
			// Open CDAuido device	
			error = mciSendString(_T("open cdaudio"), szReturn, sizeof(szReturn)/sizeof(TCHAR), hwnd);	  	  
			// 設定樂曲時間格式
			error = mciSendString(_T("set cdaudio time format tmsf"), szReturn, sizeof(szReturn)/sizeof(TCHAR), hwnd);

			// Get Audio CD Tracks, we need check errors here
			error = mciSendString(_T("status cdaudio number of tracks"), szReturn, sizeof(szReturn)/sizeof(TCHAR), hwnd);
			if(error != 0) {
				playMode = CD_NOCD;
				MessageBox(NULL, _T("Open Audio CD Fail."), _T("Error"), MB_OK);
			} else {
				tracks = atoi(szReturn);
				trackNumbers = tracks; //Give trackNumbers the track value, record it.

				for(count = 1;count <= tracks; count++) {
					wsprintf(szTracks, "Track %02d", count);
					SendMessage(hList, LB_ADDSTRING,0, (LPARAM) szTracks); 
				}

				playMode = CD_INIT; // Still zero in this moment, user need use play button to change state to play

				/******************************************************
				 * Set Focus to ListBox and Play first track
				 ******************************************************/
				currentTrack = 1;			
				SendMessage(hList, LB_SETCURSEL, (WPARAM) currentTrack - 1, 0);	// Zero-based Listbox index
			}	  		        

			break;

		case IDM_EJECT:
			KillTimer(hwnd, CD_TIMER); //Since we want open the door, stop Timer

			// Clear ListBox Items
			for(count = trackNumbers; count >= 0; --count) {
				SendMessage(hList, (UINT) LB_DELETESTRING, (WPARAM) count,  (LPARAM) 0);
			}

			// Set Trackbar to zero
			playMode = CD_NOCD;
			currentSeconds = 0;		
			SendMessage(hTrack, TBM_SETPOS,  (WPARAM) TRUE,  (LPARAM) currentSeconds);				

			error = mciSendString(_T("stop cdaudio"), szReturn, sizeof(szReturn)/sizeof(TCHAR), hwnd);
			error = mciSendString(_T("set cdaudio door open"), szReturn, sizeof(szReturn)/sizeof(TCHAR), hwnd);
			break;

		case IDM_EXIT:
			SendMessage(hwnd, WM_CLOSE, 0, 0);
			break;

		case IDM_VOLUME:	   
			// Call Volume Control program
			ShellExecute(NULL, NULL, "Sndvol32.exe", NULL, NULL, SW_SHOW);
			break;

		case IDM_ABOUT:
			ShellAbout(hwnd, _T("CD Player"),  _T("\nThis Program written by Danilo."), NULL);
			break;

			/********************************************************
			 * Handle Toolbar Button
			 ********************************************************/
		case IDM_BUTTON1:
			KillTimer(hwnd, CD_TIMER);

			if(currentTrack==trackNumbers) {
				currentTrack = 1;		
			} else {
				++currentTrack;
			}

			SendMessage(hList, LB_SETCURSEL, (WPARAM) currentTrack - 1, 0);

			if(playMode==CD_PLAY) {
				if(currentTrack==trackNumbers) {
					wsprintf(szCommand, "play cdaudio from %d", currentTrack);
				} else {
					wsprintf(szCommand, "play cdaudio from %d to %d", currentTrack, currentTrack + 1);
				}	  
				error = mciSendString(szCommand, szReturn, sizeof(szReturn)/sizeof(TCHAR), hwnd);

				SetTimer(hwnd, CD_TIMER, 1000, NULL);		
				currentSeconds = 0;

				//Get track length
				wsprintf(szCommand, "status cdaudio length track %d", currentTrack);			
				error = mciSendString(szCommand, szReturn, sizeof(szReturn)/sizeof(TCHAR), hwnd);		
				trackSeconds = timestring2Int(szReturn);

				if(playMode != CD_NOCD) {
					wsprintf(szTrack, "Track %d: %4d/%4d seconds", currentTrack, currentSeconds, trackSeconds);  
					SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM) szTrack); 
				}

				SendMessage(hTrack, TBM_SETRANGE,  (WPARAM) TRUE, (LPARAM) MAKELONG(0, trackSeconds)); 
				SendMessage(hTrack, TBM_SETPOS,  (WPARAM) TRUE,  (LPARAM) currentSeconds);
			}	

			break;			

		case IDM_BUTTON2:
			if(playMode ==CD_NOCD) {
				break;
			}			

			int2TimeString(szSeconds, currentSeconds);

			playMode = CD_PLAY;  //Set playMode to Play Status = 1		

			if(currentTrack==trackNumbers) {
				wsprintf(szCommand, "play cdaudio from %d:%s", currentTrack, szSeconds);
			} else {
				wsprintf(szCommand, "play cdaudio from %d:%s to %d:00:00:00", currentTrack, szSeconds, currentTrack + 1);			
			}	  
			error = mciSendString(szCommand, szReturn, sizeof(szReturn)/sizeof(TCHAR), hwnd);

			SetTimer(hwnd, CD_TIMER, 1000, NULL);		

			//Set TrackBar
			wsprintf(szCommand, "status cdaudio length track %d", currentTrack);			
			error = mciSendString(szCommand, szReturn, sizeof(szReturn)/sizeof(TCHAR), hwnd);		
			trackSeconds = timestring2Int(szReturn);

			if(playMode != CD_NOCD) {
				wsprintf(szTrack, "Track %d: %4d/%4d seconds", currentTrack, currentSeconds, trackSeconds);  
				SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM) szTrack); 
			}

			SendMessage(hTrack, TBM_SETRANGE,  (WPARAM) TRUE, (LPARAM) MAKELONG(0, trackSeconds)); 
			SendMessage(hTrack, TBM_SETPOS,  (WPARAM) TRUE,  (LPARAM) currentSeconds); 		
			break;

		case IDM_BUTTON3:
			// I think we don't need set currentTracks here
			playMode = CD_PAUSE;  //Set playMode to Pause Status = 2
			KillTimer(hwnd, CD_TIMER);
			error = mciSendString(_T("pause cdaudio"), szReturn, sizeof(szReturn)/sizeof(TCHAR), hwnd);
			break;

		case IDM_BUTTON4:
			playMode = CD_STOP;  //Set playMode to Stop Status = 3
			KillTimer(hwnd, CD_TIMER);
			error = mciSendString(_T("stop cdaudio"), szReturn, sizeof(szReturn)/sizeof(TCHAR), hwnd);

			currentSeconds = 0;

			//Set TrackBar		
			SendMessage(hTrack, TBM_SETPOS,  (WPARAM) TRUE,  (LPARAM) 0); 
			break;  

		case IDM_BUTTON5:	  
			KillTimer(hwnd, CD_TIMER);

			if(currentTrack==1) {
				currentTrack = trackNumbers;		
			} else {
				--currentTrack;
			}

			SendMessage(hList, LB_SETCURSEL, (WPARAM) currentTrack - 1, 0);

			if(playMode==CD_PLAY) {
				if(currentTrack==trackNumbers) {
					wsprintf(szCommand, "play cdaudio from %d", currentTrack);
				} else {
					wsprintf(szCommand, "play cdaudio from %d to %d", currentTrack, currentTrack + 1);
				}	  
				error = mciSendString(szCommand, szReturn, sizeof(szReturn)/sizeof(TCHAR), hwnd);

				SetTimer(hwnd, CD_TIMER, 1000, NULL);		
				currentSeconds = 0;

				wsprintf(szCommand, "status cdaudio length track %d", currentTrack);			
				error = mciSendString(szCommand, szReturn, sizeof(szReturn)/sizeof(TCHAR), hwnd);		
				trackSeconds = timestring2Int(szReturn);

				if(playMode != CD_NOCD) {
					wsprintf(szTrack, "Track %d: %4d/%4d seconds", currentTrack, currentSeconds, trackSeconds);  
					SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM) szTrack); 
				}

				SendMessage(hTrack, TBM_SETRANGE,  (WPARAM) TRUE, (LPARAM) MAKELONG(0, trackSeconds)); 
				SendMessage(hTrack, TBM_SETPOS,  (WPARAM) TRUE,  (LPARAM) currentSeconds);
			}

			break;	 	 			
	}
}

/******************************************************
 * OnHScroll() - Handle Trackbar message.  
 ******************************************************/
void OnHScroll(HWND hwnd, HWND hwndCtl, UINT code, UINT pos)
{
	MCIERROR error;
	TCHAR szTrack[64];
	TCHAR szReturn[1024];
	TCHAR szCommand[1024];  
	TCHAR szSeconds[256];			

	switch(code) {
		case TB_THUMBPOSITION:		
			currentSeconds = SendMessage(hTrack, TBM_GETPOS, 0, 0);

			if(playMode==CD_PLAY) {
				KillTimer(hwnd, CD_TIMER);
				int2TimeString(szSeconds, currentSeconds);				

				if(currentTrack==trackNumbers) {
					wsprintf(szCommand, "play cdaudio from %d:%s", currentTrack, szSeconds);
				} else {
					wsprintf(szCommand, "play cdaudio from %d:%s to %d:00:00:00", currentTrack, szSeconds, currentTrack + 1);			
				}	  				
				error = mciSendString(szCommand, szReturn, sizeof(szReturn)/sizeof(TCHAR), hwnd);

				SetTimer(hwnd, CD_TIMER, 1000, NULL);		

				//Get Track length
				wsprintf(szCommand, "status cdaudio length track %d", currentTrack);			
				error = mciSendString(szCommand, szReturn, sizeof(szReturn)/sizeof(TCHAR), hwnd);		
				trackSeconds = timestring2Int(szReturn);								

				//Set TrackBar
				SendMessage(hTrack, TBM_SETRANGE,  (WPARAM) TRUE, (LPARAM) MAKELONG(0, trackSeconds)); 
				SendMessage(hTrack, TBM_SETPOS,  (WPARAM) TRUE,  (LPARAM) currentSeconds); 
			}

			// Set status bar, notice playMode == -1 is NO-CD
			if(playMode != CD_NOCD) {
				wsprintf(szTrack, "Track %d: %4d/%4d seconds", currentTrack, currentSeconds, trackSeconds);  
				SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM) szTrack); 
			}

			break;
	}			
}


void OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	SendMessage(hTool, WM_SIZE, (WPARAM) state, (LPARAM) MAKELPARAM((cx),(cy)));
	SendMessage(hStatus, WM_SIZE, (WPARAM) state, (LPARAM) MAKELPARAM((cx),(cy)));
}


/******************************************************
 * OnTimer() - Handle Timer Message
 ******************************************************/
void OnTimer(HWND hwnd, UINT id)
{  
	MCIERROR error;
	TCHAR szReturn[1024];	  
	TCHAR szCommand[1024];  ;
	TCHAR szTime[64];
	TCHAR szTrack[64];
	SYSTEMTIME st;

	switch(id) {
		case MY_TIMER:
			GetLocalTime(&st);
			wsprintf(szTime, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);  

			SendMessage(hStatus, SB_SETTEXT, 1, (LPARAM) szTime);  
			break;
		case CD_TIMER:		
			++currentSeconds;

			if(currentSeconds <= trackSeconds) {
				SendMessage(hTrack, TBM_SETPOS,  (WPARAM) TRUE,  (LPARAM) currentSeconds); 
				wsprintf(szTrack, "Track %d: %4d/%4d seconds", currentTrack, currentSeconds, trackSeconds);  
				SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM) szTrack); 
			}	else {
				if(currentTrack==trackNumbers) {
					currentTrack = 1;		
				} else {
					++currentTrack;
				}

				SendMessage(hList, LB_SETCURSEL, (WPARAM) currentTrack - 1, 0);

				if(playMode==CD_PLAY) {
					if(currentTrack==trackNumbers) {
						wsprintf(szCommand, "play cdaudio from %d", currentTrack);
					} else {
						wsprintf(szCommand, "play cdaudio from %d to %d", currentTrack, currentTrack + 1);
					}	  
					error = mciSendString(szCommand, szReturn, sizeof(szReturn)/sizeof(TCHAR), hwnd);

					currentSeconds = 0;

					wsprintf(szCommand, "status cdaudio length track %d", currentTrack);			
					error = mciSendString(szCommand, szReturn, sizeof(szReturn)/sizeof(TCHAR), hwnd);		
					trackSeconds = timestring2Int(szReturn);

					SendMessage(hTrack, TBM_SETRANGE,  (WPARAM) TRUE, (LPARAM) MAKELONG(0, trackSeconds)); 
					SendMessage(hTrack, TBM_SETPOS,  (WPARAM) TRUE,  (LPARAM) currentSeconds);
				}	
			}
			break;
	}  
}

/******************************************************
 * OnClose() - Handle WM_CLOSE Message
 ******************************************************/
void OnClose(HWND hwnd)
{
	MCIERROR error;
	TCHAR szReturn[1024];	

	// We need to send stop command when we close program.
	error = mciSendString(_T("stop cdaudio"), szReturn, sizeof(szReturn)/sizeof(TCHAR), hwnd);
	error = mciSendString(_T("close cdaudio"), szReturn, sizeof(szReturn)/sizeof(TCHAR), hwnd);

	KillTimer(hwnd, MY_TIMER);
	DestroyWindow(hwnd);
}

/******************************************************
 * OnDestroy()
 ******************************************************/
void OnDestroy(HWND hwnd)
{
	PostQuitMessage(0);
}
