// Unit.h: interface for the CUnit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UNIT_H__03A84EC0_9A1C_11D3_9EE3_004095605779__INCLUDED_)
#define AFX_UNIT_H__03A84EC0_9A1C_11D3_9EE3_004095605779__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bmp.h"
#include <stdio.h>

class CUnit  
{
protected:
	int PosX, PosY;
	int DestX, DestY;
	int CurrentFrame;
	int TurnDestFrame;
	BOOL IsTurning;
	BOOL IsSelected;

public:
	int GetCurrentFrame() { return CurrentFrame; };
	int GetX(){ return PosX; };
	int GetY(){ return PosY; };
	void SetDestination(int iDestX, int iDestY){ DestX = iDestX; DestY = iDestY; };
	void SetPosition(int iPosX, int iPosY){ PosX=iPosX; PosY=iPosY; };
	void Select(BOOL b) {IsSelected=b;};
	BOOL Selected() {return IsSelected;};
	virtual int Update()=0;
	CUnit *next,*prev;
	CUnit();
	virtual ~CUnit();

};

#endif // !defined(AFX_UNIT_H__03A84EC0_9A1C_11D3_9EE3_004095605779__INCLUDED_)