// Misile.cpp: implementation of the CMisile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Misile.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMisile::CMisile()
{

}

CMisile::~CMisile()
{

}

int CMisile::Update()
{
	if (target)
	{
		DestX=target->GetX();
		DestY=target->GetY();
		if (DestX!=PosX)
		{
			if (DestX>PosX)
			{
				PosX+=10;
			}
			if (DestX<PosX)
			{
				PosX-=10;
			}
		}
		if (DestY!=PosY)
		{
			if (DestY>PosY) 
			{
				PosY+=10;
			}
			if (DestY<PosY) 
			{
				PosY-=10;
			}
		}
	}
	return 1;
}
