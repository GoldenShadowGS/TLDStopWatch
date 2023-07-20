#pragma once
#include "PCH.h"

class RenderableText
{
public:
	RenderableText(IDWriteTextFormat* pFormat, float x, float y, float sx, float sy, float s);
	void SetPosition(float x, float y);
	void SetText(const std::wstring& text);
	void Draw(ID2D1HwndRenderTarget* rt, ID2D1SolidColorBrush* brush);
	void Draw(ID2D1BitmapRenderTarget* brt, ID2D1SolidColorBrush* brush);
private:
	D2D1_RECT_F m_Rect;
	IDWriteTextFormat* pTextFormat = nullptr;
	float m_x;
	float m_y;
	float m_sx;
	float m_sy;
	std::wstring m_Text;
};