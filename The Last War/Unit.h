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
public:
	CBmp SpriteArray[33];
	int GetCurrentFrame() { return CurrentFrame; };
	int GetX(){ return PosX; };
	int GetY(){ return PosY; };
	int SetDestination(int iDestX, int iDestY){ DestX = iDestX; DestY = iDestY; };
	int SetPosition(int iPosX, int iPosY){ PosX=iPosX; PosY=iPosY; };
	int Update();
	CUnit();
	virtual ~CUnit();

};

#endif // !defined(AFX_UNIT_H__03A84EC0_9A1C_11D3_9EE3_004095605779__INCLUDED_)
