unit loadingDisplay.Client;

interface
uses
  Winapi.Windows,Winapi.Messages, System.SysUtils, System.Variants, System.Classes, Logger;

type
  TLoading = class(TThread)
  public
	  procedure startLoading(hWnd: HWND; lparam:LPCWSTR);
    procedure stopLoading();
  private
  	procedure Move_Win(HWND: HWND);
    function RegisterWindowMask(hInst: HINST): bool;
    procedure CreateMaskWindow(hWndParent: HWND; hInst: hInst; MT: bool);
    procedure DisPlay(displayHWnd: HWND);
    procedure MaskMessage();
    procedure SetWindowMask(HWND: HWND; MT: bool);
    procedure QuitMaskWindow();

end;

var
  loading: TLoading;
  delMaskHandle: HWND;
  delMaskRegister: integer; // 注册窗口返回值
  delMaskMessage: TMsg;// 蒙版窗口消息
  parentHwnd: HWND;
  fORhwnd : HWND;
  rectWindow: TRect; // 客户区坐标
  poWindow: TPoint; // 屏幕坐标
  oldWindowProc: long;
  wcMask: TWndClass;
  maskThread : HWND;
  hBmp: HBITMAP;
implementation

uses
  main;
// 蒙版回调函数
function WinMaskProc(theWindow: HWND; theMessage, Wparam, LParam: longint)
  : longint; stdcall; export;
var
  hdcParam: HDC;
  mdcParam: HDC;
  psStruct: PAINTSTRUCT;
  rect: TRect;

  bm : BITMAP;
  px : integer;
  py : integer;
  onPiontX : integer;
  onPointY : integer;
begin

  case theMessage of
    WM_CREATE:
      begin

      end;
    WM_PAINT:
      begin
        hdcParam := BeginPaint(theWindow, psStruct);
        mdcParam := CreateCompatibleDC(hdcParam);

        GetObject(hBmp, sizeof(bm), @bm );
        px := bm.bmWidth;
        py := bm.bmHeight;
        onPiontX :=  (rectWindow.Right - rectWindow.Left -px)div 2;
        onPointY :=  (rectWindow.Bottom- rectWindow.Top -py)div 2 ;

        SelectObject(mdcParam, hBmp);
        BitBlt(hdcParam, onPiontX ,onPointY , px, py, mdcParam, 0, 0, SRCCOPY);  //需要根据具体情况调整位置
        DeleteDC(mdcParam);
        EndPaint(theWindow, psStruct);
      end;
    WM_QUIT:
      begin
        PostQuitMessage(0);
        exit;
      end;
  end;
  result := DefWindowProc(theWindow, theMessage, Wparam, LParam);
end;

function NewWndProc(theWindow: HWND;
  theMessage, Wparam, LParam: longint): longint; stdcall; export;
begin
  if theMessage = WM_MOVE then
  begin
    loading.Move_Win(theWindow);
  end;

  if theWindow = WM_SIZE then
  begin
    loading.Move_Win(theWindow);
  end;
  result := CallWindowProc(TFNWndProc(oldWindowProc), theWindow, theMessage,
    Wparam, LParam);
end;


procedure TLoading.Move_Win(HWND: HWND);
begin
  GetClientRect(HWND,rectWindow);
  poWindow.X := rectWindow.Left;
  poWindow.Y := rectWindow.Top;
  ClientToScreen(HWND,&poWindow);

  if delMaskHandle <> 0 then
  begin
    MoveWindow(delMaskHandle, poWindow.X, poWindow.Y, rectWindow.Right - rectWindow.Left,
      rectWindow.Bottom - rectWindow.Top, TRUE);
  end;
end;

