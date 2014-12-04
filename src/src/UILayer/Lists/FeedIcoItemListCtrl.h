#pragma once
#include "MuleListCtrl.h"
#include "WorkLayer\rss\RssFeed.h"
#include "TitleMenu.h"
#include "MatchString.h"

// CFeedIcoItemListCtrl
class CxImage;
class CMenuXP;
//typedef CListCtrl CBaseListCtrl;

class CFeedIcoItemListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CFeedIcoItemListCtrl)

public:
	CFeedIcoItemListCtrl();
	virtual ~CFeedIcoItemListCtrl();

	LRESULT InsertTypeGroup(int nItem, int nGroupID, const CString& strHeader, DWORD dwState = LVGS_NORMAL, DWORD dwAlign = LVGA_HEADER_LEFT);

	//对单个FEED进行分组
	BOOL GroupFeedByType(int nIndex, int nType);

	//对所有FEED进行分组
	BOOL GroupAllFeedsByType(int nType);//Type暂时还没用到
	BOOL SetItemGroupId(int nItem, int nGroupID);

	int AddFeed(CRssFeed & feed, BOOL bIsNew);
	void RemoveFeed(CRssFeed & feed);

	//删除某个item在groupMap中的数据, bUpdate标识 是否 更新到新的分类信息, 删除IDS_DEFAULT分组中的数据
	void RemoveItemforMap(CRssFeed & feed, int nIndex, bool bUpdate = false);

	BOOL UpdatePoster(CRssFeed* feed);

	int GetItemIndex(CRssFeed* feed);//根据rss data找到相关item index

	/// 更新toolbar按钮状态
	void UpdateToolBarState(CToolBarCtrl & toolbar);

	/// 取得当前选择的index
	int GetCurrSelectIndex();

	/// 取得feed
	CRssFeed * GetFeed(int iItemIndex);

	//切割字符串适应2行
	CString GetExtentString(const CString & strCaption);

	//根据Group名称取得Group Num
	int GetGroupCounts(CString strGroupCaption);

	void Init();

	// 字符串匹配筛选
	void FilterItemsBySearchKey(CString* searchKey);

protected:
	DECLARE_MESSAGE_MAP()
	virtual void PreSubclassWindow();
	afx_msg void OnDestroy();

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();

	afx_msg void OnNMDblclkFeedIcoItemListCtrl(NMHDR *pNMHDR, LRESULT *pResult);

private:

	//根据资源ID获取CxImage
	CxImage* GetPng(LPCTSTR lpszPngResource);

	//加封面图片到imgList
	int	AddImg2List(CxImage* img);

	//设置Item的封面图片在imgList中的ID号
	BOOL SetItemPoster(int nItem, int nImageListId);
	
	//获得当前Group数目,暂时只分组第一级分类
	int GetGroupCount();

	//删除列表中自己保存的分组数据
	void RemoveAllGroupData();

	///只限VISTA+系统使用
	//group 是否可以展开
	BOOL IsGroupStateEnabled();
	//获得group rect 
	CRect GetRectbyGroupID(int nGroupID, UINT mask);
	//根据group ID获得group具体信息
	LVGROUP GetGroupInfoByIndex(int iGroupId, UINT mask);
	//获得Group属性
	BOOL HasGroupState(int nGroupId, DWORD dwState);
	///只限VISTA+系统使用

	//获取group标题
	CString GetGroupCaption(int nGroupId);

	////设置Group注脚
	//BOOL SetGroupFooter(int nGroupId, CString & strFooter);

	////获取Group注脚
	//CString GetGroupFooter(int nGroupId);

	//获取一个item的group ID
	int GetGroupIDByItemIndex(int nIndex);

	//获取group详细信息,交给DrawGroup具体绘制
	void DrawGroupInfo(CDC & dcMem, const CRect & rectClip, const CRect & rectClient);
	//具体绘制group
	void DrawGroup(CDC & dcMem, const CRect & rectGroup, const CString & strGroupCaption, const CString & strGroupTotals);
	//绘制items
	void DrawItems(CDC & dcMem, const CRect & rectClip, const CRect & rectClient);

	//设置Group高度
	void SetGroupHeight(int nHeight);

	/// 更新toolbar按钮状态
	void UpdateMenuState(CTitleMenu	& menuRSS);
	
	//XP系统下group高度
	int nGroupHeight;

	int m_nLastHotItemIndex; // Save the last hot item index

private:
	CImageList				m_PosterImageList;

	CxImage*				m_imageRss_PosterBg;//封面背景图
	CxImage*				m_imageRss_PosterSelBg;//封面选中状态背景图
	CxImage*				m_imageRss_GroupBg;//Rss Group区域背景
	CxImage*				m_imageRss_PosterDefault;//默认封面图

	CxImage*				m_imageRss_Group_Drop_Up_Nor;//Vista WIN7 下拉按钮
	CxImage*				m_imageRss_Group_Drop_Up_Hov;
	CxImage*				m_imageRss_Group_Drop_Down_Nor;
	CxImage*				m_imageRss_Group_Drop_Down_Hov;

	CFont                 m_FontBold, *m_pOldFont;

	CMenuXP*				m_pMenuXP;

	//Group Name | Group 中所有items集合
	CSimpleMap< CString, CSimpleArray<int> > m_mapGroups;

	// 字符串匹配筛选
	StringMatcher m_stringMatcher;

public:
	afx_msg void OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult);
protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnLvnHotTrack(NMHDR *pNMHDR, LRESULT *pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};


