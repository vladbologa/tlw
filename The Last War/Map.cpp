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

void CMap::Load(char *filename)
{
	int i,j;
	FILE *in;

	in = fopen(filename, "rb");

	fread(&SizeX,sizeof(BYTE),1,in);
	fread(&SizeY,sizeof(BYTE),1,in);
	for (i = 0; i < SizeY; i++)
		for (j = 0; j < SizeX; j++)
		{
			fread(&Terrain[i][j], sizeof(BYTE), 1, in);
			if (Terrain[i][j]==2)
			{
				Free[i*2][j*2]=0;
				Free[i*2+1][j*2]=0;
				Free[i*2][j*2+1]=0;
				Free[i*2+1][j*2+1]=0;
			}
			else 
			{
				Free[i*2][j*2]=1;
				Free[i*2+1][j*2]=1;
				Free[i*2][j*2+1]=1;
				Free[i*2+1][j*2+1]=1;
			}
		}
	
	/*FILE *fout; 
	fout = fopen("c:\\debug.txt", "wt");
	fprintf(fout, "Terrain Map\n");
	for (i = 0; i < SizeY; i++)
	{
		for (j = 0; j < SizeX; j++)
			fprintf(fout, "%d ", Terrain[i][j]);
		fprintf(fout, "\n");
	}
	fprintf(fout, "Free Map\n");
	for (i = 0; i < 2*SizeY; i++)
	{
		for (j = 0; j < 2*SizeX; j++)
			fprintf(fout, "%d ", Free[i][j]);
		fprintf(fout, "\n");
	}
	fclose(fout);*/

	fclose(in);
}
			
BYTE CMap::GetTerrainType(BYTE i, BYTE j)
{
	if ((i<SizeY)&&(j<SizeX))
		return Terrain[i][j];
	return 0;
}

BOOL CMap::Available(BYTE i, BYTE j)
{
	return Free[i][j];
}