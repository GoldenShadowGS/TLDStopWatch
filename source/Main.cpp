#include "PCH.h"
#include "Renderer.h"
#include "Time.h"
#include "Input.h"
#include "Math.h"
#include "Sound.h"

const WCHAR* szTitle = L"TLD StopWatch";
const WCHAR* szWindowClass = L"TLDTimerWindowsClass";
HWND hMainWindow = nullptr;
HINSTANCE hInst;
Renderer g_Renderer;
Timer g_Timer;
ClockInfo clockinfo = {};
Input g_Input = {};
SoundManager g_soundManager;
float g_MouseAngle = 0.0f;

// Sounds
WavFile Alarm;
WavFile StopClick;
WavFile StartClick;
WavFile Click1;
WavFile Click2;

//Grab Handles
BOOL MinuteHandleGrabbed = FALSE;
BOOL AlarmHandleGrabbed = FALSE;

D2D1_RECT_F g_ClientRect = {};

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
ATOM MyRegisterClass(HINSTANCE hInstance);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
		return FALSE;

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	hMainWindow = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
		CW_USEDEFAULT, 0, 400, 400, nullptr, nullptr, hInstance, nullptr);
	if (!hMainWindow)
		return FALSE;

	RECT winrect;
	GetClientRect(hMainWindow, &winrect);

	g_ClientRect.left = (float)winrect.left;
	g_ClientRect.right = (float)winrect.right;
	g_ClientRect.top = (float)winrect.top;
	g_ClientRect.bottom = (float)winrect.bottom;

	clockinfo.scale = 0.75f;
	clockinfo.minuteAngle = 270;
	clockinfo.hourAngle = 270;
	clockinfo.centerX = float(winrect.right - winrect.left) / 2.0f;
	clockinfo.centerY = float(winrect.bottom - winrect.top) / 2.0f;

	if (!g_Renderer.Init(hMainWindow, clockinfo))
		return FALSE;

	if (FAILED(g_soundManager.Init(16)))
		return FALSE;


	// Load Sounds
	Alarm.Load(IDSOUND_ALARM);
	StartClick.Load(IDSOUND_GOCLICK);
	StopClick.Load(IDSOUND_STOPCLICK);
	Click1.Load(IDSOUND_CLICK1);
	Click2.Load(IDSOUND_CLICK2);

	ShowWindow(hMainWindow, nCmdShow);
	UpdateWindow(hMainWindow);

	return TRUE;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex = {};

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = 0;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wcex.lpszMenuName = 0;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = 0;

	return RegisterClassExW(&wcex);
}

void setClockInfo()
{
	float startmin = g_Timer.getGameMinutes();
	float endmin = g_Timer.getAlarmMinutes();
	clockinfo.minuteAngle = MinutesToAngle(startmin);
	clockinfo.hourAngle = MinutesToAngle(g_Timer.getGameHours());
	clockinfo.minAngleRad = fmod(Deg2Rad(clockinfo.minuteAngle + 270.0f), PI2);
	clockinfo.alarmAngleRad = fmod(Deg2Rad(MinutesToAngle(g_Timer.getAlarmMinutes()) + 270.0f), PI2);

	float startminclip = fmod(startmin, 60.0f);
	float endminclip = fmod(endmin, 60.0f);
	float minutesRange = endmin - startmin;
	clockinfo.wraps = int(minutesRange / 60.0f);
	clockinfo.rangeAngleRad = angleNormalize(MinutesToAngleRads(endminclip - startminclip));
	clockinfo.AlarmSet = (endmin > startmin);
}

void ToggleTimer()
{
	if (!g_Timer.isTiming())
	{
		g_Timer.StartTimer();
		g_soundManager.Play(StartClick, 0.6f);
	}
	else
	{
		g_Timer.StopTimer();
		g_soundManager.Play(StopClick, 0.6f);
	}
}

float GetAngleDistance(float angle1, float angle2)
{
	float dist = abs(angle1 - angle2);
	if (dist > PI)
		dist = PI2 - dist;
	return dist;
}

