#include "PCH.h"
#include "Renderer.h"
#include "Bitmap.h"
#include "Resource.h"
#include "Math.h"

Renderer::Renderer()
{}

Renderer::~Renderer()
{
	SafeRelease(&pD2DFactory);
	SafeRelease(&pBitmapRenderTarget);
	SafeRelease(&pBackGroundBitmap);
	SafeRelease(&pRenderTarget);
	SafeRelease(&pBlackBrush);
	SafeRelease(&pWhiteBrush);
	SafeRelease(&pRedPinkBrush);
	SafeRelease(&pGoColorBrush);
	SafeRelease(&pAlarmHoverBrush);
	SafeRelease(&pStopsCollection);
	SafeRelease(&pStopsCollectionHover);
	SafeRelease(&pRadialGradientBrush);
	SafeRelease(&pRadialGradientBrushHover);
}

BOOL Renderer::Init(HWND hwnd, const ClockInfo& clockinfo, D2D1_RECT_F clientRect)
{
	m_hWnd = hwnd;
	m_ClientRect = clientRect;
	HRESULT hr;
	D2D1_FACTORY_OPTIONS factoryoptions = { D2D1_DEBUG_LEVEL_NONE };
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, factoryoptions, &pD2DFactory);
	if (FAILED(hr))
		return false;

	// Obtain the size of the drawing area.
	RECT rc;
	GetClientRect(hwnd, &rc);

	// Create a Direct2D render target          
	D2D1_RENDER_TARGET_PROPERTIES rendertargetproperties = D2D1::RenderTargetProperties();
	D2D1_PRESENT_OPTIONS options = D2D1_PRESENT_OPTIONS_NONE;
	D2D1_HWND_RENDER_TARGET_PROPERTIES hwnRTP = D2D1::HwndRenderTargetProperties(hwnd, D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top), options);

	hr = pD2DFactory->CreateHwndRenderTarget(rendertargetproperties, hwnRTP, &pRenderTarget);
	if (FAILED(hr))
		return false;

	hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.02f, 0.02f, 0.02f, 1.0f), &pBlackBrush);
	hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f), &pWhiteBrush);
	hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.95f, 0.5f, 0.6f, 0.5f), &pRedPinkBrush);
	hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.79f, 0.67f, 0.9f, 1.0f), &pGoColorBrush);
	hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.75f, 0.64f, 0.86f, 1.0f), &pStopColorBrush);
	hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.75f, 0.1f, 0.1f, 1.0f), &pAlarmHoverBrush);

	hr = pRenderTarget->CreateCompatibleRenderTarget(&pBitmapRenderTarget);

	// Text Init
	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&pDWriteFactory));
	if (FAILED(hr))
		return false;

	float fontscale = clockinfo.OuterRadius / 200.0f;
	hr = pDWriteFactory->CreateTextFormat(L"Arial", NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 28.0f * fontscale, L"en-us", &pTextFormat);
	if (FAILED(hr))
		return false;

	pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

	// Border Colors
	D2D1_GRADIENT_STOP gstop[2] = {};
	gstop[0].color = { 0.8f, 0.0f, 0.89f, 1.0f };
	gstop[0].position = { 0.85f };
	gstop[1].color = { 0.0f, 0.1f, 0.0f, 1.0f };
	gstop[1].position = { 1.0f };
	hr = pRenderTarget->CreateGradientStopCollection(gstop, 2, &pStopsCollection);
	if (FAILED(hr))
		return false;

	gstop[0].color = { 0.9f, 0.0f, 0.95f, 1.0f };
	gstop[0].position = { 0.85f };
	gstop[1].color = { 0.0f, 0.1f, 0.0f, 1.0f };
	gstop[1].position = { 1.0f };
	hr = pRenderTarget->CreateGradientStopCollection(gstop, 2, &pStopsCollectionHover);
	if (FAILED(hr))
		return false;

	hr = pRenderTarget->CreateRadialGradientBrush(
		D2D1::RadialGradientBrushProperties(D2D1::Point2F(clockinfo.centerX, clockinfo.centerY), D2D1::Point2F(0, 0), clockinfo.OuterRadius, clockinfo.OuterRadius), pStopsCollection, &pRadialGradientBrush);

	hr = pRenderTarget->CreateRadialGradientBrush(
		D2D1::RadialGradientBrushProperties(D2D1::Point2F(clockinfo.centerX, clockinfo.centerY), D2D1::Point2F(0, 0), clockinfo.OuterRadius, clockinfo.OuterRadius), pStopsCollectionHover, &pRadialGradientBrushHover);


	// Bitmaps
	minutehandbitmap = std::make_unique<Bitmap>();
	minutehandhighlightedbitmap = std::make_unique<Bitmap>();
	minutehandbitmapShadow = std::make_unique<Bitmap>();
	hourhandbitmap = std::make_unique<Bitmap>();
	hourhandbitmapShadow = std::make_unique<Bitmap>();

	float scale = clockinfo.InnerRadius / 170.0f;
	//pivot offsets
	float offsetx = 0.0565476f;
	float offsety = 0.486842f;
	if (!minutehandbitmap->Load(pRenderTarget, IDBITMAP_MINUTEHAND, 250, 10, 10, offsetx, offsety, scale))
		return false;
	if (!minutehandhighlightedbitmap->Load(pRenderTarget, IDBITMAP_MINUTEHAND, 222, 234, 52, offsetx, offsety, scale))
		return false;
	offsetx = 0.278274f;
	offsety = 0.503289f;
	if (!minutehandbitmapShadow->Load(pRenderTarget, IDBITMAP_MINUTEHANDSHADOW, 1, 1, 1, offsetx, offsety, scale))
		return false;
	offsetx = 0.088462f;
	offsety = 0.48913f;
	if (!hourhandbitmap->Load(pRenderTarget, IDBITMAP_HOURHAND, 30, 50, 30, offsetx, offsety, scale))
		return false;
	offsetx = 0.294231f;
	offsety = 0.494565f;
	if (!hourhandbitmapShadow->Load(pRenderTarget, IDBITMAP_HOURHANDSHADOW, 1, 1, 1, offsetx, offsety, scale))
		return false;


	// Alarm Arcs
	arcRenderer.SetBrush(pRedPinkBrush);
	arcRenderer.Setalpha(0.5f);

	arcRendererThin.SetBrush(pBlackBrush);
	arcRendererThin.Setalpha(0.75f);

	return true;
}

