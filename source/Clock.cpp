#include "PCH.h"
#include "Clock.h"

ClockApp::ClockApp()
{}

BOOL ClockApp::Init(HINSTANCE hInstance, int nCmdShow, int clientradius)
{
	hInst = hInstance;

	RegisterClockWindowClass(hInstance);

	DWORD style = WS_POPUP;
	DWORD exstyle = WS_EX_LAYERED;
	m_ClientSize = clientradius;
	int WindowSize = m_ClientSize + 10; // adding a buffer of 10 pixels
	RECT winrect = { 0, 0, WindowSize, WindowSize };
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
	m_ClientRectF.right = (float)m_WindowWidth;
	m_ClientRectF.top = 0.0f;
	m_ClientRectF.bottom = (float)m_WindowHeight;

	m_Clockinfo.OuterRadius = m_ClientSize / 2.0f;
	m_Clockinfo.InnerRadius = m_Clockinfo.OuterRadius * 0.85f;
	m_Clockinfo.minuteAngle = 0.0f;
	m_Clockinfo.hourAngle = 0.0f;
	m_Clockinfo.centerX = float(m_WindowWidth) / 2.0f;
	m_Clockinfo.centerY = float(m_WindowHeight) / 2.0f;
	float scale = m_Clockinfo.OuterRadius / 200.0f;
	m_Clockinfo.UI_Scale = scale * 0.8f;
	m_Clockinfo.shadowoffsetx *= scale;
	m_Clockinfo.shadowoffsety *= scale;
	m_Clockinfo.angleOffset = -HalfPI;

	//Set UI button positions
	float radius = m_Clockinfo.OuterRadius * 1.15f;
	m_Clockinfo.buttonpos[HIGHLIGHTED_CLOSE].x = cos(-QuarterPI) * radius + m_Clockinfo.centerX;
	m_Clockinfo.buttonpos[HIGHLIGHTED_CLOSE].y = sin(-QuarterPI) * radius + m_Clockinfo.centerY;

	m_Clockinfo.buttonpos[HIGHLIGHTED_RESET].x = cos(QuarterPI * -3.0f) * radius + m_Clockinfo.centerX;
	m_Clockinfo.buttonpos[HIGHLIGHTED_RESET].y = sin(QuarterPI * -3.0f) * radius + m_Clockinfo.centerY;

	m_Clockinfo.buttonpos[HIGHLIGHTED_ADD].x = cos(QuarterPI) * radius + m_Clockinfo.centerX;
	m_Clockinfo.buttonpos[HIGHLIGHTED_ADD].y = sin(QuarterPI) * radius + m_Clockinfo.centerY;

	m_Clockinfo.buttonpos[HIGHLIGHTED_MINUS].x = cos(QuarterPI * 3.0f) * radius + m_Clockinfo.centerX;
	m_Clockinfo.buttonpos[HIGHLIGHTED_MINUS].y = sin(QuarterPI * 3.0f) * radius + m_Clockinfo.centerY;
	m_Clockinfo.ButtonHighlightRadius = 36.0f * m_Clockinfo.UI_Scale;
	m_Clockinfo.ButtonBackGroundRadius = 36.0f * m_Clockinfo.UI_Scale * 1.25f;

	outerRatio = m_Clockinfo.OuterRadius / m_Clockinfo.InnerRadius;

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
	m_Clockinfo.minAngleRad = angleNormalize(MinutesToAngleRads(m_Timer.getGameMinutes()));
	m_Clockinfo.alarmAngleRad = angleNormalize(MinutesToAngleRads(m_Timer.getAlarmMinutes()));
	m_Clockinfo.secAngleRad = angleNormalize(MinutesToAngleRads(m_Timer.getGameSeconds()));
	m_Clockinfo.minuteAngle = Rad2Deg(m_Clockinfo.minAngleRad);
	m_Clockinfo.hourAngle = HoursToAngle(m_Timer.getGameHours());

	float startmin = m_Timer.getGameMinutes();
	float endmin = m_Timer.getAlarmMinutes();
	float startminclip = fmod(startmin, 60.0f);
	float endminclip = fmod(endmin, 60.0f);
	float minutesRange = endmin - startmin;
	m_Clockinfo.wraps = int(minutesRange / 60.0f);
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
	static GrabbedElement PrevGrabbed = GRABBED_NONE;
	if (m_Clockinfo.grabbedElement != PrevGrabbed)
	{
		if (PrevGrabbed == GRABBED_ALARM || m_Clockinfo.grabbedElement == GRABBED_ALARM)
		{
			m_soundManager.Play(Click1, 0.1f);
		}
		else if (PrevGrabbed == GRABBED_MINUTEHAND || m_Clockinfo.grabbedElement == GRABBED_MINUTEHAND)
		{
			m_soundManager.Play(Click2, 0.1f);
		}
		PrevGrabbed = m_Clockinfo.grabbedElement;
	}
}

