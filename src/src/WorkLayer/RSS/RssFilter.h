/*
 * $Id: RssFilter.h 19972 2010-06-25 06:32:16Z yaveymao $
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

#include <afxstr.h>
#include "RssFeed.h"

class CRssFilter;


/**
 * @brief 用于获取符合下载条件的资源的接口
 */
struct IRetrieveRssFilterResult
{
	/// 用于获取符合下载条件的资源
	virtual void OnRetrieveRssFilterResult(const CRssFilter & filter, CRssFeed::Item & item) = 0;
};

/**
 * @brief 订阅过滤器，用于从订阅中过滤出符合条件的资源
 */
class CRssFilter
{
public:
	/// 构造函数
	CRssFilter();

	/// 析构函数
	virtual ~CRssFilter(void);

public:
	/// 清除所有配置
	void Clear();

	// 取得符合条件的资源
	//void StartFilter(IRetrieveRssFilterResult & result);

	/// 取得符合条件的资源
	bool StartFilter(CRssFeed & rfApplyFeed, IRetrieveRssFilterResult & result) const;

private:
	/// 判断项目是否符合过滤条件
	bool IsRequest(const CRssFeed::Item & item) const;

public:
	CString				m_strName;			///< 过滤器名称
	CString				m_strFilter;		///< 过滤关键字
	CString				m_strNot;			///< 过滤排除关键字
	CString				m_strQuality;		///< 过滤质量
	CString				m_strFileType;		///< 过滤文件类型
	CRssFeed *			m_pApplyFeed;		///< 指定过滤器应用于哪个订阅，为空则应用于所有订阅
	DWORD				m_uMinSizeKB;		///< 过滤资源的最小大小（KB）
	DWORD				m_uMaxSizeKB;		///< 过滤资源的最大大小（KB）
	bool				m_bAutoStart;		///< 指定创建任务后，是否自动开始下载
	bool				m_bEnable;			///< 过滤器是否生效
};

/**
 * @brief 维护订阅过滤器列表
 */
class CRssFilterList : public std::map<CString, CRssFilter>
{
public:
	/// 构造函数
	CRssFilterList()
		//:
		//m_bModified(false)
	{
	}

	/// 析构函数
	virtual ~CRssFilterList(void)
	{
	}

public:
	/// 增加一个filter
	CRssFilter & AddFilter(const CString & strName)
	{
		CRssFilter & result = (*this)[strName];
		if ( result.m_strName.IsEmpty() )
		{
			result.m_strName = strName;
		}
		//m_bModified = true;
		return result;
	}

	/// 删除一个过滤器
	void DeleteFilter(const CString & strName)
	{
		erase(strName);
		//m_bModified = true;
	}

	/// 取得符合条件的资源
	void StartFilter(CRssFeed & rfApplyFeed, IRetrieveRssFilterResult & result)
	{
		// feed有过滤器，则使用指定的过滤器
		if ( CRssFilter * pFilter = rfApplyFeed.m_pFilter )
		{
			ASSERT(pFilter->m_pApplyFeed == &rfApplyFeed);
			pFilter->StartFilter(rfApplyFeed, result);
			return;
		}

		// 使用默认过滤器
		for ( CRssFilterList::iterator it = this->begin();
			  it != this->end();
			  ++it
			)
		{
			if ( it->second.StartFilter(rfApplyFeed, result) )
			{
				break;
			}
		}
	}

public:
	//bool	m_bModified;
};