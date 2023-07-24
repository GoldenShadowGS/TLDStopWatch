#pragma once
#include "PCH.h"
#include "Renderer.h"
#include "Time.h"
#include "Input.h"
#include "Math.h"
#include "Sound.h"
#include "Elements.h"

class ClockApp
{
public:
	ClockApp();
	BOOL Init(HINSTANCE hInstance, int nCmdShow, int clientradius);
	ATOM RegisterClockWindowClass(HINSTANCE hInstance);
	void update();
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK ClockWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
private:
	void setClockInfo();
	void ToggleTimer();
	void SoundUpdate();
	void MouseMoved(HWND hWnd, LPARAM lParam);
	void AdjustTimer();
	void MouseHoverElement(HighLightedElement element);
	void MouseGrab(GrabbedElement element);
	void GenerateText();
	SoundManager m_soundManager;
	Renderer m_Renderer;
	Timer m_Timer;
	ClockInfo m_Clockinfo = {};
	const WCHAR* m_Title = L"TLD StopWatch";
	const WCHAR* m_WindowClass = L"TLDTimerWindowsClass";
	HWND hMainWindow = nullptr;
	HINSTANCE hInst = {};
	Input m_Input = {};
	float m_MouseAngle = 0.0f;
	D2D1_RECT_F m_ClientRectF = {};
	INT m_WindowWidth = 0;
	INT m_WindowHeight = 0;
	INT m_ClientSize = 0;

	WavFile Alarm;
	WavFile StopClick;
	WavFile StartClick;
	WavFile Click1;
	WavFile Click2;
	POINT lastLocation = {};
	float outerRatio = 0.0f;
	HighLightedElement HoveredElement = HIGHLIGHTED_NONE;
	BOOL MouseinWindow = FALSE;
	BOOL MouseTimedOut = FALSE;
	BOOL AlarmSoundSet = FALSE;
	std::chrono::time_point<std::chrono::steady_clock> LastMouseMovedTime;
};