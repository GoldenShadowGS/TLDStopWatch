#include "PCH.h"
#include "Shape.h"
#include "Renderer.h"

void PlayShape::Init(ID2D1Factory* pD2DFactory, const ClockInfo& clockinfo)
{
	SafeRelease(&Geometry);
	SafeRelease(&Sink);
	HRESULT hr = pD2DFactory->CreatePathGeometry(&Geometry);
	if (FAILED(hr)) return;

	hr = Geometry->Open(&Sink);
	if (FAILED(hr)) return;
	Sink->SetFillMode(D2D1_FILL_MODE_WINDING);

	float width = clockinfo.InnerRadius * 0.6f;
	float angle = 0.0f;
	D2D1_POINT_2F p1 = { cos(angle) * width + clockinfo.centerX, sin(angle) * width + clockinfo.centerY };
	angle = PI2 / 3.0f;
	D2D1_POINT_2F p2 = { cos(angle) * width + clockinfo.centerX, sin(angle) * width + clockinfo.centerY };
	angle *= 2.0f;
	D2D1_POINT_2F p3 = { cos(angle) * width + clockinfo.centerX, sin(angle) * width + clockinfo.centerY };
	Sink->BeginFigure(p1, D2D1_FIGURE_BEGIN_FILLED);
	Sink->AddLine(p2);
	Sink->AddLine(p3);
	Sink->EndFigure(D2D1_FIGURE_END_CLOSED);
	hr = Sink->Close();
}

void PlayShape::Draw(ID2D1HwndRenderTarget* rt, ID2D1SolidColorBrush* Brush)
{
	rt->FillGeometry(Geometry, Brush);
}




void PauseShape::Init(ID2D1Factory* pD2DFactory, const ClockInfo& clockinfo)
{
	SafeRelease(&Geometry);
	SafeRelease(&Sink);
	HRESULT hr = pD2DFactory->CreatePathGeometry(&Geometry);
	if (FAILED(hr)) return;

	hr = Geometry->Open(&Sink);
	if (FAILED(hr)) return;
	Sink->SetFillMode(D2D1_FILL_MODE_WINDING);

	float width = clockinfo.InnerRadius * 0.13f;
	float height = clockinfo.InnerRadius * 0.5f;
	float spacing = clockinfo.InnerRadius * 0.25f;
	D2D1_POINT_2F p1 = { clockinfo.centerX - width - spacing, clockinfo.centerY - height };
	D2D1_POINT_2F p2 = { clockinfo.centerX + width - spacing, clockinfo.centerY - height };
	D2D1_POINT_2F p3 = { clockinfo.centerX + width - spacing, clockinfo.centerY + height };
	D2D1_POINT_2F p4 = { clockinfo.centerX - width - spacing, clockinfo.centerY + height };
	Sink->BeginFigure(p1, D2D1_FIGURE_BEGIN_FILLED);
	Sink->AddLine(p2);
	Sink->AddLine(p3);
	Sink->AddLine(p4);
	Sink->EndFigure(D2D1_FIGURE_END_CLOSED);
	p1 = { clockinfo.centerX - width + spacing, clockinfo.centerY - height };
	p2 = { clockinfo.centerX + width + spacing, clockinfo.centerY - height };
	p3 = { clockinfo.centerX + width + spacing, clockinfo.centerY + height };
	p4 = { clockinfo.centerX - width + spacing, clockinfo.centerY + height };
	Sink->BeginFigure(p1, D2D1_FIGURE_BEGIN_FILLED);
	Sink->AddLine(p2);
	Sink->AddLine(p3);
	Sink->AddLine(p4);
	Sink->EndFigure(D2D1_FIGURE_END_CLOSED);
	hr = Sink->Close();
}

void PauseShape::Draw(ID2D1HwndRenderTarget* rt, ID2D1SolidColorBrush* Brush)
{
	rt->FillGeometry(Geometry, Brush);
}