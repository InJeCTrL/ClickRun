#include<Windows.h>
#include<stdio.h>

// 可交互的控件ID
#define ID_BtnSel			3301	// 鼠标左右键按键选择
#define ID_FuncSel			3302	// 模拟方式选择
#define ID_TLSet			3303	// 时间间隔设置
#define	ID_HKSet			3304	// 热键设置
#define ID_UorELock			3305	// 锁定/解锁配置
#define ID_URL				3306	// 主页地址

// 热键ID
#define HotKey				1011

static HWND hFont;			// 统一字体

static HWND hTip1;			// 鼠标左右键选择文本
static HWND hRadio_Left;	// 鼠标左键按钮
static HWND hRadio_Right;	// 鼠标右键按钮

static HWND hTip2;			// 模拟方式选择文本
static HWND hCombo_Func;	// 方式选择框

static HWND hTip3;			// 时间间隔文本
static HWND hText_TL;		// 时间间隔输入框

static HWND hTip4;			// 热键设置提示文本
static HWND hCombo_HK;		// 热键设置选择框

static HWND hBtn_UorE;		// 锁定解锁按钮

static HWND hSepLine;		// 分割线
static HWND hURL;			// 超文本链接文本框

// 触发按键 1：左键 0：右键
INT LeftorRight = 1;
// 模拟点击方式 0：mouse_event 1：SendInput 2：WinIO
INT Func = 0;
// 点击时间间隔(ms)
INT dig_TL = 500;
// 选定的热键编号
INT HK_Index = 0;
// 标志配置是否锁定
BOOL CfgLocked = FALSE;
// 字符串形式的点击时间间隔
TCHAR str_TL[20] = { 0 };
// 标记连点是否正在进行
BOOL ClickRunning = FALSE;
// 连点线程句柄
HANDLE hClickThread = NULL;

// 备选热键列表
const TCHAR str_HKList[12][4] = { 
	TEXT("F1"), TEXT("F2"), TEXT("F3"), TEXT("F4"), 
	TEXT("F5"), TEXT("F6"), TEXT("F7"), TEXT("F8"), 
	TEXT("F9"), TEXT("F10"), TEXT("F11"), TEXT("F12")
};



// 解锁当前配置（无GUI部分）
INT UnLock_NoGUI(HWND thishwnd)
{
	// 结束连点线程（若存在）
	if (ClickRunning)
		TerminateThread(hClickThread, 0);
	// 注销热键
	UnregisterHotKey(thishwnd, HotKey);
	// 标记解锁
	CfgLocked = FALSE;

	return 0;
}

// 获取上一次保存配置的信息或默认配置
// 并刷新界面配置显示
INT FlashConfig()
{
	HKEY hKey = NULL;
	DWORD dwType = REG_DWORD;
	DWORD dwSize = sizeof(INT);

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\ClickRun"), NULL, KEY_READ | KEY_WOW64_64KEY, &hKey) == ERROR_SUCCESS)
	{
		
		RegQueryValueEx(hKey, TEXT("LeftorRight"), NULL, &dwType, (LPBYTE)(&LeftorRight), &dwSize);
		RegQueryValueEx(hKey, TEXT("Func"), NULL, &dwType, (LPBYTE)(&Func), &dwSize);
		RegQueryValueEx(hKey, TEXT("TL"), NULL, &dwType, (LPBYTE)(&dig_TL), &dwSize);
		RegQueryValueEx(hKey, TEXT("HK"), NULL, &dwType, (LPBYTE)(&HK_Index), &dwSize);
	}
	RegCloseKey(hKey);
	if (LeftorRight)
	{
		SendMessage(hRadio_Left, BM_SETCHECK, 1, 0);
	}
	else
	{
		SendMessage(hRadio_Right, BM_SETCHECK, 1, 0);
	}
	SendMessage(hCombo_Func, CB_SETCURSEL, Func, 0);
	SendMessage(hText_TL, WM_SETTEXT, NULL, _itow(dig_TL, str_TL, 10));
	SendMessage(hCombo_HK, CB_SETCURSEL, HK_Index, 0);

	return 0;
}

// 保存当前配置到注册表
INT SaveConfig()
{
	HKEY hKey = NULL;
	DWORD dwType = REG_DWORD;
	DWORD dwSize = sizeof(INT);
	DWORD dwDispositon = 0;

	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\ClickRun"), NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS | KEY_WOW64_64KEY, NULL, &hKey, &dwDispositon) == ERROR_SUCCESS)
	{
		RegSetValueEx(hKey, TEXT("LeftorRight"), NULL, dwType, (LPCBYTE)(&LeftorRight), dwSize);
		RegSetValueEx(hKey, TEXT("Func"), NULL, dwType, (LPCBYTE)(&Func), dwSize);
		RegSetValueEx(hKey, TEXT("TL"), NULL, dwType, (LPCBYTE)(&dig_TL), dwSize);
		RegSetValueEx(hKey, TEXT("HK"), NULL, dwType, (LPCBYTE)(&HK_Index), dwSize);
		RegCloseKey(hKey);
		return 0;
	}
	else
	{
		return 1;
	}
	
}

