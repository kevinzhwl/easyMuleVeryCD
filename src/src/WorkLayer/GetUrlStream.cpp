/*
 * $Id: GetUrlStream.cpp
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
#include <windows.h>
#include <wininet.h>

#include "GetUrlStream.h"

#pragma comment(lib,"Wininet.lib")

CGetUrlStream::CGetUrlStream(void)
	:
	m_uLastError(0)
{

}

CGetUrlStream::~CGetUrlStream(void)
{
}


bool CGetUrlStream::OpenUrl( const CString & strUrl )
{
	m_uLastError = 0;
	m_strStream.clear();

    // 建立会话
    HINTERNET hSession = InternetOpen( _T("CGetUrlStream"),
                                       INTERNET_OPEN_TYPE_PRECONFIG,
                                       NULL,
								       NULL,
								       0
								     );
    if ( hSession == NULL ) {
		SetLastError();
        return false;
    }
	
	// 设置超时60s
	unsigned int timeout = 60 * 1000;
	InternetSetOption( hSession, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout) );
	InternetSetOption( hSession, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout) );

	// 打开URL
	HINTERNET hUrl = InternetOpenUrl( hSession, 
									  strUrl,
									  NULL,
									  0,
									  INTERNET_FLAG_RELOAD,
									  NULL
									);

	
    if( hUrl == NULL ) 
	{
		SetLastError();
        InternetCloseHandle( hSession );
        return false;
    }

	TCHAR szBuffer[80];
	DWORD dwLen = _countof(szBuffer);
	BOOL bResult = HttpQueryInfo( hUrl, HTTP_QUERY_STATUS_CODE, szBuffer, &dwLen, NULL);

	if ( !bResult || (DWORD)_ttol(szBuffer) != 200 )
	{
		return FALSE;
	}

	// 开始下载
	m_strStream.reserve( 64 * 1024 );
    char  buffer[8 * 1024];
	DWORD reads = 0;

	for (;;) 
	{
		if ( !InternetReadFile( hUrl, buffer, sizeof(buffer), &reads ) ) 
		{
			break;
		}
		
		if ( reads == 0 ) {
			break;
		}

		m_strStream.append( buffer, reads );
	}

	InternetCloseHandle( hUrl );
    InternetCloseHandle( hSession );

	return true;
}

inline void CGetUrlStream::SetLastError()
{
	m_uLastError = GetLastError();
}