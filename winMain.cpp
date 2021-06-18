#include "stdafx.h"
#include "winMain.h"
#include "stdio.h"
#include <tchar.h>
#include <windows.h>
#include <thread>

HANDLE g_Semaphore = NULL;
HINSTANCE g_hInstance = 0;
RECT rect;	//客户区坐标
POINT PO;	//屏幕坐标

HWND M_hWnd;	//父窗口句柄
HWND maskHwnd;//蒙版句柄
HWND g_hWnd;
LONG oldWindowProc; //旧的属性
HWND hWndButtonStart;
HWND hWndButtonStop;
LPCTSTR lpClassName = "MASKWnd";  // 注册窗口的名称遮罩

HANDLE hThread1;
DWORD dThread1;
MSG msgMask;
extern void QuitMaskWindow();

//主窗口的回调函数
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		hWndButtonStart = CreateWindow(_T("Button"), _T("开始处理"),
			WS_CHILD | WS_VISIBLE,
			20, 20, 100, 20, hWnd, (HMENU)iBtnID[0], g_hInstance, NULL);

		hWndButtonStart = CreateWindow(_T("Button"), _T("停止处理"),
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
			//创建一个线程
			startLoading();
		case IDB_PUSHBUTTON2:
			//销毁一个窗口
			stopLoading();
		default:
			break;
		}
	default:
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//蒙版回调函数
LRESULT CALLBACK WinMaskProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		HBITMAP hBmp;  //位图绘制句柄
		HDC mdc = CreateCompatibleDC(hdc); //创建兼容的缓存DC对象

		//加载图片到位图绘制对象中
		hBmp = (HBITMAP)LoadImage(NULL, _T("D:\\2.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

		//缓存DC 选择位图绘制一个对象
		SelectObject(mdc, hBmp);

		//将缓存的位图在复制到窗口DC上
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
	//获取窗口的客户区坐标
	GetClientRect(hWnd, &rect);	//对于客户区左上角为0
	PO.x = rect.left;		//0
	PO.y = rect.top;		//0
							//屏幕坐标转换，转换前需要给PO结构体赋予客户区坐标，也就是给PO结构体赋予初始值Windows才能转换
	ClientToScreen(hWnd, &PO);
	//移动窗口
	if (maskHwnd != NULL) {
		MoveWindow(maskHwnd, PO.x, PO.y, rect.right - rect.left, rect.bottom - rect.top, TRUE);
	}
}


//新的子类化的消息
LRESULT NewWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_MOVE)//随着父窗口一起移动
	{
		MOVE_win(hWnd);
	}
	if (message == WM_SIZE)//窗口大小一致
	{
		MOVE_win(hWnd);
	}

	return CallWindowProc((WNDPROC)oldWindowProc, hWnd, message, wParam, lParam);
}

//注册主窗口
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

//注册蒙版函数
BOOL RegisterWindowMask(LPCTSTR lpMaskName, HINSTANCE hInstance)
{
	WNDCLASS wcMask = { 0 };
	wcMask.style = CS_HREDRAW | CS_VREDRAW;
	wcMask.lpfnWndProc = WinMaskProc;//指向窗口过程函数
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

//创建主窗口
HWND CreateMain(LPCSTR lpClassName, LPCSTR lpWndName)
{
	HWND hWnd = CreateWindowEx(0, lpClassName, lpWndName,WS_OVERLAPPEDWINDOW, 150, 150, 1500, 700, NULL, NULL, g_hInstance, NULL);
	return hWnd;
}

//创建蒙版窗口
HWND CreateMaskWindow(HWND hWndParent, LPCSTR lpMaskName, HINSTANCE hInstance, BOOL MT) //父窗口 类名 资源实例句柄  是否模板
{
	HWND hWnd = NULL;
	//设置风格
	LONG maskstyle;
	if (MT == TRUE)
	{
		maskstyle = WS_EX_LAYERED;
	}
	else
	{
		maskstyle = WS_EX_LAYERED | WS_EX_TRANSPARENT;
	}
	//创建具有拓展风格的窗口
	hWnd = CreateWindowEx(maskstyle, lpMaskName, NULL, WS_POPUP, 0, 0, 0, 0, hWndParent, NULL, hInstance, NULL);
	if (MT == TRUE)
	{
		EnableWindow(hWnd, FALSE);
	}
	SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 150, LWA_ALPHA);
	return hWnd;
}

//显示主窗口  
void Display(HWND hWnd)
{
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
}

//主窗口消息循环
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
	//注册窗口
	RegisterWindowMask(lpClassName, GetModuleHandle(NULL));
	//创建遮罩窗口
	maskHwnd = CreateMaskWindow(hwnd, lpClassName, GetModuleHandle(NULL), MT);
	//移动遮罩窗口
	MOVE_win(hwnd);
	//子类化父类消息
	oldWindowProc = GetWindowLong(hwnd, GWL_WNDPROC);
	WNDPROC lpfbOldProc = (WNDPROC)SetWindowLong(hwnd, GWL_WNDPROC, (DWORD)NewWndProc);
	Display(maskHwnd);
	//设置父窗口焦点
	HWND parentHwnd = GetParent(maskHwnd);//获取句柄
	SetForegroundWindow(parentHwnd);//激活
	MaskMessage();
}


//销毁遮罩窗口
void QuitMaskWindow()
{
	//设置回原始的消息处理函数，否则下次子类化会导致消息死循环
	HWND fORhwnd = GetParent(maskHwnd);	//获取父窗口句柄
	WNDPROC lpfnOldProc = (WNDPROC)SetWindowLong(fORhwnd, GWL_WNDPROC, (DWORD)oldWindowProc);	//setwindowlong会返回设置后的新风格
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


//主窗口进行启动
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE  hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
	g_hInstance = hInstance;
	BOOL nRet = Register("Main", WndProc);

	if (!nRet)
	{
		MessageBox(NULL, _T("注册失败"), _T("Infor"), MB_OK);
		return 0;
	}

	M_hWnd = CreateMain(_T("Main"), _T("loadingDisplay"));

	Display(M_hWnd);
	Message();
	return 0;
};