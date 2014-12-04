/* 
 * $Id: ED2KLink.h 19246 2010-04-29 08:23:47Z dgkang $
 * 
 * this file is part of eMule
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
#include "shahashset.h"
class CSafeMemFile;

struct SUnresolvedHostname
{
	SUnresolvedHostname()
	{
		nPort = 0;
	}
	CStringA strHostname;
	uint16 nPort;
	CString strURL;
};


class CED2KLink
{
public:
	static CED2KLink* CreateLinkFromUrl(const TCHAR* url);
	
	virtual ~CED2KLink();

	typedef enum { 
					kServerList, 
					kServer , 
					kFile , 
					kRss,		// rss link, ADDED by VC-yavey on 2010-04-16
					kInvalid 
				} LinkType;

	virtual LinkType GetKind() const = 0;
	virtual void GetLink(CString& lnk) const = 0;
	virtual class CED2KServerListLink* GetServerListLink() = 0;
	virtual class CED2KServerLink* GetServerLink() = 0;
	virtual class CED2KFileLink* GetFileLink() = 0;

	// ADDED by VC-yavey on 2010-04-16 <begin>
	virtual class CED2KRssLink* GetRssLink()
	{
		return NULL;
	}
	// ADDED by VC-yavey on 2010-04-16 <end>
};


class CED2KServerLink : public CED2KLink
{
public:
	CED2KServerLink(const TCHAR* ip,const TCHAR* port);
	virtual ~CED2KServerLink();

	virtual LinkType GetKind() const;
	virtual void GetLink(CString& lnk) const;
	virtual CED2KServerListLink* GetServerListLink();
	virtual CED2KServerLink* GetServerLink();
	virtual CED2KFileLink* GetFileLink();

	const CString& GetAddress() const { return m_strAddress; }
	uint16 GetPort() const { return m_port;}
	void GetDefaultName(CString& defName) const { defName = m_defaultName; }

private:
	CED2KServerLink();
	CED2KServerLink(const CED2KServerLink&);
	CED2KServerLink& operator=(const CED2KServerLink&);

	CString m_strAddress;
	uint16 m_port;
	CString m_defaultName;
};


class CED2KFileLink : public CED2KLink
{
public:
	CED2KFileLink(const TCHAR* pszName, const TCHAR* pszSize, const TCHAR* pszHash, const CStringArray& param, const TCHAR* pszSources);
	virtual ~CED2KFileLink();

	virtual LinkType GetKind() const;
	virtual void GetLink(CString& lnk) const;
	virtual CED2KServerListLink* GetServerListLink();
	virtual CED2KServerLink* GetServerLink();
	virtual CED2KFileLink* GetFileLink();
	
	const TCHAR* GetName() const			{ return m_name; }
	const uchar* GetHashKey() const			{ return m_hash;}
	const CAICHHash& GetAICHHash() const	{ return m_AICHHash;}
	EMFileSize GetSize() const				{ return (uint64)_tstoi64(m_size); }	
	bool HasValidSources() const			{ return (SourcesList != NULL); }
	bool HasHostnameSources() const			{ return (!m_HostnameSourcesList.IsEmpty()); }
	bool HasValidAICHHash() const			{ return m_bAICHHashValid; }

	CSafeMemFile* SourcesList;
	CSafeMemFile* m_hashset;
	CTypedPtrList<CPtrList, SUnresolvedHostname*> m_HostnameSourcesList;
	CString m_strFilepath;

private:
	CED2KFileLink();
	CED2KFileLink(const CED2KFileLink&);
	CED2KFileLink& operator=(const CED2KFileLink&);

	CString m_name;
	CString m_size;
	uchar	m_hash[16];
	bool	m_bAICHHashValid;
	CAICHHash m_AICHHash;
};


class CED2KServerListLink : public CED2KLink
{
public:
	CED2KServerListLink(const TCHAR* pszAddress);
	virtual ~CED2KServerListLink();

	virtual LinkType GetKind() const;
	virtual void GetLink(CString& lnk) const;
	virtual CED2KServerListLink* GetServerListLink();
	virtual CED2KServerLink* GetServerLink();
	virtual CED2KFileLink* GetFileLink();

	const TCHAR* GetAddress() const { return m_address; }

private:
	CED2KServerListLink();
	CED2KServerListLink(const CED2KFileLink&);
	CED2KServerListLink& operator=(const CED2KFileLink&);

	CString m_address;
};


//ADDED by VC-yavey on 2010-04-16	<begin>
// RSS link
class CED2KRssLink : public CED2KLink
{
public:
	CED2KRssLink(LPCTSTR lpszRssUrl)
		:
		m_strRssUrl(lpszRssUrl)
	{
	}
	virtual ~CED2KRssLink()
	{
	}

	virtual LinkType GetKind() const
	{
		return kRss;
	}

	virtual void GetLink(CString& lnk) const
	{
		lnk.Format(_T("ed2k://|subscribe|%s|/"), m_strRssUrl);
	}
	virtual class CED2KServerListLink* GetServerListLink()
	{
		return NULL;
	}
	virtual class CED2KServerLink* GetServerLink()
	{
		return NULL;
	}
	virtual class CED2KFileLink* GetFileLink()
	{
		return NULL;
	}
	virtual class CED2KRssLink* GetRssLink()
	{
		return this;
	}

	const CString & GetRssUrl() const
	{
		return m_strRssUrl;
	}

private:
	CString			m_strRssUrl;
};

//ADDED by VC-yavey on 2010-04-16	<end>