// Bmp.cpp: implementation of the CBmp class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Bmp.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBmp::CBmp()
{
	m_hBitmap = NULL;
	ZeroMemory(&m_BITMAP, sizeof(BITMAP));
	m_iWidth = 0;
	m_iHeight = 0;
}

CBmp::~CBmp()
{
	Cleanup();
}

void CBmp::Cleanup()
{
	if(m_hBitmap)
	{
		DeleteObject(m_hBitmap);
		m_hBitmap = NULL;
	}
	ZeroMemory(&m_BITMAP, sizeof(BITMAP));
}

BOOL CBmp::Load(LPCTSTR sFileName)
{
	if(m_hBitmap)
		Cleanup();

	m_hBitmap = LoadImage(NULL, sFileName, IMAGE_BITMAP, 0, 0,
		LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);

	if(!m_hBitmap)
		return FALSE;

	int iBytes = GetObject(m_hBitmap, sizeof(BITMAP), (LPVOID)&m_BITMAP);
	if(!iBytes || iBytes > sizeof(BITMAP))
	{
		Cleanup();
		return FALSE;
	}

	return TRUE;
}

BOOL CBmp::Load(UINT uiResId, HINSTANCE hInstance)
{
	LPTSTR szResourceName = MAKEINTRESOURCE(uiResId);

	m_hBitmap = LoadImage(hInstance, szResourceName, IMAGE_BITMAP, 0, 0,
		LR_CREATEDIBSECTION | LR_DEFAULTSIZE);

	return TRUE;
}

BOOL CBmp::Draw(const HDC& hdc, int x, int y)
{
	if(!m_hBitmap)
		return FALSE;

	HDC hdcMem = CreateCompatibleDC(hdc);
	if(!hdcMem)
		return FALSE;

	HGDIOBJ hOldBm = SelectObject(hdcMem, m_hBitmap);
	BitBlt(hdc, x, y, GetWidth(), GetHeight(), hdcMem, 0, 0, SRCCOPY);
	SelectObject(hdcMem, hOldBm);

	DeleteDC(hdcMem);

	return TRUE;
}

BOOL CBmp::GetPaletteEntries(int iStart, int iNumEntries, LPPALETTEENTRY pPal)
{
	if(!m_hBitmap)
		return FALSE;

	if(iStart < 0 || iStart > 256 || (iNumEntries - iStart) > 256 || iNumEntries > 256)
		return FALSE;

	RGBQUAD	rgb[256];
	ZeroMemory(&rgb, sizeof(rgb));
	
	HDC hdc = CreateDC("DISPLAY", NULL, NULL, NULL);
	if(!hdc)
		return FALSE;

	HDC hdcMem = CreateCompatibleDC(hdc);
	if(!hdcMem)
	{
		DeleteDC(hdc);
		return FALSE;
	}

	HGDIOBJ hOldBm = SelectObject(hdcMem, m_hBitmap);
	int iTableEntries = GetDIBColorTable(hdcMem, 0, 256, rgb);
	SelectObject(hdcMem, hOldBm);

	int iEnd;
	if(iStart + iNumEntries > iTableEntries)
		iEnd = iTableEntries;
	else
		iEnd = iNumEntries;

	for (int i = iStart; i < iEnd; i++)
	{
        pPal[i].peRed = rgb[i].rgbRed;
        pPal[i].peGreen = rgb[i].rgbGreen;
        pPal[i].peBlue = rgb[i].rgbBlue;
        pPal[i].peFlags = 0;
    }

	DeleteDC(hdcMem);
	DeleteDC(hdc);

	if(iTableEntries)
		return TRUE;
	else
		return FALSE;
}