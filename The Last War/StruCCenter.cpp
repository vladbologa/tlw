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

}

int CStructCCenter::Update()
{
	return 1;
}

void CStructCCenter::SetPosition(int iPosX, int iPosY, CMap *Map)
{
	int px=iPosX/80, py=iPosY/80,i,j;
	PosX=iPosX; PosY=iPosY;
	for (i=0; i<3; i++)
		for (j=0; j<2; j++)
			Map->Hold(py+j,px+i);
}
