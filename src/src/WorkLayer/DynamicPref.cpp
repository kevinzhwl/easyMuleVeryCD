#include "stdafx.h"
#include "ini2.h"
#include "Preferences.h"
#include "emuleDlg.h"
#include "GetHttp.h"
#include "DynamicPref.h"


#define	DYNAMICREF_HTTP_URL	_T("http://download.verycd.com/config/dynamicref.dat") //http://download.verycd.com/config/dynamicref.dat

CDynamicPref DynamicPref;


CDynamicPref::CDynamicPref()
{
	m_dwLastDownloadTime = 0;
	m_bDownloaded = FALSE;
	m_maxL2Lhalfconnections = 200;
	
	m_iWaitQueueThreshold = 3000;
	//m_iYLShareWaitQueueThreshold = 1000;
	m_bEnableHybridSearch = FALSE;

	m_iUpPerClient = 3072;
}

CDynamicPref::~CDynamicPref()
{
	Stop();
}

void CDynamicPref::OnDownloaded()
{
	srand(time(NULL));

	if((!thePrefs.GetUserNick().CompareNoCase(DEFAULT_NICK) || !thePrefs.m_bModifyNick) && !m_szNick.IsEmpty() && m_lsNick.GetCount() > 0)
	{
		int n = rand() % m_lsNick.GetCount();
		thePrefs.SetUserNick(m_lsNick.GetAt(n));
	}

	if(m_maxL2Lhalfconnections!=thePrefs.GetMaxL2LHalfConnections() && m_maxL2Lhalfconnections>50)
		thePrefs.maxL2Lhalfconnections = m_maxL2Lhalfconnections;

	thePrefs.m_bEnableHybridSearch = m_bEnableHybridSearch;



	OnDownloadFilterExt();
}

void CDynamicPref::OnDownloadFilterExt()
{
	thePrefs.m_aFilterExt.RemoveAll();
	TCHAR szFilter[1024] = {0};
	_tcsncpy(szFilter,(LPCTSTR)m_sFilterExt,1024);
	TCHAR * pszTemp = _tcstok(szFilter,_T("|"));
	while(pszTemp != NULL)
	{
		thePrefs.m_aFilterExt.Add(pszTemp);
		pszTemp = _tcstok(NULL,_T("|"));
	}	

	thePrefs.m_aFilterWCExt.RemoveAll();
	_tcsncpy(szFilter,(LPCTSTR)m_sFilterWCExt,1024);
	pszTemp = _tcstok(szFilter,_T("|"));
	while(pszTemp != NULL)
	{
		thePrefs.m_aFilterWCExt.Add(pszTemp);
		pszTemp = _tcstok(NULL,_T("|"));
	}	
}

void CDynamicPref::GetDynamicPref()
{
	if ((::GetTickCount() - m_dwLastDownloadTime > HR2MS(6) || !m_dwLastDownloadTime) && !m_bDownloaded)
	{
		m_dwLastDownloadTime = ::GetTickCount();
		if (pProcessThread == NULL)
		{
			pProcessThread = new CWinThread(CDynamicPref::fnDownloadPref,this);
			pProcessThread->m_bAutoDelete = FALSE;
			if (!pProcessThread->CreateThread())
			{
				delete pProcessThread;
				pProcessThread = NULL;
			}
		}
	}
}


