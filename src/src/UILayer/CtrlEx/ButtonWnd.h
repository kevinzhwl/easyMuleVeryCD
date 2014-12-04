#pragma once

#include "ButtonST.h"

// CButtonWnd

class CButtonWnd : public CWnd
{
	DECLARE_DYNAMIC(CButtonWnd)

public:
	CButtonWnd();
	virtual ~CButtonWnd();

public:
	BOOL		Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID = 0);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

public:
	CButtonST m_btIco;
	CButtonST m_btItem;
		
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};


