#pragma once
#include "PCH.h"

class ArcRenderer
{
public:
	ArcRenderer();
	~ArcRenderer();
	void Init(float x, float y, float InnerRadius, float OuterRadius, float AngleOffset);
	void SetRadius(float InnerRadius, float OuterRadius);
	void Draw(ID2D1Factory* factory, ID2D1HwndRenderTarget* rt, ID2D1SolidColorBrush* brush, float startangle, float endangle);
private:
	ID2D1PathGeometry* Geometry = nullptr;
	ID2D1GeometrySink* Sink = nullptr;
	float m_AngleOffset = 0.0f;
	float m_centerx = 0.0f;
	float m_centery = 0.0f;
	float m_radiusInner = 1.0f;
	float m_radiusOuter = 1.0f;
};