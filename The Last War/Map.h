// Map.h: interface for the CMap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAP_H__0DD36C40_D262_11CD_9F42_004095605779__INCLUDED_)
#define AFX_MAP_H__0DD36C40_D262_11CD_9F42_004095605779__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>

class CMap  
{
	BYTE SizeX, SizeY;
	BYTE Free[256][256];
	BYTE Terrain[128][128];
public:
	BYTE GetSizeY(){return SizeY;}
	BYTE GetSizeX(){return SizeX;}
	BYTE GetTerrainType(BYTE i, BYTE j);
	void Hold(int i, int j) {Free[i][j]=0;}
	void Release(int i, int j) {Free[i][j]=1;}
	BOOL Available(BYTE i, BYTE j);
	void Load(char *filename);
	CMap();
	virtual ~CMap();

};

#endif // !defined(AFX_MAP_H__0DD36C40_D262_11CD_9F42_004095605779__INCLUDED_)