void Renderer::Render(ClockInfo& clockinfo)
{
	if (pRenderTarget)
	{
		pRenderTarget->BeginDraw();
		pRenderTarget->SetTransform(D2D1::IdentityMatrix());

		if (clockinfo.RedrawBackGround)
		{
			RenderBackGround(clockinfo, &pBackGroundBitmap, (clockinfo.Timing ? pGoColorBrush : pStopColorBrush), (clockinfo.BorderHover ? pRadialGradientBrushHover : pRadialGradientBrush));
			clockinfo.RedrawBackGround = FALSE;
		}
		if (pBackGroundBitmap)
			pRenderTarget->DrawBitmap(pBackGroundBitmap, m_ClientRect);

		float scale = clockinfo.OuterRadius / 200.0f;
		float radius = clockinfo.InnerRadius;
		if (clockinfo.AlarmSet || clockinfo.AlarmHover)
		{
			D2D1_ELLIPSE ellipse = { { clockinfo.centerX,  clockinfo.centerY }, radius, radius };
			float circleRadius = radius + 6.5f * scale;
			const float segmentradius = circleRadius / 15.0f;
			const float halfsegmentradius = segmentradius / 2.0f;
			for (int i = 0; i < clockinfo.wraps; i++)
			{
				circleRadius -= segmentradius;
				ellipse.radiusX = circleRadius - halfsegmentradius;
				ellipse.radiusY = circleRadius - halfsegmentradius;
				pRenderTarget->DrawEllipse(ellipse, pBlackBrush, 1.0f);
				ellipse.radiusX = circleRadius;
				ellipse.radiusY = circleRadius;
				pRenderTarget->DrawEllipse(ellipse, pRedPinkBrush, segmentradius);
			}

			// Alarm Pink Arc Section
			arcRenderer.SetRadiusInner(circleRadius - segmentradius * 1.5f);
			arcRenderer.SetRadiusOuter(circleRadius - halfsegmentradius);
			arcRenderer.SetCenter(clockinfo.centerX, clockinfo.centerY);
			arcRenderer.SetAngles(clockinfo.minAngleRad, clockinfo.alarmAngleRad, clockinfo.rangeAngleRad);
			arcRenderer.Draw(pD2DFactory, pRenderTarget);

			arcRendererThin.SetRadiusInner(circleRadius - segmentradius * 1.5f + 0.5f);
			arcRendererThin.SetRadiusOuter(circleRadius - segmentradius * 1.5f - 0.5f);
			arcRendererThin.SetCenter(clockinfo.centerX, clockinfo.centerY);
			arcRendererThin.SetAngles(clockinfo.minAngleRad, clockinfo.alarmAngleRad, clockinfo.rangeAngleRad);
			arcRendererThin.Draw(pD2DFactory, pRenderTarget);

			//Draw Alarm Tick
			float alarmAngle = Rad2Deg(clockinfo.alarmAngleRad);
			D2D1_POINT_2F centerpoint = { clockinfo.centerX, clockinfo.centerY };
			D2D1_POINT_2F point1 = { clockinfo.centerX + radius - ((clockinfo.wraps + 1) * segmentradius), clockinfo.centerY };
			D2D1_POINT_2F point2 = { clockinfo.centerX + radius , clockinfo.centerY };
			pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation(alarmAngle, centerpoint));
			pRenderTarget->DrawLine(point1, point2, clockinfo.AlarmHover ? pAlarmHoverBrush : pBlackBrush, clockinfo.AlarmHover ? 6.0f : 3.0f);
		}


		// Second Hand
		pRenderTarget->SetTransform(D2D1::IdentityMatrix());
		float secanglecos = cos(clockinfo.secAngleRad);
		float secanglesin = sin(clockinfo.secAngleRad);
		float radius1 = radius * -0.25f;
		float radius2 = radius * 0.9f;
		D2D1_POINT_2F point1 = { clockinfo.centerX + secanglecos * radius1, secanglesin * radius1 + clockinfo.centerY };
		D2D1_POINT_2F point2 = { clockinfo.centerX + secanglecos * radius2, secanglesin * radius2 + clockinfo.centerY };
		pRenderTarget->DrawLine(point1, point2, pBlackBrush, 3.0f * scale);


		// Draw Bitmap Clock Hands
		minutehandbitmapShadow->Draw(pRenderTarget, clockinfo.minuteAngle, clockinfo.centerX + clockinfo.shadowoffsetx, clockinfo.centerY + clockinfo.shadowoffsety);
		hourhandbitmapShadow->Draw(pRenderTarget, clockinfo.hourAngle, clockinfo.centerX + clockinfo.shadowoffsetx, clockinfo.centerY + clockinfo.shadowoffsety);
		hourhandbitmap->Draw(pRenderTarget, clockinfo.hourAngle, clockinfo.centerX, clockinfo.centerY);
		if (clockinfo.HandHover)
			minutehandhighlightedbitmap->Draw(pRenderTarget, clockinfo.minuteAngle, clockinfo.centerX, clockinfo.centerY);
		else
			minutehandbitmap->Draw(pRenderTarget, clockinfo.minuteAngle, clockinfo.centerX, clockinfo.centerY);

		//pRenderTarget->SetTransform(D2D1::IdentityMatrix());
		//D2D1_ELLIPSE ellipse = { { clockinfo.centerX,  clockinfo.centerY }, 20, 20 };
		//pRenderTarget->DrawEllipse(ellipse, pBlackBrush, 1.0f);


		//pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

		//WCHAR mytext[] = L"Hello World\n Whoops";

		//pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		//D2D1_RECT_F rect = { (float)rc.left,(float)rc.top, (float)rc.right, (float)rc.bottom };
		//pRenderTarget->DrawTextW(mytext, ARRAYSIZE(mytext) - 1, pTextFormat, rect, pBlackBrush);
		HRESULT hr = pRenderTarget->EndDraw();
	}
}