// 连点执行者，循环放在最里层，减少不必要的重复判断
DWORD WINAPI ClickRunner(LPVOID lpParam)
{
	INPUT Down = { 0 }, Up = { 0 };// SendInput使用（按下、抬起）

	switch (Func)
	{
		// mouse_event
		case 0:
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
		// SendInput
		case 1:
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
// 若是正数字符串则返回正数值，否则返回0
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
			return 0;// 不是纯正数字符串
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

	switch (message)
	{
	case WM_CREATE:
		hFont = CreateFont(-14, -7, 0, 0, 400, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, TEXT("微软雅黑"));
		hTip1 = CreateWindow(TEXT("Static"), TEXT("按键选择："), WS_CHILD | WS_VISIBLE, 15, 10, 300, 100, hWnd, NULL, hWnd, 0);
		hRadio_Left = CreateWindow(TEXT("Button"), TEXT("鼠标左键"), WS_CHILD | WS_VISIBLE | BS_LEFT | BS_AUTORADIOBUTTON, 60, 35, 80, 20, hWnd, (HMENU)ID_BtnSel, hWnd, 0);
		hRadio_Right = CreateWindow(TEXT("Button"), TEXT("鼠标右键"), WS_CHILD | WS_VISIBLE | BS_LEFT | BS_AUTORADIOBUTTON, 200, 35, 80, 20, hWnd, (HMENU)ID_BtnSel, hWnd, 0);
		hTip2 = CreateWindow(TEXT("Static"), TEXT("模拟方式选择："), WS_CHILD | WS_VISIBLE, 15, 60, 300, 100, hWnd, NULL, hWnd, 0);
		hCombo_Func = CreateWindow(TEXT("ComboBox"), TEXT(""), CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 40, 85, 280, 100, hWnd, (HMENU)ID_FuncSel, hWnd, 0);
		hTip3 = CreateWindow(TEXT("Static"), TEXT("时间间隔(ms)："), WS_CHILD | WS_VISIBLE, 15, 120, 300, 100, hWnd, NULL, hWnd, 0);
		hText_TL = CreateWindow(TEXT("Edit"), TEXT("500"), ES_CENTER | WS_CHILD | WS_VISIBLE, 30, 145, 80, 25, hWnd, (HMENU)ID_TLSet, hWnd, 0);
		hTip4 = CreateWindow(TEXT("Static"), TEXT("热键设置："), WS_CHILD | WS_VISIBLE, 210, 120, 300, 100, hWnd, NULL, hWnd, 0);
		hCombo_HK = CreateWindow(TEXT("ComboBox"), TEXT(""), CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 225, 145, 80, 1000, hWnd, (HMENU)ID_HKSet, hWnd, 0);
		hBtn_UorE = CreateWindow(TEXT("Button"), TEXT("锁定当前配置"), ES_CENTER | WS_CHILD | WS_VISIBLE, 25, 180, 290, 50, hWnd, (HMENU)ID_UorELock, hWnd, 0);
		hSepLine = CreateWindow(TEXT("Static"), TEXT(""), SS_ETCHEDHORZ | WS_CHILD | WS_VISIBLE, 5, 240, 332, 10, hWnd, NULL, hWnd, 0);
		hURL = CreateWindow(TEXT("Edit"), TEXT("软件主页：injectrl.github.io/ClickRun"), ES_READONLY | WM_NOTIFY | ES_CENTER | WS_CHILD | WS_VISIBLE, 23, 247, 300, 20, hWnd, (HMENU)ID_URL, hWnd, 0);

		// 设置各控件字体
		SendMessage(hTip1, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hRadio_Left, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hRadio_Right, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hTip2, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hTip3, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hTip4, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hText_TL, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hBtn_UorE, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hURL, WM_SETFONT, (WPARAM)hFont, NULL);
		// 向模拟方式ComboBox添加选项
		SendMessage(hCombo_Func, CB_ADDSTRING, 0, TEXT("1 - mouse_event"));
		SendMessage(hCombo_Func, CB_ADDSTRING, 0, TEXT("2 - SendInput"));
		// 向热键设置ComboBox添加选项
		for (i = 0; i < 12; i++)
		{
			SendMessage(hCombo_HK, CB_ADDSTRING, 0, str_HKList[i]);
		}
		FlashConfig();
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		// 打开主页
		case ID_URL:
			if (HIWORD(wParam) == WM_KEYDOWN)
				ShellExecute(NULL, TEXT("open"), TEXT("https://injectrl.github.io/ClickRun/"), NULL, NULL, SW_SHOWNORMAL);
			break;
		// 单击了锁定/解锁
		case ID_UorELock:
			// 已经是锁定状态
			if (CfgLocked)
			{
				// 非控件解锁
				UnLock_NoGUI(hWnd);
				// 控件解锁
				EnableWindow(hRadio_Left, TRUE);
				EnableWindow(hRadio_Right, TRUE);
				EnableWindow(hCombo_Func, TRUE);
				EnableWindow(hText_TL, TRUE);
				EnableWindow(hCombo_HK, TRUE);
				EnableWindow(hRadio_Left, TRUE);
				SendMessage(hBtn_UorE, WM_SETTEXT, 0, TEXT("锁定当前配置"));
			}
			// 已经是解锁状态，尝试锁定
			else
			{
				// 检查按键选择
				if (SendMessage(hRadio_Left, BM_GETCHECK, 0, 0) == BST_CHECKED)
					LeftorRight = 1;// 选择了鼠标左键
				else if (SendMessage(hRadio_Right, BM_GETCHECK, 0, 0) == BST_CHECKED)
					LeftorRight = 0;// 选择了鼠标右键
				else
				{
					MessageBox(hWnd, TEXT("左右键未选择！"), TEXT("无法锁定"), MB_ICONERROR);
					break;
				}
				// 检查模拟方式选择
				if ((Func = SendMessage(hCombo_Func, CB_GETCURSEL, 0, 0)) == -1)
				{
					MessageBox(hWnd, TEXT("模拟方式未选择！"), TEXT("无法锁定"), MB_ICONERROR);
					break;
				}
				// 检查时间间隔
				GetWindowText(hText_TL, str_TL, 19);// 获取时间间隔字符串
				if ((dig_TL = IsPosDigitStr(str_TL)) == 0)
				{
					MessageBox(hWnd, TEXT("未输入合法的时间间隔！"), TEXT("无法锁定"), MB_ICONERROR);
					break;
				}
				// 检查热键设置
				if ((HK_Index = SendMessage(hCombo_HK, CB_GETCURSEL, 0, 0)) == -1)
				{
					MessageBox(hWnd, TEXT("热键未选择！"), TEXT("无法锁定"), MB_ICONERROR);
					break;
				}
				// 基本检查完成，热键初始化检查
				if (!RegisterHotKey(hWnd, HotKey, NULL, VK_F1 + HK_Index))
				{
					MessageBox(hWnd, TEXT("热键注册失败，请尝试其他热键！"), TEXT("无法锁定"), MB_ICONERROR);
					break;
				}
				// 检查成功，锁定配置
				EnableWindow(hRadio_Left, FALSE);
				EnableWindow(hRadio_Right, FALSE);
				EnableWindow(hCombo_Func, FALSE);
				EnableWindow(hText_TL, FALSE);
				EnableWindow(hCombo_HK, FALSE);
				SendMessage(hBtn_UorE, WM_SETTEXT, 0, TEXT("(配置已生效)解锁当前配置"));
				CfgLocked = TRUE;// 标记锁定
				SaveConfig();
			}
			break;
		default:
			break;
		}
		break;
	case WM_DESTROY:
		if (CfgLocked)
			UnLock_NoGUI(hWnd);// 注销热键，结束线程
		DeleteObject(hFont);// 销毁字体
		PostQuitMessage(0);
		break;
	case WM_HOTKEY:
		// 正在连点，销毁连点线程
		if (ClickRunning)
		{
			TerminateThread(hClickThread, 0);
			ClickRunning = FALSE;
		}
		// 不在连点，创建连点线程
		else
		{
			hClickThread = CreateThread(NULL, 0, ClickRunner, NULL, 0, NULL);
			ClickRunning = TRUE;
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
	WNDCLASSEX WC;// 窗体类
	HWND hwnd;// 主窗体
	INT width = GetSystemMetrics(SM_CXSCREEN);
	INT height = GetSystemMetrics(SM_CYSCREEN);

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
	WC.lpszClassName = TEXT("WND");
	WC.hIconSm = 0;

	RegisterClassEx(&WC);
	hwnd = CreateWindow(TEXT("WND"), TEXT("ClickRun鼠标连点器"), WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, width / 2 - 178, height / 2 - 155, 356, 310, NULL, 0, 0, 0);
	ShowWindow(hwnd, 1);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}