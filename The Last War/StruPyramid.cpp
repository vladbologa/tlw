// StruPyramid.cpp: implementation of the CStruPyramid class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StruPyramid.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStructPyramid::CStructPyramid()
{
}

CStructPyramid::~CStructPyramid()
{
	int px=PosX/80, py=PosY/80,i,j;
	for (i=0; i<3; i++)
		for (j=0; j<2; j++)
			map->Release(py+j,px+i);
}

int CStructPyramid::Update()
{
	CStructure::Update();
	return 1;
}

void CStructPyramid::SetPosition(int iPosX, int iPosY, CMap *Map)
{
	int px=iPosX/80, py=iPosY/80,i,j;
	map=Map;
	PosX=iPosX; PosY=iPosY;
	for (i=0; i<3; i++)
		for (j=0; j<2; j++)
			Map->Hold(py+j,px+i);
}
