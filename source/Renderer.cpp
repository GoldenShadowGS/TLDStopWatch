#include "PCH.h"
#include "Renderer.h"
#include "Bitmap.h"
#include "Resource.h"
#include "Math.h"

extern D2D1_RECT_F g_ClientRect;

template <class T> void SafeRelease(T** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

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
	SafeRelease(&pGradientBrush1);
	SafeRelease(&pStopsCollection);
	SafeRelease(&pRadialGradientBrush);
}

BOOL Renderer::Init(HWND hwnd, const ClockInfo& clockinfo)
{
	m_hWnd = hwnd;
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

	hr = pRenderTarget->CreateCompatibleRenderTarget(&pBitmapRenderTarget);

	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&pDWriteFactory));
	if (FAILED(hr))
		return false;


	hr = pDWriteFactory->CreateTextFormat(L"Arial", NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 24.0f, L"en-us", &pTextFormat);
	if (FAILED(hr))
		return false;

	D2D1_GRADIENT_STOP gstop[2] = {};
	gstop[0].color = { 0.8f, 0.8f, 0.89f, 1.0f };
	gstop[0].position = { 0.75f };
	gstop[1].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	gstop[1].position = { 1.0f };
	hr = pRenderTarget->CreateGradientStopCollection(gstop, 2, &pStopsCollection);
	if (FAILED(hr))
		return false;

	D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES gradprops = {};
	gradprops.startPoint = { 0,0 };
	gradprops.endPoint = { 500,500 };
	D2D1_BRUSH_PROPERTIES brushprops = {};
	brushprops.opacity = 1.0f;
	brushprops.transform = D2D1::Matrix3x2F::Identity();
	hr = pRenderTarget->CreateLinearGradientBrush(gradprops, brushprops, pStopsCollection, &pGradientBrush1);
	if (FAILED(hr))
		return false;

	hr = pRenderTarget->CreateRadialGradientBrush(
		D2D1::RadialGradientBrushProperties(D2D1::Point2F(100, 100), D2D1::Point2F(0, 0), 400, 400), pStopsCollection, &pRadialGradientBrush);


	minutehandbitmap = std::make_unique<Bitmap>();
	minutehandbitmapShadow = std::make_unique<Bitmap>();
	hourhandbitmap = std::make_unique<Bitmap>();
	hourhandbitmapShadow = std::make_unique<Bitmap>();

	//minute hand  168,19  
	if (!minutehandbitmap->Load(pRenderTarget, IDBITMAP_MINUTEHAND, 245, 30, 20, 9.5f, 9.25f))
		return false;

	if (!minutehandbitmapShadow->Load(pRenderTarget, IDBITMAP_MINUTEHANDSHADOW, 1, 1, 1, 93.5, 19.125f))
		return false;

	//hourhand 130,23
	if (!hourhandbitmap->Load(pRenderTarget, IDBITMAP_HOURHAND, 30, 50, 30, 11.5f, 11.25f))
		return false;

	if (!hourhandbitmapShadow->Load(pRenderTarget, IDBITMAP_HOURHANDSHADOW, 1, 1, 1, 76.5f, 22.75f))
		return false;

	arcRenderer.SetBrush(pRedPinkBrush);
	arcRenderer.Setalpha(0.5f);

	arcRendererThin.SetBrush(pBlackBrush);
	arcRendererThin.Setalpha(0.75f);

	pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

	ClockNumbers.reserve(12);
	constexpr float TwoPi = 2.0f * 3.141592f;
	constexpr float anglesegment = ((float)1.0f / 12.0f) * TwoPi;
	constexpr float offsetangle = 0.18f;
	for (int i = 0; i < 12; i++)
	{
		float angle = ((float)i / 12.0f) * TwoPi + anglesegment;
		float x = sin(angle) * 95.0f;
		float y = -cos(angle) * 95.0f;
		ClockNumbers.emplace_back(pTextFormat, x, y, 30.0f, 30.0f, 1.0f);
		ClockNumbers.back().SetText(std::to_wstring((i + 1) * 5));
	}

	RenderBackGround(clockinfo);
	if (!pBackGroundBitmap)
		return false;

	return true;
}

