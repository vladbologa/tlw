// Structure.cpp: implementation of the CStructure class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Structure.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStructure::CStructure()
{
	iLife=1000;
	bIsSelected=bIsAttacked=bBuildingUnit=bBuildingComplete=bSelectPlace=FALSE;
	iExplodeFrame=iES=iUnitType=0;
}

CStructure::~CStructure()
{
}

int CStructure::Update()
{
	if (bBuildingUnit)
	{
		if (iUnitConstruction>0) iUnitConstruction--;
		else
		{
			bBuildingUnit=FALSE;
			bBuildingComplete=TRUE;
		}
	}
	if (iLife<=0)
	{
		iES++;
		if (!(iES%4)) iES=0;
		if (!iES) iExplodeFrame++;
	}
	return 1;
}

void CStructure::BuildUnit(int type)
{
	iUnitType=type;
	bBuildingUnit=TRUE;
	switch (type)
	{
	case 1:
		iUnitConstruction=600;
		break;
	case 2:
		iUnitConstruction=900;
		break;
	case 3:
		iUnitConstruction=3000;
		break;
	case 4:
		iUnitConstruction=2000;
		break;
	case 5:
		iUnitConstruction=2000;
		break;
	case 6:
		iUnitConstruction=1000;
		break;
	}
	iInitial=iUnitConstruction;
}

BOOL CStructure::UBComplete()
{
	if (bBuildingComplete)
	{
		if (iUnitType<3)
			bBuildingComplete=FALSE;
		return TRUE;
	}
	return FALSE;
}

int CStructure::PercentageComplete()
{
	return (iInitial-iUnitConstruction)*100/iInitial;
}

void CStructure::PlaceBuilding()
{
	bBuildingComplete=FALSE;
	bSelectPlace=FALSE;
}
