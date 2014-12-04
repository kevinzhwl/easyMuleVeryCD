/*
 * $Id: SearchBarCtrl.h 20835 2010-11-18 10:36:09Z dgkang $
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
#include "SearchEdit.h"
#include "ButtonST.h"

#define IMAGE_WIDTH		20
#define MARGIN_WIDTH	4
//#define BUTTON_WIDTH	20
// CSearchBar

class CSearchBarCtrl : public CWnd
{
	DECLARE_DYNAMIC(CSearchBarCtrl)

public:
	CSearchBarCtrl();
	virtual ~CSearchBarCtrl();

protected:
	CFont	m_Font;
	CSearchEdit m_SearchEdit;
	CImageList m_ImageList;
	CButtonST m_pButtonClear;//ËÑË÷¿ò"X"°´Å¥

protected:
	void ShowImage(int nImageNumber);
	void DrawTriangle(CDC* pDC, CRect &rect);

	int			m_nCurrentIcon;
	ESearchType m_eType;

	BOOL		m_isFocus;

	UINT		m_Type;

	CMenu		m_TypeMenu;

	BOOL		m_bHover;
protected:
	DECLARE_MESSAGE_MAP()
public:
	BOOL Create(CWnd* pParentWnd, CRect rect, DWORD dwStyle = WS_CHILD | WS_VISIBLE, UINT nID = 123);
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	CSearchEdit* GetEditor(void);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	LRESULT	OnEraseBkgndEx(WPARAM wParam, LPARAM lParam);
protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

private:
	LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);

public:
	void SetSearchType(ESearchType eType);
	ESearchType GetSearchType(void);

	void SetType(UINT type);
	UINT GetType(void);

	void ShowClearButton(BOOL bShow);//ÏÔÊ¾ËÑË÷¿ò"X"°´Å¥
	void ClearEdit();//Clear SearchEdit
	void SwitchSearchEditInfo(int nTabType);//ÇÐ»»ËÑË÷¿òÖÐµÄinfo
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	//afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	//afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnClearBtClicked(WPARAM wParam, LPARAM lParam);
};
