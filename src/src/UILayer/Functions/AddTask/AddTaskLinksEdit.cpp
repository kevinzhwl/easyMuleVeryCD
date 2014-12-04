/*
 * $Id: AddTaskLinksEdit.cpp 20835 2010-11-18 10:36:09Z dgkang $
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
// AddTaskLinksEdit.cpp : 实现文件
//

#include "stdafx.h"
#include "AddTaskLinksEdit.h"
#include ".\addtasklinksedit.h"
#include "UserMsgs.h"
#include "Util.h"
#include "otherfunctions.h"
#include "DlgAddTask.h"

#include "GetUrlStream.h"
#include ".\..\..\WorkLayer\UIMessage.h"
// CAddTaskLinksEdit

IMPLEMENT_DYNAMIC(CAddTaskLinksEdit, CEdit)
CAddTaskLinksEdit::CAddTaskLinksEdit()
{
	m_pDoc = NULL;
	pThread = NULL;
}

CAddTaskLinksEdit::~CAddTaskLinksEdit()
{
	if (pThread)
	{
		StopUpdateRSSThread();
	}
	for ( set<CRssFeed*>::iterator it = m_setLastRssfeed.begin(); m_setLastRssfeed.end() != it; ++it )
	{
		delete *it;
	}
}


BEGIN_MESSAGE_MAP(CAddTaskLinksEdit, CEdit)
	ON_CONTROL_REFLECT(EN_CHANGE, OnEnChange)
	
	ON_MESSAGE(UM_ADDTASK_DOC_ADDED, OnDocAdded)
	ON_MESSAGE(UM_ADDTASK_DOC_MODIFIED, OnDocModified)
	ON_MESSAGE(UM_ADDTASK_DOC_URL_ADDED, OnDocUrlAdded)
	ON_MESSAGE(UM_ADDTASK_DOC_URL_MODIFIED, OnDocUrlModified)
	ON_MESSAGE(UM_ADDTASK_DOC_URL_REMOVED, OnDocUrlRemoved)

	ON_MESSAGE(WM_ADDTASK_RSS_UPDATE, OnUpdateRss)

END_MESSAGE_MAP()


void CAddTaskLinksEdit::UpdateLinksByWindowText()
{
	if (NULL == m_pDoc)
		return;

	//AfxMessageBox(_T("abc"));
	set<CRssFeed*>	setRssUpdatefeed;//订阅
	//	取现在Doc里的所有ed2k的fileHash，组成set。
	set<CFileHashKey>	setKeysInDoc;
	setKeysInDoc = m_pDoc->GetAllKeysInDoc();

	//	把EditCtrl里的内容变成一行一行的字符串，并把每一行放入列表中。
	CString	strText;
	GetWindowText(strText);
	CList<CString>		lstLinks;
	::ConvertStrToStrList(&lstLinks, strText);


	CFileHashKey			key;
	CAddTaskDoc::SItem		docItem;
	CString					strLine;
	CED2KLink				*pLink = NULL;

	//CList<CString>			lstUrls;
	CMapStringToPtr			setUrls;
	CString					strPrefix;

	// Added by VC-yavey on 2010-04-16 <begin> 
	// 初始化rss url
	m_pDoc->GetRssUrlSet().clear();
	// Added by VC-yavey on 2010-04-16 <end>

	// 分析每一行，并做相应的处理。
	POSITION	pos = lstLinks.GetHeadPosition();

	if ( pos == NULL && !m_setLastRssfeed.empty() )
	{
		set<CRssFeed*>::iterator		it;//删除掉无用的rss订阅对象
		for ( it = m_setLastRssfeed.begin(); m_setLastRssfeed.end() != it; ++it )
		{
				delete *it;
		}
		m_setLastRssfeed.clear();
	}

	bool bHasRssLink = false;

	while (NULL != pos)
	{
		strLine = lstLinks.GetNext(pos);

		strLine = Decode3URL(strLine);
		
		strPrefix = strLine.Left(strLine.Find(_T(':')));

		if (0 == strPrefix.CompareNoCase(_T("ed2k")))
		{
			try
			{
				pLink = NULL;

				// MODIFIED by VC-yavey on 2010-04-16
				// 处理订阅协议
				pLink = CED2KLink::CreateLinkFromUrl(strLine);

			}
			catch (...)
			{
				SAFE_DELETE(pLink);
			}

			if (NULL != pLink)
			{
				// MODIFIED by VC-yavey on 2010-04-16
				// 处理订阅协议
				switch ( pLink->GetKind() )
				{
					case CED2KLink::kFile:
						key = ((CED2KFileLink*)pLink)->GetHashKey();

						// MODIFIED by VC-yavey on 2010-04-16, 统一放到最后删除
						//SAFE_DELETE(pLink);

						setKeysInDoc.erase(key);

						docItem.strLinkText = strLine;
						docItem.bCheck = TRUE;
						m_pDoc->SetItem(key, docItem, CAddTaskDoc::IM_TEXT | CAddTaskDoc::IM_CHECK, GetSafeHwnd());
						break;
					
					case CED2KLink::kRss:
						{
							bHasRssLink = true;

							CString strRssLink;
							CAddTaskDoc::RssUrlSet & setUrl = m_pDoc->GetRssUrlSet();
							strRssLink = ((CED2KRssLink*)pLink)->GetRssUrl();

							//added by lost		判断上次获取到订阅中的
							if ( !m_setLastRssfeed.empty() )
							{
								bool	bAlready = false;
								set<CRssFeed*>::iterator		it;
								for ( it = m_setLastRssfeed.begin(); m_setLastRssfeed.end() != it; it++ )
								{
									ASSERT( *it != NULL);
									CRssFeed * pFeed = *it;
									if ( pFeed->m_strFeedUrl  ==  strRssLink )
									{
										bAlready = true;
										setRssUpdatefeed.insert( (*it) );
										break;
									}
								}

								if (!bAlready)
								{
									setUrl.insert( strRssLink ) ;
								}
							}
							else
							{
								setUrl.insert( strRssLink ) ;
							}
							//added by lost

						}
						break;
				}
				SAFE_DELETE(pLink);
			}
		}
		else if (0 == strPrefix.CompareNoCase(_T("http"))
			|| 0 == strPrefix.CompareNoCase(_T("ftp")))
		{
			setUrls.SetAt(strLine, NULL);
			//lstUrls.AddTail(strLine);
		}
	}

	CAddTaskDoc::RssUrlSet & setUrl = m_pDoc->GetRssUrlSet();
	if (!setUrl.empty())
	{
		if (pThread == NULL)
		{
			UPDATERSS_PARAM * paramRSS = new UPDATERSS_PARAM;
			paramRSS->setRssLink		= setUrl;
			paramRSS->thisEdit		= this;

			pThread = AfxBeginThread( ThreadUpdateRSS, paramRSS );//线程获取FEED信息
		}
	}

	//从获取到的FEED集合中得到所有文件hash
	set<CFileHashKey>	setKeysInLastFeed;
	if ( !setRssUpdatefeed.empty() )
	{
		set<CRssFeed*>::iterator		itRss;
		for ( itRss = setRssUpdatefeed.begin(); setRssUpdatefeed.end() != itRss; itRss++ )
		{
			CRssFeed* pFeed = *itRss;
			CED2KLink				*pLinkRss = NULL;
			for ( CRssFeed::ItemIterator it = pFeed->GetItemIterBegin(); it != pFeed->GetItemIterEnd(); ++it )
			{
				CRssFeed::Item & item = it->second;

				try
				{
					pLinkRss = NULL;
					pLinkRss = CED2KLink::CreateLinkFromUrl(item.m_strEnclosure);
				}
				catch (...)
				{
					SAFE_DELETE(pLinkRss);
				}

				if (NULL != pLinkRss)
				{
					switch ( pLinkRss->GetKind() )
					{
					case CED2KLink::kFile:

						key = ((CED2KFileLink*)pLinkRss)->GetHashKey();
						setKeysInLastFeed.insert(key);
					}
				}
			}
			SAFE_DELETE(pLinkRss);
		}

		if ( !m_setRssHash.empty() )
		{
			m_setRssHash.clear();//直接清除set

			//set<CFileHashKey>::iterator		itKey;
			//for ( itKey = m_setRssHash.begin(); m_setRssHash.end() != itKey; itKey++ )//erase后就不可以再用itKey++了 切记
			//{
			//		itKey = m_setRssHash.erase(itKey);
			//}
		}
		m_setRssHash = setKeysInLastFeed;

		set<CRssFeed*>::iterator		it;//删除掉无用的rss订阅对象
		for ( it = m_setLastRssfeed.begin(); m_setLastRssfeed.end() != it; )
		{
			if (setRssUpdatefeed.find(*it) == setRssUpdatefeed.end())
			{
				delete *it;
				it = m_setLastRssfeed.erase(it);
			}
			else
			{
				++it;
			}
		}
		setRssUpdatefeed.clear();//清空set
	}

	// EditCtrl里没有的ed2k链接，则在doc里把它删除。
	set<CFileHashKey>::iterator		it;
	for (it = setKeysInDoc.begin();
		setKeysInDoc.end() != it;
		it++)
	{
		//docItem.bCheck = FALSE;
		//m_pDoc->SetItem(*it, docItem, CAddTaskDoc::IM_CHECK, GetSafeHwnd());
		if ( !setKeysInLastFeed.empty() && setKeysInLastFeed.find(*it) != setKeysInLastFeed.end() )
		{//与Feed中某hash值匹配
			continue;
		}

		m_pDoc->RemoveItem(*it);
	}

	if (bHasRssLink)
		(((CDlgAddTask*)GetParent())->m_chkboxRssAutoDown).ShowWindow(SW_SHOW);
	else
		(((CDlgAddTask*)GetParent())->m_chkboxRssAutoDown).ShowWindow(SW_HIDE);

	m_pDoc->UpdateUrlItems(&setUrls, GetSafeHwnd());
}


LRESULT CAddTaskLinksEdit::OnUpdateRss(WPARAM wParam, LPARAM lParam)
{
	if ( (int)lParam <= 0 )
	{//没有成功获取到RSS
		return -1;
	}

	set<CRssFeed*> setGetRssfeed	=		*((set<CRssFeed*>*)wParam);
	if (!setGetRssfeed.empty())
	{
		CED2KLink				*pLinkRss = NULL;
		CFileHashKey			key;
		CAddTaskDoc::SItem		docItem;

		set<CFileHashKey>	setKeysInDoc;
		setKeysInDoc = m_pDoc->GetAllKeysInDoc();

		CString	strText;
		GetWindowText(strText);//edit中数据

		set<CRssFeed*>::iterator		itRss;
		for ( itRss = setGetRssfeed.begin(); setGetRssfeed.end() != itRss; itRss++ )
		{
			m_setLastRssfeed.insert(*itRss);//加入到全局set中去

			CRssFeed * pFeed	= *itRss;
			CString strFeedUrl = pFeed->m_strFeedUrl;
			if (strText.Find(strFeedUrl) == -1)//线程返回时 需确定edit中还存在这个url
			{
				continue;
			}

			for ( CRssFeed::ItemIterator it = pFeed->GetItemIterBegin(); it != pFeed->GetItemIterEnd(); ++it )
			{
				CRssFeed::Item & item = it->second;

				try
				{
					pLinkRss = NULL;
					pLinkRss = CED2KLink::CreateLinkFromUrl(item.m_strEnclosure);
				}
				catch (...)
				{
					SAFE_DELETE(pLinkRss);
				}

				if (NULL != pLinkRss)
				{
					switch ( pLinkRss->GetKind() )
					{
					case CED2KLink::kFile:

						key = ((CED2KFileLink*)pLinkRss)->GetHashKey();
						setKeysInDoc.erase(key);
						m_setRssHash.insert(key);//加入到全局key中 

						docItem.strLinkText = item.m_strEnclosure;
						docItem.bCheck = TRUE;
						m_pDoc->SetItem(key, docItem, CAddTaskDoc::IM_TEXT | CAddTaskDoc::IM_CHECK, GetSafeHwnd());
					}
				}
			}
		}
	}

	return 0;
}

UINT CAddTaskLinksEdit::ThreadUpdateRSS( LPVOID pUpdateParam )
{
	set<CString>& 	setLink	=  ((UPDATERSS_PARAM*)pUpdateParam)->setRssLink;
	CAddTaskLinksEdit* thisedit = ((UPDATERSS_PARAM*)pUpdateParam)->thisEdit;

	set<CRssFeed*>	setGetRssfeed;//订阅
	int nGeted = 0;
	
	set<CString>::iterator		it;
	for ( it = setLink.begin(); setLink.end() != it; it++ )
	{	
		CString strLink = *it;
		CGetUrlStream sUrl;

		BOOL bRet = FALSE;
		int cTry = 3;
		do 
		{
			// TRACE("\n %s  - try to get XML: %d\n", __FUNCTION__, 3 - cTry);
			bRet = sUrl.OpenUrl(strLink);
			if ( --cTry == 0)
				break;
		} 
		while (bRet == FALSE);
		
		CRssFeed * pFeed = new CRssFeed();
		pFeed->m_strFeedUrl = strLink;

		if ( pFeed->Refresh(NULL, sUrl.GetStream(), NULL) )//刷新feed信息
		{//获取成功			
			setGetRssfeed.insert(pFeed);
			nGeted ++;
		}
		else
			continue;
	}

	if (thisedit->m_hWnd)
	{
		::SendMessage(thisedit->m_hWnd, WM_ADDTASK_RSS_UPDATE, (WPARAM)(&setGetRssfeed), (LPARAM)nGeted);
	}

	thisedit->pThread = NULL;
	return 0;
}

void	CAddTaskLinksEdit::StopUpdateRSSThread()
{
	if (pThread)
	{
		if (pThread->m_hThread)
		{
			DWORD dwWaitRes = WaitForSingleObject(pThread->m_hThread, 1300);
			if (dwWaitRes == WAIT_TIMEOUT)
			{
				VERIFY( TerminateThread(pThread->m_hThread, (DWORD)-1) );
			}
			else if (dwWaitRes == -1)
			{
				TRACE("*** Failed to wait for process thread termination - Error %u\n", GetLastError());
				ASSERT(0); 
			}
		}
		delete pThread;
		pThread = NULL;
	}
}

void CAddTaskLinksEdit::AddText(LPCTSTR lpszText)
{
	CString	str;
	CString	strText;
	GetWindowText(strText);

	if (strText.IsEmpty())
	{
		strText.Append(lpszText);
		strText.Append(_T("\r\n"));
	}
	else
	{
		str = strText.Right(2);
		if (_T("\r\n") == str)
		{
			strText.Append(lpszText);
			strText.Append(_T("\r\n"));
		}
		else
		{
			strText.Append(_T("\r\n"));
			strText.Append(lpszText);
			strText.Append(_T("\r\n"));
		}
	}

	SetWindowText(strText);
}

void CAddTaskLinksEdit::RemoveLine(LPCTSTR lpszText)
{
	if (NULL == lpszText
		|| lpszText[0] == _T('\0'))
		return;

	CString	strOldText;
	GetWindowText(strOldText);
	CONST TCHAR * pszURL = strOldText;

	CString url;
	CString result = _T("");
	INT len = _tcslen(pszURL), i = 0, j = 0;
	for(i = 0, j = _tcscspn(pszURL + i, _T("\r\n")); i < len; i += j + 1, j = _tcscspn(pszURL + i, _T("\r\n")))
	{
		if(j > 10)
		{
			url.SetString(pszURL + i, j);
			url = url.Trim();
			if(url != _T("") && url != lpszText)
				result = result + url + _T("\r\n");
		}
	}

	SetWindowText(result);
}

void CAddTaskLinksEdit::SetText(const CFileHashKey &key, LPCTSTR lpszText)
{
	int	iStartPos, iEndPos;
	if ( !FindLineByKey(key, iStartPos, iEndPos) )
	{
		AddText(lpszText);
	}
	else
	{
		RemoveText(key);
		AddText(lpszText);
	}
}

void CAddTaskLinksEdit::RemoveText(const CFileHashKey &key)
{
	int	iStartPos, iEndPos;
	if ( !FindLineByKey(key, iStartPos, iEndPos) )
		return;

	CString	strOldText;
	CString	strNewText;

	GetWindowText(strOldText);
	strNewText = RemoveLine(strOldText, iStartPos, iEndPos);
	SetWindowText(strNewText);
}

CString	CAddTaskLinksEdit::RemoveLine(const CString &str, int iStart, int iEnd)
{
	CString strTemp = str.Mid(iEnd + 1, 2);
	if (_T("\r\n") == strTemp)
		iEnd += 2;
	
	
	CString	strNew;

	strNew = str.Left(iStart);
	if (iEnd < str.GetLength() - 1)
		strNew += str.Mid(iEnd + 1);

	return strNew;
}

BOOL CAddTaskLinksEdit::FindLineByKey(const CFileHashKey &key, int &iStartPos, int &iEndPos)
{
	BOOL				bDone;
	int					iTmpStartPos;
	int					iTmpEndPos;
	CString				strLine;
	CED2KLink			*pLink = NULL;
	CFileHashKey		keyCmp;

	CString	str;
	GetWindowText(str);

	bDone = FALSE;
	iTmpStartPos = iTmpEndPos = 0;

	do {
		iStartPos = iTmpStartPos;

		iTmpEndPos = str.Find(_T("\r\n"), iTmpStartPos);
		if (-1 == iTmpEndPos)
		{
			iEndPos = str.GetLength() - 1;

			strLine = str.Mid(iTmpStartPos);
			bDone = TRUE;
		}
		else
		{
			iEndPos = iTmpEndPos - 1;

			strLine = str.Mid(iTmpStartPos, iTmpEndPos - iTmpStartPos);
			iTmpStartPos = iTmpEndPos + 2;
		}

		try
		{
			if (!strLine.IsEmpty())
			{
				pLink = CED2KLink::CreateLinkFromUrl(strLine);
				if (CED2KLink::kFile == pLink->GetKind())
					keyCmp = ((CED2KFileLink*)pLink)->GetHashKey();
				SAFE_DELETE(pLink);

				if (key == keyCmp)
				{
					return TRUE;
				}
			}
		}
		catch(...)
		{
		}

	} while(!bDone);

	return FALSE;
}

// CAddTaskLinksEdit 消息处理程序

void CAddTaskLinksEdit::OnEnChange()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CEdit::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	UpdateLinksByWindowText();
}

LRESULT CAddTaskLinksEdit::OnDocAdded(WPARAM wParam, LPARAM lParam)
{
	if (NULL == m_pDoc)			return 0;
	if (0 == lParam)			return 0;

	CFileHashKey			*pKey = (CFileHashKey*) lParam;
	CAddTaskDoc::SItem		item;

	if (!m_pDoc->GetItem(*pKey, item))
		return 0;

	AddText(item.strLinkText);

	return 0;
}
LRESULT CAddTaskLinksEdit::OnDocModified(WPARAM wParam, LPARAM lParam)
{
	if (NULL == m_pDoc)						return 0;
	if (0 == wParam || 0 == lParam)			return 0;

	DWORD					dwModifiedMask	= wParam;
	CFileHashKey			*pKey = (CFileHashKey*) lParam;
	CAddTaskDoc::SItem		item;

	if (! m_pDoc->GetItem(*pKey, item))
		return 0;

	if (CAddTaskDoc::IM_CHECK & dwModifiedMask)
	{
		if ( !m_setRssHash.empty() && m_setRssHash.find(*pKey) != m_setRssHash.end() )
		{
			
		}
		else 
		{
			if (item.bCheck)
				SetText(*pKey, item.strLinkText);
			else
				RemoveText(*pKey);
		}
	}
	else if (CAddTaskDoc::IM_TEXT & dwModifiedMask)
	{
		if ( !m_setRssHash.empty() && m_setRssHash.find(*pKey) != m_setRssHash.end() )
		{

		}
		else 
		{
			SetText(*pKey, item.strLinkText);
		}
	}

	return 0;
}

LRESULT CAddTaskLinksEdit::OnDocUrlAdded(WPARAM wParam, LPARAM lParam)
{
	if (0 == lParam)
		return 0;
	
	CString	strUrl = (LPCTSTR) lParam;

	AddText(strUrl);

	return 0;
}

LRESULT CAddTaskLinksEdit::OnDocUrlModified(WPARAM wParam, LPARAM lParam)
{
	if (0 == lParam)
		return 0;

	BOOL	bCheck = (BOOL) wParam;
	CString	strUrl = (LPCTSTR) lParam;

	if (bCheck)
		AddText(strUrl);
	else
		RemoveLine(strUrl);

	return 0;
}

LRESULT CAddTaskLinksEdit::OnDocUrlRemoved(WPARAM wParam, LPARAM lParam)
{
	if (0 == lParam)
		return 0;

	RemoveLine((LPCTSTR) lParam);

	return 0;
}

set<CRssFeed*>& CAddTaskLinksEdit::GetLastRssFeedSet()
{
	return m_setLastRssfeed;
}
