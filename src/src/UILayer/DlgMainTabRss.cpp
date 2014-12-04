/*
 * $Id: DlgMainTabRss.cpp 20779 2010-11-11 02:15:21Z gaoyuan $
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

// UILayer\DlgMainTabRss.cpp : 实现文件
//

#include "stdafx.h"
#include "TabItem_Normal.h"
#include "TabItem_Wnd.h"
#include "PageTabBkDraw.h"
#include "eMule.h"
#include "emuleDlg.h"
#include "TabWnd.h"
#include "UILayer\DlgMainTabRss.h"
#include "UILayer\RssFeedWnd.h"
#include "UILayer\RssFilterWnd.h"
#include "UILayer\ToolBarCtrls\TbcRss.h"
#include "WorkLayer\RSS\RssFeedManager.h"
#include ".\dlgmaintabrss.h"
#include "DlgAddTask.h"
#include "CmdFuncs.h"
#include "Version.h"
#include "StatForServer.h"
#include "ThreadsMgr.h"

// CDlgMainTabRss 对话框

IMPLEMENT_DYNAMIC(CDlgMainTabRss, CDialog)
CDlgMainTabRss::CDlgMainTabRss(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgMainTabRss::IDD, pParent)
{
	m_pFeedManager = new CRssFeedManager;
	m_pwndRssFeed = new CRssFeedWnd;
	m_pUpdateThread = NULL;
/*
	m_pToolBar = new CTbcRss;
	m_plistHistory = new CFeedItemListCtrl;
	m_pwndRssFilter = new CRssFilterWnd;
	m_pTabWnd = new CTabWnd;
	m_posRss = NULL;
*/
	CGlobalVariable::s_wndRssCtrl = this;
}

CDlgMainTabRss::~CDlgMainTabRss()
{
	Clear();
}

/// 清除资源
void CDlgMainTabRss::Clear()
{
	CGlobalVariable::s_wndRssCtrl = NULL;

	/*
	delete m_pTabWnd;
	m_pTabWnd = NULL;

	delete m_plistHistory;
	m_plistHistory = NULL;

	delete m_pwndRssFilter;
	m_pwndRssFilter = NULL;

	delete m_pToolBar;
	m_pToolBar = NULL;
	*/

	if ( m_pwndRssFeed != NULL )
	{
		m_pwndRssFeed->DestroyWindow();
		delete m_pwndRssFeed;
		m_pwndRssFeed = NULL;
	}

	delete m_pFeedManager;
	m_pFeedManager = NULL;
	m_pUpdateThread = NULL;
}

// 通知feed已更新（由线程调用）
void CDlgMainTabRss::OnNotifyRssFeedUpdated(const CString & strUrl, const char * lpszStream)
{
	SendMessage(WM_RSS_FEED_UPDATE, (WPARAM)(&strUrl), (LPARAM)lpszStream);
}

// 获取RSS过滤后的结果
void CDlgMainTabRss::OnRetrieveRssFilterResult(const CRssFilter & /*filter*/, CRssFeed::Item & item)
{
	// 需要检查是否为历史项目
	BOOL bDownloadResult = RequestDownloadFile(&item, FALSE);
	if ( bDownloadResult )
	{
		// 对应的feed设置为提醒状态
		//ASSERT(m_pwndRssFeed != NULL);
		//if ( CFeedItemListCtrl::CtrlFeed * pCtrlFeed = m_pwndRssFeed->m_listFeedItem.GetCtrlFeed(item.m_pFeed) )
		//{
		//	pCtrlFeed->m_bNew = true;
		//}
	}
}

// 请求下载一个文件
BOOL CDlgMainTabRss::RequestDownloadFile(CRssFeed::Item * pItem, BOOL bIgnoreHistoryFlag)
{
	ASSERT(m_pFeedManager != NULL);
//	ASSERT(m_plistHistory != NULL);

	if ( pItem == NULL ||								// 无项目，不下载
		 (!bIgnoreHistoryFlag && pItem->m_bIsHistory) ||	// （不忽略历史）为历史项目，不下载
		 pItem->m_strEnclosure.IsEmpty() ||				// 无下载url，不下载
		 !pItem->CanDownload()							// 当前状态不可下载，不下载
	   )
	{
		return FALSE;
	}

	ASSERT(pItem->m_pFeed != NULL);

	// 下载资源
	CPartFile * pPartFile = CDlgAddTask::SilenceAddNewTask(pItem->m_strEnclosure, pItem->m_pFeed->m_strSaveDir);
	if ( pPartFile == NULL )
	{
		pPartFile = CRssFeed::GetPartFile(pItem->m_strEnclosure);
	}

	if ( !pItem->m_bIsHistory )
	{
		// 记录最后下载的日期
		if ( pItem->m_timePubDate > pItem->m_pFeed->m_timeLastDownload )
		{
			pItem->m_pFeed->m_timeLastDownload = pItem->m_timePubDate;

			// 保存feed
			SaveFeed(*static_cast<CRssFeed*>(pItem->m_pFeed));
		}

		// 标记为历史
		//CRssFeed::Item & itemHistory = m_pFeedManager->MarkHistory(*pItem);
		m_pFeedManager->MarkHistory(*pItem);
		ASSERT(pItem->m_bIsHistory);
	}

	// 保存partfile
	//ASSERT(pItem->GetPartFile() == NULL);
	pItem->SetPartFile(pPartFile);

	// 添加到历史页面中
	//m_plistHistory->AddFeedItem(itemHistory);

	// 订阅下载数+1
	theStatForServer.m_statRssInfo.wStartRssTasks++;

	return TRUE;
}

