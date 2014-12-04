/*
 * $Id: RssDao.cpp 20509 2010-09-02 08:22:40Z huhonggang $
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

#include "StdAfx.h"
#include ".\rssdao.h"
#include "Database.h"
#include "StringConversion.h"
#include "rss\RssFilter.h"
#include "Preferences.h"


CRssDao::CRssDao(void)
	:
	m_db(NULL)
{
}

CRssDao::~CRssDao(void)
{
	Close();
}

// 执行sql
void CRssDao::ExecSql(CONST CHAR * strSql)
{
	ASSERT(m_db != NULL);
	if ( m_db == NULL )
	{
		return;
	}
	BOOL bResult = m_db->Exec(strSql);
	ASSERT(bResult);
}

// 打开数据库
BOOL CRssDao::Open(const CString & strFilename)
{
	try
	{
		Close();
		m_db = new CDatabase(StrToUtf8(strFilename));
		if( !m_db->InitSucc() )
		{
			delete m_db;
			m_db = NULL;
		}

	}
	catch(...)
	{
		m_db = NULL;
	}

	if ( m_db == NULL )
	{
		return FALSE;
	}

	// 初始化成功，根据需要建表

	// RssCatalog
	ExecSql("CREATE TABLE IF NOT EXISTS RssCatalogs ("
					"Id INTEGER,"
					"Name TEXT,"
					"ParentId INTEGER,"
					"PRIMARY KEY(Id) ON CONFLICT REPLACE"
			")"
		);

	// RssFeed
	ExecSql("CREATE TABLE IF NOT EXISTS RssFeeds ("
					"Id INTEGER PRIMARY KEY AUTOINCREMENT,"
					"FeedUrl TEXT UNIQUE ON CONFLICT REPLACE,"
					"Alias TEXT,"
					"LastUpdate INTEGER,"
					"LastDownload INTEGER,"
					"LastPubDate INTEGER,"
					"DownloadedTime INTEGER,"
					"SaveDir TEXT,"
					"Title TEXT"
			")"
		   );

	m_db->Exec("ALTER TABLE RssFeeds ADD UpdateInterval INTEGER");
	m_db->Exec("ALTER TABLE RssFeeds ADD AutoDownload INTEGER");
	m_db->Exec("ALTER TABLE RssFeeds ADD Description TEXT");
	m_db->Exec("ALTER TABLE RssFeeds ADD CatalogId INTEGER");

	// FeedHistory
	ExecSql("CREATE TABLE IF NOT EXISTS HistoryFeedItems ("
					"FeedUrl TEXT,"
					"Guid TEXT,"
					"Title TEXT,"
					"PubDate INTEGER,"
					"Enclosure TEXT,"
					"PRIMARY KEY(FeedUrl, Guid) ON CONFLICT REPLACE"
			")"
		   );

	// Filter
	ExecSql("CREATE TABLE IF NOT EXISTS FeedFilters ("
					"Id INTEGER PRIMARY KEY AUTOINCREMENT,"
					"Name TEXT UNIQUE ON CONFLICT REPLACE,"
					"FilterStr TEXT,"
					"FilterNot TEXT,"
					"Quality TEXT,"
					"FileType TEXT,"
					"ApplyFeedUrl TEXT,"
					"MinSize INTEGER,"
					"MaxSize INTEGER,"
					"AutoStart INTEGER,"
					"Enable INTEGER"
		    ")"
		   );


	// 初始化类别表
	InitCatalogs();

	return TRUE;
}

// 关闭数据库
void CRssDao::Close()
{
	try 
	{
		delete m_db;
		m_db = NULL;
	}
	catch (...)
	{
	}
}

// 加载Rss Feed
void CRssDao::LoadRssFeed(CRssFeedList & listFeed)
{
	if ( !IsOpen() )
	{
		return;
	}

	ASSERT(m_db != NULL);
	CStringA strSql;
	strSql.Format("select "
				  "FeedUrl, Alias, LastUpdate, LastDownload, LastPubDate, DownloadedTime, SaveDir, Title, "
				  "UpdateInterval, AutoDownload, Description, ifnull(CatalogId, %d) CatalogId "
				  "from RssFeeds",
				  FeedCatalog::INVALID_CATALOG_ID
				 );
	BOOL bResult = m_db->Prepare(strSql);
	if ( bResult )
	{
		CString strDefaultSaveDir = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
		while( m_db->Step() )
		{
			CRssFeed & feed = listFeed.AddFeed(OptUtf8ToStr((LPCSTR)m_db->GetColText(0)));
			feed.m_strAlias = OptUtf8ToStr((LPCSTR)m_db->GetColText(1));
			feed.m_tmLastUpdate = (time_t)m_db->GetColInt64(2);
			feed.m_timeLastDownload = m_db->GetColInt64(3);
			feed.m_timeLastPub = m_db->GetColInt64(4);
			feed.m_timeDownloaded = m_db->GetColInt64(5);
			feed.m_strSaveDir = OptUtf8ToStr((LPCSTR)m_db->GetColText(6));
			if ( feed.m_strSaveDir.IsEmpty() )
			{
				feed.m_strSaveDir = strDefaultSaveDir;
			}
			feed.m_strTitle = OptUtf8ToStr((LPCSTR)m_db->GetColText(7));
			feed.SetUpdateInterval(UINT(m_db->GetColInt(8)));
			if ( feed.GetUpdateInterval() == 0 )
			{
				// 可能是使用了db最初版，导致新加的字段为0
				// 将新加的字段值设置为系统定义
				feed.SetUpdateInterval(CRssFeed::UNDEFINED_VALUE);
				feed.SetAutoDownload(CRssFeed::UNDEFINED_VALUE);
			}
			else
			{
				feed.SetAutoDownload(UINT(m_db->GetColInt(9)));
			}
			feed.m_strDescription = OptUtf8ToStr((LPCSTR)m_db->GetColText(10));
			
			// 类别对应关系
			feed.m_uCatalogId = UINT(m_db->GetColInt(11));
			if ( feed.m_uCatalogId != FeedCatalog::INVALID_CATALOG_ID )
			{
				FeedCatalog & catalog = CFeedCatalogs::GetInstance()[feed.m_uCatalogId];
				ASSERT(catalog.m_uId == feed.m_uCatalogId);
				ASSERT(catalog.IsLeaf());
				catalog.m_setFeeds.insert(&feed);
			}
		}
		m_db->Finalize();
	}
}

// 删除Rss Feed
void CRssDao::DeleteRssFeed(const CString & strFeedUrl)
{
	if ( !IsOpen() )
	{
		return;
	}

	ASSERT(m_db != NULL);

	CStringA strSql;
	strSql.Format("delete from RssFeeds where FeedUrl = '%s'",
				  static_cast<const char *>(CStrToSqlStr(strFeedUrl))
				 );
	ExecSql(strSql);
}

// 保存Rss Feed
void CRssDao::SaveRssFeed(const CRssFeed & feed)
{
	if ( !IsOpen() )
	{
		return;
	}
	ASSERT(m_db != NULL);

	CStringA strSql;
	strSql.Format("insert into RssFeeds (FeedUrl, Alias, LastUpdate, LastDownload, LastPubDate, DownloadedTime, SaveDir, Title, "
										"UpdateInterval, AutoDownload, Description, CatalogId) "
					"values ('%s', '%s', %I64u, %I64u, %I64u, %I64u, '%s', '%s', %u, %u, '%s', %d)",
				  static_cast<const char *>(CStrToSqlStr(feed.m_strFeedUrl)),
				  static_cast<const char *>(CStrToSqlStr(feed.m_strAlias)),
				  static_cast<UINT64>(feed.m_tmLastUpdate),
				  static_cast<UINT64>(feed.m_timeLastDownload.GetTime()),
				  static_cast<UINT64>(feed.m_timeLastPub.GetTime()),
				  static_cast<UINT64>(feed.m_timeDownloaded.GetTime()),
				  static_cast<const char *>(CStrToSqlStr(feed.m_strSaveDir)),
				  static_cast<const char *>(CStrToSqlStr(feed.m_strTitle)),
				  static_cast<UINT>(feed.GetUpdateInterval()),
				  static_cast<UINT>(feed.GetAutoDownload()),
				  static_cast<const char *>(CStrToSqlStr(feed.m_strDescription)),
				  static_cast<int>(feed.m_uCatalogId)
				 );
	ExecSql(strSql);
}

// 保存所有的Rss Feed
void CRssDao::SaveRssFeeds(const CRssFeedList & listFeed)
{
	if ( !IsOpen() )
	{
		return;
	}

	BeginTrans();

	for ( CRssFeedList::const_iterator it = listFeed.begin();
		  it != listFeed.end();
		  ++it
		)
	{
		SaveRssFeed(it->second);
	}

	EndTrans();
}

/// 产生insert catalog表的sql
void CRssDao::GenerateInsertCatalogSql(CStringA & strSql, unsigned int uId, const CString & strName, unsigned int uParentId)
{
	strSql.Format("insert into RssCatalogs(Id, Name, ParentId) "
				  "values (%d, '%s', %d)",
				  static_cast<int>(uId),
				  static_cast<const char *>(CStrToSqlStr(strName)),
				  static_cast<int>(uParentId)
				 );
}

/// 初始化Catalogs数据库
void CRssDao::InitCatalogs()
{
	// 初始化数据随版本绑定

	if ( !IsOpen() )
	{
		return;
	}

	ASSERT(m_db != NULL);

	int iRecc = 0;
	BOOL bResult = m_db->Prepare("select count(*) recc from RssCatalogs");
	if ( bResult )
	{
		if ( m_db->Step() )
		{
			iRecc = m_db->GetColInt(0);
			m_db->Finalize();
		}
	}

	if ( iRecc == 0 )
	{
		// 初始化数据块
		struct CATALOG_INIT_DATA
		{
			unsigned int	m_uId;
			const char *	m_lpszName;
			unsigned int	m_uParentId;

			CATALOG_INIT_DATA(unsigned int uId, const char * lpszName, unsigned int	uParentId)
				:
				m_uId(uId),
				m_lpszName(lpszName),
				m_uParentId(uParentId)
			{
			}
		};

		CATALOG_INIT_DATA initData[] = 
			{
				// 电影
				CATALOG_INIT_DATA(2,  "电影", FeedCatalog::INVALID_CATALOG_ID),
				CATALOG_INIT_DATA(3,  "动作", 2),
				CATALOG_INIT_DATA(4,  "喜剧", 2),
				CATALOG_INIT_DATA(5,  "爱情", 2),
				CATALOG_INIT_DATA(6,  "文艺", 2),
				CATALOG_INIT_DATA(7,  "剧情", 2),
				CATALOG_INIT_DATA(8,  "科幻", 2),
				CATALOG_INIT_DATA(9,  "魔幻", 2),
				CATALOG_INIT_DATA(10, "悬疑", 2),
				CATALOG_INIT_DATA(11, "惊悚", 2),
				CATALOG_INIT_DATA(12, "恐怖", 2),
				CATALOG_INIT_DATA(13, "罪案", 2),
				CATALOG_INIT_DATA(14, "战争", 2),
				CATALOG_INIT_DATA(15, "纪录", 2),
				CATALOG_INIT_DATA(16, "动画", 2),
				CATALOG_INIT_DATA(17, "音乐歌舞", 2),
				CATALOG_INIT_DATA(76, "电影周边", 2),

				// 剧集
				CATALOG_INIT_DATA(18, "剧集", FeedCatalog::INVALID_CATALOG_ID),
				CATALOG_INIT_DATA(19, "欧美", 18),
				CATALOG_INIT_DATA(20, "港台", 18),
				CATALOG_INIT_DATA(21, "大陆", 18),
				CATALOG_INIT_DATA(22, "日剧", 18),
				CATALOG_INIT_DATA(23, "韩剧", 18),

				// 音乐
				CATALOG_INIT_DATA(24, "音乐", FeedCatalog::INVALID_CATALOG_ID),
				CATALOG_INIT_DATA(25, "华语音乐", 24),
				CATALOG_INIT_DATA(26, "欧美音乐", 24),
				CATALOG_INIT_DATA(27, "日韩音乐", 24),
				CATALOG_INIT_DATA(28, "MV", 24),
				CATALOG_INIT_DATA(29, "演唱会", 24),
				CATALOG_INIT_DATA(30, "原声音乐", 24),
				CATALOG_INIT_DATA(31, "古典音乐", 24),
				CATALOG_INIT_DATA(32, "新世纪音乐", 24),
				CATALOG_INIT_DATA(33, "其它音乐", 24),

				// 游戏
				CATALOG_INIT_DATA(34, "游戏", FeedCatalog::INVALID_CATALOG_ID),
				CATALOG_INIT_DATA(35, "光盘版游戏", 34),
				CATALOG_INIT_DATA(36, "硬盘版游戏", 34),
				CATALOG_INIT_DATA(37, "电视游戏", 34),
				CATALOG_INIT_DATA(38, "掌机游戏", 34),
				CATALOG_INIT_DATA(39, "网络游戏", 34),
				CATALOG_INIT_DATA(40, "游戏周边", 34),

				// 动漫
				CATALOG_INIT_DATA(41, "动漫", FeedCatalog::INVALID_CATALOG_ID),
				CATALOG_INIT_DATA(42, "电视动画", 41),
				CATALOG_INIT_DATA(43, "剧场动画", 41),
				CATALOG_INIT_DATA(44, "OVA", 41),
				CATALOG_INIT_DATA(45, "漫画", 41),
				CATALOG_INIT_DATA(78, "原创动漫", 41),
				CATALOG_INIT_DATA(46, "动漫周边", 41),

				// 综艺
				CATALOG_INIT_DATA(47, "综艺", FeedCatalog::INVALID_CATALOG_ID),
				CATALOG_INIT_DATA(48, "综艺娱乐", 47),
				CATALOG_INIT_DATA(49, "艺人合集", 47),
				CATALOG_INIT_DATA(50, "体育节目", 47),
				CATALOG_INIT_DATA(51, "新闻综合", 47),
				CATALOG_INIT_DATA(52, "晚会典礼", 47),
				CATALOG_INIT_DATA(53, "科教节目", 47),
				CATALOG_INIT_DATA(54, "纪录片", 47),

				// 软件
				CATALOG_INIT_DATA(56, "软件", FeedCatalog::INVALID_CATALOG_ID),
				CATALOG_INIT_DATA(57, "操作系统", 56),
				CATALOG_INIT_DATA(58, "应用软件", 56),
				CATALOG_INIT_DATA(59, "网络软件", 56),
				CATALOG_INIT_DATA(60, "系统工具", 56),
				CATALOG_INIT_DATA(61, "多媒体类", 56),
				CATALOG_INIT_DATA(62, "行业软件", 56),
				CATALOG_INIT_DATA(63, "编程开发", 56),
				CATALOG_INIT_DATA(64, "安全相关", 56),

				// 资料
				CATALOG_INIT_DATA(65, "资料", FeedCatalog::INVALID_CATALOG_ID),
				CATALOG_INIT_DATA(69, "电子图书", 65),
				CATALOG_INIT_DATA(77, "素材", 65),
				CATALOG_INIT_DATA(70, "杂志期刊", 65),
				CATALOG_INIT_DATA(71, "有声读物", 65),
				CATALOG_INIT_DATA(75, "其它资料", 65),

				// 教育
				CATALOG_INIT_DATA(79, "教育", FeedCatalog::INVALID_CATALOG_ID),
				CATALOG_INIT_DATA(80, "人文社科", 79),
				CATALOG_INIT_DATA(68, "理工科", 79),
				CATALOG_INIT_DATA(74, "艺术体育", 79),
				CATALOG_INIT_DATA(81, "医学", 79),
				CATALOG_INIT_DATA(72, "商学", 79),
				CATALOG_INIT_DATA(66, "计算机", 79),
				CATALOG_INIT_DATA(67, "外语", 79),
				CATALOG_INIT_DATA(73, "其它", 79)

			};

		BeginTrans();

		CStringA strSql;
		for (int i = 0; i < sizeof(initData) / sizeof(initData[0]); ++i)
		{		
			GenerateInsertCatalogSql(strSql, initData[i].m_uId, CString(initData[i].m_lpszName), initData[i].m_uParentId);
			ExecSql(strSql);
		}

		EndTrans();
	}
}

/// 加载全部catalog
void CRssDao::LoadCatalogs(CFeedCatalogs & catalogs)
{
	if ( !IsOpen() )
	{
		return;
	}

	ASSERT(m_db != NULL);

	BOOL bResult = m_db->Prepare("select "
								 "Id, Name, ParentId  "
								 "from RssCatalogs order by ParentId, Id"
								);
	if ( bResult )
	{
		catalogs.clear();
		while( m_db->Step() )
		{
			unsigned int uId = UINT(m_db->GetColInt(0));
			FeedCatalog & catalog = catalogs[uId];
			catalog.m_uId = uId;
			catalog.m_strName = OptUtf8ToStr((LPCSTR)m_db->GetColText(1));
			
			// 对应父子关系
			unsigned int uParentId = UINT(m_db->GetColInt(2));
			if ( uParentId != FeedCatalog::INVALID_CATALOG_ID )
			{
				FeedCatalog & parentCatalog = catalogs[uParentId];
				ASSERT(parentCatalog.m_uId == uParentId);
				catalog.m_pParent = &parentCatalog;
				parentCatalog.m_setChilds.insert(&catalog);
			}
			else
			{
				catalog.m_pParent = NULL;
			}
		}
		m_db->Finalize();
	}

}


/// 保存一个catalog
void CRssDao::SaveCatalog(const FeedCatalog & catalog)
{
	if ( !IsOpen() )
	{
		return;
	}
	ASSERT(m_db != NULL);

	CStringA strSql;
	GenerateInsertCatalogSql(strSql, 
							 catalog.m_uId, 
							 catalog.m_strName, 
							 catalog.m_pParent ? catalog.m_pParent->m_uId : FeedCatalog::INVALID_CATALOG_ID
							);
	ExecSql(strSql);
}


// 加载FeedHistory
void CRssDao::LoadFeedHistory(CRssFeedList & listHistory)
{
	if ( !IsOpen() )
	{
		return;
	}

	ASSERT(m_db != NULL);
	BOOL bResult = m_db->Prepare("select "
									"FeedUrl, Guid, Title, PubDate, Enclosure "
								 "from HistoryFeedItems"
								);
	if ( bResult )
	{
		while( m_db->Step() )
		{
			CRssFeed & rssFeedHistory = listHistory.AddFeed(OptUtf8ToStr((LPCSTR)m_db->GetColText(0)));
			CRssFeed::Item & item = rssFeedHistory.AddFeedItem(OptUtf8ToStr((LPCSTR)m_db->GetColText(1)));
			item.m_bIsHistory = true;
			//item.m_strGuid = strGuid;
			item.m_strTitle = OptUtf8ToStr((LPCSTR)m_db->GetColText(2));
			item.m_timePubDate = m_db->GetColInt64(3);
			item.m_strEnclosure = OptUtf8ToStr((LPCSTR)m_db->GetColText(4));
		}
		m_db->Finalize();
	}

}

// 保存历史项目
void CRssDao::SaveHistoryFeedItem(const CRssFeed::Item & item)
{
	if ( !IsOpen() )
	{
		return;
	}
	ASSERT(m_db != NULL);
	ASSERT(item.m_pFeed != NULL);
	if ( item.m_pFeed == NULL )
	{
		return;
	}

	CStringA strSql;
	strSql.Format("insert into HistoryFeedItems (FeedUrl, Guid, Title, PubDate, Enclosure) "
					"values ('%s', '%s', '%s', %I64u, '%s')",
				  static_cast<const char *>(CStrToSqlStr(item.m_pFeed->m_strFeedUrl)),
				  static_cast<const char *>(CStrToSqlStr(item.m_strGuid)),
				  static_cast<const char *>(CStrToSqlStr(item.m_strTitle)),
				  static_cast<UINT64>(item.m_timePubDate.GetTime()),
				  static_cast<const char *>(CStrToSqlStr(item.m_strEnclosure))
				 );
	ExecSql(strSql);
}

// 保存feed下所有历史项目
void CRssDao::SaveHistoryFeed(const CRssFeed & feed)
{
	if ( !IsOpen() )
	{
		return;
	}

	for ( CRssFeed::ItemConstIterator it = feed.GetItemIterBegin();
		  it != feed.GetItemIterEnd();
		  ++it
		)
	{
		const CRssFeed::Item & item = it->second;
		if ( item.m_bIsHistory )
		{
			SaveHistoryFeedItem(item);
		}
	}
}

// 保存feed list下所有历史项目
void CRssDao::SaveHistoryFeeds(const CRssFeedList & listHistory)
{
	if ( !IsOpen() )
	{
		return;
	}
	
	BeginTrans();

	for ( CRssFeedList::const_iterator it = listHistory.begin();
		  it != listHistory.end();
		  ++it
		)
	{
		SaveHistoryFeed(it->second);
	}

	EndTrans();
}

// 加载FeedFilter
void CRssDao::LoadFeedFilter(CRssFilterList & listFilter, CRssFeedList & listFeed)
{
	if ( !IsOpen() )
	{
		return;
	}

	ASSERT(m_db != NULL);
	BOOL bResult = m_db->Prepare("select "
									"Name, FilterStr, FilterNot, Quality, FileType, ApplyFeedUrl, MinSize, MaxSize, AutoStart, Enable "
								 "from FeedFilters"
								);
	if ( bResult )
	{
		while( m_db->Step() )
		{
			CRssFilter & filter = listFilter.AddFilter(OptUtf8ToStr((LPCSTR)m_db->GetColText(0)));
			filter.m_strFilter = OptUtf8ToStr((LPCSTR)m_db->GetColText(1));
			filter.m_strNot = OptUtf8ToStr((LPCSTR)m_db->GetColText(2));
			filter.m_strQuality = OptUtf8ToStr((LPCSTR)m_db->GetColText(3));
			filter.m_strFileType = OptUtf8ToStr((LPCSTR)m_db->GetColText(4));
			if ( CRssFeed * pFeed = listFeed.FindFeed(OptUtf8ToStr((LPCSTR)m_db->GetColText(5))) )
			{ 
				filter.m_pApplyFeed = pFeed;
				pFeed->m_pFilter = &filter;
			}
			filter.m_uMinSizeKB = m_db->GetColInt(6);
			filter.m_uMaxSizeKB = m_db->GetColInt(7);
			filter.m_bAutoStart = (m_db->GetColInt(8) != 0);
			filter.m_bEnable = (m_db->GetColInt(9) != 0);
		}
		m_db->Finalize();
	}
}

// 删除Rss Feed
void CRssDao::DeleteFilter(const CString & strFilterName)
{
	if ( !IsOpen() )
	{
		return;
	}

	ASSERT(m_db != NULL);

	CStringA strSql;
	strSql.Format("delete from FeedFilters where Name = '%s'",
				  static_cast<const char *>(CStrToSqlStr(strFilterName))
				 );
	ExecSql(strSql);
}

// 保存过滤器
void CRssDao::SaveFilter(const CRssFilter & filter)
{
	if ( !IsOpen() )
	{
		return;
	}

	ASSERT(m_db != NULL);

	CStringA strSql;
	strSql.Format("insert into FeedFilters (Name, FilterStr, FilterNot, Quality, FileType, ApplyFeedUrl, MinSize, MaxSize, AutoStart, Enable ) "
					"values ('%s', '%s', '%s', '%s', '%s', '%s', %I64u, %I64u, %d, %d)",
				  static_cast<const char *>(CStrToSqlStr(filter.m_strName)),
				  static_cast<const char *>(CStrToSqlStr(filter.m_strFilter)),
				  static_cast<const char *>(CStrToSqlStr(filter.m_strNot)),
				  static_cast<const char *>(CStrToSqlStr(filter.m_strQuality)),
				  static_cast<const char *>(CStrToSqlStr(filter.m_strFileType)),
				  static_cast<const char *>(CStrToSqlStr(filter.m_pApplyFeed ? filter.m_pApplyFeed->m_strFeedUrl : _T(""))),
				  static_cast<UINT64>(filter.m_uMinSizeKB),
				  static_cast<UINT64>(filter.m_uMaxSizeKB),
				  static_cast<int>(filter.m_bAutoStart),
				  static_cast<int>(filter.m_bEnable)
				 );
	ExecSql(strSql);
}

// 保存列表中所有过滤器
void CRssDao::SaveFilters(const CRssFilterList & listFilter)
{
	if ( !IsOpen() )
	{
		return;
	}

	BeginTrans();

	for ( CRssFilterList::const_iterator it = listFilter.begin();
		  it != listFilter.end();
		  ++it
		)
	{
		SaveFilter(it->second);
	}

	EndTrans();
}

// 将普通字符串转换为sql语句中的合法字符串
CStringA CRssDao::CStrToSqlStr(const CString & str)
{
	if ( str.IsEmpty() )
	{
		return CStringA();
	}

	CStringA strResult = StrToUtf8(str);
	strResult.Replace("'", "''");
	return strResult;
}

