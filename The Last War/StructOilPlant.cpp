// StructOilPlant.cpp: implementation of the CStructOilPlant class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StructOilPlant.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStructOilPlant::CStructOilPlant()
{

}

CStructOilPlant::~CStructOilPlant()
{

}

int CStructOilPlant::Update()
{
	return 1;
}

void CStructOilPlant::SetPosition(int iPosX, int iPosY, CMap *Map)
{
	int px=iPosX/80, py=iPosY/80,i,j;
	PosX=iPosX; PosY=iPosY;
	for (i=0; i<4; i++)
		for (j=0; j<3; j++)
			Map->Hold(py+j,px+i);
}