// 初始化UI
void CDlgMainTabRss::InitUI()
{
	//ASSERT(m_pTabWnd != NULL);
	//ASSERT(m_pToolBar != NULL);
	ASSERT(m_pwndRssFeed != NULL);
//	ASSERT(m_plistHistory != NULL);
//	ASSERT(m_pwndRssFilter != NULL);

	ModifyStyle(0, WS_CLIPCHILDREN, 0);

//	m_pTabWnd->Create(WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN, CRect(0, 0, 0, 0), this, 0);
//	m_pTabWnd->SetBarBkDraw(new CPageTabBkDraw);

	//	add Toolbar
	/*
	m_pToolBar->Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, AFX_IDW_TOOLBAR);
	m_pToolBar->SetOwner(this);
	m_pToolBar->SetIndent(8);
	m_pToolBar->SetBkDraw(new CPageTabBkDraw);

	CTabItem_Wnd	*pTabItemWnd = new CTabItem_Wnd;
	pTabItemWnd->SetItemWnd(m_pToolBar, FALSE);
	pTabItemWnd->SetDynDesireLength(TRUE);
	m_pTabWnd->AddTab(pTabItemWnd);
	pTabItemWnd = NULL;

	// add rss feed table
	m_pwndRssFeed->Create(m_pwndRssFeed->IDD, this);
	m_pwndRssFeed->SetOwner(this);
	CTabItem_Normal	*pTI_Normal = new CTabItem_Normal;
	pTI_Normal->SetCaption(_T("最新"));
	pTI_Normal->SetRelativeWnd(m_pwndRssFeed->GetSafeHwnd());
	//pTI_Normal->SetDesireLength(300);
	pTI_Normal->SetDynDesireLength(TRUE);
	m_posRss = m_pTabWnd->AddTab(pTI_Normal, TRUE);
	pTI_Normal = NULL;

	// add history table
	m_plistHistory->Create(WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS | 
						   LVS_NOLABELWRAP | LVS_ALIGNLEFT | WS_TABSTOP, 
						   CRect(0, 0, 100, 100),
						   this, 
						   IDC_RW_LIST_HISTORY
						  );
	m_plistHistory->Init();
	m_plistHistory->SetOwner(this);
	pTI_Normal = new CTabItem_Normal;
	pTI_Normal->SetCaption(_T("历史"));
	pTI_Normal->SetRelativeWnd(m_plistHistory->GetSafeHwnd());
	//pTI_Normal->SetDesireLength(300);
	pTI_Normal->SetDynDesireLength(TRUE);
	m_pTabWnd->AddTab(pTI_Normal);
	pTI_Normal = NULL;

	// add filter table
	m_pwndRssFilter->Create(m_pwndRssFilter->IDD, this);
	m_pwndRssFilter->SetOwner(this);
	pTI_Normal = new CTabItem_Normal;
	pTI_Normal->SetCaption(_T("过滤器"));
	pTI_Normal->SetRelativeWnd(m_pwndRssFilter->GetSafeHwnd());
	//pTI_Normal->SetDesireLength(300);
	pTI_Normal->SetDynDesireLength(TRUE);
	m_pTabWnd->AddTab(pTI_Normal);
	pTI_Normal = NULL;*/

	m_pwndRssFeed->Create(m_pwndRssFeed->IDD, this);
	m_pwndRssFeed->SetOwner(this);
	m_pwndRssFeed->ShowWindow(SW_NORMAL);
}

// 设置feed新的已下载时间（处理最新已完成数）
void CDlgMainTabRss::SetFeedNewDownloadedTime(CRssFeed & feed)
{
	if ( this == NULL )
	{
		return;
	}

	ASSERT(m_pFeedManager != NULL);
	m_pFeedManager->SetNewDownloadedTime(feed);
}

// 保存订阅设置
void CDlgMainTabRss::SaveFeed(const CRssFeed & feed)
{
	if ( this == NULL )
	{
		return;
	}

	ASSERT(m_pFeedManager != NULL);
	m_pFeedManager->SaveFeed(feed);
}

