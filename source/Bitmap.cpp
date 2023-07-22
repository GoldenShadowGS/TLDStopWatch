#pragma once
#include "PCH.h"
#include "Bitmap.h"
#include "Resource.h"
#include "ResourceLoader.h"
#include "Renderer.h"

Bitmap::Bitmap()
{}

bool Bitmap::Load(ID2D1HwndRenderTarget* rt, int resource, BYTE r, BYTE g, BYTE b, float pivotx, float pivoty, float scale)
{
	FileLoader(resource);
	m_ExpandedPixels.resize(m_RawPixels.size() * 4);
	for (UINT64 i = 0; i < m_RawPixels.size(); i++)
	{
		const size_t expandedindex = i * 4;
		const float alpha = (255 - m_RawPixels[i]) / 255.0f;
		m_ExpandedPixels[expandedindex + 0] = BYTE(b * alpha);
		m_ExpandedPixels[expandedindex + 1] = BYTE(g * alpha);
		m_ExpandedPixels[expandedindex + 2] = BYTE(r * alpha);
		m_ExpandedPixels[expandedindex + 3] = BYTE(alpha * 255.0f);
	}
	m_RawPixels.clear();
	m_Pivot = D2D1::Point2F(pivotx, pivoty);

	D2D1_SIZE_U bitmapsize = {};
	bitmapsize.width = m_PixelWidth;
	bitmapsize.height = m_PixelHeight;
	m_Size.width = (float)m_PixelWidth;
	m_Size.height = (float)m_PixelHeight;

	D2D1_BITMAP_PROPERTIES bitmapprops = {};
	bitmapprops.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	bitmapprops.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	bitmapprops.dpiX = 96.0f;
	bitmapprops.dpiY = 96.0f;
	HRESULT hr = rt->CreateBitmap(bitmapsize, m_ExpandedPixels.data(), m_Pitch, bitmapprops, &pBitmap);
	if (FAILED(hr))
		return false;
	//Rescale
	if (scale != 1.0f)
	{
		ID2D1Bitmap* pScaledBitmap = nullptr;
		D2D1_SIZE_F scaledsize = { bitmapsize.width * scale, bitmapsize.height * scale };
		hr = rt->CreateCompatibleRenderTarget(scaledsize, &pBitmapRenderTarget);

		pBitmapRenderTarget->BeginDraw();

		D2D1_RECT_F BitmapRect = { 0.0f, 0.0f, scaledsize.width, scaledsize.height };
		pBitmapRenderTarget->DrawBitmap(pBitmap, BitmapRect);
		hr = pBitmapRenderTarget->EndDraw();

		//After Drawing, retrieve the bitmap from the render target
		hr = pBitmapRenderTarget->GetBitmap(&pScaledBitmap);
		SafeRelease(&pBitmap);
		SafeRelease(&pBitmapRenderTarget);
		if (pScaledBitmap)
		{
			pBitmap = pScaledBitmap;
			m_Size = pBitmap->GetSize();
		}
	}
	return true;
}

void Bitmap::Draw(ID2D1HwndRenderTarget* rt, float angle, float x, float y)
{
	if (pBitmap)
	{
		const float offsetx = m_Pivot.x * m_Size.width;
		const float offsety = m_Pivot.y * m_Size.height;
		D2D1_POINT_2F Pivot = { offsetx + x, offsety + y };
		rt->SetTransform(D2D1::Matrix3x2F::Rotation(angle - 90, Pivot) * D2D1::Matrix3x2F::Translation(-offsetx, -offsety));

		D2D1_RECT_F	rect = D2D1::RectF(x, y, x + m_Size.width, y + m_Size.height);

		rt->DrawBitmap(pBitmap, rect, 1.0f);
	}
}

void Bitmap::DrawSprite(ID2D1HwndRenderTarget* rt, UINT index, const Sprite& sprite, float angle, float x, float y)
{
	if (pBitmap)
	{
		UINT Xindex = index % sprite.cols;
		UINT Yindex = index / sprite.rows;
		const float sourcex = sprite.spritewidth * Xindex;
		const float sourcey = sprite.spriteheight * Yindex;
		const float offsetx = m_Pivot.x * m_Size.width;
		const float offsety = m_Pivot.y * m_Size.height;
		D2D1_POINT_2F Pivot = { offsetx + x, offsety + y };
		rt->SetTransform(D2D1::Matrix3x2F::Rotation(angle - 90, Pivot) * D2D1::Matrix3x2F::Translation(-offsetx, -offsety));

		D2D1_RECT_F	rect = D2D1::RectF(x, y, x + m_Size.width, y + m_Size.height);
		D2D1_RECT_F	sourcerect = D2D1::RectF(sourcex, sourcey, sourcex + sprite.spritewidth, sourcey + sprite.spriteheight);

		rt->DrawBitmap(pBitmap, rect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &sourcerect);
	}
}

inline UINT Align(UINT uLocation, UINT uAlign)
{
	assert(!((0 == uAlign) || (uAlign & (uAlign - 1))));
	return ((uLocation + (uAlign - 1)) & ~(uAlign - 1));
}



bool Bitmap::FileLoader(int resource)
{
	struct BMPHeader
	{
		char bm[2];
		unsigned int bmpfilesize;
		short appspecific1;
		short appspecific2;
		unsigned int pixeldataoffset;
	} bmpHeader = {};
	struct DIBHeader
	{
		unsigned int dibSize;
		unsigned int width;
		unsigned int height;
		short colorplane;
		short bitsperpixel;
		unsigned int pixelcompression;
		unsigned int rawPixelDataSize;
	} dibHeader = {};

	ResourceLoader resLoader(resource, L"BITMAPDATA");
	resLoader.Read(bmpHeader.bm, 2);
	if (!(bmpHeader.bm[0] == 'B' && bmpHeader.bm[1] == 'M'))
		return false;
	resLoader.Read(&bmpHeader.bmpfilesize, sizeof(bmpHeader.bmpfilesize));
	resLoader.Read(&bmpHeader.appspecific1, sizeof(bmpHeader.appspecific1));
	resLoader.Read(&bmpHeader.appspecific2, sizeof(bmpHeader.appspecific2));
	resLoader.Read(&bmpHeader.pixeldataoffset, sizeof(bmpHeader.pixeldataoffset));
	resLoader.Read(&dibHeader.dibSize, sizeof(dibHeader.dibSize));
	resLoader.Read(&dibHeader.width, sizeof(dibHeader.width));
	resLoader.Read(&dibHeader.height, sizeof(dibHeader.height));
	resLoader.Read(&dibHeader.colorplane, sizeof(dibHeader.colorplane));
	resLoader.Read(&dibHeader.bitsperpixel, sizeof(dibHeader.bitsperpixel));
	resLoader.Read(&dibHeader.pixelcompression, sizeof(dibHeader.pixelcompression));
	resLoader.Read(&dibHeader.rawPixelDataSize, sizeof(dibHeader.rawPixelDataSize));

	m_PixelWidth = dibHeader.width;
	m_PixelHeight = dibHeader.height;
	m_Pitch = Align(dibHeader.width * 4, 16);

	const size_t buffersize = dibHeader.rawPixelDataSize;
	m_RawPixels.resize(buffersize);
	resLoader.Seek(bmpHeader.pixeldataoffset);
	resLoader.Read(m_RawPixels.data(), buffersize);

	return true;
}