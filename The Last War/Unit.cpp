// Unit.cpp: implementation of the CUnit class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Unit.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CUnit::CUnit()
{
	CurrentFrame=0;
}

void CUnit::SetParent(CStructure *p)
{
	Parent=p;
	SetPosition(Parent->GetX(),Parent->GetY());
	SetDestination(PosX+150+rand()%10,PosY);
}

CUnit::~CUnit()
{
	IsTurning=FALSE;
}