void Renderer::Render(const ClockInfo& clockinfo)
{
	if (pRenderTarget)
	{
		pRenderTarget->BeginDraw();
		pRenderTarget->SetTransform(D2D1::IdentityMatrix());

		if (pBackGroundBitmap)
			pRenderTarget->DrawBitmap(pBackGroundBitmap, g_ClientRect);

		if (clockinfo.AlarmSet)
		{
			float radius = 185.0f * clockinfo.scale;
			D2D1_ELLIPSE ellipse = { { clockinfo.centerX,  clockinfo.centerY }, radius, radius };

			float circleRadius = 132.5f;
			for (int i = 0; i < clockinfo.wraps; i++)
			{
				circleRadius -= 10.0f;
				ellipse.radiusX = circleRadius-5.0f;
				ellipse.radiusY = circleRadius-5.0f;
				pRenderTarget->DrawEllipse(ellipse, pBlackBrush, 1.0f);
				ellipse.radiusX = circleRadius;
				ellipse.radiusY = circleRadius;
				pRenderTarget->DrawEllipse(ellipse, pRedPinkBrush, 10.0f);
			}

			// Alarm Pink Arc Section
			arcRenderer.SetRadiusInner(circleRadius - 15.0f);
			arcRenderer.SetRadiusOuter(circleRadius - 5.0f);
			arcRenderer.SetCenter(clockinfo.centerX, clockinfo.centerY);
			arcRenderer.SetAngles(clockinfo.minAngleRad, clockinfo.alarmAngleRad, clockinfo.rangeAngleRad);
			arcRenderer.Draw(pD2DFactory, pRenderTarget);

			arcRendererThin.SetRadiusInner(circleRadius - 15.5f);
			arcRendererThin.SetRadiusOuter(circleRadius - 14.5f);
			arcRendererThin.SetCenter(clockinfo.centerX, clockinfo.centerY);
			arcRendererThin.SetAngles(clockinfo.minAngleRad, clockinfo.alarmAngleRad, clockinfo.rangeAngleRad);
			arcRendererThin.Draw(pD2DFactory, pRenderTarget);
			
			//Draw Alarm Tick
			float alarmAngle = Rad2Deg(clockinfo.alarmAngleRad);
			D2D1_POINT_2F centerpoint = { clockinfo.centerX, clockinfo.centerY };
			D2D1_POINT_2F point1 = { clockinfo.centerX + radius - ((clockinfo.wraps+1) * 10.0f + 10.0f), clockinfo.centerY };
			D2D1_POINT_2F point2 = { clockinfo.centerX + radius + 14.0f, clockinfo.centerY };
			pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation(alarmAngle, centerpoint));
			pRenderTarget->DrawLine(point1, point2, pBlackBrush, 3.0f);
		}

		// Draw Bitmap Clock Hands
		minutehandbitmapShadow->Draw(pRenderTarget, clockinfo.minuteAngle, clockinfo.scale, clockinfo.centerX + clockinfo.minuteshadowoffsetx * clockinfo.scale, clockinfo.centerY + clockinfo.minuteshadowoffsety * clockinfo.scale);
		hourhandbitmapShadow->Draw(pRenderTarget, clockinfo.hourAngle, clockinfo.scale, clockinfo.centerX + clockinfo.hourshadowoffsetx * clockinfo.scale, clockinfo.centerY + clockinfo.hourshadowoffsety * clockinfo.scale);
		hourhandbitmap->Draw(pRenderTarget, clockinfo.hourAngle, clockinfo.scale, clockinfo.centerX, clockinfo.centerY);
		minutehandbitmap->Draw(pRenderTarget, clockinfo.minuteAngle, clockinfo.scale, clockinfo.centerX, clockinfo.centerY);


		//pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

		//WCHAR mytext[] = L"Hello World\n Whoops";

		//pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		//D2D1_RECT_F rect = { (float)rc.left,(float)rc.top, (float)rc.right, (float)rc.bottom };
		//pRenderTarget->DrawTextW(mytext, ARRAYSIZE(mytext) - 1, pTextFormat, rect, pBlackBrush);
		HRESULT hr = pRenderTarget->EndDraw();
	}
}

void Renderer::RenderBackGround(const ClockInfo& clockinfo)
{
	if (pBitmapRenderTarget)
	{
		pBitmapRenderTarget->BeginDraw();
		pBitmapRenderTarget->SetTransform(D2D1::IdentityMatrix());
		pBitmapRenderTarget->Clear(ClearColor);


		D2D1_POINT_2F center = { clockinfo.centerX,  clockinfo.centerY };
		D2D1_ELLIPSE ellipse = {};
		float radius = 185.0f * clockinfo.scale;
		ellipse.radiusX = radius;
		ellipse.radiusY = radius;
		ellipse.point = center;
		pRadialGradientBrush->SetCenter(center);
		pRadialGradientBrush->SetRadiusX(radius * 1.2f);
		pRadialGradientBrush->SetRadiusY(radius * 1.2f);
		pBitmapRenderTarget->DrawEllipse(ellipse, pRadialGradientBrush, 30.0f * clockinfo.scale);
		radius = 170.0f * clockinfo.scale;
		ellipse.radiusX = radius;
		ellipse.radiusY = radius;
		pBitmapRenderTarget->DrawEllipse(ellipse, pBlackBrush, 5.0f * clockinfo.scale);
		pBitmapRenderTarget->FillEllipse(ellipse, pWhiteBrush);

		// Draw Large Ticks
		for (int i = 0; i < 12; i++)
		{
			float angle = ((float)i / 12.0f) * 360.0f;
			D2D1_POINT_2F centerpoint = { clockinfo.centerX, clockinfo.centerY };
			D2D1_POINT_2F point1 = { clockinfo.centerX + radius - 14.0f, clockinfo.centerY };
			D2D1_POINT_2F point2 = { clockinfo.centerX + radius, clockinfo.centerY };
			pBitmapRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation(angle, centerpoint));
			pBitmapRenderTarget->DrawLine(point1, point2, pBlackBrush, 2.0f);
		}
		// Draw Small ticks
		for (int i = 0; i < 60; i++)
		{
			float angle = ((float)i / 60.0f) * 360.0f;
			D2D1_POINT_2F centerpoint = { clockinfo.centerX, clockinfo.centerY };
			D2D1_POINT_2F point1 = { clockinfo.centerX + radius - 8.0f, clockinfo.centerY };
			D2D1_POINT_2F point2 = { clockinfo.centerX + radius, clockinfo.centerY };
			pBitmapRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation(angle, centerpoint));
			pBitmapRenderTarget->DrawLine(point1, point2, pBlackBrush, 0.75f);
		}
		// Draw Clock Numbers
		pBitmapRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
		for (int i = 0; i < 12; i++)
		{
			ClockNumbers[i].SetPosition(clockinfo.centerX, clockinfo.centerY);
			ClockNumbers[i].Draw(pBitmapRenderTarget, pBlackBrush);
		}

		HRESULT hr = pBitmapRenderTarget->EndDraw();

		hr = pBitmapRenderTarget->GetBitmap(&pBackGroundBitmap);
	}
}