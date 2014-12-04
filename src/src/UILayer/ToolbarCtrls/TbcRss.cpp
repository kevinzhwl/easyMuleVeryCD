/*
 * $Id: TbcRss.cpp $
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
// TbcRss.cpp : 实现文件
//

#include "stdafx.h"
#include "TbcRss.h"
#include "emule.h"
#include "MenuCmds.h"
#include "CIF.h"
//#include "WndMgr.h"
// CTbcRss

IMPLEMENT_DYNAMIC(CTbcRss, CToolBarCtrlZ)
CTbcRss::CTbcRss()
{
}

CTbcRss::~CTbcRss()
{
}


BEGIN_MESSAGE_MAP(CTbcRss, CToolBarCtrlZ)
	ON_WM_CREATE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

CString	CTbcRss::GetButtonText(UINT i)
{
	switch (i)
	{
		case 0:
			return _T("添加订阅");

		case 1:
			return _T("刷新");
	}

	return _T("");
}

void CTbcRss::Localize()
{
	for ( int i = 0; i < BUTTON_COUNT; ++i )
	{
		ToolBarCtrl_SetText(this, i, GetButtonText(i));
	}

	if ( NULL != GetParent() )
	{
		GetParent()->SendMessage(WM_SIZE);
	}
}

void CTbcRss::InitImageList()
{
	AddImageIcon(_T("PNG_TBOPENFILE"));
	AddImageIcon(_T("PNG_BTREFRESH"));

	AddDisableImageIcon(_T("PNG_TBOPENFILE_D"));
	AddDisableImageIcon(_T("PNG_BTREFRESH_D"));
}

// CTbcRss 消息处理程序

int CTbcRss::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CToolBarCtrlZ::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	//theWndMgr.m_pTbcRss = this;
	InitImageList();

	TBBUTTON	tbb[BUTTON_COUNT];
	CString		str;

	tbb[0].idCommand = MP_OPEN;
	tbb[1].idCommand = MP_REFRESH;

	for (int i = 0; i < BUTTON_COUNT; i++)
	{
		AddSingleString(GetButtonText(i));
		tbb[i].iString = i;
		tbb[i].iBitmap = i;
		tbb[i].fsState = TBSTATE_ENABLED;
		tbb[i].fsStyle = TBSTYLE_BUTTON | BTNS_AUTOSIZE;
	}
	AddButtons(BUTTON_COUNT, tbb);

	//EnableButton(MP_REFRESH);

	Localize();
	return 0;
}

void CTbcRss::OnDestroy()
{
	__super::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
	//theWndMgr.m_pTbcRss = NULL;

}
