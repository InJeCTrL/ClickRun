#include<Windows.h>
#include<stdio.h>

//可交互的控件ID
#define ID_BtnSel			3301//鼠标左右键按键选择
#define ID_FuncSel			3302//模拟方式选择
#define ID_TLSet			3303//时间间隔设置
#define	ID_HKSet			3304//热键设置
#define ID_UorELock			3305//锁定/解锁配置
#define ID_URL				3306//主页地址

//热键ID
#define HotKey				1011

INT LockFlag = 0;//标志配置是否锁定
INT LeftorRight = 0;//1：左键 0：右键
INT Func = 0;//0：mouse_event 1：SendInput 2：WinIO
TCHAR str_TL[20] = { 0 };//字符串形式的时间间隔
INT dig_TL = 0;//整型形式的时间间隔
TCHAR str_HKList[12][4] = { L"F1",L"F2",L"F3",L"F4",L"F5",L"F6",L"F7",L"F8",L"F9",L"F10",L"F11",L"F12" };//备选热键列表
INT HK_Index = 0;//选定的热键编号

INT ClickRunning = 0;//标记连点是否正在进行

HANDLE hClickThread = NULL;//连点线程句柄

//解锁当前配置（无GUI部分）
INT UnLock_NoGUI(HWND thishwnd)
{
	//结束连点线程（若存在）
	if (ClickRunning)
		TerminateThread(hClickThread, 0);
	//注销热键
	UnregisterHotKey(thishwnd, HotKey);
	LockFlag = 0;//标记解锁

	return 0;
}
//连点执行者，循环放在最里层，减少不必要的重复判断
DWORD WINAPI ClickRunner(LPVOID lpParam)
{
	INPUT Down = { 0 }, Up = { 0 };//SendInput使用（按下、抬起）

	switch (Func)
	{
	case 0://mouse_event
		if (LeftorRight)
		{
			while (TRUE)
			{
				mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
				mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
				Sleep(dig_TL);
			}
		}
		else
		{
			while (TRUE)
			{
				mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
				mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
				Sleep(dig_TL);
			}
		}
		break;
	case 1://SendInput
		Down.type = INPUT_MOUSE;
		Up.type = INPUT_MOUSE;
		if (LeftorRight)
		{
			Down.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
			Up.mi.dwFlags = MOUSEEVENTF_LEFTUP;
		}
		else
		{
			Down.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
			Up.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
		}
		while (TRUE)
		{
			SendInput(TRUE, &Down, sizeof(INPUT));
			SendInput(TRUE, &Up, sizeof(INPUT));
			Sleep(dig_TL);
		}
		break;
	default:
		break;
	}
	return 0;
}
//若是正数字符串则返回正数值，否则返回0
INT IsPosDigitStr(LPTSTR in_str)
{
	INT ret = 0;
	INT i;
	INT len = lstrlen(in_str);
	if (!len)
		return 0;
	for (i = 0; i < len; i++)
	{
		if (!isdigit(in_str[i]))
			return 0;//不是纯正数字符串
		else
		{
			ret *= 10;
			ret += in_str[i] - '0';
		}
	}
	return ret;
}
LRESULT WINAPI CtlProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i;

	static HWND hFont;			//统一字体

	static HWND hTip1;			//鼠标左右键选择文本
	static HWND hRadio_Left;	//鼠标左键按钮
	static HWND hRadio_Right;	//鼠标右键按钮

	static HWND hTip2;			//模拟方式选择文本
	static HWND hCombo_Func;	//方式选择框

	static HWND hTip3;			//时间间隔文本
	static HWND hText_TL;		//时间间隔输入框

	static HWND hTip4;			//热键设置提示文本
	static HWND hCombo_HK;		//热键设置选择框

	static HWND hBtn_UorE;		//锁定解锁按钮

	static HWND hSepLine;		//分割线
	static HWND hURL;			//超文本链接文本框

	switch (message)
	{
	case WM_CREATE:
		hFont = CreateFont(-14, -7, 0, 0, 400, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, TEXT("微软雅黑"));
		hTip1 = CreateWindow(L"Static", L"按键选择：", WS_CHILD | WS_VISIBLE, 15, 10, 300, 100, hWnd, NULL, hWnd, 0);
		hRadio_Left = CreateWindow(L"Button", L"鼠标左键", WS_CHILD | WS_VISIBLE | BS_LEFT | BS_AUTORADIOBUTTON, 60, 35, 80, 20, hWnd, (HMENU)ID_BtnSel, hWnd, 0);
		hRadio_Right = CreateWindow(L"Button", L"鼠标右键", WS_CHILD | WS_VISIBLE | BS_LEFT | BS_AUTORADIOBUTTON, 200, 35, 80, 20, hWnd, (HMENU)ID_BtnSel, hWnd, 0);
		hTip2 = CreateWindow(L"Static", L"模拟方式选择：", WS_CHILD | WS_VISIBLE, 15, 60, 300, 100, hWnd, NULL, hWnd, 0);
		hCombo_Func = CreateWindow(L"ComboBox", L"", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 40, 85, 280, 100, hWnd, (HMENU)ID_FuncSel, hWnd, 0);
		hTip3 = CreateWindow(L"Static", L"时间间隔(ms)：", WS_CHILD | WS_VISIBLE, 15, 120, 300, 100, hWnd, NULL, hWnd, 0);
		hText_TL = CreateWindow(L"Edit", L"500", ES_CENTER | WS_CHILD | WS_VISIBLE, 30, 145, 80, 25, hWnd, (HMENU)ID_TLSet, hWnd, 0);
		hTip4 = CreateWindow(L"Static", L"热键设置：", WS_CHILD | WS_VISIBLE, 210, 120, 300, 100, hWnd, NULL, hWnd, 0);
		hCombo_HK = CreateWindow(L"ComboBox", L"", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 225, 145, 80, 1000, hWnd, (HMENU)ID_HKSet, hWnd, 0);
		hBtn_UorE = CreateWindow(L"Button", L"锁定当前配置", ES_CENTER | WS_CHILD | WS_VISIBLE, 25, 180, 290, 50, hWnd, (HMENU)ID_UorELock, hWnd, 0);
		hSepLine = CreateWindow(L"Static", L"", SS_ETCHEDHORZ | WS_CHILD | WS_VISIBLE, 5, 240, 332, 10, hWnd, NULL, hWnd, 0);
		hURL = CreateWindow(L"Edit", L"软件主页：injectrl.coding.me/ClickRun", ES_READONLY | WM_NOTIFY | ES_CENTER | WS_CHILD | WS_VISIBLE, 23, 247, 300, 20, hWnd, (HMENU)ID_URL, hWnd, 0);

		//设置各控件字体
		SendMessage(hTip1, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hRadio_Left, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hRadio_Right, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hTip2, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hTip3, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hTip4, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hText_TL, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hBtn_UorE, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hURL, WM_SETFONT, (WPARAM)hFont, NULL);
		//向模拟方式ComboBox添加选项
		SendMessage(hCombo_Func, CB_ADDSTRING, 0, L"1 - mouse_event");
		SendMessage(hCombo_Func, CB_ADDSTRING, 0, L"2 - SendInput");
		//向热键设置ComboBox添加选项
		for (i = 0; i < 12; i++)
			SendMessage(hCombo_HK, CB_ADDSTRING, 0, str_HKList[i]);
		
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_URL:
			if (HIWORD(wParam) == WM_KEYDOWN)
				ShellExecute(NULL, L"open", L"http://injectrl.coding.me/ClickRun", NULL, NULL, SW_SHOWNORMAL);//打开主页
			break;
		case ID_UorELock://单击了锁定/解锁
			if (LockFlag)
			{//已经是锁定状态
				//非控件解锁
				UnLock_NoGUI(hWnd);
				//控件解锁
				EnableWindow(hRadio_Left, TRUE);
				EnableWindow(hRadio_Right, TRUE);
				EnableWindow(hCombo_Func, TRUE);
				EnableWindow(hText_TL, TRUE);
				EnableWindow(hCombo_HK, TRUE);
				EnableWindow(hRadio_Left, TRUE);
				SendMessage(hBtn_UorE, WM_SETTEXT, 0, L"锁定当前配置");
			}
			else
			{//已经是解锁状态，尝试锁定
				//检查按键选择
				if (SendMessage(hRadio_Left, BM_GETCHECK, 0, 0) == BST_CHECKED)
					LeftorRight = 1;//选择了鼠标左键
				else if (SendMessage(hRadio_Right, BM_GETCHECK, 0, 0) == BST_CHECKED)
					LeftorRight = 0;//选择了鼠标右键
				else
				{
					MessageBox(hWnd, L"左右键未选择！", L"无法锁定", MB_ICONERROR);
					break;
				}
				//检查模拟方式选择
				if ((Func = SendMessage(hCombo_Func, CB_GETCURSEL, 0, 0)) == -1)
				{
					MessageBox(hWnd, L"模拟方式未选择！", L"无法锁定", MB_ICONERROR);
					break;
				}
				//检查时间间隔
				GetWindowText(hText_TL, str_TL, 19);//获取时间间隔字符串
				if ((dig_TL = IsPosDigitStr(str_TL)) == 0)
				{
					MessageBox(hWnd, L"未输入合法的时间间隔！", L"无法锁定", MB_ICONERROR);
					break;
				}
				//检查热键设置
				if ((HK_Index = SendMessage(hCombo_HK, CB_GETCURSEL, 0, 0)) == -1)
				{
					MessageBox(hWnd, L"热键未选择！", L"无法锁定", MB_ICONERROR);
					break;
				}
				//基本检查完成，热键初始化检查
				if (!RegisterHotKey(hWnd, HotKey, NULL, VK_F1 + HK_Index))
				{
					MessageBox(hWnd, L"热键注册失败，请尝试其他热键！", L"无法锁定", MB_ICONERROR);
					break;
				}
				//检查成功，锁定配置
				EnableWindow(hRadio_Left, FALSE);
				EnableWindow(hRadio_Right, FALSE);
				EnableWindow(hCombo_Func, FALSE);
				EnableWindow(hText_TL, FALSE);
				EnableWindow(hCombo_HK, FALSE);
				SendMessage(hBtn_UorE, WM_SETTEXT, 0, L"解锁当前配置");
				LockFlag = 1;//标记锁定
			}
			break;
		default:
			break;
		}
		break;
	case WM_DESTROY:
		if (LockFlag)
			UnLock_NoGUI(hWnd);//注销热键，结束线程
		DeleteObject(hFont);//销毁字体
		PostQuitMessage(0);
		break;
	case WM_HOTKEY:
		if (ClickRunning)
		{//正在连点，销毁连点线程
			TerminateThread(hClickThread, 0);
			ClickRunning = 0;
		}
		else
		{//不在连点，创建连点线程
			hClickThread = CreateThread(NULL, 0, ClickRunner, NULL, 0, NULL);
			ClickRunning = 1;
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam); 
	}
	return 0;
}
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine,int nCmdShow)
{
	MSG msg;
	WNDCLASSEX WC;//窗体类
	HWND hwnd;//主窗体

	WC.cbSize = sizeof(WNDCLASSEX);
	WC.style = CS_HREDRAW | CS_VREDRAW;
	WC.lpfnWndProc = CtlProc;
	WC.cbClsExtra = 0;
	WC.cbWndExtra = 0;
	WC.hInstance = hInstance;
	WC.hIcon = 0;
	WC.hCursor = 0;
	WC.hbrBackground = (HBRUSH)GetSysColorBrush(COLOR_BTNFACE);
	WC.lpszMenuName = 0;
	WC.lpszClassName = L"WND";
	WC.hIconSm = 0;

	RegisterClassEx(&WC);
	hwnd = CreateWindow(L"WND", L"ClickRun鼠标连点器", WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, 0, 0, 356, 310, NULL, 0, 0, 0);
	ShowWindow(hwnd, 1);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}