// 添加订阅
void CDlgMainTabRss::AddFeed(const CString & strFeedUrl, const CString & strSaveDir, BOOL bAutoDownload)
{
	if ( this == NULL )
	{
		return;
	}

	ASSERT(m_pFeedManager != NULL);
	ASSERT(m_pwndRssFeed != NULL);

	CString strPrompt;
	if ( m_pFeedManager->FindFeed(strFeedUrl) )
	{
		strPrompt = GetResString(IDS_RW_FEED_EXISTS_PROMPT);
	}
	else
	{
		CRssFeed & feed = m_pFeedManager->AddFeed(strFeedUrl, strSaveDir);
		m_pwndRssFeed->AddFeed(feed, TRUE);
		m_pwndRssFeed->AddICOFeed(feed, TRUE);
		feed.SetAutoDownload(bAutoDownload);
		if ( bAutoDownload )
		{
			feed.SetUpdateInterval(30 * 60);
		}
		else
		{
			feed.SetUpdateInterval(CRssFeed::MANUAL_REFRESH_VALUE);
		}
		//SaveRssConfig();

		StartUpdateFeed(feed, FALSE);
		strPrompt.Format(GetResString(IDS_RW_ADD_FEED_SUCC_PROMPT), static_cast<LPCTSTR>(strFeedUrl));
	}
	
	CGlobalVariable::ShowNotifier(strPrompt, TBN_IMPORTANTEVENT);
}

// 这个AddFeed()不更新Feed
void CDlgMainTabRss::AddFeed(CRssFeed* pFeed, const CString & strSaveDir, BOOL bAutoDownload)
{

	ASSERT(m_pFeedManager != NULL);
	ASSERT(m_pwndRssFeed != NULL);

	CString strPrompt;
	if ( m_pFeedManager->FindFeed(pFeed->m_strFeedUrl) )
	{
		strPrompt = GetResString(IDS_RW_FEED_EXISTS_PROMPT);
	}
	else
	{
		CRssFeed & feed = m_pFeedManager->AddFeed(pFeed->m_strFeedUrl, strSaveDir);
		feed = *pFeed;

		m_pwndRssFeed->AddFeed(feed, TRUE);
		m_pwndRssFeed->AddICOFeed(feed, TRUE);
		feed.SetAutoDownload(bAutoDownload);
		if ( bAutoDownload )
		{
			feed.SetUpdateInterval(30 * 60);
		}
		else
		{
			feed.SetUpdateInterval(CRssFeed::MANUAL_REFRESH_VALUE);
		}
			
		feed.m_iState |= CRssFeed::fsRefresh;
		feed.m_bLastUpdateSucc = false;

		// 检查Id是否是类别树中的叶子
		CFeedCatalogs::iterator it = CFeedCatalogs::Find(feed.m_uCatalogId);

		if ( it != CFeedCatalogs::GetEnd() && it->second.IsLeaf() )
		{			
			it->second.m_setFeeds.insert(&feed);
			TRACE(TEXT("\n--- insert to CFeedCatalogs in New Fn --- \n"));
		}
		
		CRssFeedList listHistory = m_pFeedManager->GetHistoryFeedList();
		for (CRssFeedBase::ItemList::iterator it = feed.GetItemIterBegin(); it != feed.GetItemIterEnd(); ++it)
		{
			if (listHistory.FindFirstItem(it->second.m_strGuid))
			{
				it->second.m_bIsHistory = TRUE;
			}

			if ( !it->second.m_strEnclosure.IsEmpty() )
			{
				if ( !it->second.m_bIsHistory)
				{
					if ( it->second.GetPartFile() || it->second.GetShareFile() )
					{
						m_pFeedManager->MarkHistory(it->second);
					}
				}
			}	
		}

		feed.m_iState &= ~int(CRssFeed::fsRefresh);
		m_pFeedManager->SaveFeed(feed);

		feed.GetPoster(*this);

		// feed更新成功，重画指定的feed及其子项目
		m_pwndRssFeed->RedrawFeed(feed);

		m_pwndRssFeed->m_listFeedItem.UpdateSort();//更新排序

		//需要将更新到的订阅名称设置一下,以便ListCtrl设置Caption 区域
		int nIndex = m_pwndRssFeed->m_listIcoFeedItem.GetItemIndex(&feed);

		CString strCaption = m_pwndRssFeed->m_listIcoFeedItem.GetExtentString(feed.GetDisplayName());
		m_pwndRssFeed->m_listIcoFeedItem.SetItemText(nIndex, 0, strCaption);
		
		strPrompt.Format(GetResString(IDS_RW_ADD_FEED_SUCC_PROMPT), static_cast<LPCTSTR>(pFeed->m_strFeedUrl));
	}

	CGlobalVariable::ShowNotifier(strPrompt, TBN_IMPORTANTEVENT);	
}

