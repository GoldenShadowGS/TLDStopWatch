#pragma once
#include "PCH.h"
#include "Bitmap.h"
#include "RenderableText.h"
#include "ArcRenderer.h"
#include "Input.h"
#include "Resource.h"
#include "Shape.h"

template <class T> void SafeRelease(T** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

class Renderer
{
public:
	Renderer();
	~Renderer();
	BOOL Init(HWND hwnd, const ClockInfo& clockinfo, D2D1_RECT_F clientRect);
	void Render(ClockInfo& clockinfo);
private:
	void RenderBackGround(const ClockInfo& clockinfo, ID2D1Bitmap** pBitmap, ID2D1SolidColorBrush* pBKGBrush, ID2D1RadialGradientBrush* pRadialBrush);
	HWND m_hWnd = nullptr;
	ID2D1Factory* pD2DFactory = nullptr;
	ID2D1BitmapRenderTarget* pBitmapRenderTarget = nullptr;
	ID2D1Bitmap* pBackGroundBitmap = nullptr;
	ID2D1HwndRenderTarget* pRenderTarget = nullptr;

	ID2D1SolidColorBrush* pBlackBrush = nullptr;
	ID2D1SolidColorBrush* pWhiteBrush = nullptr;
	ID2D1SolidColorBrush* pRedPinkBrush = nullptr;
	ID2D1SolidColorBrush* pTransparentBlackBrush = nullptr;
	ID2D1SolidColorBrush* pGoColorBrush = nullptr;
	ID2D1SolidColorBrush* pStopColorBrush = nullptr;
	ID2D1SolidColorBrush* pAlarmHoverBrush = nullptr;
	ID2D1SolidColorBrush* pBorderBrush = nullptr;
	ID2D1SolidColorBrush* pAdjustBorderTimeBrush = nullptr;

	ID2D1GradientStopCollection* pStopsCollection = nullptr;
	ID2D1GradientStopCollection* pStopsCollectionHover = nullptr;
	ID2D1RadialGradientBrush* pRadialGradientBrush = nullptr;

	D2D1::ColorF ClearColor = { 0.0f,0.0f,0.0f };

	IDWriteFactory* pDWriteFactory = nullptr;
	IDWriteTextFormat* pTextFormat = nullptr;

	ArcRenderer arcRenderer;
	ArcRenderer arcRendererThin;
	ArcRenderer arcAdjustTime;

	std::unique_ptr<Bitmap> minutehandbitmap;
	std::unique_ptr<Bitmap> minutehandhighlightedbitmap;
	std::unique_ptr<Bitmap> hourhandbitmap;
	std::unique_ptr<Bitmap> minutehandbitmapShadow;
	std::unique_ptr<Bitmap> hourhandbitmapShadow;
	std::unique_ptr<Bitmap> UIbitmap;
	std::unique_ptr<Bitmap> UIHoverbitmap;

	PlayShape playshape;
	PauseShape pauseshape;

	D2D1_RECT_F m_ClientRect = {};

	Sprite UIsprite = {};
};