#include "PCH.h"
#include "Clock.h"
#include "Time.h"

ClockApp::ClockApp()
{}
// TODO Different Background numbers for 60 and 12
BOOL ClockApp::Init(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;

	RegisterClockWindowClass(hInstance);

	DWORD style = WS_POPUP; // WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX    // WS_POPUP
	DWORD exstyle = WS_EX_LAYERED; // WS_EX_LAYERED
	m_ClientSize = 300;  // 250
	RECT winrect = { 0, 0, m_ClientSize, m_ClientSize };
	AdjustWindowRectEx(&winrect, style, false, exstyle);

	m_WindowWidth = winrect.right - winrect.left;
	m_WindowHeight = winrect.bottom - winrect.top;

	hMainWindow = CreateWindowExW(exstyle, m_WindowClass, m_Title, style,
		CW_USEDEFAULT, 0, m_WindowWidth, m_WindowHeight, nullptr, nullptr, hInstance, nullptr);

	if (!hMainWindow)
		return FALSE;

	SetWindowLongPtrW(hMainWindow, GWLP_USERDATA, (INT64)this);
	SetLayeredWindowAttributes(hMainWindow, RGB(0, 0, 0), 255, LWA_COLORKEY);
	SetWindowPos(hMainWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	m_ClientRectF.left = 0.0f;
	m_ClientRectF.right = (float)m_ClientSize;
	m_ClientRectF.top = 0.0f;
	m_ClientRectF.bottom = (float)m_ClientSize;


	m_Clockinfo.OuterRadius = m_ClientSize / 2.0f;
	m_Clockinfo.InnerRadius = m_Clockinfo.OuterRadius * 0.9f;
	m_Clockinfo.minuteAngle = 270;
	m_Clockinfo.hourAngle = 270;
	m_Clockinfo.centerX = float(m_ClientSize) / 2.0f;
	m_Clockinfo.centerY = float(m_ClientSize) / 2.0f;
	float scale = m_Clockinfo.OuterRadius / 200.0f;
	m_Clockinfo.shadowoffsetx *= scale;
	m_Clockinfo.shadowoffsety *= scale;

	if (!m_Renderer.Init(hMainWindow, m_Clockinfo, m_ClientRectF))
		return FALSE;

	if (FAILED(m_soundManager.Init(16)))
		return FALSE;

	// Load Sounds from executable
	Alarm.Load(IDSOUND_ALARM);
	StartClick.Load(IDSOUND_GOCLICK);
	StopClick.Load(IDSOUND_STOPCLICK);
	Click1.Load(IDSOUND_CLICK1);
	Click2.Load(IDSOUND_CLICK2);

	ShowWindow(hMainWindow, nCmdShow);
	UpdateWindow(hMainWindow);

	return TRUE;
}

ATOM ClockApp::RegisterClockWindowClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex = {};

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = ClockApp::WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_STOPWATCHICON));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wcex.lpszMenuName = 0;
	wcex.lpszClassName = m_WindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALLICON));

	return RegisterClassExW(&wcex);
}

void ClockApp::setClockInfo()
{
	float startmin = m_Timer.getGameMinutes();
	float endmin = m_Timer.getAlarmMinutes();
	m_Clockinfo.minuteAngle = MinutesToAngle(startmin);
	m_Clockinfo.hourAngle = MinutesToAngle(m_Timer.getGameHours());
	m_Clockinfo.minAngleRad = fmod(Deg2Rad(m_Clockinfo.minuteAngle + 270.0f), PI2);
	m_Clockinfo.alarmAngleRad = fmod(Deg2Rad(MinutesToAngle(m_Timer.getAlarmMinutes()) + 270.0f), PI2);
	m_Clockinfo.secAngleRad = fmod(Deg2Rad(MinutesToAngle(m_Timer.getGameSeconds()) + 270.0f), PI2);

	float startminclip = fmod(startmin, 60.0f);
	float endminclip = fmod(endmin, 60.0f);
	float minutesRange = endmin - startmin;
	m_Clockinfo.wraps = int(minutesRange / 60.0f);
	m_Clockinfo.rangeAngleRad = angleNormalize(MinutesToAngleRads(endminclip - startminclip));
	m_Clockinfo.AlarmSet = (endmin > startmin);
}

