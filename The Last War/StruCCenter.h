// StruCCenter.h: interface for the CStruCCenter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STRUCCENTER_H__D9377588_7386_11D5_95DF_A7276E930B33__INCLUDED_)
#define AFX_STRUCCENTER_H__D9377588_7386_11D5_95DF_A7276E930B33__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Structure.h"

class CStructCCenter : public CStructure  
{	
	int modxold, modyold;
	int odd;
public:
	int Update();
	CStructCCenter();
	virtual ~CStructCCenter();

};

#endif // !defined(AFX_STRUCCENTER_H__D9377588_7386_11D5_95DF_A7276E930B33__INCLUDED_)
