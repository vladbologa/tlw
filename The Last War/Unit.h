// Unit.h: interface for the CUnit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UNIT_H__03A84EC0_9A1C_11D3_9EE3_004095605779__INCLUDED_)
#define AFX_UNIT_H__03A84EC0_9A1C_11D3_9EE3_004095605779__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bmp.h"
#include "structure.h"

class CUnit  
{
protected:
	CStructure *Parent;
	int PosX, PosY;
	int DestX, DestY;
	int CurrentFrame, TurnDestFrame;
	int iSubType;
	int iLife;
	BOOL bIsTurning, bIsSelected, bIsAttacked;

public:
	int GetSubType() { return iSubType; };
	void SetSubType(int iNewType) { iSubType=iNewType; };
	void SetParent(CStructure *p);
	void Damage(int iDamage) { iLife-=iDamage; };
	int GetCurrentFrame() { return CurrentFrame; };
	int GetX(){ return PosX; };
	int GetY(){ return PosY; };
	int GetDestX() { return DestX; };
	int GetDestY() { return DestY; };
	void SetDestination(int iDestX, int iDestY){ DestX = iDestX; DestY = iDestY; };
	void SetPosition(int iPosX, int iPosY){ PosX=iPosX; PosY=iPosY; };
	void Select(BOOL b) {bIsSelected=b;}
	void Attack(BOOL b) {bIsAttacked=b;}
	BOOL Selected() {return bIsSelected;}
	BOOL Attacked() {return bIsAttacked;}
	virtual int Update()=0;
	CUnit *next,*prev;
	CUnit();
	virtual ~CUnit();

};

#endif // !defined(AFX_UNIT_H__03A84EC0_9A1C_11D3_9EE3_004095605779__INCLUDED_)