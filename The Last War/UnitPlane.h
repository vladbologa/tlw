// UnitPlane.h: interface for the CUnitPlane class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UNITPLANE_H__7BF6C8C6_08DF_41CE_A156_A63ECFB1B868__INCLUDED_)
#define AFX_UNITPLANE_H__7BF6C8C6_08DF_41CE_A156_A63ECFB1B868__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Unit.h"

class CUnitPlane : public CUnit  
{
	int modxold, modyold;
	int odd;
public:
	int Update();
	CUnitPlane();
	virtual ~CUnitPlane();

};

#endif // !defined(AFX_UNITPLANE_H__7BF6C8C6_08DF_41CE_A156_A63ECFB1B868__INCLUDED_)
