// StructOilPlant.h: interface for the CStructOilPlant class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STRUCTOILPLANT_H__54AE6ADD_D594_458F_B0E1_EBD532C923A3__INCLUDED_)
#define AFX_STRUCTOILPLANT_H__54AE6ADD_D594_458F_B0E1_EBD532C923A3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Structure.h"

class CStructOilPlant : public CStructure  
{
public:
	int Update();
	CStructOilPlant();
	virtual int GetType() {return 2; };
	virtual ~CStructOilPlant();

};

#endif // !defined(AFX_STRUCTOILPLANT_H__54AE6ADD_D594_458F_B0E1_EBD532C923A3__INCLUDED_)
