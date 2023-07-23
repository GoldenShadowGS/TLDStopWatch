#pragma once
#include "PCH.h"
#include "Renderer.h"
#include "Time.h"
#include "Input.h"
#include "Math.h"
#include "Sound.h"

class ClockApp
{
public:
	ClockApp();
	BOOL Init(HINSTANCE hInstance, int nCmdShow);
	ATOM RegisterClockWindowClass(HINSTANCE hInstance);
	void update();
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK ClockWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
private:
	void setClockInfo();
	void ToggleTimer();
	void SoundUpdate();
	SoundManager m_soundManager;
	Renderer m_Renderer;
	Timer m_Timer;
	ClockInfo m_Clockinfo = {};
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
	const WCHAR* m_Title = L"TLD StopWatch";
	const WCHAR* m_WindowClass = L"TLDTimerWindowsClass";
	//BOOL MinuteHandleGrabbed = FALSE;
	//BOOL AlarmHandleGrabbed = FALSE;

	// Window Dragging Variables
	BOOL dragging = FALSE;
	POINT lastLocation = {};
	float MouseRadialRatio = 0.0f;
	float outerRatio = 0.0f;
	BOOL MouseinWindow = FALSE;
	BOOL CanStartTimer = TRUE;
	BOOL MouseTimeout = FALSE;
	std::chrono::time_point<std::chrono::steady_clock> LastMouseMovedTime;
};