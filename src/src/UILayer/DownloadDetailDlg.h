/*
 * $Id: DownloadDetailDlg.h 19524 2010-05-20 10:09:21Z dgkang $
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


// CDownloadDetailDlg 对话框

class CDownloadDetailDlg : public CDialog
{
	DECLARE_DYNAMIC(CDownloadDetailDlg)

public:
	CDownloadDetailDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDownloadDetailDlg();

	void SetPartFileInfo(CKnownFile	*file);
// 对话框数据
	enum { IDD = IDD_DOWNLOAD_DETAIL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};
