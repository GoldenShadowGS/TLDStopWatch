#include "PCH.h"
#include "RenderableText.h"

// TODO ReWrite this class
RenderableText::RenderableText(IDWriteTextFormat* pFormat, float x, float y, float sx, float sy, float s) : pTextFormat(pFormat), m_x(x), m_y(y), m_sx(sx), m_sy(sy)
{
	m_Rect = D2D1::RectF(x - sx / 2.0f, y - sy / 2.0f, x + sx / 2.0f, y + sy / 2.0f);
}

void RenderableText::SetPosition(float x, float y)
{
	m_Rect = D2D1::RectF(m_x - m_sx / 2.0f + x, m_y - m_sy / 2.0f + y, m_x + m_sx / 2.0f + x, m_y + m_sy / 2.0f + y);
}

void RenderableText::SetText(const std::wstring& text)
{
	m_Text = text;
}

void RenderableText::Draw(ID2D1HwndRenderTarget* rt, ID2D1SolidColorBrush* brush)
{
	rt->DrawTextW(m_Text.c_str(), (UINT)m_Text.size(), pTextFormat, m_Rect, brush);
}

void RenderableText::Draw(ID2D1BitmapRenderTarget* brt, ID2D1SolidColorBrush* brush)
{
	brt->DrawTextW(m_Text.c_str(), (UINT)m_Text.size(), pTextFormat, m_Rect, brush);
}