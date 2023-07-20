#pragma once
#include "PCH.h"

class ArcRenderer
{
public:
	ArcRenderer();
	~ArcRenderer();
	void SetAngles(float start, float end, float range)
	{
		m_startangle = start;
		m_endangle = end;
		m_rangeangle = range;
	}
	void SetRadiusInner(float radius)
	{
		m_radiusInner = radius;
	}
	void SetRadiusOuter(float radius)
	{
		m_radiusOuter = radius;
	}
	void SetCenter(float x, float y)
	{
		m_centerx = x; m_centery = y;
	}
	void Setalpha(float alpha)
	{
		m_alpha = alpha;
	}
	void SetBrush(ID2D1SolidColorBrush* b)
	{
		Brush = b; ;
	}
	void Draw(ID2D1Factory* factory, ID2D1HwndRenderTarget* rt);
private:
	ID2D1PathGeometry* Geometry = nullptr;
	ID2D1GeometrySink* Sink = nullptr;
	ID2D1SolidColorBrush* Brush = nullptr;  //non owning pointer
	float m_startangle = 0.0f;
	float m_endangle = 0.0f;
	float m_rangeangle = 0.0f;
	float m_centerx = 0.0f;
	float m_centery = 0.0f;
	float m_radiusInner = 1.0f;
	float m_radiusOuter = 1.0f;
	float m_alpha = 1.0f;
};