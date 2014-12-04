/*
 * $Id: UpdateInfo.h 20689 2010-10-20 04:12:33Z dgkang $
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

#define UPDATE_NODOWNLOAD  0
#define UPDATE_DOWNLOADING 1
#define UPDATE_DOWNLOADED  2



// CUpdateInfo 命令目标

class CUpdateInfo : public CObject
{
public:
	CUpdateInfo();
	virtual ~CUpdateInfo();

public:
	void SetUpdateHash(CString hash);
	CString GetUpdateHash(void);

	int	 GetUpdateState(const uchar* hash);

	BOOL UpdateInfoFileExists(void);

	BOOL isUpdateFile(CString hash);

	CString GetUpdateInfoFile(void);

	int Compare(CString hash);

	BOOL DeleteUpdate(CString hash);

	BOOL ClearUpdateHistory(const uchar * hashkey);//清除历史升级任务
	bool IsUpdateTask(const CString& strTaskPath);

	/*
	*  启动时根据此标志位判断是否启动更新包
	*  added by zhuhui 2010-5-12
	*/
	BOOL GetUpdateFlag(void);

	void SetUpdateFlag(BOOL bUpdateFlag);

protected:
	BOOL	OpenUpdateInfo(void);
	void	SaveUpdateInfo(void);

protected:
	CString	m_Hash;
	BOOL    m_bUpdateFlag;  //zhuhui[2010-5-12]:启动时安装更新包标识
public:
	virtual void Serialize(CArchive& ar);
	void Serialize1(CArchive& ar);

};
