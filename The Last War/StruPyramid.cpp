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

}

int CStructPyramid::Update()
{
	return 1;
}

void CStructPyramid::SetPosition(int iPosX, int iPosY, CMap *Map)
{
	int px=iPosX/80, py=iPosY/80,i,j;
	PosX=iPosX; PosY=iPosY;
	for (i=0; i<3; i++)
		for (j=0; j<2; j++)
			Map->Hold(py+j,px+i);
}
