// Structure.h: interface for the CStructure class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STRUCTURE_H__D9377583_7386_11D5_95DF_A7276E930B33__INCLUDED_)
#define AFX_STRUCTURE_H__D9377583_7386_11D5_95DF_A7276E930B33__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bmp.h"
#include "map.h"

class CStructure
{
protected:
	int PosX, PosY;
	int DestX, DestY;
	int iLife, iPlayer, iExplodeFrame, iES;
	BOOL bIsSelected, bIsAttacked;
	CMap *map;

public:
	virtual int GetType()=0;
	void Damage(int iDamage) { iLife-=iDamage; }
	int GetExplodeFrame() { return iExplodeFrame; }
	int GetX(){ return PosX; }
	int GetY(){ return PosY; }
	void SetPlayer(int Player) {iPlayer=Player;}
	int GetPlayer() {return iPlayer;}
	void SetDestination(int iDestX, int iDestY){ DestX = iDestX; DestY = iDestY; }
	virtual void SetPosition(int iPosX, int iPosY, CMap *Map)=0;
	void Select(BOOL b) {bIsSelected=b;}
	void Attack(BOOL b) {bIsAttacked=b;}
	BOOL Selected() {return bIsSelected;}
	BOOL Attacked() {return bIsAttacked;}
	BOOL Destroyed() {return (iExplodeFrame>=29?TRUE:FALSE);}	
	virtual int Update();
	CStructure *next,*prev;
	CStructure();
	virtual ~CStructure();

};


#endif // !defined(AFX_STRUCTURE_H__D9377583_7386_11D5_95DF_A7276E930B33__INCLUDED_)
