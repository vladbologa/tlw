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
	iLife=100;
	bIsTurning=bIsSelected=bIsAttacked=FALSE;
}

void CUnit::SetParent(CStructure *p)
{
	Parent=p;
	SetPosition(Parent->GetX(),Parent->GetY());
	SetDestination(PosX+150+rand()%10,PosY);
}

CUnit::~CUnit()
{
}
