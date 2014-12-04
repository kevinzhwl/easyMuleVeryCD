/*
 * $Id: RssFeedManager.cpp 20462 2010-08-26 09:18:05Z yaveymao $
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

#include "StdAfx.h"
#include "RssfeedManager.h"
#include "Preferences.h"
#include "ini2.h"


#define  RSS_INI_CONFIG_NAME	_T("rss.ini")
#define  RSS_DB_CONFIG_NAME		_T("rss.db")

#define  RSS_FEED_SECTION		_T("RssFeed")
#define  RSS_FEED_URL_ITEM		_T("FeedUrl")
#define  RSS_ALIAS_ITEM			_T("Alias")
#define  RSS_LAST_UPDATE		_T("LastUpdate")
#define  RSS_LAST_DOWNLOAD		_T("LastDownload")
#define  RSS_LAST_PUB_DATE		_T("LastPubDate")
#define  RSS_SAVE_DIR			_T("SaveDir")
#define  RSS_HAS_COMPLETE_FILE	_T("HasComplete")
#define  RSS_DOWNLOADED_TIME	_T("DownloadedTime")

#define  RSS_HISTORY_SECTION	_T("FeedHistory")
//#define  RSS_FEED_URL_ITEM		_T("FeedUrl")
//#define  RSS_ALIAS_ITEM			_T("Alias")
#define  RSS_GUID_ITEM			_T("Guid")
#define  RSS_TITLE_ITEM			_T("Title")
#define  RSS_PUBDATE_ITEM		_T("PubDate")
#define  RSS_ENCLOSURE_ITEM		_T("Enclosure")

#define  RSS_FILTER_SECTION		_T("FeedFilter")
#define	 RSS_FILTER_NAME		_T("Name")
#define  RSS_FILTER_STR			_T("FilterStr")
#define  RSS_FILTER_NOT			_T("FilterNot")
#define  RSS_FILTER_SAVE_DIR	_T("SaveDir")
#define  RSS_FILTER_QUALITY		_T("Quality")
#define  RSS_FILTER_FILE_TYPE	_T("FileType")
#define  RSS_FILTER_APPLY_FEED	_T("ApplyFeed")
#define  RSS_FILTER_MIN_SIZE	_T("MinSize")
#define  RSS_FILTER_MAX_SIZE	_T("MaxSize")
#define  RSS_FILTER_AUTO_START	_T("AutoStart")
#define  RSS_FILTER_ENABLE		_T("Enable")

#define  MAX_ITEM_COUNT			256
#define  MAX_SECTION_COUNT		256

CRssFeedManager::CRssFeedManager(void)
{
}

CRssFeedManager::~CRssFeedManager(void)
{
	m_dao.Close();
	//SaveConfig(true);
}

// 取得ini配置文件名字
CString CRssFeedManager::GetIniConfigFilename()
{
	CString str;
	str.Format(_T("%s") RSS_INI_CONFIG_NAME, thePrefs.GetMuleDirectory(EMULE_CONFIGDIR));
	return str;
}

// 取得db配置文件名字
CString CRssFeedManager::GetDbConfigFilename()
{
	CString str;
	str.Format(_T("%s") RSS_DB_CONFIG_NAME, thePrefs.GetMuleDirectory(EMULE_CONFIGDIR));
	return str;
}

// 设置历史别名
void CRssFeedManager::SetHistoryAlias(CRssFeedBase & feedHistory)
{
	if ( const CRssFeed * feed = m_listFeed.FindFeed(feedHistory.m_strFeedUrl) )
	{
		SetHistoryAlias(feedHistory, *feed);
	}
}

// 设置历史别名
void CRssFeedManager::SetHistoryAlias(CRssFeedBase & feedHistory, const CRssFeedBase & feedNormal)
{
	if ( feedHistory.m_strAlias.IsEmpty() && !feedNormal.m_strAlias.IsEmpty() )
	{
		feedHistory.m_strAlias = feedNormal.m_strAlias;
	}
}

// 从配置文件中创建RssFeed和历史信息
void CRssFeedManager::LoadConfig()
{
	m_listFeed.clear();
	m_listFeedHistory.clear();
	m_listFilter.clear();

	// 打开db
	BOOL bOpenDbResult = m_dao.Open(GetDbConfigFilename());
	ASSERT(bOpenDbResult);

	// 加载订阅类别信息
	m_dao.LoadCatalogs(CFeedCatalogs::GetInstance());

	// 如有ini，则先从ini中载入，再转存到db中
	CString strIniConfigFile = GetIniConfigFilename();
	if ( PathFileExists(strIniConfigFile) )
	{
		LoadIniConfig(strIniConfigFile);

		// 需要把ini中的数据转存为db
		m_dao.SaveRssFeeds(m_listFeed);
		m_dao.SaveHistoryFeeds(m_listFeedHistory);
		m_dao.SaveFilters(m_listFilter);

		// ini配置文件改名备份
		MoveFileEx(strIniConfigFile, strIniConfigFile + _T(".bak"), MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING);
	}

	// 从数据库中读取
	m_dao.LoadRssFeed(m_listFeed);
	m_dao.LoadFeedHistory(m_listFeedHistory);
	m_dao.LoadFeedFilter(m_listFilter, m_listFeed);

	// 添加一个默认全局过滤器
	//if ( m_listFilter.empty() )
	{
		CRssFilter filterDefault;
		filterDefault.m_strName = _T("DefaultFilter");
		CRssFilter & filterAdded = AddFilter(filterDefault);
		ASSERT(filterAdded.m_bEnable);
	}

	// 加载完毕，设置为未修改状态
	//ResetModified();
}

//// 保存到配置文件中
//void CRssFeedManager::SaveConfig(bool bSaveAlways)
//{
//	SaveIniConfig(bSaveAlways);
//}

// 从ini配置文件中创建RssFeed和历史信息
void CRssFeedManager::LoadIniConfig(const CString & strConfigFile)
{
	CIni ini(strConfigFile);
	CString strName;	
	CString strFeedUrl;
	CString strGuid;
	CString strFilterName;

	CString strDefaultSaveDir = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);

	// 读取rss url
	ini.SetSection(RSS_FEED_SECTION);
	for ( DWORD i = 0; i < MAX_ITEM_COUNT; ++i )
	{
		strName.Format(RSS_FEED_URL_ITEM _T("%u"), i);
		strFeedUrl =  ini.GetString(strName);
		if ( strFeedUrl.IsEmpty() )
		{
			break;
		}

		CRssFeed & feed = m_listFeed.AddFeed(strFeedUrl);

		strName.Format(RSS_ALIAS_ITEM _T("%u"), i);
		feed.m_strAlias = ini.GetString(strName);

		strName.Format(RSS_LAST_UPDATE _T("%u"), i);
		feed.m_tmLastUpdate = (time_t)ini.GetUInt64(strName);

		strName.Format(RSS_LAST_DOWNLOAD _T("%u"), i);
		feed.m_timeLastDownload = ini.GetUInt64(strName);

		strName.Format(RSS_LAST_PUB_DATE _T("%u"), i);
		feed.m_timeLastPub = ini.GetUInt64(strName);

		strName.Format(RSS_DOWNLOADED_TIME _T("%u"), i);
		feed.m_timeDownloaded = ini.GetUInt64(strName);

		strName.Format(RSS_SAVE_DIR _T("%u"), i);
		feed.m_strSaveDir = ini.GetStringUTF8(strName);
		if ( feed.m_strSaveDir.IsEmpty() )
		{
			feed.m_strSaveDir = strDefaultSaveDir;
		}
		//strName.Format(RSS_HAS_COMPLETE_FILE _T("%u"), i);
		//feed.m_bHasNewComplete = ini.GetBool(strName, false);
	}

	// 读取历史
	for ( DWORD uSection = 0; uSection < MAX_SECTION_COUNT; ++uSection )
	{
		strName.Format(RSS_HISTORY_SECTION _T("%u"), uSection);
		ini.SetSection(strName);

		strFeedUrl =  ini.GetString(RSS_FEED_URL_ITEM);
		if ( strFeedUrl.IsEmpty() )
		{
			break;
		}

		// 读取一个feed url下的所有历史项目
		CRssFeed & rssFeedHistory = m_listFeedHistory.AddFeed(strFeedUrl);
		rssFeedHistory.m_strAlias = ini.GetString(RSS_ALIAS_ITEM);
		if ( rssFeedHistory.m_strAlias.IsEmpty() )
		{
			SetHistoryAlias(rssFeedHistory);
		}

		for ( DWORD uItem = 0; uItem < MAX_ITEM_COUNT; ++uItem )
		{
			strName.Format(RSS_GUID_ITEM _T("%u"), uItem);
			strGuid = ini.GetString(strName);
			if ( strGuid.IsEmpty() )
			{
				break;
			}

			CRssFeed::Item & item = rssFeedHistory.AddFeedItem(strGuid);
			item.m_bIsHistory = true;
			//item.m_strGuid = strGuid;

			strName.Format(RSS_TITLE_ITEM _T("%u"), uItem);
			item.m_strTitle = ini.GetStringUTF8(strName);

			strName.Format(RSS_PUBDATE_ITEM _T("%u"), uItem);
			item.m_timePubDate = ini.GetUInt64(strName);

			strName.Format(RSS_ENCLOSURE_ITEM _T("%u"), uItem);
			item.m_strEnclosure = ini.GetString(strName);

		} // end for (读取某个url历史项目)

	} // end for (读取所有历史项目)


	// 读取过滤器
	for ( DWORD uSection = 0; uSection < MAX_SECTION_COUNT; ++uSection )
	{
		strName.Format(RSS_FILTER_SECTION _T("%u"), uSection);
		ini.SetSection(strName);

		strFilterName =  ini.GetString(RSS_FILTER_NAME);
		if ( strFilterName.IsEmpty() )
		{
			break;
		}
		CRssFilter & filter = m_listFilter.AddFilter(strFilterName);
		filter.m_strFilter = ini.GetStringUTF8(RSS_FILTER_STR);
		filter.m_strNot = ini.GetStringUTF8(RSS_FILTER_NOT);
		//filter.m_strSaveDir = ini.GetString(RSS_FILTER_SAVE_DIR);
		filter.m_strQuality = ini.GetString(RSS_FILTER_QUALITY);
		filter.m_strFileType = ini.GetString(RSS_FILTER_FILE_TYPE);
		if ( CRssFeed * pFeed = FindFeed(ini.GetString(RSS_FILTER_APPLY_FEED)) )
		{ 
			filter.m_pApplyFeed = pFeed;
			pFeed->m_pFilter = &filter;
		}

		filter.m_uMinSizeKB = ini.GetInt(RSS_FILTER_MIN_SIZE, 0);
		filter.m_uMaxSizeKB = ini.GetInt(RSS_FILTER_MAX_SIZE, 0);
		filter.m_bAutoStart = ini.GetBool(RSS_FILTER_AUTO_START, false);
		filter.m_bEnable = ini.GetBool(RSS_FILTER_ENABLE, true);
	}
}

//// 保存到ini配置文件中
//void CRssFeedManager::SaveIniConfig(bool bSaveAlways)
//{
//	if ( !bSaveAlways && !IsModified() )
//	{
//		return;
//	}
//
//	CIni ini(GetIniConfigFilename());
//	CString strName;
//
//	// 保存feed url
//	if ( bSaveAlways || m_listFeed.m_bModified )
//	{	
//		ini.SetSection(RSS_FEED_SECTION);
//		DWORD uFeedIndex = 0;
//		for ( CRssFeedList::const_iterator it = m_listFeed.begin();
//			  it != m_listFeed.end();
//			  ++it, ++uFeedIndex
//			)
//		{
//			const CRssFeed & feed = it->second;
//			strName.Format(RSS_FEED_URL_ITEM _T("%u"), uFeedIndex);
//			ini.WriteString(strName, feed.m_strFeedUrl);
//
//			strName.Format(RSS_ALIAS_ITEM _T("%u"), uFeedIndex);
//			ini.WriteString(strName, feed.m_strAlias);
//
//			strName.Format(RSS_LAST_UPDATE _T("%u"), uFeedIndex);
//			ini.WriteUInt64(strName, feed.m_tmLastUpdate);
//
//			strName.Format(RSS_LAST_DOWNLOAD _T("%u"), uFeedIndex);
//			ini.WriteUInt64(strName, feed.m_timeLastDownload.GetTime());
//
//			strName.Format(RSS_LAST_PUB_DATE _T("%u"), uFeedIndex);
//			ini.WriteUInt64(strName, feed.m_timeLastPub.GetTime());
//
//			strName.Format(RSS_DOWNLOADED_TIME _T("%u"), uFeedIndex);
//			ini.WriteUInt64(strName, feed.m_timeDownloaded.GetTime());
//
//			strName.Format(RSS_SAVE_DIR _T("%u"), uFeedIndex);
//			ini.WriteStringUTF8(strName, feed.m_strSaveDir);
//
//			//strName.Format(RSS_HAS_COMPLETE_FILE _T("%u"), uFeedIndex);
//			//ini.WriteBool(strName, feed.m_bHasNewComplete);
//		}
//
//		// 删除不再使用的feed url
//		for ( DWORD i = uFeedIndex; i < MAX_ITEM_COUNT; ++i )
//		{
//			strName.Format(RSS_FEED_URL_ITEM _T("%u"), i);
//			if ( ini.GetString(strName).IsEmpty() )
//			{
//				break;
//			}
//			ini.DeleteKey(strName);
//
//			strName.Format(RSS_ALIAS_ITEM _T("%u"), i);
//			ini.DeleteKey(strName);
//		}
//	}
//
//	// 保存历史
//	if ( bSaveAlways || m_listFeedHistory.m_bModified )
//	{	
//		DWORD uFeedIndex = 0;
//		for ( CRssFeedList::const_iterator it = m_listFeedHistory.begin();
//			  it != m_listFeedHistory.end();
//			  ++it
//			)
//		{
//			if ( it->second.IsEmpty() )
//			{
//				continue;
//			}
//
//			strName.Format(RSS_HISTORY_SECTION _T("%u"), uFeedIndex);
//			ini.DeleteSection(strName);
//			ini.SetSection(strName);
//
//			// 保存某一个feed url下的历史
//			const CRssFeed & rssFeedHistory = it->second;
//			ini.WriteString(RSS_FEED_URL_ITEM, rssFeedHistory.m_strFeedUrl);
//			ini.WriteString(RSS_ALIAS_ITEM, rssFeedHistory.m_strAlias);
//			DWORD uItem = 0;
//			for ( CRssFeed::ItemConstIterator it = rssFeedHistory.GetItemIterBegin();
//				it != rssFeedHistory.GetItemIterEnd();
//				++it, ++uItem
//				)
//			{
//				const CRssFeed::Item & item = it->second;
//
//				strName.Format(RSS_GUID_ITEM _T("%u"), uItem);
//				ini.WriteString(strName, item.m_strGuid);
//
//				strName.Format(RSS_TITLE_ITEM _T("%u"), uItem);
//				ini.WriteStringUTF8(strName, item.m_strTitle);
//
//				strName.Format(RSS_PUBDATE_ITEM _T("%u"), uItem);
//				ini.WriteUInt64(strName, item.m_timePubDate.GetTime());
//
//				strName.Format(RSS_ENCLOSURE_ITEM _T("%u"), uItem);
//				ini.WriteString(strName, item.m_strEnclosure);
//			} // end for (保存某个feed url历史项目)
//
//			++uFeedIndex;
//		} // end for (保存所有历史项目)
//
//		// 删除不再使用的历史section
//		for ( DWORD i = uFeedIndex; i < MAX_ITEM_COUNT; ++i )
//		{
//			strName.Format(RSS_HISTORY_SECTION _T("%u"), i);
//			ini.SetSection(strName);
//			if ( ini.GetString(RSS_FEED_URL_ITEM).IsEmpty() )
//			{
//				break;
//			}
//			ini.DeleteSection(strName);
//		}
//	}
//
//	// 保存过滤器
//	if ( bSaveAlways || m_listFilter.m_bModified )
//	{
//		DWORD uFilterIndex = 0;
//		for ( CRssFilterList::const_iterator it = m_listFilter.begin();
//			  it != m_listFilter.end();
//			  ++it
//			)
//		{
//			strName.Format(RSS_FILTER_SECTION _T("%u"), uFilterIndex);
//			ini.DeleteSection(strName);
//			ini.SetSection(strName);
//			
//			const CRssFilter & filter = it->second;
//			ini.WriteString(RSS_FILTER_NAME, filter.m_strName);
//			ini.WriteStringUTF8(RSS_FILTER_STR, filter.m_strFilter);
//			ini.WriteStringUTF8(RSS_FILTER_NOT, filter.m_strNot);
//			//ini.WriteString(RSS_FILTER_SAVE_DIR, filter.m_strSaveDir);
//			ini.WriteString(RSS_FILTER_QUALITY, filter.m_strQuality);
//			ini.WriteString(RSS_FILTER_FILE_TYPE, filter.m_strFileType);
//			ini.WriteString(RSS_FILTER_APPLY_FEED, filter.m_pApplyFeed ? filter.m_pApplyFeed->m_strFeedUrl : _T(""));
//			ini.WriteInt(RSS_FILTER_MIN_SIZE, filter.m_uMinSizeKB);
//			ini.WriteInt(RSS_FILTER_MAX_SIZE, filter.m_uMaxSizeKB);
//			ini.WriteBool(RSS_FILTER_AUTO_START, filter.m_bAutoStart);
//			ini.WriteBool(RSS_FILTER_ENABLE, filter.m_bEnable);
//			++uFilterIndex;
//		}
//
//		// 删除不再用的过滤器
//		for ( DWORD i = uFilterIndex; i < MAX_ITEM_COUNT; ++i )
//		{
//			strName.Format(RSS_FILTER_SECTION _T("%u"), i);
//			ini.SetSection(strName);
//			if ( ini.GetString(RSS_FILTER_NAME).IsEmpty() )
//			{
//				break;
//			}
//			ini.DeleteSection(strName);
//		}
//	}
//
//	// 保存完毕，设置为未修改状态
//	ResetModified();
//}

// 删除一个feed
void CRssFeedManager::DeleteFeed(CRssFeed & feed)
{
	// 查找是否存在
	//CRssFeed * feed = FindFeed(strFeedUrl);
	//if ( feed == NULL )
	//{
	//	return;
	//}

	// 现在是一对一关系，所以不需要再查找了
	//// 查找是否有过滤器应用到此feed	
	//CRssFilterList::iterator it = m_listFilter.begin();
	//while ( it != m_listFilter.end() )
	//{
	//	if ( it->second.m_pApplyFeed == feed )
	//	{
	//		// 删除此过滤器
	//		it = m_listFilter.erase(it);
	//	}
	//	else 
	//	{
	//		++it;
	//	}
	//}


	// 分类集合中删除对应的订阅
	CRssFeedBase::RemoveFeedFromCatalogs(feed);

	// 删除feed过滤器
	if ( feed.m_pFilter != NULL )
	{
		// feed过滤器必须对应
		ASSERT(feed.m_pFilter->m_pApplyFeed == &feed);
		DeleteFilter(*feed.m_pFilter);
	}

	CString strFeedUrl = feed.m_strFeedUrl;

	// 从feed中删除
	m_listFeed.DeleteFeed(strFeedUrl);

	// 从db中删除
	m_dao.DeleteRssFeed(strFeedUrl);

	// 历史不删除
}


//// 删除一个历史项目
//void CRssFeedManager::DeleteHistory(const CString & strGuid)
//{
//	// 历史中删除
//	for ( CRssFeedList::iterator it = m_listFeedHistory.begin();
//		  it != m_listFeedHistory.end();
//		  ++it
//		)
//	{
//		it->second.DeleteItem(strGuid);
//	}
//
//	// 已修改状态
//	m_listFeedHistory.m_bModified = true;
//}

// 标记项目为历史
//CRssFeed::Item * CRssFeedManager::MarkHistory(const CString & strGuid)
//{
//	CRssFeed::Item * pResult = NULL;
//	for ( CRssFeedList::iterator it = m_listFeed.begin();
//		  it != m_listFeed.end();
//		  ++it
//		)
//	{
//		// 查找feed中是否包含指定guid的项目
//		if ( CRssFeed::Item * item = it->second.FindItem(strGuid) )
//		{
//			pResult = &MarkHistory(it->second, *item);
//		}
//	}
//	return pResult;
//}

// 标记项目为历史
CRssFeed::Item & CRssFeedManager::MarkHistory(CRssFeed::Item & item)
{
	// 设置为历史
	item.m_bIsHistory = true;

	ASSERT(item.m_pFeed != NULL);
	if ( item.m_pFeed == NULL )
	{
		return item;
	}

	// 保存到历史feed
	CRssFeed & feedHistory = m_listFeedHistory.AddFeed(item.m_pFeed->m_strFeedUrl);
	SetHistoryAlias(feedHistory, *item.m_pFeed);
	CRssFeed::Item & result = feedHistory.AddFeedItem(item);

	// 保存历史项目
	m_dao.SaveHistoryFeedItem(result);

	return result;
}

// 增加一个feed
CRssFeed & CRssFeedManager::AddFeed(const CString & strFeedUrl, const CString & strSaveDir)
{
	CRssFeed & result = m_listFeed.AddFeed(strFeedUrl);
	result.m_strSaveDir = strSaveDir;
	m_dao.SaveRssFeed(result);
	return result;
}

// 增加一个过滤器
CRssFilter & CRssFeedManager::AddFilter(const CRssFilter & filter)
{
	CRssFilter & result = m_listFilter.AddFilter(filter.m_strName);
	result = filter;
	
	if ( result.m_pApplyFeed != NULL )
	{
		result.m_pApplyFeed->m_pFilter = &result;
	}

	m_dao.SaveFilter(result);
	return result;
}

// 删除一个过滤器
void CRssFeedManager::DeleteFilter(CRssFilter & filter)
{
	CString strName = filter.m_strName;

	// 列表中删除
	m_listFilter.DeleteFilter(strName);

	// db中删除
	m_dao.DeleteFilter(strName);
}

// 刷新feed
bool CRssFeedManager::RefreshFeed(CRssFeed & feed, const char * lpszUrlStream)
{
	bool bResult = feed.Refresh(this, lpszUrlStream, &m_listFeedHistory);
	feed.m_iState &= ~int(CRssFeed::fsRefresh);
	if ( bResult )
	{
		// 更新成功，保存最新状态
		//m_listFeed.m_bModified = true;
		m_dao.SaveRssFeed(feed);
	}

	return bResult;
}

// 刷新所有feed
//void CRssFeedManager::RefreshFeed()
//{
//	for ( CRssFeedList::iterator it = m_listFeed.begin(); 
//		  it != m_listFeed.end();
//		  ++it
//		)
//	{
//		RefreshFeed(it->second);
//	}
//}

// 处理feed更新完成消息，返回一个可被自动下载的feed
LRESULT CRssFeedManager::OnFeedUpdateMsg(const CString & strUrl, const char * lpszStream)
{
	CRssFeed * feed = FindFeed(strUrl);
	if ( feed != NULL )
	{
		//bool bIsUpdated = feed->IsUpdated();

		if ( !RefreshFeed(*feed, lpszStream) )
		{
			feed = NULL;
		}
		//else if ( !bIsUpdated )
		//{
		//	// 第一次更新时也不返回feed，后续的更新才允许自动下载
		//	feed = NULL;
		//}
	}
	m_FeedUpdate.OnUpdateFeedStopped(strUrl);
	return reinterpret_cast<LRESULT>(feed);
}


#if defined(_DEBUG)
void CRssFeedManager::test()
{
	//CRssFeedManager fm;
	//fm.LoadConfig();
	//fm.AddFeed(_T("http://www.mininova.org/rss.xml?sub=40"));
	//fm.AddFeed(_T("http://www.verycd.com/sto/movie/horror/feed"));
	//fm.AddFeed(_T("http://tech.163.com/digi/special/00161K7K/rss_digisyt.xml"));
	//fm.AddFeed(_T("http://192.168.2.42/subs/rss_film1.xml"));
	//fm.SaveConfig();

	//fm.RefreshFeed(_T("http://192.168.2.42/subs/rss_film1.xml"));
	//fm.DeleteFeed(_T("http://192.168.2.42/subs/rss_film1.xml"));
	//fm.SaveConfig();
	//
	//fm.RefreshFeed();
	//fm.MarkHistory(_T("http://www.mininova.org/tor/3189427"));
	//fm.MarkHistory(_T("http://www.mininova.org/tor/2673167"));
	//fm.MarkHistory(_T("http://www.mininova.org/tor/1805335"));
	//fm.MarkHistory(_T("http://tech.163.com/digi/10/0317/06/61V6A0LM001618S7.html"));
	//fm.MarkHistory(_T("http://tech.163.com/digi/10/0315/06/61Q1E6LU001618SQ.html"));
	//fm.RefreshFeed();
	//fm.SaveConfig();

	//fm.DeleteHistory(_T("http://www.mininova.org/tor/2673167"));
	//fm.DeleteHistory(_T("http://tech.163.com/digi/10/0315/06/61Q1E6LU001618SQ.html"));
	//fm.SaveConfig();

	//CRssFilter & filter = fm.AddFilter(_T("filter1"));
	//filter.m_strFilter = _T("PeS");
	//filter.m_strNot = _T("aLl");
	////filter.m_pApplyFeed = &fm.AddFeed(_T("http://www.mininova.org/rss.xml?sub=9"));
	////fm.RefreshFeed(*filter.m_pApplyFeed);

	//CRssFilter & filter2 = fm.AddFilter(_T("filter2"));
	//filter2.m_strFilter = _T("eVil");
	//
	//fm.AddFilter(_T("filter3")).m_strFilter = _T("tv sHow");

	//CRssFilter & filter4 = fm.AddFilter(_T("filter4"));
	//filter4.m_strFilter = _T("恶魔");
	//filter4.m_strNot = _T("不包含");

	//fm.RefreshFeed();
	//fm.StartFilter();
	//fm.RefreshFeed();
	//fm.DeleteFilter(_T("filter3"));
	//fm.SaveConfig(true);
	//fm.DeleteHistory(_T("http://www.mininova.org/tor/2912769"));
	//fm.DeleteFeed(_T("http://www.mininova.org/rss.xml?sub="));
	//fm.DeleteFeed(_T("http://www.mininova.org/rss.xml?sub=9"));
	//fm.SaveConfig();

}
#endif