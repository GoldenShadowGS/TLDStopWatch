#pragma once
#include "PCH.h"
#include "Bitmap.h"
#include "RenderableText.h"
#include "ArcRenderer.h"
#include "Input.h"
#include "Resource.h"

template <class T> void SafeRelease(T** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

struct ClockInfo
{
	//hands
	float centerX = 0.0f;
	float centerY = 0.0f;
	float minuteAngle = 0.0f;
	float hourAngle = 0.0f;
	float minAngleRad = 0.0f;
	float alarmAngleRad = 0.0f;
	float rangeAngleRad = 0.0f;
	int wraps = 0;
	BOOL AlarmSet = FALSE;
	float shadowoffsetx = 5.0f;
	float shadowoffsety = 6.0f;
	float InnerRadius = 0.0f;
	float OuterRadius = 0.0f;
};

class Renderer
{
public:
	Renderer();
	~Renderer();
	BOOL Init(HWND hwnd, const ClockInfo& clockinfo, D2D1_RECT_F clientRect);
	void Render(const ClockInfo& clockinfo);
private:
	void RenderBackGround(const ClockInfo& clockinfo);
	HWND m_hWnd = nullptr;
	ID2D1Factory* pD2DFactory = nullptr;
	//ID2D1DeviceContext* pDeviceContext = nullptr;
	ID2D1BitmapRenderTarget* pBitmapRenderTarget = nullptr;
	ID2D1Bitmap* pBackGroundBitmap = nullptr;
	ID2D1HwndRenderTarget* pRenderTarget = nullptr;
	ID2D1SolidColorBrush* pBlackBrush = nullptr;
	ID2D1SolidColorBrush* pWhiteBrush = nullptr;
	ID2D1SolidColorBrush* pRedPinkBrush = nullptr;
	ID2D1LinearGradientBrush* pGradientBrush1 = nullptr;
	ID2D1GradientStopCollection* pStopsCollection = nullptr;
	ID2D1RadialGradientBrush* pRadialGradientBrush = nullptr;

	D2D1::ColorF ClearColor = {0.0f,0.0f,0.0f};

	IDWriteFactory* pDWriteFactory = nullptr;
	IDWriteTextFormat* pTextFormat = nullptr;

	ArcRenderer arcRenderer;
	ArcRenderer arcRendererThin;

	std::unique_ptr<Bitmap> minutehandbitmap;
	std::unique_ptr<Bitmap> hourhandbitmap;
	std::unique_ptr<Bitmap> minutehandbitmapShadow;
	std::unique_ptr<Bitmap> hourhandbitmapShadow;

	D2D1_RECT_F m_ClientRect = {};
};