// partfile被删除时调用
void CDlgMainTabRss::ProcessDeletePartFile(const CKnownFile * pPartFile)
{
	if ( this == NULL )
	{
		return;
	}

	if ( pPartFile == NULL || !pPartFile->IsKindOf(RUNTIME_CLASS(CPartFile)) )
	{
		return;
	}

	if ( CRssFeed::Item * pItem = GetFeedItem(static_cast<const CPartFile *>(pPartFile)) )
	{
		// 通知删除
		pItem->OnDeletePartFile();

		// 重画feed item
		ASSERT(m_pwndRssFeed != NULL);
		m_pwndRssFeed->RedrawFeedItem(*pItem);
	}

}
// partfile状态更改时调用
void CDlgMainTabRss::ProcessPartFileStateChanged(const CPartFile * pPartFile)
{
	if ( this == NULL )
	{
		return;
	}

	if ( CRssFeed::Item * pItem = GetFeedItem(pPartFile) )
	{
		ASSERT(m_pwndRssFeed != NULL);

		//// 若资源非字幕，且已下载完成，则设置订阅的新已完成文件标志
		//if ( !pItem->IsSubtitle() && pPartFile != NULL && pPartFile->GetStatus(false) == PS_COMPLETE )
		//{
		//	ASSERT(pItem->m_pFeed != NULL);
		//	pItem->m_pFeed->m_bHasNewComplete = true;
		//}

		switch ( pItem->GetState() )
		{
			case CRssFeed::isDownloading:
				// 正在下载文件时，下载已完成时间需要清空
				pItem->ResetDownloadedTime();
				break;

			case CRssFeed::isDownloaded:
				// 已下载完成，计数
				theStatForServer.m_statRssInfo.wDownloadedRssTasks++;
				break;
		}

		// 重画feed所在行
		m_pwndRssFeed->RedrawFeedLine(pItem->m_pFeed);

		// 重画feed item
		m_pwndRssFeed->RedrawFeedItem(*pItem);

		CDownloadTabWnd & wd = theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_DownloadTabWnd;
		m_pwndRssFeed->UpdateToolBarState(wd.m_Toolbar);//更新toolbar状态
	}

}
// 根据partfile查找feed item
CRssFeed::Item * CDlgMainTabRss::GetFeedItem(const CPartFile * pPartFile)
{
	if ( pPartFile != NULL )
	{
		ASSERT(m_pFeedManager != NULL);
		if ( CRssFeed::Item *pItem = m_pFeedManager->GetFeedList().FindFirstItem(m_pFeedManager, pPartFile) )
		{
			ASSERT(pItem->GetPartFile() == pPartFile);
			return pItem;
		}
	}

	return NULL;
}
// 初始化feed管理器
void CDlgMainTabRss::InitFeedManager()
{
	ASSERT(m_pFeedManager != NULL);

	m_pFeedManager->LoadConfig();

	// 添加所有Feed
	AddAllFeeds();

	m_pwndRssFeed->m_listFeedItem.UpdateSort(false);//初始化排序
	// 添加所有历史项目
	//AddAllHistoryItems();

	// 添加所有过滤器
	//AddAllFilters();
}

// 添加所有Feed
void CDlgMainTabRss::AddAllFeeds()
{
	ASSERT(m_pFeedManager != NULL);
	ASSERT(m_pwndRssFeed != NULL);

	m_pwndRssFeed->BeginUpdateFeed();	
	// 添加feed
	CRssFeedList & listFeed = m_pFeedManager->GetFeedList();
	for ( CRssFeedList::iterator it = listFeed.begin();
		  it != listFeed.end();
		  ++it
		)
	{
		CRssFeed & feed = it->second;
		feed.GetPoster(*this);
		m_pwndRssFeed->AddFeed(feed, FALSE);
	}

	//更简洁的加载feed
	CFeedCatalogs & feedCatalogs = CFeedCatalogs::GetInstance();
	for ( CFeedCatalogs::iterator it = feedCatalogs.GetBegin(); it != feedCatalogs.GetEnd(); ++it )
	{
		FeedCatalog & feedCatlog = it->second;
		for ( CRssFeedSet::const_iterator it = feedCatlog.m_setFeeds.begin(); it != feedCatlog.m_setFeeds.end(); ++it )
		{
			CRssFeedBase* pFeed = *it;
			if ( pFeed != NULL )
			{		
				CRssFeed & Feed = *( (CRssFeed*)pFeed);
				m_pwndRssFeed->AddICOFeed(Feed, FALSE);
			}	
		}
	}
	
	StartUpdateAllFeeds(FALSE);

	//加载完毕,一次性进行分组
	m_pwndRssFeed->m_listIcoFeedItem.GroupAllFeedsByType(0);
	m_pwndRssFeed->EndUpdateFeed();
}

//// 添加所有历史项目
//void CDlgMainTabRss::AddAllHistoryItems()
//{
//	ASSERT(m_pFeedManager != NULL);
//	ASSERT(m_plistHistory != NULL);
//
//	CRssFeedList & listFeed = m_pFeedManager->GetHistoryFeedList();
//	m_plistHistory->SetRedraw(FALSE);
//	for ( CRssFeedList::iterator it = listFeed.begin();
//		  it != listFeed.end();
//		  ++it
//		)
//	{
//		CRssFeed & feed = it->second;
//
//		// 重画全部feed items
//		m_plistHistory->AddAllFeedItem(feed);
//	}
//	m_plistHistory->SetRedraw(TRUE);
//}

