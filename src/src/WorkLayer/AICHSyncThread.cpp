/* 
 * $Id: AICHSyncThread.cpp 20835 2010-11-18 10:36:09Z dgkang $
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

#include "StdAfx.h"
#include "aichsyncthread.h"
#include "shahashset.h"
#include "safefile.h"
#include "knownfile.h"
#include "sha.h"
//#include "emule.h"
//#include "emuledlg.h"
#include "sharedfilelist.h"
#include "knownfilelist.h"
#include "preferences.h"
//#include "sharedfileswnd.h"
#include "UIMessage.h"
#include "Log.h"
#include "ThreadsMgr.h"
#include ".\aichsyncthread.h"
#include "GlobalVariable.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////////////////
///CAICHCleanThread


std::vector<AICH_MD4> CAICHCleanThread::m_vtDeleteShareFile;
LONG CAICHCleanThread::m_nThreadClean = 0;

IMPLEMENT_DYNCREATE(CAICHCleanThread, CWinThread)

CAICHCleanThread::CAICHCleanThread()
{

}

void CAICHCleanThread::Add(const AICH_MD4 & vt)
{
	for(size_t i = 0; i < m_vtDeleteShareFile.size(); i++)
	{
		if (!memcmp(m_vtDeleteShareFile[i].m_abyMD4,vt.m_abyMD4,16))
		{
			return;
		}
	}
	m_vtDeleteShareFile.push_back(vt);
}

void CAICHCleanThread::Process()
{
	static time_t tSaveTime = ::time(NULL);
	if (::time(NULL) - tSaveTime > 60 * 10)
	{	
		if ( !Mutex::IsLock(&CAICHCleanThread::m_nThreadClean))
		{
			CAICHCleanThread::Save();
		}
		tSaveTime = time(NULL);
	}

	static time_t tFirstRun = ::time(NULL);
	static BOOL bRuned = FALSE;
	if (time(NULL) - tFirstRun > 60 * 15 && !bRuned)
	{
		bRuned = TRUE; tFirstRun = time(NULL);
		CAICHCleanThread::Save();
		tSaveTime = time(NULL);
		CThreadsMgr::BegingThreadAndRecDown(CThreadsMgr::CleanProc_WaitAndDelWinThd,RUNTIME_CLASS(CAICHCleanThread), THREAD_PRIORITY_BELOW_NORMAL,0);
	}
}

BOOL CAICHCleanThread::Load(std::vector<AICH_MD4>& vt)
{
	Mutex l(&m_nThreadClean);

	CString szFile = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR);
	szFile.Append(SHARED_FILE_DELETE_MET);

	{
		CSafeFile file;
		CFileException fexp;
		if (!file.Open(szFile,CFile::modeRead | CFile::osSequentialScan|CFile::typeBinary | CFile::modeNoTruncate | CFile::shareDenyNone, &fexp))
		{
			if (fexp.m_cause != CFileException::fileNotFound)
			{
				CString strError(_T("Failed to load ") SHARED_FILE_DELETE_MET _T(" file"));
				TCHAR szError[MAX_CFEXP_ERRORMSG];
				if (fexp.GetErrorMessage(szError, ARRSIZE(szError)))
				{
					strError += _T(" - ");
					strError += szError;
				}
				CGlobalVariable::QueueLogLine(true, _T("%s"), strError);
			}
			return FALSE;
		}

		DWORD dwFileSize = (DWORD)file.GetLength();
		if( (dwFileSize > 1 && (dwFileSize - 1) % 36 != 0 ) || dwFileSize == 0 )
			return FALSE;

		uint8 nVer = file.ReadUInt8();
		if (nVer != SHARED_DEL_MET_VERSION)
			return FALSE;

		while (file.GetPosition() < dwFileSize)
		{
			if (file.GetPosition() + 36 > dwFileSize)
			{
				AfxThrowFileException(CFileException::endOfFile, 0, file.GetFileName());
			}
			AICH_MD4 am;
			file.Read(am.m_abyMD4,16);
			file.Read(am.m_abySHA,20);
			vt.push_back(am);
		}
		file.Close();
	}
	::DeleteFile(szFile);
	return TRUE;
}

BOOL CAICHCleanThread::Save()
{
	if (m_vtDeleteShareFile.empty())
	{
		return FALSE;
	}

	CString szFile = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR);
	szFile.Append(SHARED_FILE_DELETE_MET);

	CSafeFile file;
	CFileException fexp;
	if (!file.Open(szFile,CFile::modeCreate | CFile::modeReadWrite | CFile::osSequentialScan|CFile::typeBinary | CFile::modeNoTruncate | CFile::shareDenyRead, &fexp))
	{
		if (fexp.m_cause != CFileException::fileNotFound)
		{
			CString strError(_T("Failed to load ") SHARED_FILE_DELETE_MET _T(" file"));
			TCHAR szError[MAX_CFEXP_ERRORMSG];
			if (fexp.GetErrorMessage(szError, ARRSIZE(szError)))
			{
				strError += _T(" - ");
				strError += szError;
			}
			CGlobalVariable::QueueLogLine(true, _T("%s"), strError);
		}
		return FALSE;
	}

	DWORD dwFileSize = (DWORD)file.GetLength();
	if (dwFileSize > 1 && (dwFileSize - 1) % 36 != 0 )
		return FALSE;

	if (dwFileSize > 1)
	{
		uint8 nVer = file.ReadUInt8();
		if (nVer != SHARED_DEL_MET_VERSION)
		{
			file.Seek(0,CFile::begin);
			file.WriteUInt8(SHARED_DEL_MET_VERSION);
		}
	}
	else
	{
		file.WriteUInt8(SHARED_DEL_MET_VERSION);
	}

	while (file.GetPosition() < dwFileSize)
	{
		if (file.GetPosition() + 36 > dwFileSize)
		{
			AfxThrowFileException(CFileException::endOfFile, 0, file.GetFileName());
		}
		file.Seek(36, CFile::current);
	}

	for(size_t i = 0; i < m_vtDeleteShareFile.size(); i++ )
	{
		file.Write(m_vtDeleteShareFile[i].m_abyMD4,16);
		file.Write(m_vtDeleteShareFile[i].m_abySHA,20);
	}
	file.Flush();
	m_vtDeleteShareFile.clear();
	return TRUE;
}


BOOL CAICHCleanThread::InitInstance()
{
	DbgSetThreadName("CAICHCleanThread");
	InitThreadLocale();
	return TRUE;
}

int CAICHCleanThread::Run()
{
	CUnregThreadAssist	uta(m_nThreadID);
	
	if (!CGlobalVariable::IsRunning())
		return 0;
	
	std::vector<AICH_MD4> vt;
	if( CAICHCleanThread::Load(vt) && !vt.empty())
	{
		CSingleLock l(&CAICHHashSet::m_mutKnown2File,TRUE);
		CleanKnown64File(vt);
	}
	return 0;
}



const AICH_MD4* CAICHCleanThread::Find(const std::vector<AICH_MD4>& vt,LPBYTE lpSHA)
{
	for(size_t i = 0; i < vt.size(); i++)
	{
		if (!memcmp(vt[i].m_abySHA,lpSHA,20))
		{
			return &vt[i];
		}
	}
	return NULL;
}

void CAICHCleanThread::CleanKnown64File(const std::vector<AICH_MD4>& vt)
{
	CString szNewFile,fullpath;
	{
		szNewFile = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR);
		szNewFile.Append(KNOWN2_MET_FILENAME_NEW);
		CSafeFile NewFile;
		CFileException Newfexp;

		if (!NewFile.Open(szNewFile,CFile::modeCreate| CFile::modeWrite | CFile::osSequentialScan | CFile::typeBinary | CFile::shareDenyWrite | CFile::shareDenyRead, &Newfexp))
		{
			if (Newfexp.m_cause != CFileException::fileNotFound)
			{
				CString strError(_T("Failed to load ") KNOWN2_MET_FILENAME_NEW _T(" file"));
				TCHAR szError[MAX_CFEXP_ERRORMSG];
				if (Newfexp.GetErrorMessage(szError, ARRSIZE(szError)))
				{
					strError += _T(" - ");
					strError += szError;
				}
				CGlobalVariable::QueueLogLine(true, _T("%s"), strError);
			}
			return;
		}

		fullpath = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR);
		fullpath.Append(KNOWN2_MET_FILENAME);
		CSafeFile file;
		CFileException fexp;
		if (!PathFileExists(fullpath))
			return;

		if (!file.Open(fullpath,CFile::modeRead | CFile::osSequentialScan|CFile::typeBinary|CFile::shareDenyWrite, &fexp))
		{
			if (fexp.m_cause != CFileException::fileNotFound)
			{
				CString strError(_T("Failed to load ") KNOWN2_MET_FILENAME _T(" file"));
				TCHAR szError[MAX_CFEXP_ERRORMSG];
				if (fexp.GetErrorMessage(szError, ARRSIZE(szError)))
				{
					strError += _T(" - ");
					strError += szError;
				}
				CGlobalVariable::QueueLogLine(true, _T("%s"), strError);
			}
			return;
		}

		try 
		{
			uint8 header = file.ReadUInt8();
			if (header != KNOWN2_MET_VERSION)
			{
				AfxThrowFileException(CFileException::endOfFile, 0, file.GetFileName());
			}
			NewFile.WriteUInt8(KNOWN2_MET_VERSION);
		 
			uint32 nExistingSize = (UINT)file.GetLength();
			while (file.GetPosition() < nExistingSize)
			{
				CAICHHash CurrentHash(&file);
				uint32 nHashCount = file.ReadUInt32();

				if (file.GetPosition() + nHashCount*HASHSIZE > nExistingSize)
				{
					AfxThrowFileException(CFileException::endOfFile, 0, file.GetFileName());
				}

				if ( !Find(vt,CurrentHash.GetRawHash()))
				{
					NewFile.Write(CurrentHash.GetRawHash(),20);
					NewFile.WriteUInt32(nHashCount);

					BYTE* buffer = new BYTE[nHashCount*CAICHHash::GetHashSize()];
					file.Read(buffer, nHashCount*CAICHHash::GetHashSize());
					NewFile.Write(buffer,nHashCount*CAICHHash::GetHashSize());
					delete[] buffer;
				}
				else
				{
					file.Seek(nHashCount*HASHSIZE, CFile::current);
				}
			}
			file.Close();
			NewFile.Flush();
			NewFile.Close();
		}
		catch(CFileException* error)
		{
			if (error->m_cause == CFileException::endOfFile)
				CGlobalVariable::QueueLogLine(true, GetResString(IDS_ERR_MET_BAD), KNOWN2_MET_FILENAME);
			else
			{
				TCHAR buffer[MAX_CFEXP_ERRORMSG];
				error->GetErrorMessage(buffer, ARRSIZE(buffer));
				CGlobalVariable::QueueLogLine(true,GetResString(IDS_ERR_SERVERMET_UNKNOWN),buffer);
			}
			error->Delete();
		}		
	}
	::MoveFileEx(szNewFile,fullpath,MOVEFILE_REPLACE_EXISTING);
}




/////////////////////////////////////////////////////////////////////////////////////////
///CAICHSyncThread
IMPLEMENT_DYNCREATE(CAICHSyncThread, CWinThread)

CAICHSyncThread::CAICHSyncThread()
{

}

BOOL CAICHSyncThread::InitInstance()
{
	DbgSetThreadName("AICHSyncThread");
	InitThreadLocale();
	return TRUE;
}

int CAICHSyncThread::Run()
{
	CUnregThreadAssist	uta(m_nThreadID);

	if ( !CGlobalVariable::IsRunning() )
		return 0;
	// we need to keep a lock on this file while the thread is running
	CSingleLock lockKnown2Met(&CAICHHashSet::m_mutKnown2File);
	lockKnown2Met.Lock();
	
	CSafeFile file;
	bool bJustCreated = ConvertToKnown2ToKnown264(&file);
	
	// we collect all masterhashs which we find in the known2.met and store them in a list
	CList<CAICHHash> liKnown2Hashs;
	CString fullpath = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR);
	fullpath.Append(KNOWN2_MET_FILENAME);
	
	CFileException fexp;
	uint32 nLastVerifiedPos = 0;

	if (!bJustCreated && !file.Open(fullpath,CFile::modeCreate|CFile::modeReadWrite|CFile::modeNoTruncate|CFile::osSequentialScan|CFile::typeBinary|CFile::shareDenyNone, &fexp)){
		if (fexp.m_cause != CFileException::fileNotFound){
			CString strError(_T("Failed to load ") KNOWN2_MET_FILENAME _T(" file"));
			TCHAR szError[MAX_CFEXP_ERRORMSG];
			if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
				strError += _T(" - ");
				strError += szError;
			}
			LogError(LOG_STATUSBAR, _T("%s"), strError);
		}
		return false;
	}
	try {
		if (file.GetLength() >= 1){
			uint8 header = file.ReadUInt8();
			if (header != KNOWN2_MET_VERSION){
				AfxThrowFileException(CFileException::endOfFile, 0, file.GetFileName());
			}
			//setvbuf(file.m_pStream, NULL, _IOFBF, 16384);
			uint32 nExistingSize = (UINT)file.GetLength();
			uint32 nHashCount;
			while (file.GetPosition() < nExistingSize){
				liKnown2Hashs.AddTail(CAICHHash(&file));
				nHashCount = file.ReadUInt32();
				if (file.GetPosition() + nHashCount*CAICHHash::GetHashSize() > nExistingSize){
					AfxThrowFileException(CFileException::endOfFile, 0, file.GetFileName());
				}
				// skip the rest of this hashset
				file.Seek(nHashCount*CAICHHash::GetHashSize(), CFile::current);
				nLastVerifiedPos = (UINT)file.GetPosition();
			}
		}
		else
			file.WriteUInt8(KNOWN2_MET_VERSION);
	}
	catch(CFileException* error){
		if (error->m_cause == CFileException::endOfFile){
			LogError(LOG_STATUSBAR,GetResString(IDS_ERR_MET_BAD), KNOWN2_MET_FILENAME);
			// truncate the file to the size to the last verified valid pos
			try{
				file.SetLength(nLastVerifiedPos);
				if (file.GetLength() == 0){
					file.SeekToBegin();
					file.WriteUInt8(KNOWN2_MET_VERSION);
				}
			}
			catch(CFileException* error2){
				error2->Delete();
			}
		}
		else{
			TCHAR buffer[MAX_CFEXP_ERRORMSG];
			error->GetErrorMessage(buffer, ARRSIZE(buffer));
			LogError(LOG_STATUSBAR,GetResString(IDS_ERR_SERVERMET_UNKNOWN),buffer);
		}
		error->Delete();
		return false;
	}
	
	// now we check that all files which are in the sharedfilelist have a corresponding hash in out list
	// those how don'T are added to the hashinglist
	CList<CAICHHash> liUsedHashs;	
	CSingleLock sharelock(&CGlobalVariable::sharedfiles->m_mutWriteList);
	sharelock.Lock();

	for (int i = 0; i < CGlobalVariable::sharedfiles->GetCount(); i++){
		CKnownFile* pCurFile = CGlobalVariable::sharedfiles->GetFileByIndex(i);
		if (pCurFile != NULL && !pCurFile->IsPartFile() ){
			if (!CGlobalVariable::IsRunning()) // in case of shutdown while still hashing
				return 0;
			if (pCurFile->GetAICHHashset()->GetStatus() == AICH_HASHSETCOMPLETE){
				bool bFound = false;
				for (POSITION pos = liKnown2Hashs.GetHeadPosition();pos != 0;)
				{
					CAICHHash current_hash = liKnown2Hashs.GetNext(pos);
					if (current_hash == pCurFile->GetAICHHashset()->GetMasterHash()){
						bFound = true;
						liUsedHashs.AddTail(current_hash);
						//theApp.QueueDebugLogLine(false, _T("%s - %s"), current_hash.GetString(), pCurFile->GetFileName());
#ifdef _DEBUG
						// in debugmode we load and verify all hashsets
						ASSERT( pCurFile->GetAICHHashset()->LoadHashSet() );
//			 			pCurFile->GetAICHHashset()->DbgTest();
						pCurFile->GetAICHHashset()->FreeHashSet();
#endif
						break;
					}
				}
				if (bFound) // hashset is available, everything fine with this file
					continue;
			}
			pCurFile->GetAICHHashset()->SetStatus(AICH_ERROR);
			m_liToHash.AddTail(pCurFile);
		}
	}
	sharelock.Unlock();

	// removed all unused AICH hashsets from known2.met
	if (!thePrefs.IsRememberingDownloadedFiles() && liUsedHashs.GetCount() != liKnown2Hashs.GetCount()){
		file.SeekToBegin();
		try {
			uint8 header = file.ReadUInt8();
			if (header != KNOWN2_MET_VERSION){
				AfxThrowFileException(CFileException::endOfFile, 0, file.GetFileName());
			}

			uint32 nExistingSize = (UINT)file.GetLength();
			uint32 nHashCount;
			ULONGLONG posWritePos = file.GetPosition();
			ULONGLONG posReadPos = file.GetPosition();
			uint32 nPurgeCount = 0;
			while (file.GetPosition() < nExistingSize){
				CAICHHash aichHash(&file);
				nHashCount = file.ReadUInt32();
				if (file.GetPosition() + nHashCount*CAICHHash::GetHashSize() > nExistingSize){
					AfxThrowFileException(CFileException::endOfFile, 0, file.GetFileName());
				}
				if (liUsedHashs.Find(aichHash) == NULL){
					// unused hashset skip the rest of this hashset
					file.Seek(nHashCount*CAICHHash::GetHashSize(), CFile::current);
					nPurgeCount++;
				}
				else if(nPurgeCount == 0){
					// used Hashset, but it does not need to be moved as nothing changed yet
					file.Seek(nHashCount*CAICHHash::GetHashSize(), CFile::current);
					posWritePos = file.GetPosition();
				}
				else{
					// used Hashset, move position in file
					BYTE* buffer = new BYTE[nHashCount*CAICHHash::GetHashSize()];
					file.Read(buffer, nHashCount*CAICHHash::GetHashSize());
					posReadPos = file.GetPosition();
					file.Seek(posWritePos, CFile::begin);
					file.Write(aichHash.GetRawHash(), CAICHHash::GetHashSize());
					file.WriteUInt32(nHashCount);
					file.Write(buffer, nHashCount*CAICHHash::GetHashSize());
					delete[] buffer;
					posWritePos = file.GetPosition();
					file.Seek(posReadPos, CFile::begin); 
				}
			}
			posReadPos = file.GetPosition();
			file.SetLength(posWritePos);
			//  Comment UI
			//theApp.QueueDebugLogLine(false, _T("Cleaned up known2.met, removed %u hashsets (%s)"), nPurgeCount, CastItoXBytes(posReadPos-posWritePos)); 

			file.Flush();
			file.Close();
		}
		catch(CFileException* error){
			if (error->m_cause == CFileException::endOfFile){
				// we just parsed this files some ms ago, should never happen here
				ASSERT( false );
			}
			else{
				TCHAR buffer[MAX_CFEXP_ERRORMSG];
				error->GetErrorMessage(buffer, ARRSIZE(buffer));
				LogError(LOG_STATUSBAR,GetResString(IDS_ERR_SERVERMET_UNKNOWN),buffer);
			}
			error->Delete();
			return false;
		}
	}
	lockKnown2Met.Unlock();
	// warn the user if he just upgraded
	if (thePrefs.IsFirstStart() && !m_liToHash.IsEmpty() && !bJustCreated){
		LogWarning(GetResString(IDS_AICH_WARNUSER));
	}	
	if (!m_liToHash.IsEmpty()){
		//  Comment UI
		//theApp.QueueLogLine(true, GetResString(IDS_AICH_SYNCTOTAL), m_liToHash.GetCount() );
		//  comment UI
		//theApp.emuledlg->sharedfileswnd->sharedfilesctrl.SetAICHHashing(m_liToHash.GetCount());
		//SendMessage( CGlobalVariable::m_hListenWnd,WM_SHAREDFILE_SETAICHHASHING,0,m_liToHash.GetCount() ); 
		if (!CGlobalVariable::IsRunning())
			return 0;
		
		UINotify(WM_SHAREDFILE_SETAICHHASHING,0,m_liToHash.GetCount() );

		// let first all normal hashing be done before starting out synchashing
		//  Comment UI (&theApp.hashing_mut)
		CSingleLock sLock1(&CGlobalVariable::hashing_mut); // only one filehash at a time
		while (CGlobalVariable::sharedfiles->GetHashingCount() != 0){
			Sleep(100);
		}
		sLock1.Lock();
		uint32 cDone = 0;
		for (POSITION pos = m_liToHash.GetHeadPosition();pos != 0; cDone++)
		{
			if (!CGlobalVariable::IsRunning()){ // in case of shutdown while still hashing
				return 0;
			}
			//  Comment UI
			//theApp.emuledlg->sharedfileswnd->sharedfilesctrl.SetAICHHashing(m_liToHash.GetCount()-cDone);
			//SendMessage( CGlobalVariable::m_hListenWnd,WM_SHAREDFILE_SETAICHHASHING,0,m_liToHash.GetCount()-cDone ); 	
			if (!CGlobalVariable::IsRunning())
			{
				return 0;
			}

			UINotify(WM_SHAREDFILE_SETAICHHASHING,0,m_liToHash.GetCount()-cDone ); 
			/*if (theApp.emuledlg->sharedfileswnd->sharedfilesctrl.m_hWnd != NULL)
				theApp.emuledlg->sharedfileswnd->sharedfilesctrl.ShowFilesCount();*/
			//SendMessage( CGlobalVariable::m_hListenWnd,WM_SHAREDFILE_SHOWCOUNT,0,0 ); 
			UINotify(WM_SHAREDFILE_SHOWCOUNT,0,0 ); 
			CKnownFile* pCurFile = m_liToHash.GetNext(pos);
			// just to be sure that the file hasnt been deleted lately
			if (!(CGlobalVariable::knownfiles->IsKnownFile(pCurFile) && CGlobalVariable::sharedfiles->GetFileByID(pCurFile->GetFileHash())) )
				continue;
			//  Comment UI
			/*theApp.QueueLogLine(false, GetResString(IDS_AICH_CALCFILE), pCurFile->GetFileName());
			if(!pCurFile->CreateAICHHashSetOnly())
				theApp.QueueDebugLogLine(false, _T("Failed to create AICH Hashset while sync. for file %s"), pCurFile->GetFileName());*/
		}

		//  Comment UI
		//theApp.emuledlg->sharedfileswnd->sharedfilesctrl.SetAICHHashing(0);
		//SendMessage( CGlobalVariable::m_hListenWnd,WM_SHAREDFILE_SETAICHHASHING,0,0 ); 
		if (!CGlobalVariable::IsRunning())
		{
			return 0;
		}

		UINotify(WM_SHAREDFILE_SETAICHHASHING,0,0 ); 
		/*if (theApp.emuledlg->sharedfileswnd->sharedfilesctrl.m_hWnd != NULL)
			theApp.emuledlg->sharedfileswnd->sharedfilesctrl.ShowFilesCount();*/
		//SendMessage( CGlobalVariable::m_hListenWnd,WM_SHAREDFILE_SHOWCOUNT,0,0 ); 
		UINotify(WM_SHAREDFILE_SHOWCOUNT,0,0 ); 
		sLock1.Unlock();
	}

	//  Comment UI
	//theApp.QueueDebugLogLine(false, _T("AICHSyncThread finished"));
	
	return 0;
}

