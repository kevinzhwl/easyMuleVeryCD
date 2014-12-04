/*
 * $Id: RssDao.h 20462 2010-08-26 09:18:05Z yaveymao $
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

class CDatabase;
class CRssFilterList;

#include "rss/RssFeed.h"


/**
 * @brief 用于保存和读取订阅配置数据
 */
class CRssDao
{
public:
	CRssDao(void);
	virtual ~CRssDao(void);

public:
	/// 打开数据库
	BOOL Open(const CString & strFilename);

	/// 是否已经打开
	BOOL IsOpen() const
	{
		return m_db != NULL;
	}

	/// 关闭数据库
	void Close();

	/// 加载Rss Feed
	void LoadRssFeed(CRssFeedList & listFeed);

	/// 加载FeedHistory
	void LoadFeedHistory(CRssFeedList & listHistory);

	/// 加载FeedFilter
	void LoadFeedFilter(CRssFilterList & listFilter, CRssFeedList & listFeed);

	/// 删除Rss Feed
	void DeleteRssFeed(const CString & strFeedUrl);

	/// 保存Rss Feed
	void SaveRssFeed(const CRssFeed & feed);

	/// 保存所有的Rss Feed
	void SaveRssFeeds(const CRssFeedList & listFeed);

	/// 保存历史项目
	void SaveHistoryFeedItem(const CRssFeed::Item & item);

	/// 保存feed下所有历史项目
	void SaveHistoryFeed(const CRssFeed & feed);

	/// 保存feed list下所有历史项目
	void SaveHistoryFeeds(const CRssFeedList & listHistory);

	/// 保存过滤器
	void SaveFilter(const CRssFilter & filter);

	/// 保存列表中所有过滤器
	void SaveFilters(const CRssFilterList & listFilter);

	/// 删除Rss Feed
	void DeleteFilter(const CString & strFilterName);

	/// 初始化Catalogs数据库
	void InitCatalogs();

	/// 加载全部catalog
	void LoadCatalogs(CFeedCatalogs & catalogs);

	/// 保存一个catalog
	void SaveCatalog(const FeedCatalog & catalog);

private:
	/// 执行sql
	void ExecSql(CONST CHAR * strSql);

	/// 开始事务
	void BeginTrans()
	{
		ExecSql("begin transaction");
	}

	/// 结束事务
	void EndTrans()
	{
		ExecSql("end transaction");
	}

	/// 产生insert catalog表的sql
	void GenerateInsertCatalogSql(CStringA & strSql, unsigned int uId, const CString & strName, unsigned int uParentId);

private:
	/// 将普通字符串转换为sql语句中的合法字符串
	static CStringA CStrToSqlStr(const CString & str);

private:
	CDatabase * m_db;
};
