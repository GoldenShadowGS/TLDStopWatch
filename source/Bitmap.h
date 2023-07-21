#pragma once
#include "PCH.h"


class Bitmap
{
private:
	ID2D1Bitmap* pBitmap = nullptr;
	ID2D1BitmapRenderTarget* pBitmapRenderTarget = nullptr;
	D2D1_POINT_2F m_Pivot = {};
	UINT m_PixelWidth = 0;
	UINT m_PixelHeight = 0;
	UINT m_Pitch = 0;
	D2D1_SIZE_F m_Size = {};
	std::vector<BYTE> m_RawPixels;
	std::vector<BYTE> m_ExpandedPixels;
public:
	Bitmap();
	bool Load(ID2D1HwndRenderTarget* rt, int resource, BYTE r, BYTE g, BYTE b, float pivotx, float pivoty, float scale);
	void Draw(ID2D1HwndRenderTarget* rt, float angle, float x, float y);
private:
	bool FileLoader(int resource);
};