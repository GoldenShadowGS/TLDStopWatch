#pragma once
#include "PCH.h"
#include "Elements.h"

class PlayShape 
{
public:
	void Init(ID2D1Factory* pD2DFactory, const ClockInfo& clockinfo);
	void Draw(ID2D1HwndRenderTarget* rt, ID2D1SolidColorBrush* Brush);
protected:
	ID2D1PathGeometry* Geometry = nullptr;
	ID2D1GeometrySink* Sink = nullptr;
};

class PauseShape
{
public:
	void Init(ID2D1Factory* pD2DFactory, const ClockInfo& clockinfo);
	void Draw(ID2D1HwndRenderTarget* rt, ID2D1SolidColorBrush* Brush);
protected:
	ID2D1PathGeometry* Geometry = nullptr;
	ID2D1GeometrySink* Sink = nullptr;
};