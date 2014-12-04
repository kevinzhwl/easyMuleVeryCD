/* 
 * $Id: AICHSyncThread.h 20835 2010-11-18 10:36:09Z dgkang $
 * 
 * this file is part of eMule
 * Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#pragma once

class CKnownFile;
class CSafeFile;
#include <vector>
/////////////////////////////////////////////////////////////////////////////////////////
///CAICHSyncThread
class CAICHSyncThread : public CWinThread
{
	DECLARE_DYNCREATE(CAICHSyncThread)
protected:
	CAICHSyncThread();
public:
	virtual BOOL InitInstance();
	virtual int	Run();

protected:
	bool ConvertToKnown2ToKnown264(CSafeFile* pTargetFile);

private:
	CTypedPtrList<CPtrList, CKnownFile*> m_liToHash;
};

struct AICH_MD4{
	BYTE	m_abyMD4[16];
	BYTE	m_abySHA[20];	
};

#define SHARED_FILE_DELETE_MET _T("share_del.met")
#define SHARED_DEL_MET_VERSION 0x10

class Mutex
{
public:
	Mutex(PLONG pLong)
	{
		m_pLong = pLong;
		InterlockedIncrement(m_pLong);
	}
	~Mutex()
	{
		InterlockedDecrement(m_pLong);
	}

	static BOOL IsLock(PLONG pLong)
	{
		return InterlockedExchangeAdd(pLong,0) > 0;
	}

private:
	PLONG m_pLong;
};
class CAICHCleanThread: public CWinThread
{
	DECLARE_DYNCREATE(CAICHCleanThread)
protected:
	CAICHCleanThread();

public:
	virtual BOOL InitInstance();
	virtual int	Run();

private:	
	void CleanKnown64File(const std::vector<AICH_MD4>& vt);

public:
	static std::vector<AICH_MD4> m_vtDeleteShareFile;
	static void Process();
	static BOOL Save();
	static BOOL Load(std::vector<AICH_MD4>& vt);
	static void Add(const AICH_MD4 & vt);
	static const AICH_MD4* Find(const std::vector<AICH_MD4>& vt,LPBYTE lpSHA);
	static LONG	m_nThreadClean;
};