void Renderer::RenderBackGround(const ClockInfo& clockinfo, ID2D1Bitmap** pBitmap, ID2D1SolidColorBrush* pBKGBrush, ID2D1RadialGradientBrush* pRadialBrush)
{
	if (pBitmapRenderTarget)
	{
		pBitmapRenderTarget->BeginDraw();
		pBitmapRenderTarget->SetTransform(D2D1::IdentityMatrix());
		pBitmapRenderTarget->Clear(ClearColor);

		float scale = clockinfo.OuterRadius / 200.0f;
		D2D1_POINT_2F center = { clockinfo.centerX,  clockinfo.centerY };
		D2D1_ELLIPSE ellipse = {};
		float radius = clockinfo.OuterRadius - 36.0f * scale * 0.5f;
		ellipse.radiusX = radius;
		ellipse.radiusY = radius;
		ellipse.point = center;
		pRadialBrush->SetCenter(center);
		pRadialBrush->SetRadiusX(clockinfo.OuterRadius);
		pRadialBrush->SetRadiusY(clockinfo.OuterRadius);
		pBitmapRenderTarget->DrawEllipse(ellipse, pRadialBrush, 36.0f * scale);
		radius = clockinfo.InnerRadius;
		ellipse.radiusX = radius;
		ellipse.radiusY = radius;
		pBitmapRenderTarget->DrawEllipse(ellipse, pBlackBrush, 4.0f * scale);
		pBitmapRenderTarget->FillEllipse(ellipse, pBKGBrush);

		// Draw Large Ticks
		for (int i = 0; i < 12; i++)
		{
			float angle = ((float)i / 12.0f) * 360.0f;
			D2D1_POINT_2F centerpoint = { clockinfo.centerX, clockinfo.centerY };
			D2D1_POINT_2F point1 = { clockinfo.centerX + radius - 14.0f * scale, clockinfo.centerY };
			D2D1_POINT_2F point2 = { clockinfo.centerX + radius, clockinfo.centerY };
			pBitmapRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation(angle, centerpoint));
			pBitmapRenderTarget->DrawLine(point1, point2, pBlackBrush, 2.5f * scale);
		}
		// Draw Small ticks
		for (int i = 0; i < 60; i++)
		{
			float angle = ((float)i / 60.0f) * 360.0f;
			D2D1_POINT_2F centerpoint = { clockinfo.centerX, clockinfo.centerY };
			D2D1_POINT_2F point1 = { clockinfo.centerX + radius - 8.0f * scale, clockinfo.centerY };
			D2D1_POINT_2F point2 = { clockinfo.centerX + radius, clockinfo.centerY };
			pBitmapRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation(angle, centerpoint));
			pBitmapRenderTarget->DrawLine(point1, point2, pBlackBrush, 1.0f * scale);
		}
		// Draw Clock Numbers
		pBitmapRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
		for (int i = 0; i < 12; i++)
		{
			constexpr float anglesegment = ((float)1.0f / 12.0f) * PI2;

			for (int i = 0; i < 12; i++)
			{
				float angle = ((float)i / 12.0f) * PI2 + anglesegment;
				float x = sin(angle) * 145.0f * scale;
				float y = -cos(angle) * 145.0f * scale;
				RenderableText number(pTextFormat, x, y, 32.0f * scale, 32.0f * scale, 1.0f);
				number.SetText(std::to_wstring((i + 1) * 5));
				number.SetPosition(clockinfo.centerX, clockinfo.centerY);
				number.Draw(pBitmapRenderTarget, pBlackBrush);
			}
		}

		HRESULT hr = pBitmapRenderTarget->EndDraw();

		hr = pBitmapRenderTarget->GetBitmap(pBitmap);
	}
}