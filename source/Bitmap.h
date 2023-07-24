#pragma once
#include "PCH.h"
#include "Elements.h"

struct Sprite
{
	UINT rows;
	UINT cols;
	float spritewidth;
	float spriteheight;
};

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
	float halfwidth = 0.0f;
	float halfheight = 0.0f;
	float m_angleoffsetDeg = 0.0f;
	std::vector<BYTE> m_RawPixels;
	std::vector<BYTE> m_FlippedRawPixels;
	std::vector<BYTE> m_ExpandedPixels;
public:
	Bitmap();
	bool Load(ID2D1HwndRenderTarget* rt, int resource, BYTE r, BYTE g, BYTE b, float pivotx, float pivoty, float angleoffsetDeg, float scale);
	void FlipImage();
	void Draw(ID2D1HwndRenderTarget* rt, float angle, float x, float y);
	void DrawSprite(ID2D1HwndRenderTarget* rt, int button, const Sprite& sprite, float angle, UI_ButtonPosition buttonpos, float scale);
private:
	bool FileLoader(int resource);
};