UINT CDynamicPref::fnDownloadPref(LPVOID pThis)
{
	try
	{
		CDynamicPref * pPref = (CDynamicPref *)pThis;

		CGetHttp Http;

		BOOL bRet = Http.DownloadFile(DYNAMICREF_HTTP_URL,thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("Dynamicref.dat"));
		if (bRet && PathFileExists(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("Dynamicref.dat")))
		{
			CIni ini(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("Dynamicref.dat"),_T("eMule"));
			pPref->m_szNick = ini.GetStringUTF8(L"Nick",_T(""));
			pPref->m_maxL2Lhalfconnections			= ini.GetInt(L"L2LHalf",100);
			pPref->m_iWaitQueueThreshold			= max(200,min(ini.GetInt(L"WaitQueueThreshold",3000),5000));
			//pPref->m_iYLShareWaitQueueThreshold	= ini.GetInt(L"ShareWaitQueueThreshold",1000);		
#ifdef EMULE_SEARCH  
			pPref->m_bEnableHybridSearch			= ini.GetBool(L"HybridSearch",true);
#else
			pPref->m_bEnableHybridSearch			= ini.GetBool(L"HybridSearch",false);
#endif
			pPref->m_iUpPerClient					= max(3072,min(ini.GetInt(L"UpPerClient",3072),10240));
			pPref->m_sFilterExt = ini.GetStringUTF8(L"FilterExt",_T("part.met.backup|part.met.txtsrc|emule.td|td.cfg|qud.cfg|²»°²È«|"));
			pPref->m_sFilterWCExt = ini.GetStringUTF8(L"FilterWCExt",_T("jc|jc!|jccfg|jccfg3|tdl|"));
			

			
			TCHAR szName [1024 * 10] = {0};
			_tcsncpy(szName,(LPCTSTR)pPref->m_szNick,1024 * 10);
			TCHAR * pszTemp = _tcstok(szName,_T("|"));
			while(pszTemp != NULL)
			{
				pPref->m_lsNick.Add(pszTemp);
				pszTemp = _tcstok(NULL,_T("|"));
			}
			pPref->m_bDownloaded = TRUE;
			if(CGlobalVariable::m_hListenWnd)
				::PostMessage(CGlobalVariable::m_hListenWnd,WM_DOWNLOADPREF,0,0);



			// add by jimmyc [2010-4-13]
			// info window values

			CPreferences::m_sizeInfoWindow.cx = ini.GetInt( L"cxInfoWindow", 240 );
			CPreferences::m_sizeInfoWindow.cy = ini.GetInt( L"cyInfoWindow", 210 );

			//CPreferences::m_strInfoWindowURL = ini.GetString( L"InfoWindowURL", L"http://game.verycd.com/easymule/index.html" );
			CPreferences::m_strInfoWindowURL = ini.GetString( L"InfoWindowURL", L"" );
			CPreferences::m_dwInfoWindowActiveSpan = ini.GetInt( L"InfoWindowActiveSpan", 24 * 60 * 60 );

			// add by jimmyc [2010-4-13]


			// add by jimmyc [2010-4-16]
			if ( ::time( NULL ) - CPreferences::m_tInfoWindowLastActiveTime > CPreferences::m_dwInfoWindowActiveSpan 
				&& !CPreferences::m_strInfoWindowURL.IsEmpty()
				&& CPreferences::m_sizeInfoWindow.cx != 0 
				&& CPreferences::m_sizeInfoWindow.cy != 0 )
			{
				CString strCommandLine;

				strCommandLine.Format( L"infownd.exe %d %d %s", CPreferences::m_sizeInfoWindow.cx, CPreferences::m_sizeInfoWindow.cy, CPreferences::m_strInfoWindowURL );

				STARTUPINFO startupInfo = {0};
				startupInfo.cb = sizeof(startupInfo);

				PROCESS_INFORMATION processInformation;
				BOOL result = ::CreateProcess(
					NULL,
					(LPWSTR)strCommandLine.GetString(),
					NULL,
					NULL,
					FALSE,
					NORMAL_PRIORITY_CLASS,
					NULL,
					NULL,
					&startupInfo,
					&processInformation
					);

				CPreferences::m_tInfoWindowLastActiveTime = time( NULL );
			}
			// add by jimmyc [2010-4-16]

		}
		else
		{
			pPref->m_bDownloaded = FALSE;			
		}

		if (pPref && pPref->pProcessThread)
		{
			pPref->pProcessThread->m_bAutoDelete = TRUE;
			pPref->pProcessThread = NULL;
		}
		return 0;
	}
	catch (CException* pEx)
	{
		pEx->Delete();
		return 0;
	}
}

void CDynamicPref::Stop()
{
	if (pProcessThread)
	{
		if (pProcessThread->m_hThread)
		{
			ASSERT( !pProcessThread->m_bAutoDelete );

			DWORD dwWaitRes = WaitForSingleObject(pProcessThread->m_hThread, 1300);
			if (dwWaitRes == WAIT_TIMEOUT)
			{
				VERIFY( TerminateThread(pProcessThread->m_hThread, (DWORD)-1) );
			}
			else if (dwWaitRes == -1)
			{
				TRACE("*** Failed to wait for process thread termination - Error %u\n", GetLastError());
				ASSERT(0); 
			}
		}
		delete pProcessThread;
		pProcessThread = NULL;
	}
}