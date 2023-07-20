#pragma once
#include "PCH.h"
#include "Math.h"

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
	float getGameHours();
	float AngleToMinutes(float angle);
	void AdjustTime(float MouseAnglerads);
	void AdjustAlarmTime(float MouseAnglerads);
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