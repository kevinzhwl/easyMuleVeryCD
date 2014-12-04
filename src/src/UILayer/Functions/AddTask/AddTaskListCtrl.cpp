/*
 * $Id: AddTaskListCtrl.cpp 19920 2010-06-22 06:50:01Z dgkang $
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
// AddTaskListCtrl.cpp : 实现文件
//

#include "stdafx.h"
#include "AddTaskListCtrl.h"
#include ".\addtasklistctrl.h"
#include "DlgAddTask.h"
#include "UserMsgs.h"
#include "Util.h"
#include "ED2KLink.h"
#include "CmdFuncs.h"
#include "otherfunctions.h"
#include "StringConversion.h"

// CAddTaskListCtrl

IMPLEMENT_DYNAMIC(CAddTaskListCtrl, CListCtrl)
CAddTaskListCtrl::CAddTaskListCtrl()
{
	EnableToolTips(TRUE);//激活tooltips属性
	m_wHitMask = INITIAL_HITMASK;
}

CAddTaskListCtrl::~CAddTaskListCtrl()
{
	SItemData		*pItemData;
	POSITION	pos = m_itemDatas.GetHeadPosition();
	while (NULL != pos)
	{
		pItemData = m_itemDatas.GetNext(pos);
		SAFE_DELETE(pItemData);
	}
	m_itemDatas.RemoveAll();

	DeleteAllToolTips();//清空tooptip
}

BEGIN_MESSAGE_MAP(CAddTaskListCtrl, CListCtrl)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnLvnItemchanged)

	ON_MESSAGE(UM_ADDTASK_DOC_ADDED, OnDocAdded)
	ON_MESSAGE(UM_ADDTASK_DOC_MODIFIED, OnDocModified)
	ON_MESSAGE(UM_ADDTASK_DOC_REMOVED, OnDocRemoved)
	
	ON_MESSAGE(UM_ADDTASK_DOC_URL_ADDED, OnDocUrlAdded)
	ON_MESSAGE(UM_ADDTASK_DOC_URL_MODIFIED, OnDocUrlModified)
	ON_MESSAGE(UM_ADDTASK_DOC_URL_REMOVED, OnDocUrlRemoved)

	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()



int CAddTaskListCtrl::GetInsertIndex(LPCTSTR lpszFileName)
{
	int	i;
	int	iCount = GetItemCount();
	CString	strItemText;
	for (i = 0; i < iCount; i++)
	{
		strItemText = GetItemText(i, 0);
		if (strItemText > lpszFileName)
			break;
	}
	return i;
}

void CAddTaskListCtrl::CalStrByLinkText(LPCTSTR lpszLinkText, CString &strFileName, CString &strFileSize)
{
	CED2KFileLink	*pLink = NULL;
	try
	{
		//MODIFIED by VC-yavey on 2010-04-16	<begin>
		pLink = CreateFileLinkFromUrl(lpszLinkText);
		if ( pLink == NULL )
		{
			strFileName.Empty();
			strFileSize.Empty();
			return;
		}
		//MODIFIED by VC-yavey on 2010-04-16	<end>
	}
	catch(...)
	{
		strFileName.Empty();
		strFileSize.Empty();
		SAFE_DELETE(pLink);
		return;
	}

	strFileName = pLink->GetName();
	strFileSize = CmdFuncs::GetFileSizeDisplayStr(pLink->GetSize());
	SAFE_DELETE(pLink);
}

void CAddTaskListCtrl::CalStrByUrl(const CString &strUrl, CString &strFileName)
{		
	strFileName = GetFileNameFromUrlStr( strUrl );
}

void CAddTaskListCtrl::SetItemByDocItem(int iItemIndex, const CAddTaskDoc::SItem &item, DWORD dwItemMask)
{
	if (-1 == iItemIndex)
		return; 

	if (CAddTaskDoc::IM_TEXT & dwItemMask)
	{
		CString			strFileName;
		CString			strFileSize;
		CalStrByLinkText(item.strLinkText, strFileName, strFileSize);
		SetItemText(iItemIndex, 1, strFileName);
		SetItemText(iItemIndex, 2, strFileSize);
	}
	if (CAddTaskDoc::IM_CHECK & dwItemMask)
	{
		SetCheck(iItemIndex, item.bCheck);
	}
}

int CAddTaskListCtrl::GetItemIndexByKey(const CFileHashKey &key)
{
	SItemData	*pItemData = NULL;

	int	i;
	int	iCount = GetItemCount();
	for (i = 0; i < iCount; i++)
	{
		pItemData = (SItemData*) GetItemData(i);
 		if (IT_ED2K == pItemData->iType
			&& pItemData->fhk == key)
			return i;
	}
	return -1;
}

int CAddTaskListCtrl::GetItemIndexByUrl(LPCTSTR lpszUrl)
{
	SItemData	*pItemData = NULL;

	int	i;
	int	iCount = GetItemCount();
	for (i = 0; i < iCount; i++)
	{
		pItemData = (SItemData*) GetItemData(i);
		if (IT_URL == pItemData->iType
			&& 0 == pItemData->strUrl.CompareNoCase(lpszUrl))
			return i;
	}
	return -1;
}

void CAddTaskListCtrl::CheckAllItems(BOOL bCheck)
{
	int		i;
	for (i = 0; i < GetItemCount(); i++)
	{
		SetCheck(i, bCheck);
	}
}
void CAddTaskListCtrl::CheckAllSelectedItems(BOOL bCheck)
{
	int	iIndex;
	POSITION pos = GetFirstSelectedItemPosition();
	while (NULL != pos)
	{
		iIndex = GetNextSelectedItem(pos);
		SetCheck(iIndex, bCheck);
	}
}

BOOL CAddTaskListCtrl::CheckAllSelected()
{
	BOOL bAllCheck = TRUE;
	int i;
	for (i = 0; i < GetItemCount(); i++)
	{
		if (GetCheck(i) == FALSE)
		{
			bAllCheck = FALSE;
			break;
		}
	}

	if ( i == 0 )
		bAllCheck	= FALSE;

	return bAllCheck;
}

void	CAddTaskListCtrl::ResetHeadStat()
{
	BOOL bAllCheck = CheckAllSelected();
	if (bAllCheck)//通知父窗口
		(((CDlgAddTask*)GetParent())->m_chkboxSelectAll).SetCheck(TRUE);
	else
		(((CDlgAddTask*)GetParent())->m_chkboxSelectAll).SetCheck(FALSE);
}

// CAddTaskListCtrl 消息处理程序


void CAddTaskListCtrl::OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码


	BOOL bNewCheck = (pNMLV->uNewState >> 12) - 1;
	BOOL bOldCheck = (pNMLV->uOldState >> 12) - 1;
	if (bNewCheck != bOldCheck)
	{
		SItemData	*pItemData = (SItemData*) GetItemData(pNMLV->iItem);
		if (NULL == pItemData)
			return;

		switch (pItemData->iType)
		{
		case IT_ED2K:
			{
				CAddTaskDoc::SItem	item;
				item.bCheck = bNewCheck;
				m_pDoc->SetItem(pItemData->fhk, item, CAddTaskDoc::IM_CHECK, GetSafeHwnd());
			}
			break;
		case IT_URL:
			{
				m_pDoc->CheckUrl(pItemData->strUrl, bNewCheck, GetSafeHwnd());
			}
			break;
		}

		ResetHeadStat();
	}

	*pResult = 0;
}

LRESULT CAddTaskListCtrl::OnDocAdded(WPARAM /*wParam*/, LPARAM lParam)
{
	if (0 == lParam)
		return 0;

	CFileHashKey			*pKey = (CFileHashKey*) lParam;
	CAddTaskDoc::SItem		item;
	if (!m_pDoc->GetItem(*pKey, item))
		return 0;

	CString			strFileName;
	CString			strFileSize;
	CalStrByLinkText(item.strLinkText, strFileName, strFileSize);

	SItemData		*pItemData = new SItemData;
	pItemData->iType = IT_ED2K;
	pItemData->fhk = *pKey;
	m_itemDatas.AddTail(pItemData);

	int			iItemIndex;
	iItemIndex = InsertItem(GetInsertIndex(strFileName), _T(""));

	SetItemToolTipText((UINT)(DWORD_PTR)pItemData,strFileName);//添加tooptip提示

	SetItemData(iItemIndex, (DWORD_PTR)pItemData);
	SetItemByDocItem(iItemIndex, item);

	ResetHeadStat();

	return 0;
}
LRESULT CAddTaskListCtrl::OnDocModified(WPARAM wParam, LPARAM lParam)
{
	if (0 == wParam || 0 == lParam)
		return 0;


	DWORD					dwModifiedMask = wParam;
	CFileHashKey			*pKey = (CFileHashKey*) lParam;
	CAddTaskDoc::SItem		item;
	if (!m_pDoc->GetItem(*pKey, item))
		return 0;

	SetItemByDocItem(GetItemIndexByKey(*pKey), item, dwModifiedMask);
	return 0;
}

