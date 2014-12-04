/*
 * $Id: GetUrlStream.h
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
#include <string>

 /**
  * 通过url（http、ftp）下载数据
  */

class CGetUrlStream
{

public:
	CGetUrlStream(void);
	virtual ~CGetUrlStream(void);

public:
	// 打开指定url数据
	bool			OpenUrl( const CString & strUrl );

	// 返回读取的数据
	const char *	GetStream() const
	{ 
		return m_strStream.c_str();
	}

	// 返回读取的数据大小
	size_t			GetSize() const			
	{ 
		return m_strStream.size();
	}

	// 返回错误代码
	unsigned int	GetLastError() const	
	{ 
		return m_uLastError;
	}

private:
	void			SetLastError();

private:
	std::string		m_strStream;		// 保存读取的数据
	unsigned int	m_uLastError;		// 最后一次访问错误代码
};
