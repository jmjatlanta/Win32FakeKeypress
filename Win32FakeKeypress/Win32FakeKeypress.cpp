/*
 * Win32FakeKeypress.cpp : If this application has focus, prevent the PC from going idle
 * by introducing a keystroke on a timer.
*/

#include "stdafx.h"
#include "Win32FakeKeypress.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
HWND hWnd;
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions and classes included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

bool endLoop = false;
HANDLE threadHandle;

/**
 * used to wrap the HWND in a way that can be passed between threads
 */
struct thread_data
{
	HWND windowHandle;
	// constructor
	thread_data(HWND w) : windowHandle(w) {};
};

/**
 * the loop for throwing keystrokes based on a timer
 *
 * @param lpParameter the wrapper around the window handle
 * @returns always returns 0
 */
DWORD WINAPI doSeparateThreadLoop(LPVOID lpParameter)
{
	thread_data *data = (thread_data*)lpParameter;
	HWND myWindowHandle = data->windowHandle;
	INPUT input;

	input.type = INPUT_KEYBOARD;
	input.ki.wScan = 0;
	input.ki.time = 0;
	input.ki.dwExtraInfo = 0;

	INPUT input2;

	input2.type = INPUT_KEYBOARD;
	input2.ki.wScan = 0;
	input2.ki.time = 0;
	input2.ki.dwExtraInfo = 0;
	input2.ki.dwFlags = KEYEVENTF_KEYUP;

	while (!endLoop)
	{
		HWND fg = GetForegroundWindow();
		if (fg == myWindowHandle)
		{
			SendInput(1, &input, sizeof(INPUT));
			SendInput(1, &input2, sizeof(INPUT));
			Sleep(5000);
		}
	}
	return 0;

}

/**
 * create a seperate thread for sending keystrokes into
 * the keyboard buffer
 */
void doKeypressInSeperateThread()
{
	threadHandle = CreateThread(NULL, 0, doSeparateThreadLoop, new thread_data(hWnd), 0, 0);
	return;
}

/**
 * The main() method
 */
int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg; // a place to store the message when one is received
	HACCEL hAccelTable; // the keystroke accelerator table

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WIN32FAKEKEYPRESS, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	// load the keystroke accelerator table
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32FAKEKEYPRESS)); 

	// start the keypress thread
	doKeypressInSeperateThread();

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) // wait for a message
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) // was this not an accelerator keystroke?
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	endLoop = true;
	
	return (int) msg.wParam;
}

/**
 * Build an object about this application
 *
 * @param hInstance this instance of the application
 * @returns the return value of the RegisterClassEx call
 */
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32FAKEKEYPRESS));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_WIN32FAKEKEYPRESS);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

/**
 * save the instance handle in a global variable and create and display the main program window.
 *
 * @param hInstance the instance handle
 * @param nCmdShow ??
 * @return TRUE if window creation was successful
 */
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 640, 480, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

/**
 * Processes messages for the main window.
 *
 * @param hWnd the window handle
 * @param message the message, one of
 *   WM_COMMAND	- process the application menu
 *   WM_PAINT	- Paint the main window
 *   WM_DESTROY	- post a quit message and return
 * @param wParam more parameters from the message
 * @param lParam more parameters from the message
 * @return 0 if message was handled, otherwise a non-zero value
 */
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
		case WM_COMMAND:
			wmId    = LOWORD(wParam);
			wmEvent = HIWORD(wParam);
			// Parse the menu selections:
			switch (wmId)
			{
				case IDM_ABOUT:	// Help|About
					DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
					break;
				case IDM_EXIT: // Exit menu item clicked
					DestroyWindow(hWnd);
					break;
				default:	// do normal processing
					return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code here...
			EndPaint(hWnd, &ps);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

/**
 * message handler for the About box
 *
 * @param hDlg the handle to the dialog box
 * @param message the message coming in
 * @param wParam the parameter
 * @param lParam not used
 * 
 * @returns integer equivalent of TRUE (message was handled) or FALSE (message still needs to be handled)
 */
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
		case WM_INITDIALOG:
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
	}
	return (INT_PTR)FALSE;
}
