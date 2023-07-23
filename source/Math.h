#pragma once
#include "PCH.h"

constexpr float PI = 3.141592f;
constexpr float HalfPI = PI / 2.0f;
constexpr float QuarterPI = PI / 4.0f;
constexpr float PI2 = PI * 2.0f;
constexpr float PI3_4 = PI * 1.5f;
constexpr float Deg2RadFactor = PI / 180.0f;
constexpr float Rad2DegFactor = 180.0f / PI;

inline float Deg2Rad(float deg)
{
	return Deg2RadFactor * deg;
}

inline float Rad2Deg(float rad)
{
	return Rad2DegFactor * rad;
}

inline float angleNormalize(float angle)
{
	while (angle < 0.0f)
	{
		angle += PI2;
	}
	while (angle > PI2)
	{
		angle -= PI2;
	}
	return angle;
}

inline float GetAngleDistance(float angle1, float angle2)
{
	float dist = abs(angle1 - angle2);
	if (dist > PI)
		dist = PI2 - dist;
	return dist;
}

inline float GetPointDist(float x1, float y1, float x2, float y2)
{
	float x3 = x2 - x1;
	float y3 = y2 - y1;
	return sqrt((x3 * x3) + (y3 * y3));
}
