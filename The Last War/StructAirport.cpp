// StructAirport.cpp: implementation of the CStructAirport class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StructAirport.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStructAirport::CStructAirport()
{

}

CStructAirport::~CStructAirport()
{

}

int CStructAirport::Update()
{
	return 1;
}

void CStructAirport::SetPosition(int iPosX, int iPosY, CMap *Map)
{
	int px=iPosX/80, py=iPosY/80,i,j;
	PosX=iPosX; PosY=iPosY;
	for (i=0; i<4; i++)
		for (j=0; j<4; j++)
			Map->Hold(py+j,px+i);
}
