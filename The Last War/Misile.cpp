// Misile.cpp: implementation of the CMisile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Misile.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMissile::CMissile()
{
	StructTarget=NULL;
	UnitTarget=NULL;
	iTargetType=0;
	iDestroy=0;
}

CMissile::~CMissile()
{

}

void CMissile::SetTarget(CUnit *tu, CStructure *ts, int type)
{
	iTargetType=type;
	if (type==1) UnitTarget=tu;
	else if (type==2) StructTarget=ts;
}

int CMissile::Update()
{
	int difx, dify;
	double r,x,y;
	if (iTargetType==1) if (UnitTarget->GetExplodeFrame()) iDestroy=1;
	if (iTargetType==2) if (StructTarget->GetExplodeFrame()) iDestroy=1;
	if (iTargetType)
	{
		if (iTargetType==1)
		{
			DestX=UnitTarget->GetX()+60;
			DestY=UnitTarget->GetY()+45;
		}
		else if (iTargetType==2)
		{
			DestX=StructTarget->GetX()+60;
			DestY=StructTarget->GetY()+60;
		}
		if (DestX>PosX) difx=DestX-PosX;
		else difx=PosX-DestX;
		if (DestY>PosY) dify=DestY-PosY;
		else dify=PosY-DestY;
		
		if (difx&&dify) 
		{
			r=(double)dify/difx;
			x=sqrt(100/(1+r*r));
			y=r*x;
		}
		else if (!difx)
		{
			x=0; y=10;
		}
		else if (!dify)
		{
			x=10; y=0;
		}
		if (difx>=10)
		{
			if (DestX>PosX)
			{
				PosX+=(int)x;
			}
			if (DestX<PosX)
			{
				PosX-=(int)x;
			}
		}
		if (dify>=10)
		{
			if (DestY>PosY) 
			{
				PosY+=(int)y;
			}
			if (DestY<PosY) 
			{
				PosY-=(int)y;
			}
		}

		if (DestX>PosX) difx=DestX-PosX;
		else difx=PosX-DestX;
		if (DestY>PosY) dify=DestY-PosY;
		else dify=PosY-DestY;

		if ((difx<=10)&&(dify<=10)) 
		{
			if (iTargetType==1) UnitTarget->Damage(20);
			else if (iTargetType==2) StructTarget->Damage(20);
			iDestroy=1;
		}
	}
	return 1;
}