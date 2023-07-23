#pragma once
#include "PCH.h"

enum UI_Buttons
{
	NONE = -1,
	RESET = 0,
	CLOSE = 1,
	ADD = 2,
	MINUS = 3
};

struct UI_ButtonPosition
{
	float x = 0.0f;
	float y = 0.0f;
};

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
	std::vector<BYTE> m_RawPixels;
	std::vector<BYTE> m_ExpandedPixels;
public:
	Bitmap();
	bool Load(ID2D1HwndRenderTarget* rt, int resource, BYTE r, BYTE g, BYTE b, float pivotx, float pivoty, float scale);
	void Draw(ID2D1HwndRenderTarget* rt, float angle, float x, float y);
	void DrawSprite(ID2D1HwndRenderTarget* rt, UI_Buttons button, const Sprite& sprite, float angle, UI_ButtonPosition buttonpos, float scale);
private:
	bool FileLoader(int resource);
};