// Bmp.h: interface for the CBmp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BMP_H__33CE1747_4352_11D3_B993_004095605779__INCLUDED_)
#define AFX_BMP_H__33CE1747_4352_11D3_B993_004095605779__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CBmp
{
public:
	CBmp();
	~CBmp();

	BOOL Load(LPCTSTR sFileName);
	BOOL Load(UINT uiResId, HINSTANCE hInstance);
	BOOL Draw(const HDC& hdc, int x = 0, int y = 0);

	int GetWidth() { return m_BITMAP.bmWidth;}
	int GetHeight() { return m_BITMAP.bmHeight;}
	
	void Cleanup();

	BOOL GetPaletteEntries(int iStart, int iNumEntries, LPPALETTEENTRY pPal);

protected:
	HANDLE	m_hBitmap;
	BITMAP	m_BITMAP;
	int		m_iWidth;
	int		m_iHeight;
};

#endif // !defined(AFX_BMP_H__33CE1747_4352_11D3_B993_004095605779__INCLUDED_)
