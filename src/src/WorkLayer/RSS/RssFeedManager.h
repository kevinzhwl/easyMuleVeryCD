/*
 * $Id: RssFeedManager.h 20761 2010-11-09 01:54:12Z gaoyuan $
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

#include "RssFeed.h"
#include "RssFilter.h"
#include "RssFeedUpdate.h"
#include "RssDao.h"


/**
 * @brief 管理和维护所有订阅及其相关配置数据
 */
class CRssFeedManager
{
public:
	/// 构造函数
	CRssFeedManager(void);

	/// 析构函数
	virtual ~CRssFeedManager(void);

public:
	/// 从配置文件中创建RssFeed和历史信息
	void LoadConfig();

	/// 保存到配置文件中
	//void SaveConfig(bool bSaveAlways);

	//// 返回是否更新
	//bool IsModified() const
	//{
	//	return m_listFeed.m_bModified || m_listFeedHistory.m_bModified || m_listFilter.m_bModified;
	//}

	//// 重置更新标志
	//void ResetModified()
	//{
	//	m_listFeed.m_bModified = m_listFeedHistory.m_bModified = m_listFilter.m_bModified = false;
	//}

	/// 增加一个feed
	CRssFeed & AddFeed(const CString & strFeedUrl, const CString & strSaveDir);

	/// 查找一个feed
	CRssFeed * FindFeed(const CString & strFeedUrl)
	{
		return m_listFeed.FindFeed(strFeedUrl);
	}

	// 删除一个feed
	//void DeleteFeed(const CString & strFeedUrl);

	/// 删除一个feed
	void DeleteFeed(CRssFeed & feed);

	/// 增加一个过滤器
	CRssFilter & AddFilter(const CRssFilter & filter);

	/// 删除一个过滤器
	void DeleteFilter(CRssFilter & filter);

	// 标记项目为历史
	//CRssFeed::Item * MarkHistory(const CString & strGuid);

	/// 标记项目为历史
	CRssFeed::Item & MarkHistory(CRssFeed::Item & item);

	// 删除一个历史项目
	//void DeleteHistory(const CString & strGuid);

	// 刷新所有feed
	//void RefreshFeed();

	/// 启动更新feed线程
	void StartUpdateFeed(CRssFeed & feed, IRssFeedUpdateNotify & notify, BOOL bCreateNewThread = TRUE)
	{
		feed.m_iState |= CRssFeed::fsRefresh;
		feed.m_bLastUpdateSucc = false;
		m_FeedUpdate.StartUpdateFeed(feed.m_strFeedUrl, notify, bCreateNewThread);
	}

	/// 启动过滤器，从feed中筛选资源
	void StartFilter(CRssFeed & feed, IRetrieveRssFilterResult & result)
	{
		m_listFilter.StartFilter(feed, result);
	}

	/// 取得feed列表
	CRssFeedList & GetFeedList()
	{
		return m_listFeed;
	}

	/// 设置feed新的已下载时间（处理最新已完成数）
	void SetNewDownloadedTime(CRssFeed & feed)
	{
		CTime timeDownloaded = feed.m_timeDownloaded;
		feed.SetNewDownloadedTime();
		if ( timeDownloaded != feed.m_timeDownloaded )
		{
			m_dao.SaveRssFeed(feed);
		}
	}

	// 取得feed列表
	CRssFeedList & GetHistoryFeedList()
	{
		return m_listFeedHistory;
	}

	// 取得过滤去列表
	//CRssFilterList & GetFilterList()
	//{
	//	return m_listFilter;
	//}

	/// 保存feed
	void SaveFeed(const CRssFeed & feed)
	{
		m_dao.SaveRssFeed(feed);
	}

public:
	/// 处理feed更新完成消息，返回一个可被自动下载的feed
	afx_msg LRESULT OnFeedUpdateMsg(const CString & strUrl, const char * lpszStream);

private:
	/// 设置历史别名
	void SetHistoryAlias(CRssFeedBase & feedHistory, const CRssFeedBase & feedNormal);

	/// 设置历史别名
	void SetHistoryAlias(CRssFeedBase & feedHistory);

	/// 刷新feed
	void RefreshFeed(const CString & strFeedUrl, const char * lpszUrlStream)
	{
		if ( CRssFeed * feed =  m_listFeed.FindFeed(strFeedUrl) )
		{
			RefreshFeed(*feed, lpszUrlStream);
		}
	}

	/// 刷新feed
	bool RefreshFeed(CRssFeed & feed, const char * lpszUrlStream);

	/// 从ini配置文件中创建RssFeed和历史信息
	void LoadIniConfig(const CString & strConfigFile);

	/// 保存到ini配置文件中
	//void SaveIniConfig(bool bSaveAlways);

private:
	CRssFeedList	m_listFeed;				///< rss feed列表
	CRssFeedList	m_listFeedHistory;		///< rss feed历史下载列表
	CRssFilterList	m_listFilter;			///< feed过滤器
	CRssFeedUpdate	m_FeedUpdate;			///< 更新feed
	CRssDao			m_dao;					///< Rss数据访问对象

private:
	/// 取得ini配置文件名字
	static CString GetIniConfigFilename();

	/// 取得db配置文件名字
	static CString GetDbConfigFilename();

public:
#if defined(_DEBUG)
	static void test();
#endif

};
