#include<Windows.h>
#include<stdio.h>

// �ɽ����Ŀؼ�ID
#define ID_BtnSel			3301	// ������Ҽ�����ѡ��
#define ID_FuncSel			3302	// ģ�ⷽʽѡ��
#define ID_TLSet			3303	// ʱ��������
#define	ID_HKSet			3304	// �ȼ�����
#define ID_UorELock			3305	// ����/��������
#define ID_URL				3306	// ��ҳ��ַ

// �ȼ�ID
#define HotKey				1011

static HWND hFont;			// ͳһ����

static HWND hTip1;			// ������Ҽ�ѡ���ı�
static HWND hRadio_Left;	// ��������ť
static HWND hRadio_Right;	// ����Ҽ���ť

static HWND hTip2;			// ģ�ⷽʽѡ���ı�
static HWND hCombo_Func;	// ��ʽѡ���

static HWND hTip3;			// ʱ�����ı�
static HWND hText_TL;		// ʱ���������

static HWND hTip4;			// �ȼ�������ʾ�ı�
static HWND hCombo_HK;		// �ȼ�����ѡ���

static HWND hBtn_UorE;		// ����������ť

static HWND hSepLine;		// �ָ���
static HWND hURL;			// ���ı������ı���

// �������� 1����� 0���Ҽ�
INT LeftorRight = 1;
// ģ������ʽ 0��mouse_event 1��SendInput 2��WinIO
INT Func = 0;
// ���ʱ����(ms)
INT dig_TL = 500;
// ѡ�����ȼ����
INT HK_Index = 0;
// ��־�����Ƿ�����
BOOL CfgLocked = FALSE;
// �ַ�����ʽ�ĵ��ʱ����
TCHAR str_TL[20] = { 0 };
// ��������Ƿ����ڽ���
BOOL ClickRunning = FALSE;
// �����߳̾��
HANDLE hClickThread = NULL;

// ��ѡ�ȼ��б�
const TCHAR str_HKList[12][4] = { 
	TEXT("F1"), TEXT("F2"), TEXT("F3"), TEXT("F4"), 
	TEXT("F5"), TEXT("F6"), TEXT("F7"), TEXT("F8"), 
	TEXT("F9"), TEXT("F10"), TEXT("F11"), TEXT("F12")
};



// ������ǰ���ã���GUI���֣�
INT UnLock_NoGUI(HWND thishwnd)
{
	// ���������̣߳������ڣ�
	if (ClickRunning)
		TerminateThread(hClickThread, 0);
	// ע���ȼ�
	UnregisterHotKey(thishwnd, HotKey);
	// ��ǽ���
	CfgLocked = FALSE;

	return 0;
}

// ��ȡ��һ�α������õ���Ϣ��Ĭ������
// ��ˢ�½���������ʾ
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

// ���浱ǰ���õ�ע���
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

