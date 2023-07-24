#pragma once
#include "PCH.h"
#include "Math.h"

struct MousePos
{
	float X;
	float Y;
};

struct Input
{
	MousePos mousepos;
};

inline float getMouseAngle(const MousePos& mousepos, float centerx, float centery, float offset)
{
	float x = mousepos.X - centerx;
	float y = mousepos.Y - centery;
	return angleNormalize(atan2(y, x) - offset);
}