LRESULT CAddTaskListCtrl::OnDocRemoved(WPARAM /*wParam*/, LPARAM lParam)
{
	if (0 == lParam)
		return 0;

	CFileHashKey *pKey = (CFileHashKey*) lParam;
	int	iItemIndex = GetItemIndexByKey(*pKey);
	if (-1 == iItemIndex)
		return 0;

	CString strKey;
	strKey.Format(_T("%d"),  (UINT)GetItemData(iItemIndex));
	m_ToolTipMap.RemoveKey( strKey );//删除tootip

	DeleteItem(iItemIndex);

	ResetHeadStat();

	return 0;
}

LRESULT CAddTaskListCtrl::OnDocUrlAdded(WPARAM /*wParam*/, LPARAM lParam)
{
	if (0 == lParam)
		return 0;

	CString			strUrl = (LPCTSTR) lParam;

	CString			strFileName;
	CalStrByUrl(strUrl, strFileName);

	SItemData		*pItemData = new SItemData;
	pItemData->iType = IT_URL;
	pItemData->strUrl = strUrl;
	m_itemDatas.AddTail(pItemData);

	int			iItemIndex;
	iItemIndex = InsertItem(GetInsertIndex(strFileName), _T(""));

	SetItemToolTipText((UINT)(DWORD_PTR)pItemData,strFileName);//添加tooptip提示

	SetItemData(iItemIndex, (DWORD_PTR)pItemData);
	SetItemText(iItemIndex, 1, strFileName);
	SetCheck(iItemIndex, TRUE);

	ResetHeadStat();

	return 0;
}

