// StruCCenter.cpp: implementation of the CStruCCenter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StruCCenter.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStructCCenter::CStructCCenter()
{
}

CStructCCenter::~CStructCCenter()
{
	int px=PosX/80, py=PosY/80,i,j;
	for (i=0; i<3; i++)
		for (j=0; j<2; j++)
			map->Release(py+j,px+i);
}

int CStructCCenter::Update()
{
	CStructure::Update();
	return 1;
}

void CStructCCenter::SetPosition(int iPosX, int iPosY, CMap *Map)
{
	int px=iPosX/80, py=iPosY/80,i,j;
	map=Map;
	PosX=iPosX; PosY=iPosY;
	for (i=0; i<3; i++)
		for (j=0; j<2; j++)
			Map->Hold(py+j,px+i);
}
