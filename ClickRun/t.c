#include<Windows.h>
#include<stdio.h>

//�ɽ����Ŀؼ�ID
#define ID_BtnSel			3301//������Ҽ�����ѡ��
#define ID_FuncSel			3302//ģ�ⷽʽѡ��
#define ID_TLSet			3303//ʱ��������
#define	ID_HKSet			3304//�ȼ�����
#define ID_UorELock			3305//����/��������
#define ID_URL				3306//��ҳ��ַ

//�ȼ�ID
#define HotKey				1011

INT LockFlag = 0;//��־�����Ƿ�����
INT LeftorRight = 0;//1����� 0���Ҽ�
INT Func = 0;//0��mouse_event 1��SendInput 2��WinIO
TCHAR str_TL[20] = { 0 };//�ַ�����ʽ��ʱ����
INT dig_TL = 0;//������ʽ��ʱ����
TCHAR str_HKList[12][4] = { L"F1",L"F2",L"F3",L"F4",L"F5",L"F6",L"F7",L"F8",L"F9",L"F10",L"F11",L"F12" };//��ѡ�ȼ��б�
INT HK_Index = 0;//ѡ�����ȼ����

INT ClickRunning = 0;//��������Ƿ����ڽ���

HANDLE hClickThread = NULL;//�����߳̾��

