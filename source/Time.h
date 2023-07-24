#pragma once
#include "PCH.h"
#include "Math.h"
#include "Elements.h"

inline float MinutesToAngle(float minutes)
{
	return (fmod(minutes, 60.0f) / 60.0f) * 360;
}

inline float MinutesToAngleRads(float minutes)
{
	return (fmod(minutes, 60.0f) / 60.0f) * PI2;
}

inline float HoursToAngle(float hours)
{
	return (fmod(hours, 12.0f) / 12.0f) * 360;
}

inline float HoursToAngleRads(float hours)
{
	return (fmod(hours, 12.0f) / 12.0f) * PI2;
}

inline float RoundAngleToMinute(float anglerads)
{
	const float normalizedangle = anglerads / PI2;
	return round(normalizedangle * 60.0f);
}

inline float MinuteToAngle(float minutes)
{
	return (minutes / 60.0f) * PI2;
}

class Timer
{
public:
	Timer();
	void StartTimer();
	void StopTimer();
	void ResumeTimer();
	void SuspendTimer();
	void ResetTimer();
	void update();
	float getAlarmMinutes();
	float getAlarmHours();
	float getAlarmRemainingMinutes();
	float getAlarmRemainingHours();
	float getGameMinutes();
	float getGameSeconds();
	float getGameHours();
	float getGameDays();
	void AddMinutes(float minutes);
	float AngleToMinutes(float angle);
	void AdjustTime(float MouseAnglerads, GrabbedElement& grabbed);
	void AdjustAlarmTime(float MouseAnglerads, GrabbedElement& grabbed);
	void ResetAlarm() { m_AlarmMilliSeconds = 0; }
	BOOL isTiming() { return Timing; }
	BOOL isSuspended() { return Suspended; }
private:
	INT64 MinutesToMilliseconds(float minutes);
	std::chrono::time_point<std::chrono::steady_clock> start;
	INT64 m_CurrentMilliSecondsDuration = 0;
	INT64 m_AccumulatedMilliSecondsDuration = 0;
	INT64 m_AlarmMilliSeconds = 0;
	BOOL Timing = FALSE;
	BOOL Suspended = FALSE;
};