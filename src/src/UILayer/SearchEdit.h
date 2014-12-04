/*
 * $Id: SearchEdit.h 20835 2010-11-18 10:36:09Z dgkang $
 * 
 * this file is part of easyMule
 * Copyright (C)2002-2008 VeryCD Dev Team ( strEmail.Format("%s@%s", "emuledev", "verycd.com") / http: * www.easymule.org )
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#pragma once

#include "Localizee.h"
#include "ResetButton.h"
#include "emule.h"

// CSearchEditor

class CSearchEdit : public CEdit, public CLocalizee
{
	DECLARE_DYNAMIC(CSearchEdit)
	LOCALIZEE_WND_CANLOCALIZE()
public:
	CSearchEdit();
	virtual ~CSearchEdit();

public:
	BOOL	 m_bTipinfo;
	int		 m_nTabType;//根据当前TAB显示不同info || 0="VC Search" / 1="Smart Filter"

protected:
	DECLARE_MESSAGE_MAP()

protected:
	CFont			m_Font;
	CResetButton	m_ResetButton;

	BOOL			m_bFocusing;
public:
	BOOL Create(CWnd* pParentWnd, CRect rect, DWORD dwStyle = ES_LEFT | WS_CHILD | WS_VISIBLE, UINT nID = 123);
	afx_msg void OnEnKillfocus();
	afx_msg void OnEnSetfocus();

	void Localize();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnEnChange();

private:
	CString m_strSeachKey;
	DWORD m_dwLastTime;
	CRect m_rcEdit;//this rect

public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
