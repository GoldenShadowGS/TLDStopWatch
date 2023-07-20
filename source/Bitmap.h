#pragma once
#include "PCH.h"


class Bitmap
{
private:
	HBITMAP hBmp = nullptr;
	ID2D1Bitmap* pBitmap = nullptr;
	D2D1_POINT_2F m_Pivot = {};
	UINT m_Width = 0;
	UINT m_Height = 0;
	UINT m_Pitch = 0;
	std::vector<BYTE> m_RawPixels;
	std::vector<BYTE> m_ExpandedPixels;
public:
	Bitmap();
	bool Load(ID2D1HwndRenderTarget* rt, int resource, BYTE r, BYTE g, BYTE b, float pivotx, float pivoty);
	void Draw(ID2D1HwndRenderTarget* rt, float angle, float scale, float x, float y, float width = 0.0f, float height = 0.0f);
private:
	bool FileLoader(int resource);
};