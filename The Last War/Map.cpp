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
	SizeX = 12;
	SizeY = 16;

	FILE *in; 

	in = fopen("c:\\GameArt\\level3.lwm", "rb");

	for (int i = 0; i < SizeX; i++)
		for (int j = 0; j < SizeY; j++)
			fread(&Terrain[i][j], sizeof(BYTE), 1, in);
}
			
BOOL CMap::Load(char *filename)
{
	return FALSE;
}

BYTE CMap::GetTerrainType(int i, int j)
{
	if ((i<SizeX)&&(j<SizeY))
		return Terrain[i][j];
	return 0;
}