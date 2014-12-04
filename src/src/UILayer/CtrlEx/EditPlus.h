#pragma once


// CEditPlus

class CEditPlus : public CEdit
{
	DECLARE_DYNAMIC(CEditPlus)

public:
	CEditPlus();
	virtual ~CEditPlus();

protected:
	DECLARE_MESSAGE_MAP()
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
};


