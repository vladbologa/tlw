// UnitPlane.cpp: implementation of the CUnitPlane class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UnitPlane.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUnitPlane::CUnitPlane()
{
	modxold=modyold=odd=0;
	TurnDestFrame=0;
}

CUnitPlane::~CUnitPlane()
{

}

int CUnitPlane::Update()
{
	int modx=0, mody=0;

	if (odd==0) odd=1; else odd=0;

	if (DestX!=PosX)
	{
		if (DestX>PosX)
		{
			PosX++;
			modx=1;
		}
		if (DestX<PosX)
		{
			PosX--;
			modx=2;
		}
	}
	if (DestY!=PosY)
	{
		if (DestY>PosY) 
		{
			PosY++;
			mody=1;

		}
		if (DestY<PosY) 
		{
			PosY--;
			mody=2;
		}
	}
	
	if ((modx==0)&&(mody==0))
	{
		modx=modxold;
		mody=modyold;
	}

	if ((modx==modxold)&&(mody==modyold))
	{
		if (IsTurning==FALSE)
		{
			if ((modx==1)&&(mody==0))
				CurrentFrame=23;
			if ((modx==0)&&(mody==1))
				CurrentFrame=16;
			if ((modx==1)&&(mody==1))
				CurrentFrame=19;
			if ((modx==1)&&(mody==2))
				CurrentFrame=27;
			if ((modx==2)&&(mody==1))
				CurrentFrame=12;
			if ((modx==2)&&(mody==0))
				CurrentFrame=8;
			if ((modx==0)&&(mody==2))
				CurrentFrame=0;
			if ((modx==2)&&(mody==2))
				CurrentFrame=4;
			modxold=modx;
			modyold=mody;
		}
	}
	else
	{
		if (!IsTurning)
		{
			IsTurning=TRUE;
			if ((modx==1)&&(mody==0))
				TurnDestFrame=23;
			if ((modx==0)&&(mody==1))
				TurnDestFrame=16;
			if ((modx==1)&&(mody==1))
				TurnDestFrame=19;
			if ((modx==1)&&(mody==2))
				TurnDestFrame=27;
			if ((modx==2)&&(mody==1))
				TurnDestFrame=12;
			if ((modx==2)&&(mody==0))
				TurnDestFrame=8;
			if ((modx==0)&&(mody==2))
				TurnDestFrame=0;
			if ((modx==2)&&(mody==2))
				TurnDestFrame=4;
		}
	}
	
	if (IsTurning)
	{
		if (odd)
		{
			int nw, rw;

			if (CurrentFrame>TurnDestFrame)
			{
				nw=CurrentFrame-TurnDestFrame;
				rw=32-CurrentFrame+TurnDestFrame;
				if (nw>=rw)
					CurrentFrame++;
				else CurrentFrame--;
			}
			if (CurrentFrame<TurnDestFrame) 
			{
				nw=TurnDestFrame-CurrentFrame;
				rw=32-TurnDestFrame+CurrentFrame;
				if (nw>=rw)
					CurrentFrame--;
				else CurrentFrame++;
			}
			if (CurrentFrame==TurnDestFrame) IsTurning=FALSE;
			if (CurrentFrame==-1) CurrentFrame=31;
			if (CurrentFrame==32) CurrentFrame=0;
		}
	}
	return 0;
}