void SoundUpdate()
{
	static BOOL AlarmHandleGrabbedPrev = FALSE;
	static BOOL MinuteHandleGrabbedPrev = FALSE;
	if (AlarmHandleGrabbedPrev != AlarmHandleGrabbed)
	{
		AlarmHandleGrabbedPrev = AlarmHandleGrabbed;
		g_soundManager.Play(Click1, 0.1f);
	}
	if (MinuteHandleGrabbedPrev != MinuteHandleGrabbed)
	{
		MinuteHandleGrabbedPrev = MinuteHandleGrabbed;
		g_soundManager.Play(Click2, 0.1f);
	}
}

void update()
{
	static BOOL AlarmSet = FALSE;
	if (g_Timer.isTiming() && !g_Timer.isSuspended())
	{
		g_Timer.update();
		if (g_Timer.getGameMinutes() >= g_Timer.getAlarmMinutes() && AlarmSet && !AlarmHandleGrabbed && !MinuteHandleGrabbed)
		{
			g_soundManager.Play(Alarm, 1.0f);
			AlarmSet = FALSE;
			g_Timer.ResetAlarm();
		}
	}
	if (AlarmHandleGrabbed)
	{
		g_Timer.AdjustAlarmTime(g_MouseAngle);
		if (g_Timer.getAlarmMinutes() > g_Timer.getGameMinutes())
			AlarmSet = TRUE;
		else
			AlarmSet = FALSE;
	}
	if (MinuteHandleGrabbed)
	{
		g_Timer.AdjustTime(g_MouseAngle);
		if (g_Timer.getAlarmMinutes() > g_Timer.getGameMinutes())
			AlarmSet = TRUE;
		else
			AlarmSet = FALSE;
	}

	setClockInfo();
	SoundUpdate();
	//Debug Title Text
	std::wstring title = L"TLD StopWatch " + std::to_wstring((int)g_Timer.getGameHours()) + L"h " + std::to_wstring((int)g_Timer.getGameMinutes()%60) + L"m";
	SetWindowTextW(hMainWindow, title.c_str());
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
	{
		update();
		g_Renderer.Render(clockinfo);
		return 0;
	}
	break;
	case WM_KILLFOCUS:
	{
		AlarmHandleGrabbed = FALSE;
		MinuteHandleGrabbed = FALSE;
	}
	break;
	case WM_MOUSEMOVE:
	{
		g_Input.mousepos.X = (float)GET_X_LPARAM(lParam);
		g_Input.mousepos.Y = (float)GET_Y_LPARAM(lParam);
		g_MouseAngle = fmod(getMouseAngle(g_Input.mousepos, clockinfo.centerX, clockinfo.centerY) + PI3_4, PI2);
	}
	break;
	case WM_LBUTTONDOWN:
	{
		ToggleTimer();
	}
	break;
	case WM_LBUTTONUP:
	{
	}
	break;
	case WM_RBUTTONDOWN:
	{
		AlarmHandleGrabbed = (GetAngleDistance(g_MouseAngle, clockinfo.alarmAngleRad) < 0.4f) ? TRUE : FALSE;
		if (AlarmHandleGrabbed)
			SetCapture(hWnd);
		MinuteHandleGrabbed = FALSE;
	}
	break;
	case WM_RBUTTONUP:
	{
		ReleaseCapture();
		AlarmHandleGrabbed = FALSE;
	}
	break;
	case WM_MBUTTONDOWN:
	{
		MinuteHandleGrabbed = (GetAngleDistance(g_MouseAngle, clockinfo.minAngleRad) < 0.4f) ? TRUE : FALSE;
		if (MinuteHandleGrabbed)
		{
			g_Timer.SuspendTimer();
			SetCapture(hWnd);
		}
		AlarmHandleGrabbed = FALSE;
	}
	break;
	case WM_MBUTTONUP:
	{
		ReleaseCapture();
		if (g_Timer.isSuspended())
		{
			g_Timer.ResumeTimer();
		}
		MinuteHandleGrabbed = FALSE;
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}