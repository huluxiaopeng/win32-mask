#include "stdafx.h"
#include "winMain.h"
#include "stdio.h"
#include <tchar.h>
#include <windows.h>
#include <thread>

HANDLE g_Semaphore = NULL;
HINSTANCE g_hInstance = 0;
RECT rect;	//�ͻ�������
POINT PO;	//��Ļ����

HWND M_hWnd;	//�����ھ��
HWND maskHwnd;//�ɰ���
HWND g_hWnd;
LONG oldWindowProc; //�ɵ�����
HWND hWndButtonStart;
HWND hWndButtonStop;
LPCTSTR lpClassName = "MASKWnd";  // ע�ᴰ�ڵ���������

HANDLE hThread1;
DWORD dThread1;
MSG msgMask;
extern void QuitMaskWindow();

//�����ڵĻص�����
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		hWndButtonStart = CreateWindow(_T("Button"), _T("��ʼ����"),
			WS_CHILD | WS_VISIBLE,
			20, 20, 100, 20, hWnd, (HMENU)iBtnID[0], g_hInstance, NULL);

		hWndButtonStart = CreateWindow(_T("Button"), _T("ֹͣ����"),
			WS_CHILD | WS_VISIBLE,
			160, 20, 100, 20, hWnd, (HMENU)iBtnID[1], g_hInstance, NULL);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_COMMAND:
		switch (wParam)
		{
		case IDB_PUSHBUTTON1:
			//����һ���߳�
			startLoading();
		case IDB_PUSHBUTTON2:
			//����һ������
			stopLoading();
		default:
			break;
		}
	default:
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//�ɰ�ص�����
LRESULT CALLBACK WinMaskProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		HBITMAP hBmp;  //λͼ���ƾ��
		HDC mdc = CreateCompatibleDC(hdc); //�������ݵĻ���DC����

		//����ͼƬ��λͼ���ƶ�����
		hBmp = (HBITMAP)LoadImage(NULL, _T("D:\\2.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

		//����DC ѡ��λͼ����һ������
		SelectObject(mdc, hBmp);

		//�������λͼ�ڸ��Ƶ�����DC��
		BitBlt(hdc, 400, 20, 700, 700, mdc, 0, 0, SRCCOPY);

		DeleteDC(mdc);
		DeleteObject(hBmp);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_QUIT:
		PostQuitMessage(0);
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


void MOVE_win(HWND hWnd)
{
	//��ȡ���ڵĿͻ�������
	GetClientRect(hWnd, &rect);	//���ڿͻ������Ͻ�Ϊ0
	PO.x = rect.left;		//0
	PO.y = rect.top;		//0
							//��Ļ����ת����ת��ǰ��Ҫ��PO�ṹ�帳��ͻ������꣬Ҳ���Ǹ�PO�ṹ�帳���ʼֵWindows����ת��
	ClientToScreen(hWnd, &PO);
	//�ƶ�����
	if (maskHwnd != NULL) {
		MoveWindow(maskHwnd, PO.x, PO.y, rect.right - rect.left, rect.bottom - rect.top, TRUE);
	}
}


//�µ����໯����Ϣ
LRESULT NewWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_MOVE)//���Ÿ�����һ���ƶ�
	{
		MOVE_win(hWnd);
	}
	if (message == WM_SIZE)//���ڴ�Сһ��
	{
		MOVE_win(hWnd);
	}

	return CallWindowProc((WNDPROC)oldWindowProc, hWnd, message, wParam, lParam);
}

//ע��������
BOOL Register(LPSTR lpName, WNDPROC wndProc)
{
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.hCursor = NULL;
	wc.hIcon = NULL;
	wc.hIconSm = NULL;
	wc.hInstance = g_hInstance;
	wc.lpfnWndProc = wndProc;
	wc.lpszClassName = lpName;
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	ATOM nAtom = RegisterClassEx(&wc);
	if (nAtom == 0)
		return FALSE;
	return TRUE;
}

//ע���ɰ溯��
BOOL RegisterWindowMask(LPCTSTR lpMaskName, HINSTANCE hInstance)
{
	WNDCLASS wcMask = { 0 };
	wcMask.style = CS_HREDRAW | CS_VREDRAW;
	wcMask.lpfnWndProc = WinMaskProc;//ָ�򴰿ڹ��̺���
	wcMask.cbClsExtra = 0;
	wcMask.cbWndExtra = 0;
	wcMask.hInstance = hInstance;
	wcMask.hIcon = NULL;
	wcMask.hCursor = NULL;
	wcMask.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
	wcMask.lpszMenuName = NULL;
	wcMask.lpszClassName = lpMaskName;
	ATOM nMaskAtom = RegisterClass(&wcMask);
	if (nMaskAtom == 0)
		return FALSE;
	return TRUE;
}