// 添加所有过滤器
//void CDlgMainTabRss::AddAllFilters()
//{
//	
//	ASSERT(m_pFeedManager != NULL);
////	ASSERT(m_pwndRssFilter != NULL);
//
//	CRssFilterList & listFilter = m_pFeedManager->GetFilterList();
//	m_pwndRssFilter->BeginUpdateFilter();
//	for ( CRssFilterList::iterator it = listFilter.begin();
//		  it != listFilter.end();
//		  ++it
//		)
//	{
//		CRssFilter & filter = it->second;
//
//		// 重画全部feed items
//		m_pwndRssFilter->AddFilterToList(filter);
//	}
//	m_pwndRssFilter->EndUpdateFilter();
//
//}

// 更新所有feed内容
BOOL CDlgMainTabRss::StartUpdateAllFeeds(BOOL bCheckOverdue)
{	
	if (m_pUpdateThread != NULL)
	{
		return	TRUE;
	}
	else
	{
		UPDATETHREAD_PARAM* pParam = new UPDATETHREAD_PARAM;
		pParam->bCheckOverdue = bCheckOverdue;
		pParam->bCreateNewThread = FALSE;
		pParam->listFeed = m_pFeedManager->GetFeedList();
		CThreadsMgr::BegingThreadAndRecDown(CThreadsMgr::CleanProc_WaitAndDelWinThd, (AFX_THREADPROC)&UpdateThread, pParam);
	}
	
	return TRUE;
}

UINT CDlgMainTabRss::UpdateThread(LPVOID pParam)
{
	CUnregThreadAssist uta(GetCurrentThreadId());

	BOOL bCheckOver = ((UPDATETHREAD_PARAM*)pParam)->bCheckOverdue;
	BOOL bCreateNewThread = ((UPDATETHREAD_PARAM*)pParam)->bCreateNewThread;

	CRssFeedList & listFeed = ((UPDATETHREAD_PARAM*)pParam)->listFeed;
	
	for (CRssFeedList::iterator it = listFeed.begin(); it != listFeed.end(); ++it)
	{
		CRssFeed & feed = it->second;
		CGlobalVariable::s_wndRssCtrl->StartUpdateFeed(feed, bCheckOver, bCreateNewThread);
	}

	delete (UPDATETHREAD_PARAM*)pParam;
	CGlobalVariable::s_wndRssCtrl->m_pUpdateThread = NULL;
	return 0;
}

// 更新一个feed内容
BOOL CDlgMainTabRss::StartUpdateFeed(CRssFeed & feed, BOOL bCheckOverdue, BOOL bCreateNewThread)
{
	// 最短更新时间间隔为30分钟
	UINT uOverdue = UINT(-1);
	if ( feed.UpdateIntervalIsDefined() )
	{
		// feed的UpdateInterval已定义，使用单独配置的值
		uOverdue = feed.GetUpdateInterval();
	}
	else
	{
		// feed的UpdateInterval未定义，使用全局配置的值
		uOverdue = (thePrefs.m_uRssUpdateInterval >= 30 * 60 ? thePrefs.m_uRssUpdateInterval : 30 * 60);
	}

	if ( !bCheckOverdue || feed.IsOverdue(uOverdue) )
	{
		ASSERT(m_pFeedManager != NULL);
		m_pFeedManager->StartUpdateFeed(feed, *this, bCreateNewThread);
		return TRUE;
	}

	return FALSE;
}


void CDlgMainTabRss::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgMainTabRss, CDialog)
	ON_WM_SIZE()
	ON_MESSAGE(WM_RSS_FEED_UPDATE, OnFeedUpdate)
	ON_MESSAGE(WM_RSS_POSTER_UPDATE, OnPosterUpdate)
	ON_MESSAGE(WM_RSS_REQ_DOWNLOAD_FILE, OnReqDownloadFile)
	ON_COMMAND(MP_NEW, OnAddNewFeed)
	ON_COMMAND(MP_REFRESH, OnRefreshFeed)
	ON_COMMAND(MP_CANCEL, OnDeleteCommand)
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CDlgMainTabRss 消息处理程序

void CDlgMainTabRss::OnSize(UINT nType, int cx, int cy)
{
	//ASSERT(m_pTabWnd != NULL);

	CDialog::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	
	if ( m_pwndRssFeed->GetSafeHwnd() != NULL )
	{
		m_pwndRssFeed->MoveWindow(0, 0, cx, cy, FALSE);
	}
	
}