void ClockApp::update()
{
	std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
	INT64 duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - LastMouseMovedTime).count();
	MouseTimedOut = (duration > 1500);
	if (MouseTimedOut)
	{
		m_Clockinfo.highlightedElement = HIGHLIGHTED_NONE;
	}
	if (m_Timer.isTiming() && !m_Timer.isSuspended())
	{
		m_Timer.update();
		// Don't play Alarm sound if time or alarm is being adjusted
		if (AlarmSoundSet && m_Timer.getGameMinutes() >= m_Timer.getAlarmMinutes())
		{
			if (m_Clockinfo.grabbedElement != GRABBED_ALARM)
				m_soundManager.Play(Alarm, 1.0f);
			AlarmSoundSet = FALSE;
			m_Timer.ResetAlarm();
		}
	}
	setClockInfo();
	SoundUpdate();
	GenerateText();
}

void ClockApp::GenerateText()
{
	int days = (int)m_Timer.getGameDays();
	int hours = (int)m_Timer.getGameHours() % 24;
	int minutes = (int)m_Timer.getGameMinutes() % 60;
	if (hours && days)
		swprintf_s(m_Clockinfo.TimeText, 64, L"%dd\n%dh %dm", days, hours, minutes);
	else if (hours)
		swprintf_s(m_Clockinfo.TimeText, 64, L"\n%dh %dm", hours, minutes);
	else if (days)
		swprintf_s(m_Clockinfo.TimeText, 64, L"%dd\n%dm", days, minutes);
	else
		swprintf_s(m_Clockinfo.TimeText, 64, L"\n%dm", minutes);
	int adjustminutes = (int)round(m_Clockinfo.adjustTimeMinutes);
	swprintf_s(m_Clockinfo.AdjustText, 64, L"%dm", adjustminutes);

	int alarmMinutes = ((int)m_Timer.getAlarmRemainingMinutes()) % 60;
	int alarmHours = (int)m_Timer.getAlarmRemainingHours();
	if (alarmHours)
		swprintf_s(m_Clockinfo.AlarmText, 64, L"%dh %dm", alarmHours, alarmMinutes);
	else
		swprintf_s(m_Clockinfo.AlarmText, 64, L"%dm", alarmMinutes);
}

void ClockApp::AdjustTimer()
{
	m_Clockinfo.adjustTimeMinutes = RoundAngleToMinute(m_MouseAngle);
	m_Clockinfo.adjustTimeRad = MinuteToAngle(m_Clockinfo.adjustTimeMinutes);
}

void ClockApp::MouseHoverElement(HighLightedElement element)
{
	HoveredElement = element;
	if (m_Clockinfo.grabbedElement != GRABBED_NONE)
		m_Clockinfo.highlightedElement = (HighLightedElement)m_Clockinfo.grabbedElement;
	else
		m_Clockinfo.highlightedElement = element;
}

void ClockApp::MouseGrab(GrabbedElement element)
{
	m_Clockinfo.grabbedElement = element;
}

