// StruPyramid.h: interface for the CStruPyramid class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STRUPYRAMID_H__C32A5680_7521_11D5_95DF_86B496E09E32__INCLUDED_)
#define AFX_STRUPYRAMID_H__C32A5680_7521_11D5_95DF_86B496E09E32__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Structure.h"

class CStructPyramid : public CStructure  
{
public:
	int Update();
	virtual int GetType() { return 3; };
	CStructPyramid();
	virtual ~CStructPyramid();

};

#endif // !defined(AFX_STRUPYRAMID_H__C32A5680_7521_11D5_95DF_86B496E09E32__INCLUDED_)
