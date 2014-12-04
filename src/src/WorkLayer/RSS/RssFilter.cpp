/*
 * $Id: RssFilter.cpp 19972 2010-06-25 06:32:16Z yaveymao $
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
#include "RssFilter.h"


CRssFilter::CRssFilter()
{
	Clear();
}

CRssFilter::~CRssFilter(void)
{
}

// 清除所有配置
void CRssFilter::Clear()
{
	m_strName.Empty();
	m_strFilter.Empty();
	m_strNot.Empty();
	//m_strSaveDir.Empty();
	m_strQuality.Empty();
	m_strFileType.Empty();
	m_pApplyFeed = NULL;
	m_uMinSizeKB = 0;
	m_uMaxSizeKB = 0;
	m_bAutoStart = true;
	m_bEnable = true;
}

// 取得符合条件的资源
// void CRssFilter::StartFilter(IRetrieveRssFilterResult & result)
// {
// 	if ( CRssFeed * feed = m_pApplyFeed )
// 	{
// 		StartFilter(*feed, result);
// 	}
// 	else
// 	{
// 		if ( CRssFeedList * list = result.OnGetAllRssFeed() )
// 		{
// 			for ( CRssFeedList::iterator it = list->begin();
// 				  it != list->end();
// 				  ++it
// 				)
// 			{
// 				StartFilter(it->second, result);
// 			}
// 		}
// 	}
// }

// 取得符合条件的资源
bool CRssFilter::StartFilter(CRssFeed & rfApplyFeed, IRetrieveRssFilterResult & result) const
{
	if ( !m_bEnable )
	{
		// 已禁用
		return false;
	}

	if ( m_pApplyFeed != NULL && m_pApplyFeed != &rfApplyFeed )
	{
		// 过滤器不适用于指定的订阅
		return false;
	}

	// 保存最后下载日期，OnRetrieveRssFilterResult调用后此值可能被更改
	CTime timeLastDownload = rfApplyFeed.m_timeLastDownload;
	if ( timeLastDownload >= rfApplyFeed.m_timeLastPub )
	{
		// 最新的资源已经被下载了
		return true;
	}

	// 下载最新的影片资源，如影片附带字幕，则同时下载字幕
	for ( CRssFeed::ItemIterator it = rfApplyFeed.GetItemIterBegin();
		  it != rfApplyFeed.GetItemIterEnd();
		  ++it
		)
	{
		CRssFeed::Item & item = it->second;

		/*
			下载条件：
			1) 资源发布日期 > 上次最近下载的日期
			2-1) 为最新影片（多个相同日期的最新影片均可被下载）
			2-1-1) 满足过滤器条件
			或 2-2) 为字幕,若其相关的影片已被下载，则也自动下载之
		*/

		// 1) 检查是否满足“资源发布日期 > 上次最近下载的日期”
		if ( item.m_timePubDate > timeLastDownload )
		{
			// 2-1) 为最新影片（多个相同日期的最新影片均可被下载）
			if ( item.IsLastPubFilmItem() )
			{
				// 2-1-1) 检查是否满足过滤器条件
				if ( IsRequest(item) )
				{
					result.OnRetrieveRssFilterResult(*this, item);

					// 若有字幕，则同时下载字幕
					if ( item.m_pAttachItem != NULL )
					{
						result.OnRetrieveRssFilterResult(*this, *item.m_pAttachItem);
					}
				}
			}
			// 或 2-2) 为字幕,若其相关的影片已被下载，则也自动下载之
			else if ( !item.m_bIsHistory && item.IsSubtitle() &&
					  item.m_pAttachItem != NULL && item.m_pAttachItem->m_bIsHistory 
					)
			{
				result.OnRetrieveRssFilterResult(*this, item);
			}
		}
	}

	return true;
}

// 判断项目是否符合过滤条件
bool CRssFilter::IsRequest(const CRssFeed::Item & item) const
{
	// 是否为历史项目
	if ( item.m_bIsHistory )
	{
		return false;
	}

	// 是否包含指定字符串（不区分大小写）
	if ( !m_strFilter.IsEmpty() && StrStrI(item.m_strTitle, m_strFilter) == NULL )
	{
		// not found
		return false;
	}

	// 是否包含排除字符串（不区分大小写）
	if ( !m_strNot.IsEmpty() && StrStrI(item.m_strTitle, m_strNot) != NULL )
	{
		// found
		return false;
	}
	
	// 检查大小
	if ( m_uMinSizeKB != 0 && item.m_uEnclosureLength / 1024 < m_uMinSizeKB )
	{
		return false;
	}
	if ( m_uMaxSizeKB != 0 && item.m_uEnclosureLength / 1024 > m_uMaxSizeKB )
	{
		return false;
	}

	return true;
}

