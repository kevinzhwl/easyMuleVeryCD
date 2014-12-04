/*
 * $Id: FeedItemListCtrl.h 20807 2010-11-16 11:43:07Z huhonggang $
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
#include "MuleListCtrl.h"
#include "ButtonST.h"
#include <vector>
#include "WorkLayer\rss\RssFeed.h"
#include "TitleMenu.h"
#include "MatchString.h"

// CFeedItemListCtrl
class CxImage;
class CMenuXP;

typedef CMuleListCtrl CBaseListCtrl;

class CFeedItemListCtrl : public CBaseListCtrl
{
	DECLARE_DYNAMIC(CFeedItemListCtrl)

public:
	static const int COVER_EXPAND_WIDTH = 120;	///< 封面展开时宽度
	static const int COVER_COLLAPSE_WIDTH = 22;	///< 封面展收缩时宽度

private:
	static const int ITEM_HEIGHT = 24;							///< 条目高度
	static const int COVER_LINES = 6;							///< 封面行数
	static const int COVER_HEIGHT = ITEM_HEIGHT * COVER_LINES;	///< 封面高度
	static const int SORT_ASC_FLAG = 0;							///< 升序
	static const int SORT_DESC_FLAG = 100;						///< 降序则大于等于此值

	/// 条目背景色1
	static const COLORREF ITEM_BACKGROUND_COLOR1 = RGB(0xF0, 0xF0, 0xF0);

	/// 条目背景色2
	static const COLORREF ITEM_BACKGROUND_COLOR2 = RGB(0xFF, 0xFF, 0xFF);

	/// 封面背景色
	static const COLORREF COVER_BACKGROUND_COLOR = RGB(0xF3,0xF3,0xF3);

	/// 间隔线颜色
	static const COLORREF SPLIT_LINE_COLOR = RGB(0xC0, 0xC0, 0xC0);

	/// 定时器id
	static const int TIMER_ID = 32771;

public:
	// UTC 与 本地时间时差
	static CTimeSpan s_timespan;

private:
	typedef std::vector<CButtonST *>		ItemButtons;

public:
	struct CtrlBase
	{
		virtual void DrawItem(CDC & dc, CFeedItemListCtrl & list, LPDRAWITEMSTRUCT lpDrawItemStruct) = 0;
		virtual CRssFeed * GetRssFeed()
		{
			return NULL;
		}
		virtual CRssFeed::Item * GetRssFeedItem()
		{
			return NULL;
		}

		CtrlBase()
			//: m_pbtnDownload(NULL)
		{
		}

		//CButton * m_pbtnDownload;
	};

	struct CtrlFeed : public CtrlBase
	{
		virtual void DrawItem(CDC & dc, CFeedItemListCtrl & list, LPDRAWITEMSTRUCT lpDrawItemStruct);
		virtual CRssFeed * GetRssFeed()
		{
			return m_pFeed;
		}

		CtrlFeed()
			: m_pFeed(NULL), 
			  m_bExpand(false),
			  m_bNew(false)
		{
		}

		CRssFeed *				m_pFeed;
		bool					m_bExpand;	///< 当前行是否展开
		bool					m_bNew;    ///第一次加入
	};

	struct CtrlFeedItem : public CtrlBase
	{	
		virtual void DrawItem(CDC & dc, CFeedItemListCtrl & list, LPDRAWITEMSTRUCT lpDrawItemStruct);
		virtual CRssFeed::Item * GetRssFeedItem()
		{
			return m_pFeedItem;
		}

		CtrlFeedItem()
			: m_pFeedItem(NULL),
			  m_bIsLast(FALSE)
		{
		}

		CRssFeed::Item *	m_pFeedItem;
		bool				m_bIsLast;	///< 是否为订阅中的最后一个项目
	};


public:
	CFeedItemListCtrl();
	virtual ~CFeedItemListCtrl();

	/// 初始化
	void Init(BOOL bSingle = FALSE);

public:

	/// 本地化
	void	Localize();

	/// 添加订阅
	int AddFeed(CRssFeed & feed, BOOL bIsNew)
	{
		m_stringMatcher.AddItem((void*)&feed, feed.GetDisplayName());

		int nNextIndex = GetItemCount();
		return AddFeed(nNextIndex, feed, bIsNew);
	}

	/// 重新插入feed
	void ReAddFeed(CRssFeed & feed);

	/// 删除订阅
	void RemoveFeed(CRssFeed & feed);

	/// 重新显示一个feed及其item信息
	void RedrawFeed(CRssFeed & feed);

	/// 重新显示feed（不包括子项目）
	void RedrawFeedLine(CRssFeedBase * pFeed);

	/// 重新显示一个feed item
	void RedrawFeedItem(CRssFeed::Item & item);

	/// 重新显示一个feed item
	void RedrawFeedItem(int index);

	// 添加feed项目
	//void AddFeedItem(const CRssFeed::Item & item)
	//{
	//	int nNextIndex = GetItemCount();
	//	AddFeedItem(nNextIndex, item);
	//}

	// 根据feed item状态重新设置图标
	//void ResetItemImageIndex(int iItemIndex);

	/// 取得feed item
	CRssFeed::Item * GetFeedItem(int iItemIndex);

	/// 取得feed
	CRssFeed * GetFeed(int iItemIndex);

	/// 取得项目索引
	int GetItemIndex(const POINT & pt);

	// 取得单击选中的下载按钮号
	//int GetClickDownloadBtnIndex();

	/// 根据控制信息取得行号
	int GetItemIndex(const CtrlBase * pCtrl);

	/// 根据Feed信息取得行号
	int GetItemIndex(const CRssFeed * pFeed);

	/// 根据feed取得CtrlFeed
	CtrlFeed * GetCtrlFeed(const CRssFeed * pFeed);

	/// 按钮号转item index
	int ButtonIndexToItemIndex(int iButtonIndex);

	/// 更新toolbar按钮状态
	void UpdateToolBarState(CToolBarCtrl & toolbar);

	/// 更新toolbar按钮状态
	void UpdateMenuState(CTitleMenu	& menuRSS);

	/// 取得当前选择的index
	int GetCurrSelectIndex();

	/// 手动排序
	void UpdateSort(bool bUpdate = true);

	/// 取得空行所属的feed
	CRssFeed * GetBlankLineFeed(int index);

	/// 展开到指定的折叠状态
	void SwitchToStat(int index, BOOL bExpand);

	/// 滚动指定条目至List顶部
	void ScrollIndex2Top(int nIndex);

	//设置单个详情模式
	void SetSingleStyle(BOOL bSingle){ m_bSingleStyle = bSingle; }

	// 字符串匹配筛选
	void	FilterItemsBySearchKey(CString* searchKey);

private:
	/// 初始化feed列表项目
	void InitFeedListView();

	// 初始化ImageList
	//void InitImageList();

	/// 添加订阅
	int AddFeed(int index, CRssFeed & feed, BOOL bIsNew);

	/// 添加feed下所有项目
	int AddFeedItems(int index, CRssFeed & feed);

	/// 添加feed下一个项目
	void AddFeedItem(int index, const CRssFeed::Item & item, BOOL bIsLast);

	// 画图标
	//void DrawItemIcon(CDC & dc, int iImageIndex, const RECT & rect, BOOL bSelected);

	/// 取得按钮位置
	void GetDownloadButtonRect(int index, BOOL bItem, RECT & rectBtn);

	//取得字幕图标位置
	void GetSubtitleRect(CRect & rectSubTitle , int index = -1);

	//取得完成数字rect
	void GetCompleteNumRect(int index, CString strCompleteNum, CRect & rectCompleteNum);

	// 画下载按钮
	//void DrawDownloadButton(CDC & dc, const RECT & rectBtn);

	/// 取得滚动偏移
	int GetScrollOffset(DWORD dwFlag);

	// 删除订阅下的子项目
	//void DeleteFeedItems(int index, const CRssFeed & feed);

	/// 显示下载按钮
	BOOL DrawDownloadButton(int index, CtrlBase * pCtrl, HDWP & hWinPosInfo);

	/// 重画按钮
	void RedrawAllButton();

	/// 根据feed状态取得对应的CxImage
	CxImage * GetFeedCxImage(const CRssFeed & feed);

	/// 根据feed资源项目状态取得对应的CxImage
	CxImage * GetFeedItemCxImage(const CRssFeed::Item & item);

	/// 展开/折叠feed
	void ExpandFeed(int index, CRssFeed & feed, BOOL bExpand, BOOL bSetRedraw);

	/// 切换展开折叠状态
	void SwitchExpandStat(int index);

	/// 取得可显示行数
	int GetCountPerPage2();

	/// 列对齐方式转文本输出对齐
	DWORD ConverLvFmtToDtFmt(DWORD dwLvFmt);

	/// 重新设置是否最后一行的标记
	int ResetIsLastFlag(int index);

	/// 重新设置是否最后一行的标记
	void ResetIsLastFlag();

	BOOL IsResExit();

	//根据资源ID获取CxImage
	CxImage* GetPng(LPCTSTR lpszPngResource);

	/// 取得当前页的首尾行
	BOOL GetPageRange(int & iFrom, int & iEnd);

	/// 删除全部按钮
	void DeleteAllButtons();

	/// 取得行背景色
	COLORREF GetItemBackColor(int index);

	/// 画所有条目
	void DrawItems(CDC & dcMem, const CRect & rectClip, const CRect & rectClient);

	/// 画封面信息
	void DrawCoverInfo(CDC & dc, CRect & rect, const CRssFeed & feed);

	/// 画封面列
	void DrawCoverColumnHead(BOOL bExpand);

	/// 当前封面是否展开
	BOOL IsCoverExpand() const;

	/// 切换封面列状态
	void SwitchCoverColumnState()
	{
		DrawCoverColumnHead(!IsCoverExpand());
	}

	/// 给订阅填充合适的行数以显示封面
	void FillBlankLine(int index);

	/// 给所有订阅填充合适的行数
	void FillAllFeedsBlankLine();

	/// 删除指定订阅行的空行
	void DeleteBlankLine(int index);

	/// 删除所有订阅空行
	void DeleteAllBlankLine();

	/// 将项目排序
	void DoSort(int iColIndex, BOOL bSortAscending);

	/// 取得封面完整区域
	void GetCoverRect(CRect & rect);

	/// 命中封面区域
	BOOL IsHitCover(const CPoint & pt);
	

private:
	UINT					m_ButtonID; 
	//CImageList				m_ilsItemImage;			///< Feed项目图像列表
	CxImage*				m_imageRSS_Normal;
	CxImage*				m_imageRSS_Disable;
	CxImage*				m_imageRSS_Downloading;
	CxImage*				m_imageRSS_Error;
	CxImage*				m_imageRSS_Update;
	CxImage*				m_imageTask_Complete;
	CxImage*				m_imageTask_Pause;
	CxImage*				m_imageTask_Error;
	CxImage*				m_imageTask_Stop;
	CxImage*				m_imageTask_Downloading1;
	CxImage*				m_imageTask_Downloading2;
	CxImage*				m_imageTask_Downloading3;

	CxImage*				m_imageRss_PosterBg;
	CxImage*				m_imageRss_PosterDefault;

	CxImage*				m_imageSubTitle_Green;
	CxImage*				m_imageSubTitle_Grey;
	UINT					m_nIcoState;
	ItemButtons				m_DownloadButtons;

	CFont                   m_FontBold, *m_pOldFont;

	CMenuXP*				m_pMenuXP;

	BOOL					m_bCanModifyCoverColumn;
	BOOL					m_bSingleStyle;//单个详情Style

	StringMatcher			m_stringMatcher; // 字符串匹配筛选

private:
	/// feed显示文本
	static CString GetFeedText(UINT uColIndex, const CRssFeed & feed);

	/// feed项目显示文本
	static CString GetFeedItemText(UINT uColIndex, const CRssFeed::Item & item);

	// feed项目显示图标索引
	//static UINT GetFeedItemImageIndex(const CRssFeed::Item & item);

	/// feed状态描述
	static CString GetFeedStatStr(const CRssFeed & feed);

	/// feed item状态描述
	static const CString & GetFeedItemStatStr(CRssFeed::EItemState state);

	/// 比较两行
	static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

	/// 取得行背景色
	static COLORREF GetLineBackColor(BOOL bIsEven);

private:
	//static CString s_strFeedStatStr[CRssFeed::fsMaxState];
	static CString s_strFeedItemStatStr[CRssFeed::isMaxState];

protected:
	DECLARE_MESSAGE_MAP()

public:
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnLvnDeleteitem(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemActivate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnDestroy();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnEndScroll(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnHdnBegintrack(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHdnItemchanging(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);

protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};


