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
	int px=PosX/80, py=PosY/80,i,j;
	for (i=0; i<4; i++)
		for (j=0; j<3; j++)
			map->Release(py+j,px+i);
}

int CStructOilPlant::Update()
{
	CStructure::Update();
	return 1;
}

void CStructOilPlant::SetPosition(int iPosX, int iPosY, CMap *Map)
{
	int px=iPosX/80, py=iPosY/80,i,j;
	map=Map;
	PosX=iPosX; PosY=iPosY;
	for (i=0; i<4; i++)
		for (j=0; j<3; j++)
			Map->Hold(py+j,px+i);
}