// Structure.h: interface for the CStructure class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STRUCTURE_H__D9377583_7386_11D5_95DF_A7276E930B33__INCLUDED_)
#define AFX_STRUCTURE_H__D9377583_7386_11D5_95DF_A7276E930B33__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bmp.h"
#include <stdio.h>

class CStructure
{
protected:
	int PosX, PosY;
	int DestX, DestY;
	BOOL IsSelected;

public:
	int Damage;
	int GetX(){ return PosX; };
	int GetY(){ return PosY; };
	void SetDestination(int iDestX, int iDestY){ DestX = iDestX; DestY = iDestY; };
	void SetPosition(int iPosX, int iPosY){ PosX=iPosX; PosY=iPosY; };
	void Select(BOOL b) {IsSelected=b;};
	BOOL Selected() {return IsSelected;};
	virtual int Update()=0;
	CStructure *next,*prev;
	CStructure();
	virtual ~CStructure();

};


#endif // !defined(AFX_STRUCTURE_H__D9377583_7386_11D5_95DF_A7276E930B33__INCLUDED_)