LRESULT CAddTaskListCtrl::OnDocUrlModified(WPARAM wParam, LPARAM lParam)
{
	if (0 == lParam)
		return 0;

	BOOL	bCheck = (BOOL) wParam;
	CString	strUrl = (LPCTSTR) lParam;

	int	iItemIndex;
	iItemIndex = GetItemIndexByUrl(strUrl);
	if (-1 == iItemIndex)
		return 0;

	SetCheck(iItemIndex, bCheck);
	return 0;
}

LRESULT CAddTaskListCtrl::OnDocUrlRemoved(WPARAM /*wParam*/, LPARAM lParam)
{
	if (0 == lParam)
		return 0;

	CString	strUrl = (LPCTSTR) lParam;

	int	iItemIndex;
	iItemIndex = GetItemIndexByUrl(strUrl);
	if (-1 == iItemIndex)
		return 0;

	CString strKey;
	strKey.Format(_T("%d"),  (UINT)GetItemData(iItemIndex));
	m_ToolTipMap.RemoveKey( strKey );//删除tootip

	DeleteItem(iItemIndex);

	ResetHeadStat();

	return 0;
}

void CAddTaskListCtrl::OnRButtonDown(UINT /*nFlags*/, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CMenu	menu;
	menu.CreatePopupMenu();

	menu.AppendMenu(MF_STRING, MP_SELECT, GetResString(IDS_CHECK));
	menu.AppendMenu(MF_STRING, MP_SELECT_ALL, GetResString(IDS_CHECK_ALL));
	menu.AppendMenu(MF_STRING, MP_DESELECT, GetResString(IDS_UNCHECK));
	menu.AppendMenu(MF_STRING, MP_DESELECT_ALL, GetResString(IDS_UNCHECK_ALL));

	CPoint	pt = point;
	ClientToScreen(&pt);
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();

	//CListCtrl::OnRButtonDown(nFlags, point);
}

BOOL CAddTaskListCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类
	BOOL	bProcessed = TRUE;
	switch(wParam)
	{
	case MP_SELECT:	//Select
		CheckAllSelectedItems(TRUE);
		break;
	case MP_SELECT_ALL:	//Select all
		CheckAllItems(TRUE);
		break;
	case MP_DESELECT:	//Deselect
		CheckAllSelectedItems(FALSE);
		break;
	case MP_DESELECT_ALL:	//Deselect all
		CheckAllItems(FALSE);
		break;
	default:
		bProcessed = FALSE;
		break;
	}
	if (bProcessed)
		return bProcessed;

	return CListCtrl::OnCommand(wParam, lParam);
}


INT_PTR CAddTaskListCtrl::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	// TODO: 在此添加专用代码和/或调用基类

	LVHITTESTINFO lvhitTestInfo;
	lvhitTestInfo.pt	= point;


	int nItem = ListView_SubItemHitTest(this->m_hWnd,&lvhitTestInfo);
	if ( nItem < 0 )
	{
		return -1;
	}

	UINT nFlags =   lvhitTestInfo.flags;
	if (nFlags & m_wHitMask)
	{
		RECT rcClient;
		GetClientRect( &rcClient );
		CString strKey;
		strKey.Format(_T("%u"),  (UINT) this->GetItemData(nItem));

		CString strTipText;
		if( m_ToolTipMap.Lookup(strKey, strTipText ) && !strTipText.IsEmpty())
		{

	#if _WIN32_WINNT >= 0x0501
			pTI->cbSize = sizeof(TOOLINFO) - sizeof(void *);
	#else
			pTI->cbSize = sizeof(TOOLINFO);
	#endif
			pTI->uFlags = TTF_IDISHWND;
			pTI->hwnd = m_hWnd;
			pTI->uId = (UINT_PTR)m_hWnd;
			pTI->rect = rcClient;
			pTI->lpszText = (LPTSTR)malloc(sizeof(TCHAR)*(strTipText.GetLength()+1));
			_tcscpy(pTI->lpszText, strTipText);
			return nItem;
		}
	}

	return -1;
}

BOOL CAddTaskListCtrl::SetItemToolTipText( int nItemPtr, LPCTSTR lpszToolTipText )
{
	CString strKey;
	strKey.Format(_T("%d"), (UINT) nItemPtr);
	m_ToolTipMap.SetAt( strKey, lpszToolTipText );//如有相同KEY值.则只替换ToolTip值

	return TRUE;
}

void CAddTaskListCtrl::DeleteAllToolTips()
{
	m_ToolTipMap.RemoveAll();
}