void ClockApp::ToggleTimer()
{
	if (!m_Timer.isTiming())
	{
		m_Timer.StartTimer();
		m_Clockinfo.Timing = TRUE;
		m_Clockinfo.RedrawBackGround = TRUE;
		m_soundManager.Play(StartClick, 0.6f);
	}
	else
	{
		m_Timer.StopTimer();
		m_Clockinfo.Timing = FALSE;
		m_Clockinfo.RedrawBackGround = TRUE;
		m_soundManager.Play(StopClick, 0.75f);
	}
}

void ClockApp::SoundUpdate()
{
	static BOOL AlarmHandleGrabbedPrev = FALSE;
	static BOOL MinuteHandleGrabbedPrev = FALSE;
	if (AlarmHandleGrabbedPrev != AlarmHandleGrabbed)
	{
		AlarmHandleGrabbedPrev = AlarmHandleGrabbed;
		m_soundManager.Play(Click1, 0.1f);
	}
	if (MinuteHandleGrabbedPrev != MinuteHandleGrabbed)
	{
		MinuteHandleGrabbedPrev = MinuteHandleGrabbed;
		m_soundManager.Play(Click2, 0.1f);
	}
}

void ClockApp::update()
{
	float radialDist = GetPointDist(m_Input.mousepos.X, m_Input.mousepos.Y, m_Clockinfo.centerX, m_Clockinfo.centerY);
	MouseRadialRatio = radialDist / m_Clockinfo.InnerRadius;

	float angleDist = GetAngleDistance(m_MouseAngle, m_Clockinfo.minAngleRad);
	m_Clockinfo.HandHover = ((!AlarmHandleGrabbed && angleDist * MouseRadialRatio < 0.1f && MouseRadialRatio > 0.07f && MouseRadialRatio < 1.0f) || MinuteHandleGrabbed) ? TRUE : FALSE;

	float alarmAngleDist = GetAngleDistance(m_MouseAngle, m_Clockinfo.alarmAngleRad);
	m_Clockinfo.AlarmHover = ((!MinuteHandleGrabbed && alarmAngleDist * MouseRadialRatio < 0.1f && MouseRadialRatio > 0.07f) || AlarmHandleGrabbed) ? TRUE : FALSE;

	static BOOL AlarmSet = FALSE;
	if (m_Timer.isTiming() && !m_Timer.isSuspended())
	{
		m_Timer.update();
		if (m_Timer.getGameMinutes() >= m_Timer.getAlarmMinutes() && AlarmSet && !AlarmHandleGrabbed && !MinuteHandleGrabbed)
		{
			m_soundManager.Play(Alarm, 1.0f);
			AlarmSet = FALSE;
			m_Timer.ResetAlarm();
		}
	}
	if (AlarmHandleGrabbed)
	{
		m_Timer.AdjustAlarmTime(m_MouseAngle, AlarmHandleGrabbed);
		if (m_Timer.getAlarmMinutes() > m_Timer.getGameMinutes())
			AlarmSet = TRUE;
		else
			AlarmSet = FALSE;
	}
	if (MinuteHandleGrabbed)
	{
		m_Timer.AdjustTime(m_MouseAngle, MinuteHandleGrabbed);
		if (m_Timer.getAlarmMinutes() > m_Timer.getGameMinutes())
			AlarmSet = TRUE;
		else
			AlarmSet = FALSE;
	}

	setClockInfo();
	SoundUpdate();
	//Debug Title Text
	std::wstring title = L"TLD StopWatch " + std::to_wstring((int)m_Timer.getGameHours()) + L"h " + std::to_wstring((int)m_Timer.getGameMinutes() % 60) + L"m " +
		L"Alarm: " + std::to_wstring((int)m_Timer.getAlarmMinutes()) + L"m " + std::to_wstring(m_Timer.getGameSeconds());
	SetWindowTextW(hMainWindow, title.c_str());
}