//����������
HWND CreateMain(LPCSTR lpClassName, LPCSTR lpWndName)
{
	HWND hWnd = CreateWindowEx(0, lpClassName, lpWndName,WS_OVERLAPPEDWINDOW, 150, 150, 1500, 700, NULL, NULL, g_hInstance, NULL);
	return hWnd;
}

//�����ɰ洰��
HWND CreateMaskWindow(HWND hWndParent, LPCSTR lpMaskName, HINSTANCE hInstance, BOOL MT) //������ ���� ��Դʵ�����  �Ƿ�ģ��
{
	HWND hWnd = NULL;
	//���÷��
	LONG maskstyle;
	if (MT == TRUE)
	{
		maskstyle = WS_EX_LAYERED;
	}
	else
	{
		maskstyle = WS_EX_LAYERED | WS_EX_TRANSPARENT;
	}
	//����������չ���Ĵ���
	hWnd = CreateWindowEx(maskstyle, lpMaskName, NULL, WS_POPUP, 0, 0, 0, 0, hWndParent, NULL, hInstance, NULL);
	if (MT == TRUE)
	{
		EnableWindow(hWnd, FALSE);
	}
	SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 150, LWA_ALPHA);
	return hWnd;
}

//��ʾ������  
void Display(HWND hWnd)
{
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
}

//��������Ϣѭ��
void Message()
{
	MSG nMsg = { 0 };
	while (GetMessage(&nMsg, NULL, 0, 0))
	{
		TranslateMessage(&nMsg);
		DispatchMessage(&nMsg);
	}
}

void MaskMessage()
{
	MSG nMsg = { 0 };
	while (GetMessage(&nMsg, NULL, 0, 0))
	{
		TranslateMessage(&nMsg);
		DispatchMessage(&nMsg);
	}
}

void SetMaskWindow(HWND hwnd, BOOL MT)
{
	//ע�ᴰ��
	RegisterWindowMask(lpClassName, GetModuleHandle(NULL));
	//�������ִ���
	maskHwnd = CreateMaskWindow(hwnd, lpClassName, GetModuleHandle(NULL), MT);
	//�ƶ����ִ���
	MOVE_win(hwnd);
	//���໯������Ϣ
	oldWindowProc = GetWindowLong(hwnd, GWL_WNDPROC);
	WNDPROC lpfbOldProc = (WNDPROC)SetWindowLong(hwnd, GWL_WNDPROC, (DWORD)NewWndProc);
	Display(maskHwnd);
	//���ø����ڽ���
	HWND parentHwnd = GetParent(maskHwnd);//��ȡ���
	SetForegroundWindow(parentHwnd);//����
	MaskMessage();
}


//�������ִ���
void QuitMaskWindow()
{
	//���û�ԭʼ����Ϣ�������������´����໯�ᵼ����Ϣ��ѭ��
	HWND fORhwnd = GetParent(maskHwnd);	//��ȡ�����ھ��
	WNDPROC lpfnOldProc = (WNDPROC)SetWindowLong(fORhwnd, GWL_WNDPROC, (DWORD)oldWindowProc);	//setwindowlong�᷵�����ú���·��
	DestroyWindow(maskHwnd);
}

void ThreadBeginMask1()
{
	SetMaskWindow(M_hWnd,FALSE);
}

void startLoading()
{
	thread t(ThreadBeginMask1);
	t.detach();
	return;
}

void stopLoading()
{
	//SendMessage(maskHwnd,WM_DESTROY,0,0);
	QuitMaskWindow();
	SendMessage(maskHwnd,WM_QUIT,NULL,NULL);
	return;
}


//�����ڽ�������
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE  hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
	g_hInstance = hInstance;
	BOOL nRet = Register("Main", WndProc);

	if (!nRet)
	{
		MessageBox(NULL, _T("ע��ʧ��"), _T("Infor"), MB_OK);
		return 0;
	}

	M_hWnd = CreateMain(_T("Main"), _T("loadingDisplay"));

	Display(M_hWnd);
	Message();
	return 0;
};