// ����ִ���ߣ�ѭ����������㣬���ٲ���Ҫ���ظ��ж�
DWORD WINAPI ClickRunner(LPVOID lpParam)
{
	INPUT Down = { 0 }, Up = { 0 };// SendInputʹ�ã����¡�̧��

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
// ���������ַ����򷵻�����ֵ�����򷵻�0
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
			return 0;// ���Ǵ������ַ���
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
		hFont = CreateFont(-14, -7, 0, 0, 400, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, TEXT("΢���ź�"));
		hTip1 = CreateWindow(TEXT("Static"), TEXT("����ѡ��"), WS_CHILD | WS_VISIBLE, 15, 10, 300, 100, hWnd, NULL, hWnd, 0);
		hRadio_Left = CreateWindow(TEXT("Button"), TEXT("������"), WS_CHILD | WS_VISIBLE | BS_LEFT | BS_AUTORADIOBUTTON, 60, 35, 80, 20, hWnd, (HMENU)ID_BtnSel, hWnd, 0);
		hRadio_Right = CreateWindow(TEXT("Button"), TEXT("����Ҽ�"), WS_CHILD | WS_VISIBLE | BS_LEFT | BS_AUTORADIOBUTTON, 200, 35, 80, 20, hWnd, (HMENU)ID_BtnSel, hWnd, 0);
		hTip2 = CreateWindow(TEXT("Static"), TEXT("ģ�ⷽʽѡ��"), WS_CHILD | WS_VISIBLE, 15, 60, 300, 100, hWnd, NULL, hWnd, 0);
		hCombo_Func = CreateWindow(TEXT("ComboBox"), TEXT(""), CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 40, 85, 280, 100, hWnd, (HMENU)ID_FuncSel, hWnd, 0);
		hTip3 = CreateWindow(TEXT("Static"), TEXT("ʱ����(ms)��"), WS_CHILD | WS_VISIBLE, 15, 120, 300, 100, hWnd, NULL, hWnd, 0);
		hText_TL = CreateWindow(TEXT("Edit"), TEXT("500"), ES_CENTER | WS_CHILD | WS_VISIBLE, 30, 145, 80, 25, hWnd, (HMENU)ID_TLSet, hWnd, 0);
		hTip4 = CreateWindow(TEXT("Static"), TEXT("�ȼ����ã�"), WS_CHILD | WS_VISIBLE, 210, 120, 300, 100, hWnd, NULL, hWnd, 0);
		hCombo_HK = CreateWindow(TEXT("ComboBox"), TEXT(""), CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 225, 145, 80, 1000, hWnd, (HMENU)ID_HKSet, hWnd, 0);
		hBtn_UorE = CreateWindow(TEXT("Button"), TEXT("������ǰ����"), ES_CENTER | WS_CHILD | WS_VISIBLE, 25, 180, 290, 50, hWnd, (HMENU)ID_UorELock, hWnd, 0);
		hSepLine = CreateWindow(TEXT("Static"), TEXT(""), SS_ETCHEDHORZ | WS_CHILD | WS_VISIBLE, 5, 240, 332, 10, hWnd, NULL, hWnd, 0);
		hURL = CreateWindow(TEXT("Edit"), TEXT("�����ҳ��injectrl.github.io/ClickRun"), ES_READONLY | WM_NOTIFY | ES_CENTER | WS_CHILD | WS_VISIBLE, 23, 247, 300, 20, hWnd, (HMENU)ID_URL, hWnd, 0);

		// ���ø��ؼ�����
		SendMessage(hTip1, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hRadio_Left, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hRadio_Right, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hTip2, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hTip3, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hTip4, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hText_TL, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hBtn_UorE, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hURL, WM_SETFONT, (WPARAM)hFont, NULL);
		// ��ģ�ⷽʽComboBox���ѡ��
		SendMessage(hCombo_Func, CB_ADDSTRING, 0, TEXT("1 - mouse_event"));
		SendMessage(hCombo_Func, CB_ADDSTRING, 0, TEXT("2 - SendInput"));
		// ���ȼ�����ComboBox���ѡ��
		for (i = 0; i < 12; i++)
		{
			SendMessage(hCombo_HK, CB_ADDSTRING, 0, str_HKList[i]);
		}
		FlashConfig();
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		// ����ҳ
		case ID_URL:
			if (HIWORD(wParam) == WM_KEYDOWN)
				ShellExecute(NULL, TEXT("open"), TEXT("https://injectrl.github.io/ClickRun/"), NULL, NULL, SW_SHOWNORMAL);
			break;
		// ����������/����
		case ID_UorELock:
			// �Ѿ�������״̬
			if (CfgLocked)
			{
				// �ǿؼ�����
				UnLock_NoGUI(hWnd);
				// �ؼ�����
				EnableWindow(hRadio_Left, TRUE);
				EnableWindow(hRadio_Right, TRUE);
				EnableWindow(hCombo_Func, TRUE);
				EnableWindow(hText_TL, TRUE);
				EnableWindow(hCombo_HK, TRUE);
				EnableWindow(hRadio_Left, TRUE);
				SendMessage(hBtn_UorE, WM_SETTEXT, 0, TEXT("������ǰ����"));
			}
			// �Ѿ��ǽ���״̬����������
			else
			{
				// ��鰴��ѡ��
				if (SendMessage(hRadio_Left, BM_GETCHECK, 0, 0) == BST_CHECKED)
					LeftorRight = 1;// ѡ����������
				else if (SendMessage(hRadio_Right, BM_GETCHECK, 0, 0) == BST_CHECKED)
					LeftorRight = 0;// ѡ��������Ҽ�
				else
				{
					MessageBox(hWnd, TEXT("���Ҽ�δѡ��"), TEXT("�޷�����"), MB_ICONERROR);
					break;
				}
				// ���ģ�ⷽʽѡ��
				if ((Func = SendMessage(hCombo_Func, CB_GETCURSEL, 0, 0)) == -1)
				{
					MessageBox(hWnd, TEXT("ģ�ⷽʽδѡ��"), TEXT("�޷�����"), MB_ICONERROR);
					break;
				}
				// ���ʱ����
				GetWindowText(hText_TL, str_TL, 19);// ��ȡʱ�����ַ���
				if ((dig_TL = IsPosDigitStr(str_TL)) == 0)
				{
					MessageBox(hWnd, TEXT("δ����Ϸ���ʱ������"), TEXT("�޷�����"), MB_ICONERROR);
					break;
				}
				// ����ȼ�����
				if ((HK_Index = SendMessage(hCombo_HK, CB_GETCURSEL, 0, 0)) == -1)
				{
					MessageBox(hWnd, TEXT("�ȼ�δѡ��"), TEXT("�޷�����"), MB_ICONERROR);
					break;
				}
				// ���������ɣ��ȼ���ʼ�����
				if (!RegisterHotKey(hWnd, HotKey, NULL, VK_F1 + HK_Index))
				{
					MessageBox(hWnd, TEXT("�ȼ�ע��ʧ�ܣ��볢�������ȼ���"), TEXT("�޷�����"), MB_ICONERROR);
					break;
				}
				// ���ɹ�����������
				EnableWindow(hRadio_Left, FALSE);
				EnableWindow(hRadio_Right, FALSE);
				EnableWindow(hCombo_Func, FALSE);
				EnableWindow(hText_TL, FALSE);
				EnableWindow(hCombo_HK, FALSE);
				SendMessage(hBtn_UorE, WM_SETTEXT, 0, TEXT("(��������Ч)������ǰ����"));
				CfgLocked = TRUE;// �������
				SaveConfig();
			}
			break;
		default:
			break;
		}
		break;
	case WM_DESTROY:
		if (CfgLocked)
			UnLock_NoGUI(hWnd);// ע���ȼ��������߳�
		DeleteObject(hFont);// ��������
		PostQuitMessage(0);
		break;
	case WM_HOTKEY:
		// �������㣬���������߳�
		if (ClickRunning)
		{
			TerminateThread(hClickThread, 0);
			ClickRunning = FALSE;
		}
		// �������㣬���������߳�
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
	WNDCLASSEX WC;// ������
	HWND hwnd;// ������
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
	hwnd = CreateWindow(TEXT("WND"), TEXT("ClickRun���������"), WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, width / 2 - 178, height / 2 - 155, 356, 310, NULL, 0, 0, 0);
	ShowWindow(hwnd, 1);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}