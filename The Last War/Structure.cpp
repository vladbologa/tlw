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
	bIsSelected=bIsAttacked=FALSE;
	iExplodeFrame=iES=0;
}

CStructure::~CStructure()
{
}

int CStructure::Update()
{
	if (iLife<=0)
	{
		iES++;
		if (!(iES%4)) iES=0;
		if (!iES) iExplodeFrame++;
	}
	return 1;
}