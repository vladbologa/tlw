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
public:
	BYTE GetSizeY(){return SizeY;};
	BYTE GetSizeX(){return SizeX;};
	BYTE GetTerrainType(BYTE i, BYTE j);
	
	BYTE Terrain[256][256];
	BOOL Load(char *filename);
	void Load(int level);
	CMap();
	virtual ~CMap();

};

#endif // !defined(AFX_MAP_H__0DD36C40_D262_11CD_9F42_004095605779__INCLUDED_)
