// StructAirport.h: interface for the CStructAirport class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STRUCTAIRPORT_H__7316CB8D_BC96_445A_BBEB_37F3BDAC8D42__INCLUDED_)
#define AFX_STRUCTAIRPORT_H__7316CB8D_BC96_445A_BBEB_37F3BDAC8D42__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Structure.h"

class CStructAirport : public CStructure  
{
public:
	virtual void SetPosition(int iPosX, int iPosY, CMap *Map);
	int Update();
	CStructAirport();
	virtual ~CStructAirport();
	virtual int GetType() { return 4;}
};

#endif // !defined(AFX_STRUCTAIRPORT_H__7316CB8D_BC96_445A_BBEB_37F3BDAC8D42__INCLUDED_)
