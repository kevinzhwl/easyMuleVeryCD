/*
 * $Id: SearchEdit.cpp 20835 2010-11-18 10:36:09Z dgkang $
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
// SearchEditor.cpp : 实现文件
//

#include "stdafx.h"
#include "SearchEdit.h"
#include "resource.h"
#include "otherfunctions.h"
#include "SearchParams.h"
#include "CmdFuncs.h"
#include "emuledlg.h"
#include "StringConversion.h"
#include "SearchButton.h"
#include ".\searchedit.h"
#include ".\rssfeedwnd.h"

// CSearchEditor

IMPLEMENT_DYNAMIC(CSearchEdit, CEdit)
CSearchEdit::CSearchEdit()
{
	m_bTipinfo = TRUE;
	m_Font.CreateFont(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,DEFAULT_PITCH|FF_DONTCARE, _T("宋体"));
	m_bFocusing = FALSE;
	m_dwLastTime = 0;
	m_rcEdit = 0;

	if (thePrefs.m_bStartShowHomePage)//搜索框中字符串需要考虑 "是否显示资源页签"
		m_nTabType = 0;
	else
		m_nTabType = 1;
}

CSearchEdit::~CSearchEdit()
{
}

BEGIN_MESSAGE_MAP(CSearchEdit, CEdit)
	ON_CONTROL_REFLECT(EN_KILLFOCUS, OnEnKillfocus)
	ON_CONTROL_REFLECT(EN_SETFOCUS, OnEnSetfocus)
	ON_WM_CREATE()
	ON_WM_LBUTTONUP()
//	ON_WM_CHAR()
ON_CONTROL_REFLECT(EN_CHANGE, &CSearchEdit::OnEnChange)
ON_WM_TIMER()
ON_WM_KEYDOWN()
END_MESSAGE_MAP()


BOOL CSearchEdit::Create(CWnd* pParentWnd, CRect rect, DWORD dwStyle, UINT nID)
{
	dwStyle = dwStyle | ES_AUTOHSCROLL;
	return CEdit::Create(dwStyle, rect, pParentWnd, nID);
}

void CSearchEdit::OnEnKillfocus()
{
	CString str;
	GetWindowText(str);

	str.Remove(' ');

	if (str == _T(""))
	{
		m_bTipinfo = TRUE;

		CString strInfo;
		if (m_nTabType == 1)
			strInfo = GetResString(IDS_SEARCHEDIT_INPUT_FILTER);
		else
			strInfo = GetResString(IDS_SEARCHEDIT_INPUT_VCSEARCH);

		SetWindowText( strInfo );
	}
}

void CSearchEdit::OnEnSetfocus()
{
	if (m_bTipinfo)
	{
		SetWindowText(_T(""));
		m_bTipinfo = FALSE;
	}
	m_bFocusing = TRUE;
}

void CSearchEdit::Localize()
{
	if (m_bTipinfo)
	{
		CString strInfo;
		if (m_nTabType == 1)
			strInfo = GetResString(IDS_SEARCHEDIT_INPUT_FILTER);
		else
			strInfo = GetResString(IDS_SEARCHEDIT_INPUT_VCSEARCH);

		SetWindowText( strInfo );
	}

}
int CSearchEdit::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetFont(&m_Font);

	CString strInfo;
	if (m_nTabType == 1)
		strInfo = GetResString(IDS_SEARCHEDIT_INPUT_FILTER);
	else
		strInfo = GetResString(IDS_SEARCHEDIT_INPUT_VCSEARCH);

	SetWindowText( strInfo );

	return 0;
}

void CSearchEdit::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bFocusing)
	{
		m_bFocusing = FALSE;
		SetSel(0, -1);
	}

	__super::OnLButtonUp(nFlags, point);
}

LRESULT CSearchEdit::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if ( WM_INPUTLANGCHANGEREQUEST == message )
	{
		if(GetKeyboardLayout(0) == (HKL)lParam)
			return 0;
		UINT count = GetKeyboardLayoutList(0,NULL);
		if(count == 0) return 0;
		HKL* lpList = new HKL[count];

		count = GetKeyboardLayoutList(count,lpList);
		BOOL bFound = FALSE;
		for(int i=0;i<count;i++)
		{
			if((HKL)lParam == lpList[i])
			{
				bFound = TRUE;
				break;
			}
		}
		if(lpList)
		{
			delete[] lpList;
			lpList = NULL;
		}
		if(!bFound)
		{
			CString sID;
			sID.Format( L"%08x",lParam );
			LoadKeyboardLayout(sID,KLF_ACTIVATE);
		}
		else
		{
			ActivateKeyboardLayout((HKL)lParam,KLF_REORDER);
		}

		return 0;
	}

	return __super::WindowProc(message, wParam, lParam);
}

void CSearchEdit::OnEnChange()
{
	if (m_bTipinfo)
		return;
	
	GetWindowText(m_strSeachKey);

	CString strInfo;
	if (m_nTabType == 1)
		strInfo = GetResString(IDS_SEARCHEDIT_INPUT_FILTER);
	else
		strInfo = GetResString(IDS_SEARCHEDIT_INPUT_VCSEARCH);

	if (m_strSeachKey == strInfo)
	{
		return;
	}

	if ( m_rcEdit.IsRectEmpty() )
	{//记录原始edit rect
		CRect rcClient;
		GetWindowRect(rcClient);
		m_rcEdit = rcClient;
	}

	if ( m_strSeachKey.GetLength() > 0 )
	{//缩短edit length
		SetWindowPos(NULL, 0, 0, m_rcEdit.Width()-19, m_rcEdit.Height(), SWP_NOMOVE | SWP_NOZORDER );
		((CSearchBarCtrl*)GetParent())->ShowClearButton(TRUE);
	}
	else
	{//恢复
		SetWindowPos(NULL, 0, 0, m_rcEdit.Width(), m_rcEdit.Height(), SWP_NOMOVE | SWP_NOZORDER );
		((CSearchBarCtrl*)GetParent())->ShowClearButton(FALSE);
	}

	DWORD dwNowTime = ::GetTickCount();
	DWORD dwTick = dwNowTime - m_dwLastTime;

	if (dwTick <= 200)
		KillTimer(0);

	SetTimer(0, 200, NULL);
	m_dwLastTime = dwNowTime;
}

void CSearchEdit::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 0)
	{
		::SendMessage(CGlobalVariable::m_hListenWnd, WM_SEARCHEDIT_CHANGE, 0, (LPARAM)(&m_strSeachKey));
		KillTimer(nIDEvent);
	}

	__super::OnTimer(nIDEvent);
}

void CSearchEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar)
	{
	case VK_DOWN:
		{
			::SendMessage(CGlobalVariable::m_hListenWnd, WM_SEARCHEDIT_ONKEYDOWN, 0, (LPARAM)nChar);
		}
		break;
	}
	
	__super::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CSearchEdit::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if ( VK_ESCAPE == pMsg->wParam )
		{
			CString str;
			GetWindowText(str);
			if (!str.IsEmpty())
				SetWindowText(_T(""));
		}
		else if ( VK_RETURN == pMsg->wParam )
		{
			if (m_nTabType == 1)
			{
				::SendMessage(CGlobalVariable::m_hListenWnd, WM_SEARCHEDIT_ONKEYDOWN, 0, (LPARAM)VK_RETURN);
				return TRUE;
			}
		}
	}

	return __super::PreTranslateMessage(pMsg);
}
