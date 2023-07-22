#include "PCH.h"
#include "Time.h"
#include "Math.h"
#include "Sound.h"

//extern BOOL MinuteHandleGrabbed;
//extern BOOL AlarmHandleGrabbed;

constexpr float TimeFactor = 5000.0f;
constexpr INT64 I64TimeFactor = (INT64)TimeFactor;
constexpr float InvTimeFactor = 1.0f / TimeFactor;

inline INT64 RoundToMinute(INT64 ms)
{
	INT64 floored = (ms / I64TimeFactor) * I64TimeFactor;
	if ((ms - floored) > (I64TimeFactor / 2))
		return floored + I64TimeFactor;
	else
		return floored;
}

Timer::Timer()
{}

void Timer::StartTimer()
{
	Timing = TRUE;
	start = std::chrono::steady_clock::now();
}

void Timer::StopTimer()
{
	Timing = FALSE;
	m_AccumulatedMilliSecondsDuration += m_CurrentMilliSecondsDuration;
	m_CurrentMilliSecondsDuration = 0;
	start = std::chrono::steady_clock::now();
}

void Timer::ResumeTimer()
{
	Suspended = FALSE;
	start = std::chrono::steady_clock::now();
}

void Timer::SuspendTimer()
{
	Suspended = TRUE;
	m_AccumulatedMilliSecondsDuration += m_CurrentMilliSecondsDuration;
	m_CurrentMilliSecondsDuration = 0;
	start = std::chrono::steady_clock::now();
}

void Timer::ResetTimer()
{
	m_CurrentMilliSecondsDuration = 0;
	m_AccumulatedMilliSecondsDuration = 0;
	start = std::chrono::steady_clock::now();
}

void Timer::update()
{
	std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
	m_CurrentMilliSecondsDuration = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
	if (m_AlarmMilliSeconds < m_AccumulatedMilliSecondsDuration + m_CurrentMilliSecondsDuration)
		m_AlarmMilliSeconds = m_AccumulatedMilliSecondsDuration + m_CurrentMilliSecondsDuration;
}

float Timer::getGameMinutes()
{
	return (m_AccumulatedMilliSecondsDuration + m_CurrentMilliSecondsDuration) * InvTimeFactor;
}

float Timer::getGameSeconds()
{
	constexpr float secondsfactor = 60.0f * InvTimeFactor;
	return (m_AccumulatedMilliSecondsDuration + m_CurrentMilliSecondsDuration) * secondsfactor;
}

float Timer::getAlarmMinutes()
{
	return m_AlarmMilliSeconds * InvTimeFactor;
}

float Timer::getAlarmHours()
{
	constexpr float hourfactor = 1.0f / 60.0f * InvTimeFactor;
	return m_AlarmMilliSeconds * hourfactor;
}

float Timer::getAlarmRemainingMinutes()
{
	return getAlarmMinutes() - getGameMinutes();
}

float Timer::getAlarmRemainingHours()
{
	return getAlarmHours() - getGameHours();
}

float Timer::getGameHours()
{
	constexpr float hourfactor = 1.0f / 60.0f * InvTimeFactor;
	return (m_AccumulatedMilliSecondsDuration + m_CurrentMilliSecondsDuration) * hourfactor;
}

float Timer::AngleToMinutes(float angle)
{
	constexpr double conversion = (1.0f / PI2) * 60.0;
	return (float)((double)angle * conversion);
}

INT64 Timer::MinutesToMilliseconds(float minutes)
{
	return INT64((double)minutes * TimeFactor);
}

float GetMinuteDistance(float min1, float min2)
{
	float dist = abs(min1 - min2);
	if (dist > 30.0f)
		dist = 60.0f - dist;
	return dist;
}

void Timer::AdjustTime(float MouseAnglerads, BOOL& grabbed)
{
	INT64 CurrentMS = m_AccumulatedMilliSecondsDuration + m_CurrentMilliSecondsDuration;
	BOOL AlarmOFF = (m_AlarmMilliSeconds <= CurrentMS);
	float currentminutes = fmod(getGameMinutes(), 60.0f);
	float newminutes = fmod(AngleToMinutes(MouseAnglerads + HalfPI), 60.0f);
	float minutesRange = newminutes - currentminutes;
	if (minutesRange > 30.0f)
		minutesRange -= 60.0f;
	if (minutesRange < -30.0f)
		minutesRange += 60.0f;
	m_AccumulatedMilliSecondsDuration += MinutesToMilliseconds(minutesRange);
	m_AccumulatedMilliSecondsDuration = RoundToMinute(m_AccumulatedMilliSecondsDuration);
	CurrentMS = m_AccumulatedMilliSecondsDuration + m_CurrentMilliSecondsDuration;
	if (CurrentMS < 0)
	{
		m_AccumulatedMilliSecondsDuration = -m_CurrentMilliSecondsDuration;
		CurrentMS = m_AccumulatedMilliSecondsDuration + m_CurrentMilliSecondsDuration;
		if (abs(GetMinuteDistance(currentminutes, newminutes)) > 5.0f)
			grabbed = FALSE;
	}
	{
		INT64 LongestAlarmTime = CurrentMS + MinutesToMilliseconds(720);
		if (m_AlarmMilliSeconds > LongestAlarmTime)
		{
			m_AlarmMilliSeconds = LongestAlarmTime;
			m_AlarmMilliSeconds = RoundToMinute(m_AlarmMilliSeconds);
		}
	}
	if (m_AlarmMilliSeconds < CurrentMS || AlarmOFF)
		m_AlarmMilliSeconds = CurrentMS;
}

void Timer::AdjustAlarmTime(float MouseAnglerads, BOOL& grabbed)
{
	float alarmminutes = getAlarmMinutes();
	float timerminute = getGameMinutes();
	BOOL zeroed = FALSE;
	if (alarmminutes <= timerminute)
		zeroed = TRUE;
	float currentminutes = fmod(getAlarmMinutes(), 60.0f);
	float newminutes = fmod(AngleToMinutes(MouseAnglerads + HalfPI), 60.0f);
	float minutesRange = newminutes - currentminutes;
	if (abs(GetMinuteDistance(currentminutes, newminutes)) > 5.0f && zeroed)
		grabbed = FALSE;
	if (minutesRange > 30.0f)
		minutesRange -= 60.0f;
	if (minutesRange < -30.0f)
		minutesRange += 60.0f;
	m_AlarmMilliSeconds += MinutesToMilliseconds(minutesRange);
	INT64 CurrentMS = m_AccumulatedMilliSecondsDuration + m_CurrentMilliSecondsDuration;
	if (m_AlarmMilliSeconds < CurrentMS)
		m_AlarmMilliSeconds = CurrentMS;
	INT64 LongestAlarmTime = CurrentMS + MinutesToMilliseconds(720);
	if (m_AlarmMilliSeconds > LongestAlarmTime)
	{
		m_AlarmMilliSeconds = LongestAlarmTime;
		if (abs(GetMinuteDistance(currentminutes, newminutes)) > 5.0f)
			grabbed = FALSE;
	}
	m_AlarmMilliSeconds = RoundToMinute(m_AlarmMilliSeconds);
}