bool CAICHSyncThread::ConvertToKnown2ToKnown264(CSafeFile* pTargetFile){
	// converting known2.met to known2_64.met to support large files
	// changing hashcount from uint16 to uint32

	// there still exists a lock on known2_64.met and it should be not opened at this point
	CString oldfullpath = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR);
	oldfullpath.Append(OLD_KNOWN2_MET_FILENAME);
	CString newfullpath = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR);
	newfullpath.Append(KNOWN2_MET_FILENAME);

	if (PathFileExists(newfullpath) || !PathFileExists(oldfullpath)){
		// only continue if the old file doe and the new file does not exists
		return false;
	}

	CSafeFile oldfile;
	CFileException fexp;

	if (!oldfile.Open(oldfullpath,CFile::modeRead|CFile::osSequentialScan|CFile::typeBinary|CFile::shareDenyNone, &fexp)){
		if (fexp.m_cause != CFileException::fileNotFound){
			CString strError(_T("Failed to load ") OLD_KNOWN2_MET_FILENAME _T(" file"));
			TCHAR szError[MAX_CFEXP_ERRORMSG];
			if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
				strError += _T(" - ");
				strError += szError;
			}
			LogError(LOG_STATUSBAR, _T("%s"), strError);
		}
		// else -> known2.met also doesn't exists, so nothing to convert
		return false;
	}


	if (!pTargetFile->Open(newfullpath,CFile::modeCreate|CFile::modeReadWrite|CFile::osSequentialScan|CFile::typeBinary|CFile::shareDenyNone, &fexp)){
		if (fexp.m_cause != CFileException::fileNotFound){
			CString strError(_T("Failed to load ") KNOWN2_MET_FILENAME _T(" file"));
			TCHAR szError[MAX_CFEXP_ERRORMSG];
			if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
				strError += _T(" - ");
				strError += szError;
			}
			LogError(LOG_STATUSBAR, _T("%s"), strError);
		}
		return false;
	}

	//  Comment UI
	//theApp.QueueLogLine(false, GetResString(IDS_CONVERTINGKNOWN2MET), OLD_KNOWN2_MET_FILENAME, KNOWN2_MET_FILENAME);

	try {
		pTargetFile->WriteUInt8(KNOWN2_MET_VERSION);
		uint32 nHashCount;
		while (oldfile.GetPosition() < oldfile.GetLength()){
			CAICHHash aichHash(&oldfile);
			nHashCount = oldfile.ReadUInt16();
			if (oldfile.GetPosition() + nHashCount*CAICHHash::GetHashSize() > oldfile.GetLength()){
				AfxThrowFileException(CFileException::endOfFile, 0, oldfile.GetFileName());
			}
			BYTE* buffer = new BYTE[nHashCount*CAICHHash::GetHashSize()];
			oldfile.Read(buffer, nHashCount*CAICHHash::GetHashSize());
			pTargetFile->Write(aichHash.GetRawHash(), CAICHHash::GetHashSize());
			pTargetFile->WriteUInt32(nHashCount);
			pTargetFile->Write(buffer, nHashCount*CAICHHash::GetHashSize());
			delete[] buffer;
		}
		pTargetFile->Flush();
		oldfile.Close();
	}
	catch(CFileException* error){
		if (error->m_cause == CFileException::endOfFile){
			LogError(LOG_STATUSBAR,GetResString(IDS_ERR_MET_BAD), OLD_KNOWN2_MET_FILENAME);
			ASSERT( false );
		}
		else{
			TCHAR buffer[MAX_CFEXP_ERRORMSG];
			error->GetErrorMessage(buffer, ARRSIZE(buffer));
			LogError(LOG_STATUSBAR,GetResString(IDS_ERR_SERVERMET_UNKNOWN),buffer);
		}
		error->Delete();
		//  Comment UI
		//theApp.QueueLogLine(false, GetResString(IDS_CONVERTINGKNOWN2FAILED));
		pTargetFile->Close();
		return false;
	}
	//  Comment UI
	//theApp.QueueLogLine(false, GetResString(IDS_CONVERTINGKNOWN2DONE));
	
	// FIXME LARGE FILES (uncomment)
	//DeleteFile(oldfullpath);
	pTargetFile->SeekToBegin();
	return true;


}
