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
	SafeRelease(&pTransparentBlackBrush);
	SafeRelease(&pGoColorBrush);
	SafeRelease(&pStopColorBrush);
	SafeRelease(&pAlarmHoverBrush);
	SafeRelease(&pBorderBrush);
	SafeRelease(&pAdjustBorderTimeBrush);

	SafeRelease(&pStopsCollection);
	SafeRelease(&pStopsCollectionHover);
	SafeRelease(&pRadialGradientBrush);

	SafeRelease(&pDWriteFactory);
	SafeRelease(&pTextFormat);
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
	hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.75f), &pWhiteBrush);
	hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.95f, 0.5f, 0.6f, 0.35f), &pRedPinkBrush);
	hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.02f, 0.02f, 0.02f, 0.35f), &pTransparentBlackBrush);
	hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.75f, 0.64f, 0.86f, 1.0f), &pGoColorBrush);
	hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.66f, 0.56f, 0.76f, 1.0f), &pStopColorBrush);
	hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.75f, 0.1f, 0.1f, 1.0f), &pAlarmHoverBrush);
	hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.86f, 0.36f, 0.90f, 0.25f), &pBorderBrush);
	hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.26f, 0.05f, 0.55f, 0.75f), &pAdjustBorderTimeBrush);

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
	gstop[0].color = { 0.7f, 0.0f, 0.8f, 1.0f };
	gstop[0].position = { 0.85f };
	gstop[1].color = { 0.4f, 0.1f, 0.4f, 1.0f };
	gstop[1].position = { 1.0f };
	hr = pRenderTarget->CreateGradientStopCollection(gstop, 2, &pStopsCollection);
	if (FAILED(hr))
		return false;

	gstop[0].color = { 0.9f, 0.0f, 0.95f, 1.0f };
	gstop[0].position = { 0.85f };
	gstop[1].color = { 0.4f, 0.1f, 0.4f, 1.0f };
	gstop[1].position = { 1.0f };
	hr = pRenderTarget->CreateGradientStopCollection(gstop, 2, &pStopsCollectionHover);
	if (FAILED(hr))
		return false;

	hr = pRenderTarget->CreateRadialGradientBrush(
		D2D1::RadialGradientBrushProperties(D2D1::Point2F(clockinfo.centerX, clockinfo.centerY), D2D1::Point2F(0, 0), clockinfo.OuterRadius, clockinfo.OuterRadius), pStopsCollection, &pRadialGradientBrush);

	// Bitmaps
	minutehandbitmap = std::make_unique<Bitmap>();
	minutehandhighlightedbitmap = std::make_unique<Bitmap>();
	minutehandbitmapShadow = std::make_unique<Bitmap>();
	hourhandbitmap = std::make_unique<Bitmap>();
	hourhandbitmapShadow = std::make_unique<Bitmap>();
	UIbitmap = std::make_unique<Bitmap>();
	UIHoverbitmap = std::make_unique<Bitmap>();

	float scale = clockinfo.InnerRadius / 170.0f;
	//pivot offsets
	float offsetx = 0.0565476f;
	float offsety = 0.486842f;
	float angleOffsetDegrees = Rad2Deg(clockinfo.angleOffset);
	if (!minutehandbitmap->Load(pRenderTarget, IDBITMAP_MINUTEHAND, 250, 10, 10, offsetx, offsety, angleOffsetDegrees, scale))
		return false;
	if (!minutehandhighlightedbitmap->Load(pRenderTarget, IDBITMAP_MINUTEHAND, 222, 234, 207, offsetx, offsety, angleOffsetDegrees, scale))
		return false;
	offsetx = 0.278274f;
	offsety = 0.503289f;
	if (!minutehandbitmapShadow->Load(pRenderTarget, IDBITMAP_MINUTEHANDSHADOW, 1, 1, 1, offsetx, offsety, angleOffsetDegrees, scale))
		return false;
	offsetx = 0.088462f;
	offsety = 0.48913f;
	if (!hourhandbitmap->Load(pRenderTarget, IDBITMAP_HOURHAND, 30, 50, 30, offsetx, offsety, angleOffsetDegrees, scale))
		return false;
	offsetx = 0.294231f;
	offsety = 0.494565f;
	if (!hourhandbitmapShadow->Load(pRenderTarget, IDBITMAP_HOURHANDSHADOW, 1, 1, 1, offsetx, offsety, angleOffsetDegrees, scale))
		return false;

	if (!UIbitmap->Load(pRenderTarget, IDBITMAP_UI, 126, 60, 184, 0.0f, 0.0f, 0.0f, 1.0f))
		return false;
	if (!UIHoverbitmap->Load(pRenderTarget, IDBITMAP_UI, 180, 20, 170, 0.0f, 0.0f, 0.0f, 1.0f))
		return false;

	UIsprite.rows = 2;
	UIsprite.cols = 2;
	UIsprite.spritewidth = 36;
	UIsprite.spriteheight = 36;

	// Alarm Arcs
	arcRenderer.Init(clockinfo.centerX, clockinfo.centerY, clockinfo.OuterRadius, clockinfo.InnerRadius, 0.0f);
	arcRendererThin.Init(clockinfo.centerX, clockinfo.centerY, clockinfo.OuterRadius, clockinfo.InnerRadius, 0.0f);
	arcAdjustTime.Init(clockinfo.centerX, clockinfo.centerY, clockinfo.InnerRadius + 6.0f * clockinfo.UI_Scale, clockinfo.OuterRadius - 6.0f * clockinfo.UI_Scale, 0.0f);

	playshape.Init(pD2DFactory, clockinfo);
	pauseshape.Init(pD2DFactory, clockinfo);

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
			RenderBackGround(clockinfo, &pBackGroundBitmap, (clockinfo.Timing ? pGoColorBrush : pStopColorBrush), pRadialGradientBrush);
			clockinfo.RedrawBackGround = FALSE;
		}
		if (pBackGroundBitmap)
			pRenderTarget->DrawBitmap(pBackGroundBitmap, m_ClientRect);

		if (clockinfo.Timing)
			playshape.Draw(pRenderTarget, pStopColorBrush);
		else
			pauseshape.Draw(pRenderTarget, pGoColorBrush);

		float scale = clockinfo.OuterRadius / 200.0f;
		float radius = clockinfo.InnerRadius;
		if (clockinfo.AlarmSet)
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
				pRenderTarget->DrawEllipse(ellipse, pTransparentBlackBrush, 1.0f);
				ellipse.radiusX = circleRadius;
				ellipse.radiusY = circleRadius;
				pRenderTarget->DrawEllipse(ellipse, pRedPinkBrush, segmentradius);
			}

			// Alarm Arc
			const float adjustedstartangle = clockinfo.minAngleRad + clockinfo.angleOffset;
			const float adjustedendangle = clockinfo.alarmAngleRad + clockinfo.angleOffset;
			arcRenderer.SetRadius(circleRadius - segmentradius * 1.5f, circleRadius - halfsegmentradius);
			arcRenderer.Draw(pD2DFactory, pRenderTarget, pRedPinkBrush, adjustedstartangle, adjustedendangle);

			arcRendererThin.SetRadius(circleRadius - segmentradius * 1.5f + 0.5f, circleRadius - segmentradius * 1.5f - 0.5f);
			arcRendererThin.Draw(pD2DFactory, pRenderTarget, pTransparentBlackBrush, adjustedstartangle, adjustedendangle);

			//Draw Alarm Tick
			float alarmAngle = Rad2Deg(adjustedendangle);
			D2D1_POINT_2F centerpoint = { clockinfo.centerX, clockinfo.centerY };
			D2D1_POINT_2F point1 = { clockinfo.centerX + radius - ((clockinfo.wraps + 1) * segmentradius), clockinfo.centerY };
			D2D1_POINT_2F point2 = { clockinfo.centerX + radius , clockinfo.centerY };
			pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation(alarmAngle, centerpoint));
			if (clockinfo.highlightedElement == HIGHLIGHTED_ALARM)
			{
				pRenderTarget->DrawLine(point1, point2, pAlarmHoverBrush, 10.0f * scale);
				pRenderTarget->DrawLine(point1, point2, pGoColorBrush, 3.0f * scale);
			}
			else
				pRenderTarget->DrawLine(point1, point2, pBlackBrush, 3.0f);
		}
		// Adjust Time Arc
		{
			pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
			arcAdjustTime.Draw(pD2DFactory, pRenderTarget, pAdjustBorderTimeBrush, -HalfPI, max(clockinfo.adjustTimeRad - 0.0001f, 0.0f) + clockinfo.angleOffset);
		}
		//Adjust time Tick

		{
			float AdjustTickAngle = Rad2Deg(clockinfo.adjustTimeRad + clockinfo.angleOffset);
			D2D1_POINT_2F centerpoint = { clockinfo.centerX, clockinfo.centerY };
			D2D1_POINT_2F point1 = { clockinfo.centerX + clockinfo.InnerRadius, clockinfo.centerY };
			D2D1_POINT_2F point2 = { clockinfo.centerX + clockinfo.OuterRadius, clockinfo.centerY };
			pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation(AdjustTickAngle, centerpoint));
			if (clockinfo.highlightedElement == HIGHLIGHTED_ADJUST_TIMER)
			{
				pRenderTarget->DrawLine(point1, point2, pWhiteBrush, 12.0f * scale);
				pRenderTarget->DrawLine(point1, point2, pGoColorBrush, 3.0f * scale);
			}
			else
				pRenderTarget->DrawLine(point1, point2, pGoColorBrush, 3.0f);
		}

		pRenderTarget->SetTransform(D2D1::IdentityMatrix());
		if (clockinfo.highlightedElement == HIGHLIGHTED_BORDER)
		{
			float radius = (clockinfo.OuterRadius - clockinfo.InnerRadius) * 0.5f + clockinfo.InnerRadius;
			D2D1_ELLIPSE BorderEllipse = { { clockinfo.centerX,  clockinfo.centerY }, radius, radius };
			pRenderTarget->DrawEllipse(BorderEllipse, pBorderBrush, 36.0f * clockinfo.UI_Scale);
		}

		// Text
		pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

		float width = 200.0f * scale;
		float smalladjustment = 10.0f * scale;
		D2D1_RECT_F rect = { clockinfo.centerX - width, clockinfo.centerY * 1.2f, clockinfo.centerX + width, clockinfo.centerY + 200.0f };

		pRenderTarget->DrawTextW(clockinfo.TimeText, lstrlenW(clockinfo.TimeText), pTextFormat, rect, pBlackBrush);

		rect = { clockinfo.centerX, clockinfo.centerY + smalladjustment, clockinfo.centerX + width * 0.9f , clockinfo.centerY + 100.0f };
		pRenderTarget->DrawTextW(clockinfo.AdjustText, lstrlenW(clockinfo.AdjustText), pTextFormat, rect, pBlackBrush);


		rect = { clockinfo.centerX - width * 0.85f, clockinfo.centerY + smalladjustment , clockinfo.centerX , clockinfo.centerY + 100.0f };
		pRenderTarget->DrawTextW(clockinfo.AlarmText, lstrlenW(clockinfo.AlarmText), pTextFormat, rect, pBlackBrush);


		// Make Second Hand disapear when Adjusting Minute hand with Mouse
		if (clockinfo.grabbedElement != GRABBED_MINUTEHAND && clockinfo.Timing)
		{
			float secondhandAngle = clockinfo.secAngleRad + clockinfo.angleOffset;
			float secanglecos = cos(secondhandAngle);
			float secanglesin = sin(secondhandAngle);
			float radius1 = radius * -0.25f;
			float radius2 = radius * 0.9f;
			D2D1_POINT_2F point1 = { clockinfo.centerX + secanglecos * radius1, secanglesin * radius1 + clockinfo.centerY };
			D2D1_POINT_2F point2 = { clockinfo.centerX + secanglecos * radius2, secanglesin * radius2 + clockinfo.centerY };
			pRenderTarget->DrawLine(point1, point2, pBlackBrush, 3.0f * scale);
		}


		// Draw Bitmap Clock Hands
		minutehandbitmapShadow->Draw(pRenderTarget, clockinfo.minuteAngle, clockinfo.centerX + clockinfo.shadowoffsetx, clockinfo.centerY + clockinfo.shadowoffsety);
		hourhandbitmapShadow->Draw(pRenderTarget, clockinfo.hourAngle, clockinfo.centerX + clockinfo.shadowoffsetx, clockinfo.centerY + clockinfo.shadowoffsety);
		hourhandbitmap->Draw(pRenderTarget, clockinfo.hourAngle, clockinfo.centerX, clockinfo.centerY);
		if (clockinfo.highlightedElement == HIGHLIGHTED_MINUTEHAND)
			minutehandhighlightedbitmap->Draw(pRenderTarget, clockinfo.minuteAngle, clockinfo.centerX, clockinfo.centerY);
		else
			minutehandbitmap->Draw(pRenderTarget, clockinfo.minuteAngle, clockinfo.centerX, clockinfo.centerY);

		// Draw UI Buttons
		pRenderTarget->SetTransform(D2D1::IdentityMatrix());
		for (int i = 0; i < 4; i++)
		{
			if (clockinfo.highlightedElement == i)
				UIHoverbitmap->DrawSprite(pRenderTarget, i, UIsprite, 0.0f, clockinfo.buttonpos[i], clockinfo.UI_Scale * 1.04f);
			else
				UIbitmap->DrawSprite(pRenderTarget, i, UIsprite, 0.0f, clockinfo.buttonpos[i], clockinfo.UI_Scale * 0.96f);
		}

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
		{
			static ID2D1PathGeometry* Geometry = nullptr;
			static ID2D1GeometrySink* Sink = nullptr;

			auto UI_BackGround = [&] (int ButtonIndex) {

				HRESULT hr = pD2DFactory->CreatePathGeometry(&Geometry);
				if (FAILED(hr)) return;

				hr = Geometry->Open(&Sink);
				if (FAILED(hr)) return;
				Sink->SetFillMode(D2D1_FILL_MODE_WINDING);

				D2D1_ELLIPSE buttonellipse = { { clockinfo.buttonpos[ButtonIndex].x,  clockinfo.buttonpos[ButtonIndex].y }, clockinfo.ButtonBackGroundRadius, clockinfo.ButtonBackGroundRadius };
				float x = clockinfo.buttonpos[ButtonIndex].x - clockinfo.centerX;
				float y = clockinfo.buttonpos[ButtonIndex].y - clockinfo.centerY;
				float buttonangle = atan2(y, x);
				float r = clockinfo.ButtonBackGroundRadius * 2.0f;
				float xoffset = cos(buttonangle - HalfPI) * clockinfo.ButtonBackGroundRadius;
				float yoffset = sin(buttonangle - HalfPI) * clockinfo.ButtonBackGroundRadius;
				D2D1_POINT_2F p2 = { clockinfo.buttonpos[ButtonIndex].x + xoffset,  clockinfo.buttonpos[ButtonIndex].y + yoffset };
				D2D1_POINT_2F p1 = { p2.x - x * 0.5f,  p2.y - y * 0.5f };
				xoffset = cos(buttonangle + HalfPI) * clockinfo.ButtonBackGroundRadius;
				yoffset = sin(buttonangle + HalfPI) * clockinfo.ButtonBackGroundRadius;
				D2D1_POINT_2F p3 = { clockinfo.buttonpos[ButtonIndex].x + xoffset,  clockinfo.buttonpos[ButtonIndex].y + yoffset };
				D2D1_POINT_2F p4 = { p3.x - x * 0.5f,  p3.y - y * 0.5f };
				Sink->BeginFigure(p1, D2D1_FIGURE_BEGIN_FILLED);
				Sink->AddLine(p2);
				Sink->AddLine(p3);
				Sink->AddLine(p4);
				Sink->EndFigure(D2D1_FIGURE_END_CLOSED);
				hr = Sink->Close();
				pBitmapRenderTarget->FillGeometry(Geometry, pBlackBrush);
				SafeRelease(&Geometry);
				SafeRelease(&Sink);
				pBitmapRenderTarget->FillEllipse(buttonellipse, pBlackBrush);
			};

			for (int i = 0; i < 4; i++)
			{
				UI_BackGround(i);
			}
		}
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
				float x = sin(angle) * 135.0f * scale;
				float y = -cos(angle) * 135.0f * scale;
				RenderableText number(pTextFormat, x, y, 32.0f * scale, 32.0f * scale, 1.0f);
				number.SetText(std::to_wstring((i + 1) * 1));
				number.SetPosition(clockinfo.centerX, clockinfo.centerY);
				number.Draw(pBitmapRenderTarget, pBlackBrush);
			}
		}

		HRESULT hr = pBitmapRenderTarget->EndDraw();

		hr = pBitmapRenderTarget->GetBitmap(pBitmap);
	}
}