// 注册蒙版函数
function TLoading.RegisterWindowMask(hInst: HINST): bool;
begin
  wcMask.style := CS_HREDRAW OR CS_VREDRAW;
  wcMask.lpfnWndProc := @WinMaskProc;
  wcMask.cbClsExtra := 0;
  wcMask.cbWndExtra := 0;
  wcMask.HInstance := hInst;
  wcMask.hIcon := 0;
  wcMask.hCursor := 0;
  wcMask.hbrBackground := HBRUSH(COLOR_WINDOW + 5);
  wcMask.lpszMenuName := nil;
  wcMask.lpszClassName := 'testloading';
  delMaskRegister :=RegisterClassW(wcMask);

  if delMaskRegister = 0 then
  begin
    result := FALSE;
  end
  else
  begin
   result := TRUE;
  end;
end;

// 创建蒙版窗口
procedure TLoading.CreateMaskWindow(hWndParent: HWND; hInst: hInst; MT: bool);
var
  maskstyle: long;
  parentRect: TRect; // 客户区坐标
  parentPo: TPoint; // 屏幕坐标

begin
  GetClientRect(hWndParent,parentRect);
  parentPo.X := parentRect.Left;
  parentPo.Y := parentRect.Top;
  // 创建窗口
  if MT = TRUE then
  begin
    maskstyle := WS_EX_LAYERED;
  end
  else
  begin
    maskstyle := WS_EX_LAYERED OR WS_EX_TRANSPARENT;
  end;

  delMaskHandle := CreateWindowEX(maskstyle, 'testloading', nil,  WS_POPUP,
    parentPo.X, parentPo.Y, parentRect.Right - parentRect.Left, parentRect.Bottom - parentRect.Top,0, 0, hInst,nil);
  if MT = TRUE then
  begin
    EnableWindow(delMaskHandle, FALSE);
  end;
  SetLayeredWindowAttributes(delMaskHandle, RGB(0, 0, 0), 150, LWA_ALPHA);
end;


procedure TLoading.DisPlay(displayHWnd: HWND);
begin
  ShowWindow(displayHWnd, SW_SHOWNORMAL);
  UpdateWindow(displayHWnd);
end;

procedure TLoading.MaskMessage();
begin
  while GetMessage(delMaskMessage, 0, 0, 0) do
  begin
    TranslateMessage(&delMaskMessage);
    DispatchMessage(&delMaskMessage);
  end;
end;

procedure TLoading.SetWindowMask(HWND: HWND; MT: bool);
var
  lpfbOldProc: TFNWndProc;
begin
  RegisterWindowMask(GetModuleHandle(nil));
  CreateMaskWindow(HWND, GetModuleHandle(nil), MT);
  Move_Win(HWND);
  //oldWindowProc := GetWindowLong(HWND, GWL_WNDPROC);
  //lpfbOldProc := TFNWndProc(SetWindowLong(HWND, GWL_WNDPROC,
    //LongWord(NewWndProc(HWND,WM_PAINT,0,0))));
  DisPlay(delMaskHandle);
  parentHwnd := GetParent(delMaskHandle);
  SetForegroundWindow(parentHwnd);
  MaskMessage();
end;


procedure TLoading.QuitMaskWindow();
var
  lpfnOldProc : TFNWndProc;
begin
   //fORhwnd :=  GetParent(delMaskHandle);
   //lpfnOldProc :=  TFNWndProc(SetWindowLong(fORhwnd, GWL_WNDPROC,LongWord(oldWindowProc)));
   DestroyWindow(delMaskHandle);
end;

procedure ThreadBeginMask(P: Pointer);stdCall;
begin
  loading.SetWindowMask(HWND(P),FALSE);
end;

procedure TLoading.startLoading(hWnd: HWND; lparam:LPCWSTR);
var
  ID : DWORD;
begin
   //开启loading线程
   maskThread := CreateThread(nil, 0, @ThreadBeginMask, Pointer(hWnd), 0, ID);
   hBmp := LoadBitmap(Hinstance,MAKEINTRESOURCE(lparam));
end;


procedure TLoading.stopLoading();
begin
  DeleteObject(hBmp);
  QuitMaskWindow();
  SendMessage(delMaskHandle,WM_QUIT,0,0);
  CloseHandle(maskThread);
end;

end.
