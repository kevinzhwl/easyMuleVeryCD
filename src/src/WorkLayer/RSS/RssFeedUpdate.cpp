/*
 * $Id: RssFeedUpdate.cpp 20817 2010-11-17 10:29:30Z gaoyuan $
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
#include "GetUrlStream.h"
#include "RssFeedUpdate.h"
#include "WorkLayer/UIMessage.h"


// CRssFeedUpdate
CRssFeedUpdate::CRssFeedUpdate()
{
}

CRssFeedUpdate::~CRssFeedUpdate()
{
	// 需要暂停运行中的线程
	// 注意，这不是一个好的实现！
	try 
	{
		for ( UpdateThreads::iterator it = m_mapUpdateThreads.begin();
			  it != m_mapUpdateThreads.end();
			  ++it
			)
		{
			if ( CWinThread * pThread = it->second )
			{
				pThread->SuspendThread();
			}
		}
	}
	catch (...)
	{
	}
}

// 更新处理函数
UINT CRssFeedUpdate::UpdateProcess(LPVOID pUpdateParam)
{
	ASSERT(pUpdateParam != NULL);
	try 
	{				
		UPDATE_PARAM * param = static_cast<UPDATE_PARAM*>(pUpdateParam);

		// get xml
		CGetUrlStream sUrl;
		
		BOOL bRet = FALSE;
		int cTry = 3;
		do 
		{
			// TRACE("\n %s  - try to get XML: %d\n", __FUNCTION__, 3 - cTry);
			bRet = sUrl.OpenUrl(param->m_strUrl);
			if ( --cTry == 0)
				break;
		} 
		while (bRet == FALSE);

		ASSERT(param->m_pNotify != NULL);
		if (bRet)
		{
			param->m_pNotify->OnNotifyRssFeedUpdated(param->m_strUrl, sUrl.GetStream());
		}
		delete param;
	}
	catch(...)
	{
		return 1;
	}
	return 0;
}

// 启动更新
void CRssFeedUpdate::StartUpdateFeed(const CString & strUrl, IRssFeedUpdateNotify & notify, BOOL bCreateNewThread)
{	
	UPDATE_PARAM * param = new UPDATE_PARAM;
	param->m_strUrl = strUrl;
	param->m_pNotify = &notify;

	if (bCreateNewThread)
	{
		CWinThread * & pThread = m_mapUpdateThreads[strUrl];
		if ( pThread == NULL )
		{		
			pThread = AfxBeginThread(&UpdateProcess, param);
		}
	}
	else
	{	
		UpdateProcess(param);
	}
	
}

// 更新结束
void CRssFeedUpdate::OnUpdateFeedStopped(const CString & strUrl)
{
	UpdateThreads::iterator it = m_mapUpdateThreads.find(strUrl);
	if ( it != m_mapUpdateThreads.end() )
	{
		ASSERT(it->second != NULL);
		m_mapUpdateThreads.erase(it);
	}	
}