BOOL CDlgMainTabRss::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if ( pMsg->message == WM_KEYDOWN )
	{
		// Don't handle Ctrl+Tab in this window. It will be handled by main window.
		if ( pMsg->wParam == VK_TAB && GetAsyncKeyState(VK_CONTROL) < 0 )
		{
			return FALSE;
		}

		if ( VK_RETURN == pMsg->wParam || VK_ESCAPE == pMsg->wParam )
		{
			return FALSE;
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CDlgMainTabRss::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	
	InitUI();
	InitFeedManager();

	SetTimer(UPDATE_FEED_TIME_ID, UPDATE_FEED_INTERVAL, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CDlgMainTabRss::OnNotifyPosterDownload(CWebImage & webImage, const int & /* nCode */, CxImage* pImage)
{
	SendMessage(WM_RSS_POSTER_UPDATE, reinterpret_cast<WPARAM>(&webImage), reinterpret_cast<LPARAM>(pImage));
}

LRESULT CDlgMainTabRss::OnPosterUpdate(WPARAM wParam, LPARAM lParam)
{
	CWebImage * pWeb = reinterpret_cast<CWebImage*>(wParam);
	CxImage * pImage = reinterpret_cast<CxImage*>(lParam);
	ASSERT(pWeb != NULL);

	// 查找对应的feed是否存在
	CRssFeed * pFeed = NULL;
	if ( CString * pFeedUrl = static_cast<CString*>(pWeb->m_pAttachData) )
	{
		ASSERT(m_pFeedManager != NULL);
		pFeed = m_pFeedManager->FindFeed(*pFeedUrl);
		pWeb->m_pAttachData = NULL;
		delete pFeedUrl;
	}

	// 若feed不存在，则清除相关资源
	if ( pFeed == NULL )
	{
		delete pWeb;
		delete pImage;
		return FALSE;
	}

	ASSERT(pFeed->m_pWebImage == pWeb);
	
	// 更新海报时，订阅必然无海报
	ASSERT(pFeed->m_pPosterImage == NULL);

	// 保存海报
	delete pFeed->m_pPosterImage;	// 当前海报应该为空
	pFeed->m_pPosterImage = pImage;

	// 删除海报获取对象
	CWebImage * pWebImage = pFeed->m_pWebImage;
	pFeed->m_pWebImage = NULL;
	delete pWebImage;

	// 重画海报
	if ( pImage != NULL )
	{
		ASSERT(m_pwndRssFeed != NULL);
		m_pwndRssFeed->RedrawFeedLine(pFeed);
		//ICOList 不需要重绘.因为feed已经更新
		//m_pwndRssFeed->RedrawFeedItemPoster(pFeed);
	}

	return TRUE;
}

LRESULT CDlgMainTabRss::OnFeedUpdate(WPARAM wParam, LPARAM lParam)
{
	ASSERT(m_pFeedManager != NULL);
	ASSERT(m_pwndRssFeed != NULL);

	CString * pStrUrl = reinterpret_cast<CString *>(wParam);
	const char * lpszStream = reinterpret_cast<const char *>(lParam);
	ASSERT(pStrUrl != NULL);
	ASSERT(lpszStream != NULL);

	CRssFeed * pFeed = m_pFeedManager->FindFeed(*pStrUrl);
	if ( pFeed == NULL )
	{
		return NULL;
	}
	//获取feed是否已经取得分类信息
	BOOL bHasCatalogId = (pFeed->m_uCatalogId != FeedCatalog::INVALID_CATALOG_ID);

	// 更新数据
	LRESULT result = m_pFeedManager->OnFeedUpdateMsg(*pStrUrl, lpszStream);

	if ( result != NULL )
	{
		CRssFeed * pFeed = reinterpret_cast<CRssFeed*>(result);

		if ( pFeed->AutoDownloadIsDefined() )
		{
			// feed定义了是否自动下载，采用针对此feed的单独设置
			if ( pFeed->GetAutoDownload() )
			{
				// feed定义了是否自动下载，且要求自动下载
				// 启动过滤器进行自动下载
				m_pFeedManager->StartFilter(*pFeed, *this);
			}
		}
		else if ( thePrefs.m_bRssAutoDownload )
		{
			// feed未定义是否自动下载，全局设置要求自动下载
			// 启动过滤器进行自动下载
			m_pFeedManager->StartFilter(*pFeed, *this);
		}

		// feed更新成功，尝试获取海报
		pFeed->GetPoster(*this);

		// feed更新成功，重画指定的feed及其子项目
		m_pwndRssFeed->RedrawFeed(*pFeed);

		m_pwndRssFeed->m_listFeedItem.UpdateSort();//更新排序

		//需要将更新到的订阅名称设置一下,以便ListCtrl设置Caption 区域
		int nIndex = m_pwndRssFeed->m_listIcoFeedItem.GetItemIndex(pFeed);

		if ( !bHasCatalogId )//首次取得分类信息,进行分类
		{
			m_pwndRssFeed->m_listIcoFeedItem.RemoveItemforMap(*pFeed, nIndex, true);
			m_pwndRssFeed->m_listIcoFeedItem.GroupFeedByType(nIndex,0);
		}

		CString strCaption = m_pwndRssFeed->m_listIcoFeedItem.GetExtentString(pFeed->GetDisplayName());
		m_pwndRssFeed->m_listIcoFeedItem.SetItemText(nIndex, 0, strCaption);
	}
	else
	{
		// feed没有更新成功
		// 采用先删除再添加的方式更新，以防止排序错乱问题
		m_pwndRssFeed->m_listFeedItem.ReAddFeed(*pFeed);
	}

	return result;
}

LRESULT CDlgMainTabRss::OnReqDownloadFile(WPARAM wParam, LPARAM lParam)
{
	CRssFeed::Item * pItem = reinterpret_cast<CRssFeed::Item *>(wParam);
	return RequestDownloadFile(pItem, (BOOL)lParam);
}

void CDlgMainTabRss::OnTimer(UINT nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if ( nIDEvent == UPDATE_FEED_TIME_ID )
	{
		StartUpdateAllFeeds(TRUE);


		//// 每隔一段时间保存一次，以防止电驴非正常退出后丢失订阅的内容
		//SaveRssConfig();
	}

	CDialog::OnTimer(nIDEvent);
}

//// 保存配置
//void CDlgMainTabRss::SaveRssConfig()
//{
//	ASSERT(m_pFeedManager != NULL);
//
//	// 若有改动，则保存
//	m_pFeedManager->SaveConfig(false);
//}

void CDlgMainTabRss::OnDestroy()
{
	KillTimer(UPDATE_FEED_TIME_ID);

	Clear();

	CDialog::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
}

void CDlgMainTabRss::OnAddNewFeed()
{
	CmdFuncs::PopupNewTaskDlg();
}

void CDlgMainTabRss::OnRefreshFeed()
{
	ASSERT(m_pwndRssFeed != NULL);

	// 刷新指定的feed
	int index = -1;
	if (m_pwndRssFeed->m_ListShowMode == LISTSTYLE_ITEM)
	{
		index = m_pwndRssFeed->m_listFeedItem.GetCurrSelectIndex();
	}
	else if (m_pwndRssFeed->m_ListShowMode == LISTSTYLE_SINGLE)
	{
		index = m_pwndRssFeed->m_listSingleFeedItem.GetCurrSelectIndex();
	}

	if ( index < 0 )
	{
		// 没有选中的则刷新全部
		StartUpdateAllFeeds(FALSE);
		return;
	}

	CRssFeed * pFeed;
	if (m_pwndRssFeed->m_ListShowMode == LISTSTYLE_ITEM)
	{
		pFeed = m_pwndRssFeed->m_listFeedItem.GetFeed(index);
	}
	else if (m_pwndRssFeed->m_ListShowMode == LISTSTYLE_SINGLE)
	{
		pFeed = m_pwndRssFeed->m_listSingleFeedItem.GetFeed(index);
	}
	
	if ( pFeed == NULL )
	{
		return;
	}

	if ( StartUpdateFeed(*pFeed, FALSE) )
	{
		// 更新feed
		// 成功启动更新，则重画feed所在行
		m_pwndRssFeed->RedrawFeedLine(pFeed);
	}

}

void CDlgMainTabRss::OnDeleteCommand()
{
	ASSERT(m_pwndRssFeed != NULL);
	ASSERT(m_pFeedManager != NULL);

	int index = -1;
	if (m_pwndRssFeed->m_ListShowMode == LISTSTYLE_ICO)
	{
		index = m_pwndRssFeed->m_listIcoFeedItem.GetCurrSelectIndex();
	}
	else if(m_pwndRssFeed->m_ListShowMode == LISTSTYLE_ITEM)
	{
		index = m_pwndRssFeed->m_listFeedItem.GetCurrSelectIndex();
	}
	else if(m_pwndRssFeed->m_ListShowMode == LISTSTYLE_SINGLE)
	{
		index = m_pwndRssFeed->m_listSingleFeedItem.GetCurrSelectIndex();
	}

	if ( index < 0 )
	{
		return;
	}

	CRssFeed * pFeed;
	if (m_pwndRssFeed->m_ListShowMode == LISTSTYLE_ICO)
	{
		pFeed = m_pwndRssFeed->m_listIcoFeedItem.GetFeed(index);
	}
	else if (m_pwndRssFeed->m_ListShowMode == LISTSTYLE_ITEM)
	{
		pFeed = m_pwndRssFeed->m_listFeedItem.GetFeed(index);
	}
	else if(m_pwndRssFeed->m_ListShowMode == LISTSTYLE_SINGLE)
	{
		pFeed = m_pwndRssFeed->m_listSingleFeedItem.GetFeed(index);
	}

	if ( pFeed == NULL )
	{
		// 资源项目，暂时不处理
		return;
	}

	CString strPrompt;
	strPrompt.Format(GetResString(IDS_RW_DELETE_FEED_CONFIRM_PROMPT), static_cast<LPCTSTR>(pFeed->GetDisplayName()));
	int iResp = AfxMessageBox(strPrompt, MB_YESNO | MB_ICONQUESTION	| MB_DEFBUTTON2);
	if ( iResp == IDYES	)
	{
		// UI更新
		m_pwndRssFeed->RemoveFeed(*pFeed);
		m_pwndRssFeed->RemoveICOFeed(*pFeed);
		if(m_pwndRssFeed->m_ListShowMode == LISTSTYLE_SINGLE)
		{//在InfoList删除要跳转到ICO模式
			m_pwndRssFeed->SwitchList(LISTSTYLE_ICO);
		}

		// 更新数据
		m_pFeedManager->DeleteFeed(*pFeed);

		// 防止条目间隔色显示错位
		//m_pwndRssFeed->RedrawAllFeed();
	}
}

void CDlgMainTabRss::Localize()
{
	if (m_pwndRssFeed && ::IsWindow(m_pwndRssFeed->m_hWnd))
	{
		m_pwndRssFeed->m_listFeedItem.Localize();
		m_pwndRssFeed->m_listSingleFeedItem.Localize();
		m_pwndRssFeed->m_wndRssInfo.Localize();
	}
}

CString CDlgMainTabRss::GetCurrentFileCommentString()
{
	if (!m_pwndRssFeed || !::IsWindow(m_pwndRssFeed->m_hWnd))
	{
		return CString(_T(""));
	}
	
	POSITION Pos = NULL;

	if (m_pwndRssFeed->m_ListShowMode == LISTSTYLE_ICO)
	{
		Pos = m_pwndRssFeed->m_listIcoFeedItem.GetFirstSelectedItemPosition();
	}
	else if (m_pwndRssFeed->m_ListShowMode == LISTSTYLE_ITEM)
	{
		Pos = m_pwndRssFeed->m_listFeedItem.GetFirstSelectedItemPosition();
	}
	else if(m_pwndRssFeed->m_ListShowMode == LISTSTYLE_SINGLE)
	{
		Pos = m_pwndRssFeed->m_listSingleFeedItem.GetFirstSelectedItemPosition();
	}

	if (!Pos)
	{
		return CString(_T(""));
	}

	int nIndex;
	CRssFeed::Item* pItem;

	if (m_pwndRssFeed->m_ListShowMode == LISTSTYLE_ICO)
	{//ICO模式没有item
		nIndex = m_pwndRssFeed->m_listIcoFeedItem.GetNextSelectedItem(Pos);
		pItem = NULL;
	}
	else if (m_pwndRssFeed->m_ListShowMode == LISTSTYLE_ITEM)
	{
		nIndex = m_pwndRssFeed->m_listFeedItem.GetNextSelectedItem(Pos);
		pItem =  m_pwndRssFeed->m_listFeedItem.GetFeedItem(nIndex);
	}
	else if(m_pwndRssFeed->m_ListShowMode == LISTSTYLE_SINGLE)
	{
		nIndex = m_pwndRssFeed->m_listSingleFeedItem.GetNextSelectedItem(Pos);
		pItem =  m_pwndRssFeed->m_listSingleFeedItem.GetFeedItem(nIndex);
	}

	if (pItem && pItem->GetPartFile())
	{
		CPartFile * pPartFile = pItem->GetPartFile();
		
		CString strFileEd2k = CreateED2kLink(pPartFile, false);
		if( strFileEd2k.IsEmpty() )
		{
			return CString(_T(""));
		}

		bool bFileisFinished = true;
		if( pPartFile->IsKindOf(RUNTIME_CLASS(CPartFile)) )
		{
			if( ((CPartFile*)pPartFile)->GetStatus()!=PS_COMPLETE )
				bFileisFinished = false;
		}

		CString strCommentUrl = bFileisFinished ? thePrefs.m_strFinishedFileCommentUrl : thePrefs.m_strPartFileCommentUrl;
		strCommentUrl.Replace(_T("[ed2k]"),strFileEd2k);
		strCommentUrl.Replace(_T("|"), _T("%7C"));

		CString sVersion;
		sVersion.Format(_T("&v=%u"),VC_VERSION_BUILD);
		strCommentUrl += sVersion;
		return strCommentUrl;
	}
	else if (pItem)
	{
		CString strED2KUrl = pItem->m_strEnclosure;
		if( strED2KUrl.IsEmpty() || strED2KUrl.Left(7).CompareNoCase(_T("ed2k://")))
		{
			return CString(_T(""));
		}

		CString strCommentUrl = thePrefs.m_strPartFileCommentUrl;
		strCommentUrl.Replace(_T("[ed2k]"),strED2KUrl);
		strCommentUrl.Replace(_T("|"), _T("%7C"));

		CString sVersion;
		sVersion.Format(_T("&v=%u"),VC_VERSION_BUILD);
		strCommentUrl += sVersion;
		return strCommentUrl;
	}
	else
	{
		CRssFeed* pFeed;
		if (m_pwndRssFeed->m_ListShowMode == LISTSTYLE_ITEM)
		{
			pFeed = m_pwndRssFeed->m_listFeedItem.GetFeed(nIndex);
		}
		else if(m_pwndRssFeed->m_ListShowMode == LISTSTYLE_SINGLE)
		{
			pFeed = m_pwndRssFeed->m_listSingleFeedItem.GetFeed(nIndex);
		}
		
		if ( pFeed == NULL )
		{
			pFeed = m_pwndRssFeed->m_listFeedItem.GetBlankLineFeed(nIndex - 1);
		}

		if (pFeed)
		{
			CString strCommentUrl = pFeed->m_strFeedUrl;
			strCommentUrl.MakeLower();
			int nPos = strCommentUrl.Find(_T("/feed"));
			if (nPos != -1)
			{
				strCommentUrl = strCommentUrl.Left(nPos);
				strCommentUrl +=_T("/comments/");
				return strCommentUrl;	
			}
		}

		return CString();
		//CString strFilePath;
		//strFilePath = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("Default.htm");
		//return strFilePath;
	}
}
