// Map.cpp: implementation of the CMap class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Map.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMap::CMap()
{

}

CMap::~CMap()
{

}

void CMap::Load(int level)
{
	FILE *in; 

	in = fopen("data\\Maps\\map.lwm", "rb");

	fread(&SizeX,sizeof(BYTE),1,in);
	fread(&SizeY,sizeof(BYTE),1,in);
	for (int i = 0; i < SizeY; i++)
		for (int j = 0; j < SizeX; j++)
			fread(&Terrain[i][j], sizeof(BYTE), 1, in);
}
			
BOOL CMap::Load(char *filename)
{
	return FALSE;
}

BYTE CMap::GetTerrainType(BYTE i, BYTE j)
{
	if ((i<SizeY)&&(j<SizeX))
		return Terrain[i][j];
	return 0;
}