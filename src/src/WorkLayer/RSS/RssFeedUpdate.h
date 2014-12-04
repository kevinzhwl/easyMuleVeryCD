/*
 * $Id: RssFeedUpdate.h 20761 2010-11-09 01:54:12Z gaoyuan $
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

#include <map>


/**
 * @brief 用于订阅内容更新完成的通知接口
 */
struct IRssFeedUpdateNotify
{
	/// 通知feed已更新（由线程调用）
	virtual void OnNotifyRssFeedUpdated(const CString & strUrl, const char * lpszStream) = 0;
};

/**
 * @brief 用于更新订阅内容
 */
class CRssFeedUpdate
{
private:
	/// 更新线程
	typedef std::map<CString, CWinThread *>	UpdateThreads;
	
	/// 更新参数
	struct UPDATE_PARAM
	{
		CString			m_strUrl;		///< 订阅的url
		IRssFeedUpdateNotify *	m_pNotify;		///< 更新完成后的通知对象
	};

public:
	/// 构造函数
	CRssFeedUpdate();

	/// 析构函数
	virtual ~CRssFeedUpdate();

public:
	/// 启动一个异步更新订阅的过程
	void StartUpdateFeed(const CString & strUrl, IRssFeedUpdateNotify & notify, BOOL bCreateNewThread = TRUE);

	/// 更新结束后，清理相关资源
	void OnUpdateFeedStopped(const CString & strUrl);

private:
	UpdateThreads		m_mapUpdateThreads;

private:
	/// 更新处理函数
	static UINT UpdateProcess( LPVOID pUpdateParam );

};

