// Misile.h: interface for the CMisile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MISILE_H__26FC3271_CACC_40AD_9911_E631779DA4EC__INCLUDED_)
#define AFX_MISILE_H__26FC3271_CACC_40AD_9911_E631779DA4EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Unit.h"
#include "Structure.h"

class CMissile : public CUnit  
{
	CStructure *StructTarget;
	int iTargetType, iDestroy, iFrame;
	int iStrength;

public:
	int Update();
	void SetStrength(int s){iStrength=s;}
	void SetTarget(CUnit *, CStructure *, int);
	int Destroyed() {return iDestroy;}
	int GetFrame(){return iFrame;}
	CMissile();
	virtual ~CMissile();
	CMissile *next;
};

#endif // !defined(AFX_MISILE_H__26FC3271_CACC_40AD_9911_E631779DA4EC__INCLUDED_)