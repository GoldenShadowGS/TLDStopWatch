#pragma once
#include "PCH.h"

constexpr float PI = 3.141592f;
constexpr float HalfPI = PI / 2.0f;
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

//inline float minutesNormalize(float minutes)
//{
//	while (minutes < 0.0f)
//	{
//		minutes += 60.0f;
//	}
//	while (minutes > 60.0f)
//	{
//		minutes -= 60.0f;
//	}
//	return minutes;
//}