// Unit.cpp: implementation of the CUnit class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Unit.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CUnit::CUnit()
{
	iExplodeFrame=CurrentFrame=iTargetType=iShoot=0;
	iLife=100;
	bIsTurning=bIsSelected=bIsAttacked=bComputerAttack=FALSE;
}

void CUnit::SetParent(CStructure *p)
{
	Parent=p;
	SetPosition(Parent->GetX(),Parent->GetY());
	SetDestination(PosX+150+rand()%100,PosY+rand()%20);
}

CUnit::~CUnit()
{
}

void CUnit::SetTarget(CUnit *ut, CStructure *st, int type)
{
	switch (type)
	{
	case 0:
		UnitTarget=NULL;
		StructTarget=NULL;
		iTargetType=0;
		break;
	case 1:
		UnitTarget=ut;
		StructTarget=NULL;
		iTargetType=1;
		break;
	case 2:
		StructTarget=st;
		UnitTarget=NULL;
		iTargetType=2;
		break;
	}
}

int CUnit::Update()
{
	int difx, dify;
	double r,x,y,dist;
	
	if (iShoot>0) iShoot--;
	if (iTargetType==1) if (UnitTarget->GetExplodeFrame()) iTargetType=0;
	if (iTargetType==2) if (StructTarget->GetExplodeFrame()) iTargetType=0;
	if (iTargetType==1)
	{
		dist=sqrt((DestX-UnitTarget->PosX)*(DestX-UnitTarget->PosX)+(DestY-UnitTarget->PosY)*(DestY-UnitTarget->PosY));
		if (dist>300)
		{
			if (UnitTarget->PosX>DestX) difx=UnitTarget->PosX-DestX;
			else difx=DestX-UnitTarget->PosX;
			if (UnitTarget->PosY>DestY) dify=UnitTarget->PosY-DestY;
			else dify=DestY-UnitTarget->PosY;
			
			if (difx&&dify) 
			{
				r=(double)dify/difx;
				x=sqrt((dist-300)/(1+r*r));
				y=r*x;
			}
			else if (!difx)
			{
				x=0; y=dist-300;
			}
			else if (!dify)
			{
				x=dist-300; y=0;
			}
			
			if (UnitTarget->PosX>DestX)
			{
				DestX+=(int)x;
			}
			if (UnitTarget->PosX<DestX)
			{
				DestX-=(int)x;
			}
			if (UnitTarget->PosY>DestY) 
			{
				DestY+=(int)y;
			}
			if (UnitTarget->PosY<DestY) 
			{
				DestY-=(int)y;
			}
		}
	}
	if (iTargetType==2)
	{
		dist=sqrt((DestX-StructTarget->GetX())*(DestX-StructTarget->GetX())+(DestY-StructTarget->GetY())*(DestY-StructTarget->GetY()));
		if (dist>300)
		{
			if (StructTarget->GetX()>DestX) difx=StructTarget->GetX()-DestX;
			else difx=DestX-StructTarget->GetX();
			if (StructTarget->GetY()>DestY) dify=StructTarget->GetY()-DestY;
			else dify=DestY-StructTarget->GetY();
			
			if (difx&&dify) 
			{
				r=(double)dify/difx;
				x=sqrt((dist-300)/(1+r*r));
				y=r*x;
			}
			else if (!difx)
			{
				x=0; y=dist-300;
			}
			else if (!dify)
			{
				x=dist-300; y=0;
			}
			
			if (StructTarget->GetX()>DestX)
			{
				DestX+=(int)x;
			}
			if (StructTarget->GetX()<DestX)
			{
				DestX-=(int)x;
			}
			if (StructTarget->GetY()>DestY) 
			{
				DestY+=(int)y;
			}
			if (StructTarget->GetY()<DestY) 
			{
				DestY-=(int)y;
			}
		}
	}
	return 0;
}

void CUnit::SetDestination(int iDestX, int iDestY)
{
	DestX=iDestX; DestY=iDestY;
	if (DestX<0) DestX=0;
	if (DestY<0) DestY=0;
	if (DestX>=160*iMapSizeX-120) DestX=160*iMapSizeX-120;
	if (DestY>=160*iMapSizeY-120) DestY=160*iMapSizeY-120;
}

BOOL CUnit::InRange()
{
	double dist;
	if (iTargetType)
	{
		if (iTargetType==1) 
			dist=sqrt((PosX-UnitTarget->PosX)*(PosX-UnitTarget->PosX)+(PosY-UnitTarget->PosY)*(PosY-UnitTarget->PosY));
		if (iTargetType==2) 
			dist=sqrt((PosX-StructTarget->GetX())*(PosX-StructTarget->GetX())+(PosY-StructTarget->GetY())*(PosY-StructTarget->GetY()));
		if (dist>325) return FALSE;
		return TRUE;
	}
	return FALSE;
}

BOOL CUnit::Attacking()
{
	if (iTargetType) return TRUE;
	return FALSE;
}