LRESULT CALLBACK ClockApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ClockApp* clockapp = reinterpret_cast<ClockApp*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	if (clockapp)
		return clockapp->ClockWndProc(hWnd, message, wParam, lParam);
	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK ClockApp::ClockWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
	{
		update();
		m_Renderer.Render(m_Clockinfo);
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
		m_Input.mousepos.X = (float)GET_X_LPARAM(lParam);
		m_Input.mousepos.Y = (float)GET_Y_LPARAM(lParam);
		m_MouseAngle = fmod(getMouseAngle(m_Input.mousepos, m_Clockinfo.centerX, m_Clockinfo.centerY) + PI3_4, PI2);

		if (MouseRadialRatio > 1.0f)
		{
			if (!m_Clockinfo.BorderHover)
			{
				m_Clockinfo.BorderHover = TRUE;
				m_Clockinfo.RedrawBackGround = TRUE;
			}
		}
		else
		{
			if (m_Clockinfo.BorderHover)
			{
				m_Clockinfo.BorderHover = FALSE;
				m_Clockinfo.RedrawBackGround = TRUE;
			}
		}
		if (dragging)
		{
			POINT currentpos;
			GetCursorPos(&currentpos);
			int x = currentpos.x - lastLocation.x;
			int y = currentpos.y - lastLocation.y;
			MoveWindow(hWnd, x, y, m_WindowWidth, m_WindowHeight, false);
		}
	}
	break;
	case WM_LBUTTONDOWN:
	{
		// Adjusting Time
		SetCapture(hWnd);
		if (m_Clockinfo.HandHover)
		{
			m_Timer.SuspendTimer();
			MinuteHandleGrabbed = TRUE;
			AlarmHandleGrabbed = FALSE;
		}
		else if (MouseRadialRatio > 1.0f) // Dragging Window if on Clock Border
		{
			dragging = TRUE;
			GetCursorPos(&lastLocation);
			RECT rect;
			GetWindowRect(hWnd, &rect);
			lastLocation.x = lastLocation.x - rect.left;
			lastLocation.y = lastLocation.y - rect.top;
		}
		else // Toggle StopWatch timer
		{
			ToggleTimer();
		}
		//mousedownTime = std::chrono::steady_clock::now();
	}
	break;
	case WM_LBUTTONUP:
	{
		ReleaseCapture();
		if (m_Timer.isSuspended())
		{
			m_Timer.ResumeTimer();
		}
		MinuteHandleGrabbed = FALSE;
		dragging = FALSE;

		//std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
		//INT64 duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - mousedownTime).count();
		//if (duration < 250)
		//	ToggleTimer();

		// Close the App
		//if (m_Input.mousepos.X > m_ClientSize - 50 && m_Input.mousepos.Y < 50)
		//	SendMessage(hWnd, WM_CLOSE, 0, 0);
	}
	break;
	case WM_MBUTTONDOWN:
	{
		//ToggleTimer();


		//AlarmHandleGrabbed = (GetAngleDistance(m_MouseAngle, m_Clockinfo.alarmAngleRad) < 0.4f) ? TRUE : FALSE;
		//if (AlarmHandleGrabbed)
		//	SetCapture(hWnd);
		//MinuteHandleGrabbed = FALSE;
	}
	break;
	case WM_MBUTTONUP:
	{
		//ReleaseCapture();
		//AlarmHandleGrabbed = FALSE;
	}
	break;
	case WM_RBUTTONDOWN:
	{
		if (m_Clockinfo.AlarmHover)   // = (GetAngleDistance(m_MouseAngle, m_Clockinfo.alarmAngleRad) < 0.4f) ? TRUE : FALSE;
		{
			SetCapture(hWnd);
			AlarmHandleGrabbed = TRUE;
			MinuteHandleGrabbed = FALSE;
		}
		//if (m_Clockinfo.HandHover)
		//	//	MinuteHandleGrabbed = (GetAngleDistance(m_MouseAngle, m_Clockinfo.minAngleRad) < 0.4f) ? TRUE : FALSE;
		//	//if (MinuteHandleGrabbed)
		//{
		//	MinuteHandleGrabbed = TRUE;
		//	m_Timer.SuspendTimer();
		//	SetCapture(hWnd);
		//}
		//AlarmHandleGrabbed = FALSE;
	}
	break;
	case WM_RBUTTONUP:
	{
		ReleaseCapture();
		AlarmHandleGrabbed = FALSE;
		//ReleaseCapture();
		//if (m_Timer.isSuspended())
		//{
		//	m_Timer.ResumeTimer();
		//}
		//MinuteHandleGrabbed = FALSE;
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