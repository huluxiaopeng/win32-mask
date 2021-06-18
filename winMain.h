#pragma once
#include <iostream>
#include "resource.h"
using namespace std;
#define IDB_PUSHBUTTON1	1
#define IDB_PUSHBUTTON2	2
#define IDB_PUSHBUTTON3	3
#define NUM				3

int		iBtnID[] = { IDB_PUSHBUTTON1, IDB_PUSHBUTTON2, IDB_PUSHBUTTON3 };

void OpenMask();
void CloseMask();
DWORD WINAPI ThreadBeginMask(LPVOID lParam);
void SetMaskWindow(HWND hwnd, BOOL MT);
void startLoading();
void stopLoading(); 
void ThreadBeginMask1();
/*hThread1 = CreateThread(NULL, 0, ThreadBeginMask, NULL, 0, &dThread1);*/
//SetMaskWindow(M_hWnd, FALSE);
//CloseHandle(hThread1);
//DWORD WINAPI ThreadBeginMask(LPVOID lParam)
//{
//	SetMaskWindow(M_hWnd, FALSE);
//	return 0;
//}