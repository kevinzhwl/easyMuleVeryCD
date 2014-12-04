/*
 * $Id: RssFeedWnd.h 20785 2010-11-12 02:44:52Z huangqing $
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

// CRssFeedWnd 对话框

#include "afxwin.h"
#include "afxcmn.h"
#include "resource.h"
#include "ResizableLib\ResizableDialog.h"
#include "SplitterControlEx.h"
#include "FeedItemListCtrl.h"
#include "FeedIcoItemListCtrl.h"
#include "RssInfoWnd.h"
#include "WorkLayer\rss\RssFeed.h"

//class CFeedListBox : public CListBox
//{
//	DECLARE_DYNAMIC(CFeedListBox)
//
//public:
//	CFeedListBox();
//
//public:
//	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
//	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
//protected:
//	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
//
//private:
//	HICON	m_hIcon[CRssFeed::fsMaxState];
//	int		m_iItemHeight;
//};

enum ListShowMode
{
	LISTSTYLE_ITEM		= 0,
	LISTSTYLE_ICO	= 1,
	LISTSTYLE_SINGLE = 2
};

class CRssFeedWnd : public CResizableDialog
{
	DECLARE_DYNAMIC(CRssFeedWnd)

public:
	CRssFeedWnd(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CRssFeedWnd();

// 对话框数据
	enum { IDD = IDD_RW_FEED };

public:
	// 重画全部feed
	void RedrawAllFeed()
	{
		m_listFeedItem.Invalidate(TRUE);
		m_listIcoFeedItem.Invalidate(TRUE);
	}

	// 更新toolbar按钮状态
	void UpdateToolBarState(CToolBarCtrl & toolbar)
	{
		//根据当前显示的List更新Toolbar状态
		if ( m_ListShowMode == LISTSTYLE_ITEM )
		{
			m_listFeedItem.UpdateToolBarState(toolbar);
		}
		else if ( m_ListShowMode == LISTSTYLE_ICO )
		{
			m_listIcoFeedItem.UpdateToolBarState(toolbar);
		}
		else if ( m_ListShowMode == LISTSTYLE_SINGLE )
		{
			m_listSingleFeedItem.UpdateToolBarState(toolbar);
		}
	}

	// 在列表框中删除一个feed
	void RemoveFeed(CRssFeed & feed)
	{
		m_listFeedItem.RemoveFeed(feed);
	}

	// 在列表框中删除一个feed
	void RemoveICOFeed(CRssFeed & feed)
	{
		m_listIcoFeedItem.RemoveFeed(feed);
	}

	// 在列表框中添加一个feed
	void AddFeed(CRssFeed & feed, BOOL bIsNew)
	{
		m_listFeedItem.AddFeed(feed, bIsNew);
	}

	// 在列表框中添加一个feed
	void AddICOFeed(CRssFeed & feed, BOOL bIsNew)
	{
		m_listIcoFeedItem.AddFeed(feed, bIsNew);
	}

	// 重新显示一个feed及其item信息
	void RedrawFeed(CRssFeed & feed);

	// 重画feed所在的行
	void RedrawFeedLine(CRssFeedBase * pFeed)
	{
		m_listFeedItem.RedrawFeedLine(pFeed);
		m_listSingleFeedItem.RedrawFeedLine(pFeed);
	}

	// 更新ICOlist 海报
	//void RedrawFeedItemPoster(CRssFeed* item)
	//{
	//	m_listIcoFeedItem.UpdatePoster(item);
	//}

	// 重画feed item所在的行
	void RedrawFeedItem(CRssFeed::Item & item)
	{
		m_listFeedItem.RedrawFeedItem(item);
		m_listSingleFeedItem.RedrawFeedItem(item);
	}

	// 准备更新feed
	void BeginUpdateFeed()
	{
		m_listFeedItem.SetRedraw(FALSE);
		m_listIcoFeedItem.SetRedraw(FALSE);
		m_listSingleFeedItem.SetRedraw(FALSE);
	}

	// 结束更新feed
	void EndUpdateFeed()
	{
		m_listFeedItem.SetRedraw(TRUE);
		m_listIcoFeedItem.SetRedraw(TRUE);
		m_listSingleFeedItem.SetRedraw(TRUE);
		//m_lsbFeed.Invalidate();
	}

	// 下载指定feed项目
	BOOL DownloadFeedItem(int iItemIndex);

	//下载feed下所有项目
	BOOL DownloadAllItem(int nIndex);

	//下载feed下暂停/停止任务
	BOOL ResumeAllItem(int nIndex);

	// 发送下载请求消息
	BOOL SendDownloadRequestMessage(CRssFeed::Item & item);

	//打开指定下载完成feed项目
	BOOL OpenFeedItem(int iItemIndex);

	//切换2个list
	void SwitchList(ListShowMode listmode);

	//设置Toolbar关联到当前List
	void SetCurrentList();

	//设置Toolbar 关联到相应List
	void SetToolbar2List(ListShowMode listmode);

	void JumpListInfo(CRssFeed * pFeed, ListShowMode listmode = LISTSTYLE_SINGLE/*LISTSTYLE_ITEM*/);

	//自动切换List Show Mode
	void AutoSwitchListMode();//暂时没地方用

	//清除singleList中的数据,在切换视图的时候使用
	void ClearSingleList();

	//获取infoList当前Feed的前后feed
	void GetCurFeedInfo(CRssFeed * pCurFeed);

	ListShowMode m_ListShowMode;

public: // from CResizableDialog
	virtual BOOL OnInitDialog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
	//afx_msg void OnVSplitterClicked(NMHDR* pNMHDR, LRESULT* pResult);
	//afx_msg void OnSplitterMoved(NMHDR *pNMHDR, LRESULT *pResult);
	//afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	//afx_msg void OnLbnSelchangeRwFeedList();
	afx_msg void OnNMDblclkRwFeedItemList(NMHDR* pNMHDR, LRESULT *pResult);
	//afx_msg void OnNMClickRwFeedItemList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LRESULT OnDownloadButtonClicked(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLvnItemchangedRwFeedItemList(NMHDR *pNMHDR, LRESULT *pResult);

	//切换RSS LIST SHOW MODE;由 BUTTONST类 触发
	afx_msg LRESULT OnSwitchListShowModeClicked(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnListInfoClicked(WPARAM wParam, LPARAM lParam);

private:
	// 创建分隔条
	//void CreateSplitter();

	// 调整窗口左右布局
	//void AdjustVLayout(UINT uLeftWidth = -1);

	//打开文件
	void OpenFile(const CKnownFile* file);

public:
//	CFeedListBox		m_lsbFeed;			///< Feed列表框
	CFeedItemListCtrl	m_listFeedItem;		///< Feed项目列表
	CFeedItemListCtrl	m_listSingleFeedItem;		///< 单个详情Feed项目列表
	CFeedIcoItemListCtrl m_listIcoFeedItem;
//	CSplitterControlEx	m_wndVSplitter;		///< 分隔条

	CRssInfoWnd			m_wndRssInfo;

private:
	CRssFeed * m_pFeedPre;//InfoList当前Feed的前后数据
	CRssFeed * m_pFeedNext;

	CString m_strGroupTitle;

	int m_nCurrFeedCatalog;//当前在分组的位置
	int m_nFeedCatalogCount;//分组Feed数目
	//static const int DEFAULT_LEFT_LIST_WIDTH	= 168;	///< 默认左边列表宽度
	//static const int SPLITTER_RANGE_MAX			= 350;	///< 最大拖动范围
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
