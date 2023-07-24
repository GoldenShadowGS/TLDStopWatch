#pragma once
#include "PCH.h"

enum HighLightedElement
{
	HIGHLIGHTED_NONE = -1,
	HIGHLIGHTED_ADD = 0,
	HIGHLIGHTED_MINUS = 1,
	HIGHLIGHTED_RESET = 2,
	HIGHLIGHTED_CLOSE = 3,
	HIGHLIGHTED_BORDER,
	HIGHLIGHTED_MINUTEHAND,
	HIGHLIGHTED_ALARM,
	HIGHLIGHTED_CLOCKFACE,
	HIGHLIGHTED_ADJUST_TIMER
};

enum GrabbedElement
{
	GRABBED_NONE = -1,
	GRABBED_ADD = 0,
	GRABBED_MINUS = 1,
	GRABBED_RESET = 2,
	GRABBED_CLOSE = 3,
	GRABBED_BORDER,
	GRABBED_MINUTEHAND,
	GRABBED_ALARM,
	GRABBED_CLOCKFACE,
	GRABBED_ADJUST_TIMER
};

struct UI_ButtonPosition
{
	float x = 0.0f;
	float y = 0.0f;
};

struct ClockInfo
{
	//hands
	float centerX = 0.0f;
	float centerY = 0.0f;
	float minuteAngle = 0.0f;
	float hourAngle = 0.0f;
	float minAngleRad = 0.0f;
	float secAngleRad = 0.0f;
	float alarmAngleRad = 0.0f;
	float angleOffset = 0.0f;
	float adjustTimeRad = 0.0f;
	float adjustTimeMinutes = 0.0f;
	int wraps = 0;
	float shadowoffsetx = 5.0f;
	float shadowoffsety = 6.0f;
	float InnerRadius = 0.0f;
	float OuterRadius = 0.0f;
	float UI_Scale = 0.0f;
	UI_ButtonPosition buttonpos[4];
	float ButtonHighlightRadius = 0.0f;
	float ButtonBackGroundRadius = 0.0f;
	HighLightedElement highlightedElement = HIGHLIGHTED_NONE;
	GrabbedElement grabbedElement = GRABBED_NONE;
	BOOL RedrawBackGround = TRUE;
	BOOL Timing = FALSE;
	BOOL AlarmSet = FALSE;
	WCHAR TimeText[64] = {};
	WCHAR AlarmText[64] = {};
	WCHAR AdjustText[64] = {};
};