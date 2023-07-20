#include "PCH.h"
#include "ArcRenderer.h"
#include "Math.h"

ArcRenderer::ArcRenderer()
{}

ArcRenderer::~ArcRenderer()
{
	if (Sink)
		Sink->Release();
	if (Geometry)
		Geometry->Release();
}

inline D2D1_ARC_SIZE GetAngleSize(float angle)
{
	float newangle = angleNormalize(angle);
	if (fmod(angle, PI2) >= PI)
		return D2D1_ARC_SIZE_LARGE;
	else
		return D2D1_ARC_SIZE_SMALL;
}

void ArcRenderer::Draw(ID2D1Factory* factory, ID2D1HwndRenderTarget* rt)
{
	if (Sink)
		Sink->Release();
	if (Geometry)
		Geometry->Release();
	HRESULT hr = factory->CreatePathGeometry(&Geometry);
	if (FAILED(hr)) return;

	hr = Geometry->Open(&Sink);
	if (FAILED(hr)) return;
	Sink->SetFillMode(D2D1_FILL_MODE_WINDING);
	D2D1_POINT_2F center = { m_centerx, m_centery };
	D2D1_POINT_2F p1 = { center.x + m_radiusInner * cos(m_startangle),center.y + m_radiusInner * sin(m_startangle) };
	D2D1_POINT_2F p2 = { center.x + m_radiusInner * cos(m_endangle),center.y + m_radiusInner * sin(m_endangle) };
	D2D1_POINT_2F p3 = { center.x + m_radiusOuter * cos(m_endangle),center.y + m_radiusOuter * sin(m_endangle) };
	D2D1_POINT_2F p4 = { center.x + m_radiusOuter * cos(m_startangle),center.y + m_radiusOuter * sin(m_startangle) };
	Sink->BeginFigure(p1, D2D1_FIGURE_BEGIN_FILLED);
	Sink->AddArc(D2D1::ArcSegment(p2, D2D1::SizeF(m_radiusInner, m_radiusInner), 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, GetAngleSize(m_rangeangle)));
	Sink->AddLine(p3);
	Sink->AddArc(D2D1::ArcSegment(p4, D2D1::SizeF(m_radiusOuter, m_radiusOuter), 0.0f, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE, GetAngleSize(m_rangeangle)));
	Sink->EndFigure(D2D1_FIGURE_END_CLOSED);
	hr = Sink->Close();
	if (FAILED(hr)) return;

	// Render Arc Geometry
	rt->FillGeometry(Geometry, Brush);
}