void ClockApp::MouseMoved(HWND hWnd, LPARAM lParam)
{
	LastMouseMovedTime = std::chrono::steady_clock::now();
	if (!MouseinWindow)
	{
		TRACKMOUSEEVENT mouseEvent = { sizeof(TRACKMOUSEEVENT),TME_LEAVE,hMainWindow, HOVER_DEFAULT };
		TrackMouseEvent(&mouseEvent);
		MouseinWindow = TRUE;
	}
	m_Input.mousepos.X = (float)GET_X_LPARAM(lParam);
	m_Input.mousepos.Y = (float)GET_Y_LPARAM(lParam);
	m_MouseAngle = getMouseAngle(m_Input.mousepos, m_Clockinfo.centerX, m_Clockinfo.centerY, m_Clockinfo.angleOffset);

	float radialDist = GetPointDist(m_Input.mousepos.X, m_Input.mousepos.Y, m_Clockinfo.centerX, m_Clockinfo.centerY);
	float MouseRadialRatio = radialDist / m_Clockinfo.InnerRadius;

	// Find which Element the mouse is on
	if (MouseRadialRatio > 1.0f && MouseRadialRatio < outerRatio)
	{
		float AdjustAngularRange = GetAngleDistance(m_MouseAngle, m_Clockinfo.adjustTimeRad);
		if (AdjustAngularRange * MouseRadialRatio < 0.1f)
			MouseHoverElement(HIGHLIGHTED_ADJUST_TIMER);
		else
			MouseHoverElement(HIGHLIGHTED_BORDER);
	}
	else if (MouseRadialRatio < 1.0f)
	{
		// MinuteHand, Alarm and ClockFace
		// Gets the wedge slice of the angle and multiplies it with cursor distance to the center to de-wedge it
		float MinuteAngularRange = GetAngleDistance(m_MouseAngle, m_Clockinfo.minAngleRad);
		float AlarmAngularRange = GetAngleDistance(m_MouseAngle, m_Clockinfo.alarmAngleRad);
		float AlarmRadialRange = 1.0f - ((m_Clockinfo.wraps + 1) / 15.0f);
		if (AlarmAngularRange * MouseRadialRatio < 0.1f && MouseRadialRatio > AlarmRadialRange && MouseRadialRatio < 1.0f)
		{
			MouseHoverElement(HIGHLIGHTED_ALARM);
		}
		else if (MinuteAngularRange * MouseRadialRatio < 0.1f && MouseRadialRatio > 0.07f && MouseRadialRatio < 1.0f)
		{
			MouseHoverElement(HIGHLIGHTED_MINUTEHAND);
		}
		else
		{
			MouseHoverElement(HIGHLIGHTED_CLOCKFACE);
		}
	}
	else if (MouseRadialRatio > outerRatio)
	{
		// Button Tests
		BOOL buttonhit = FALSE;
		for (int i = 0; i < 4; i++)
		{
			float MouseDistButton = GetPointDist(m_Clockinfo.buttonpos[i].x, m_Clockinfo.buttonpos[i].y, m_Input.mousepos.X, m_Input.mousepos.Y);
			if (MouseDistButton < m_Clockinfo.ButtonHighlightRadius)
			{
				buttonhit = TRUE;
				MouseHoverElement((HighLightedElement)i);
				break;
			}
		}
		if (!buttonhit)
			MouseHoverElement(HIGHLIGHTED_NONE);
	}

	POINT currentpos;
	GetCursorPos(&currentpos);
	int x = currentpos.x - lastLocation.x;
	int y = currentpos.y - lastLocation.y;
	switch (m_Clockinfo.grabbedElement)
	{
	case GRABBED_BORDER:
		MoveWindow(hWnd, x, y, m_WindowWidth, m_WindowHeight, false);
		break;
	case GRABBED_ADJUST_TIMER:
		AdjustTimer();
		break;
	case GRABBED_MINUTEHAND:
		m_Timer.AdjustTime(m_MouseAngle, m_Clockinfo.grabbedElement);
		AlarmSoundSet = (m_Timer.getGameMinutes() < m_Timer.getAlarmMinutes());
		break;
	case GRABBED_ALARM:
		m_Timer.AdjustAlarmTime(m_MouseAngle, m_Clockinfo.grabbedElement);
		AlarmSoundSet = (m_Timer.getGameMinutes() < m_Timer.getAlarmMinutes());
		break;
	}
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
	case WM_MOUSELEAVE:
	{
		m_Clockinfo.grabbedElement = GRABBED_NONE;
		MouseHoverElement(HIGHLIGHTED_NONE);
		MouseinWindow = FALSE;
		m_Clockinfo.RedrawBackGround = TRUE;
	}
	break;
	case WM_MOUSEMOVE:
	{
		MouseMoved(hWnd, lParam);
	}
	break;
	case WM_LBUTTONDOWN:
	{
		SetCapture(hWnd);
		// Adjusting Time
		if (m_Clockinfo.highlightedElement == HIGHLIGHTED_MINUTEHAND)
		{
			m_Timer.SuspendTimer();
			m_Timer.AdjustTime(m_MouseAngle, m_Clockinfo.grabbedElement);
			AlarmSoundSet = (m_Timer.getGameMinutes() < m_Timer.getAlarmMinutes());
		}
		else if (m_Clockinfo.highlightedElement == HIGHLIGHTED_ADJUST_TIMER)
		{
			m_Clockinfo.grabbedElement = GRABBED_ADJUST_TIMER;
		}
		else if (m_Clockinfo.highlightedElement == HIGHLIGHTED_BORDER) // Dragging Window if on Clock Border
		{
			GetCursorPos(&lastLocation);
			RECT rect;
			GetWindowRect(hWnd, &rect);
			lastLocation.x = lastLocation.x - rect.left;
			lastLocation.y = lastLocation.y - rect.top;
		}
		if (m_Clockinfo.highlightedElement != HIGHLIGHTED_ALARM)
			m_Clockinfo.grabbedElement = (GrabbedElement)m_Clockinfo.highlightedElement;
	}
	break;
	case WM_LBUTTONUP:
	{
		ReleaseCapture();
		if (m_Timer.isSuspended())
		{
			m_Timer.ResumeTimer();
		}
		if (m_Clockinfo.grabbedElement == HoveredElement)
		{
			switch (HoveredElement)
			{
			case GRABBED_CLOCKFACE:
				ToggleTimer();
				break;
			case GRABBED_RESET:
				m_Timer.ResetTimer();
				AlarmSoundSet = FALSE;
				if (m_Timer.isTiming())
				{
					m_Timer.StopTimer();
					m_Clockinfo.Timing = FALSE;
					m_Clockinfo.RedrawBackGround = TRUE;
				}
				m_soundManager.Play(StopClick, 0.75f);
				break;
			case GRABBED_CLOSE:
				SendMessage(hWnd, WM_CLOSE, 0, 0); // Close the App
				break;
			case GRABBED_ADD:
				m_Timer.AddMinutes(m_Clockinfo.adjustTimeMinutes);
				AlarmSoundSet = (m_Timer.getGameMinutes() < m_Timer.getAlarmMinutes());
				break;
			case GRABBED_MINUS:
				m_Timer.AddMinutes(-m_Clockinfo.adjustTimeMinutes);
				AlarmSoundSet = (m_Timer.getGameMinutes() < m_Timer.getAlarmMinutes());
				break;
			}
		}
		m_Clockinfo.grabbedElement = GRABBED_NONE;
	}
	break;
	case WM_MBUTTONDOWN:
	{
	}
	break;
	case WM_MBUTTONUP:
	{
	}
	break;
	case WM_RBUTTONDOWN:
	{
		switch (HoveredElement)
		{
		case HIGHLIGHTED_ALARM:
			SetCapture(hWnd);
			m_Clockinfo.grabbedElement = GRABBED_ALARM;
			break;
		case HIGHLIGHTED_MINUTEHAND:
			if (!AlarmSoundSet)
			{
				SetCapture(hWnd);
				m_Clockinfo.grabbedElement = GRABBED_ALARM;
			}
		}
	}
	break;
	case WM_RBUTTONUP:
	{
		ReleaseCapture();
		m_Clockinfo.grabbedElement = GRABBED_NONE;
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