//������ǰ���ã���GUI���֣�
INT UnLock_NoGUI(HWND thishwnd)
{
	//���������̣߳������ڣ�
	if (ClickRunning)
		TerminateThread(hClickThread, 0);
	//ע���ȼ�
	UnregisterHotKey(thishwnd, HotKey);
	LockFlag = 0;//��ǽ���

	return 0;
}
//����ִ���ߣ�ѭ����������㣬���ٲ���Ҫ���ظ��ж�
DWORD WINAPI ClickRunner(LPVOID lpParam)
{
	INPUT Down = { 0 }, Up = { 0 };//SendInputʹ�ã����¡�̧��

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
//���������ַ����򷵻�����ֵ�����򷵻�0
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
			return 0;//���Ǵ������ַ���
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

	static HWND hFont;			//ͳһ����

	static HWND hTip1;			//������Ҽ�ѡ���ı�
	static HWND hRadio_Left;	//��������ť
	static HWND hRadio_Right;	//����Ҽ���ť

	static HWND hTip2;			//ģ�ⷽʽѡ���ı�
	static HWND hCombo_Func;	//��ʽѡ���

	static HWND hTip3;			//ʱ�����ı�
	static HWND hText_TL;		//ʱ���������

	static HWND hTip4;			//�ȼ�������ʾ�ı�
	static HWND hCombo_HK;		//�ȼ�����ѡ���

	static HWND hBtn_UorE;		//����������ť

	static HWND hSepLine;		//�ָ���
	static HWND hURL;			//���ı������ı���

	switch (message)
	{
	case WM_CREATE:
		hFont = CreateFont(-14, -7, 0, 0, 400, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, TEXT("΢���ź�"));
		hTip1 = CreateWindow(L"Static", L"����ѡ��", WS_CHILD | WS_VISIBLE, 15, 10, 300, 100, hWnd, NULL, hWnd, 0);
		hRadio_Left = CreateWindow(L"Button", L"������", WS_CHILD | WS_VISIBLE | BS_LEFT | BS_AUTORADIOBUTTON, 60, 35, 80, 20, hWnd, (HMENU)ID_BtnSel, hWnd, 0);
		hRadio_Right = CreateWindow(L"Button", L"����Ҽ�", WS_CHILD | WS_VISIBLE | BS_LEFT | BS_AUTORADIOBUTTON, 200, 35, 80, 20, hWnd, (HMENU)ID_BtnSel, hWnd, 0);
		hTip2 = CreateWindow(L"Static", L"ģ�ⷽʽѡ��", WS_CHILD | WS_VISIBLE, 15, 60, 300, 100, hWnd, NULL, hWnd, 0);
		hCombo_Func = CreateWindow(L"ComboBox", L"", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 40, 85, 280, 100, hWnd, (HMENU)ID_FuncSel, hWnd, 0);
		hTip3 = CreateWindow(L"Static", L"ʱ����(ms)��", WS_CHILD | WS_VISIBLE, 15, 120, 300, 100, hWnd, NULL, hWnd, 0);
		hText_TL = CreateWindow(L"Edit", L"500", ES_CENTER | WS_CHILD | WS_VISIBLE, 30, 145, 80, 25, hWnd, (HMENU)ID_TLSet, hWnd, 0);
		hTip4 = CreateWindow(L"Static", L"�ȼ����ã�", WS_CHILD | WS_VISIBLE, 210, 120, 300, 100, hWnd, NULL, hWnd, 0);
		hCombo_HK = CreateWindow(L"ComboBox", L"", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 225, 145, 80, 1000, hWnd, (HMENU)ID_HKSet, hWnd, 0);
		hBtn_UorE = CreateWindow(L"Button", L"������ǰ����", ES_CENTER | WS_CHILD | WS_VISIBLE, 25, 180, 290, 50, hWnd, (HMENU)ID_UorELock, hWnd, 0);
		hSepLine = CreateWindow(L"Static", L"", SS_ETCHEDHORZ | WS_CHILD | WS_VISIBLE, 5, 240, 332, 10, hWnd, NULL, hWnd, 0);
		hURL = CreateWindow(L"Edit", L"�����ҳ��injectrl.coding.me/ClickRun", ES_READONLY | WM_NOTIFY | ES_CENTER | WS_CHILD | WS_VISIBLE, 23, 247, 300, 20, hWnd, (HMENU)ID_URL, hWnd, 0);

		//���ø��ؼ�����
		SendMessage(hTip1, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hRadio_Left, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hRadio_Right, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hTip2, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hTip3, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hTip4, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hText_TL, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hBtn_UorE, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hURL, WM_SETFONT, (WPARAM)hFont, NULL);
		//��ģ�ⷽʽComboBox���ѡ��
		SendMessage(hCombo_Func, CB_ADDSTRING, 0, L"1 - mouse_event");
		SendMessage(hCombo_Func, CB_ADDSTRING, 0, L"2 - SendInput");
		//���ȼ�����ComboBox���ѡ��
		for (i = 0; i < 12; i++)
			SendMessage(hCombo_HK, CB_ADDSTRING, 0, str_HKList[i]);
		
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_URL:
			if (HIWORD(wParam) == WM_KEYDOWN)
				ShellExecute(NULL, L"open", L"http://injectrl.coding.me/ClickRun", NULL, NULL, SW_SHOWNORMAL);//����ҳ
			break;
		case ID_UorELock://����������/����
			if (LockFlag)
			{//�Ѿ�������״̬
				//�ǿؼ�����
				UnLock_NoGUI(hWnd);
				//�ؼ�����
				EnableWindow(hRadio_Left, TRUE);
				EnableWindow(hRadio_Right, TRUE);
				EnableWindow(hCombo_Func, TRUE);
				EnableWindow(hText_TL, TRUE);
				EnableWindow(hCombo_HK, TRUE);
				EnableWindow(hRadio_Left, TRUE);
				SendMessage(hBtn_UorE, WM_SETTEXT, 0, L"������ǰ����");
			}
			else
			{//�Ѿ��ǽ���״̬����������
				//��鰴��ѡ��
				if (SendMessage(hRadio_Left, BM_GETCHECK, 0, 0) == BST_CHECKED)
					LeftorRight = 1;//ѡ����������
				else if (SendMessage(hRadio_Right, BM_GETCHECK, 0, 0) == BST_CHECKED)
					LeftorRight = 0;//ѡ��������Ҽ�
				else
				{
					MessageBox(hWnd, L"���Ҽ�δѡ��", L"�޷�����", MB_ICONERROR);
					break;
				}
				//���ģ�ⷽʽѡ��
				if ((Func = SendMessage(hCombo_Func, CB_GETCURSEL, 0, 0)) == -1)
				{
					MessageBox(hWnd, L"ģ�ⷽʽδѡ��", L"�޷�����", MB_ICONERROR);
					break;
				}
				//���ʱ����
				GetWindowText(hText_TL, str_TL, 19);//��ȡʱ�����ַ���
				if ((dig_TL = IsPosDigitStr(str_TL)) == 0)
				{
					MessageBox(hWnd, L"δ����Ϸ���ʱ������", L"�޷�����", MB_ICONERROR);
					break;
				}
				//����ȼ�����
				if ((HK_Index = SendMessage(hCombo_HK, CB_GETCURSEL, 0, 0)) == -1)
				{
					MessageBox(hWnd, L"�ȼ�δѡ��", L"�޷�����", MB_ICONERROR);
					break;
				}
				//���������ɣ��ȼ���ʼ�����
				if (!RegisterHotKey(hWnd, HotKey, NULL, VK_F1 + HK_Index))
				{
					MessageBox(hWnd, L"�ȼ�ע��ʧ�ܣ��볢�������ȼ���", L"�޷�����", MB_ICONERROR);
					break;
				}
				//���ɹ�����������
				EnableWindow(hRadio_Left, FALSE);
				EnableWindow(hRadio_Right, FALSE);
				EnableWindow(hCombo_Func, FALSE);
				EnableWindow(hText_TL, FALSE);
				EnableWindow(hCombo_HK, FALSE);
				SendMessage(hBtn_UorE, WM_SETTEXT, 0, L"������ǰ����");
				LockFlag = 1;//�������
			}
			break;
		default:
			break;
		}
		break;
	case WM_DESTROY:
		if (LockFlag)
			UnLock_NoGUI(hWnd);//ע���ȼ��������߳�
		DeleteObject(hFont);//��������
		PostQuitMessage(0);
		break;
	case WM_HOTKEY:
		if (ClickRunning)
		{//�������㣬���������߳�
			TerminateThread(hClickThread, 0);
			ClickRunning = 0;
		}
		else
		{//�������㣬���������߳�
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
	WNDCLASSEX WC;//������
	HWND hwnd;//������

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
	hwnd = CreateWindow(L"WND", L"ClickRun���������", WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, 0, 0, 356, 310, NULL, 0, 0, 0);
	ShowWindow(hwnd, 1);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}