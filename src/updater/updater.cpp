// updater.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "updater.h"

#include "ProfiledUpdateThread.h"
#include ".\updater.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CUpdaterApp

BEGIN_MESSAGE_MAP(CUpdaterApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CUpdaterApp construction

CUpdaterApp::CUpdaterApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CUpdaterApp object

CUpdaterApp theApp;


// CUpdaterApp initialization

BOOL CUpdaterApp::InitInstance()
{
	CWinApp::InitInstance();
	CString sCommandLine;

	HANDLE hMutex = CreateMutex(NULL, FALSE, _T("Updater"));
	switch(GetLastError())
	{
	case ERROR_SUCCESS: // Mutes created successfully. There is no instance running
		break;

	case ERROR_ALREADY_EXISTS:	// Mutes already exists so there is a running instance of our app.
		return FALSE;

	default:	// Failed to create mutes by unknown reason
		return FALSE;
	}

	sCommandLine.Format(_T("%s"), m_lpCmdLine);
	StartApplication(sCommandLine);

	return FALSE;
}

void CUpdaterApp::StartApplication(CString sCommandLine)
{
	m_szUpdateUrl = _T("");
	if(sCommandLine.Find(PARAMETER_CHECKNEWVERSION) >= 0)
	{
		sCommandLine.MakeLower();
		if (sCommandLine.Find(_T("-beta")) >= 0)
		{
			m_szUpdateUrl = HTTP_REMOTE_ADDRES_BEAT_DOWNLOAD;
		}
		else if (sCommandLine.Find(_T("-debug")) >= 0 )
		{
			m_szUpdateUrl = HTTP_LOCAL_ADDRES_DEBUG_DOWNLOAD;
		}
		else if (sCommandLine.Find(_T("-release")) >= 0 )
		{
			m_szUpdateUrl = HTTP_REMOTE_ADDRES_REALEASE_DOWNLOAD;
		}	
		else if (sCommandLine.Find(_T("-alpha")) >= 0 )
		{
			m_szUpdateUrl = HTTP_REMOTE_ADDRES_ALPHA_DOWNLOAD;
		}
		else if (sCommandLine.Find(_T("-crack")) >= 0 )
		{
			m_szUpdateUrl = _T("http://dl.dianlv.com/update/dianlvupdate");
		}
		else if (sCommandLine.Find(_T("-english")) >= 0 )
		{
			m_szUpdateUrl = HTTP_REMOTE_ADDRES_ENGLISH_DOWNLOAD;
		}
		else
		{

		}
		if (m_szUpdateUrl.IsEmpty())
		{
			return;
		}
		CheckForUpdates();	//开始检查新版本操作
		return;
	}
	//CheckForUpdates();	//开始检查新版本操作
}

void CUpdaterApp::CheckForUpdates(void)
{
	ULONG lResult = 0;

	CProfiledUpdateThread* pProfiledUpdateThread;
	pProfiledUpdateThread = (CProfiledUpdateThread*)AfxBeginThread(RUNTIME_CLASS(CProfiledUpdateThread), THREAD_PRIORITY_NORMAL, 0, 0, NULL);

	pProfiledUpdateThread->m_bAutoDelete = FALSE;

	WaitForSingleObject(pProfiledUpdateThread->m_hThread, INFINITE);

	if (GetExitCodeThread(pProfiledUpdateThread->m_hThread, &lResult))
	{
		m_iResult = lResult;
	}	
	else
	{
		m_iResult = ERROR_UNKNOWN;
	}

	delete pProfiledUpdateThread;
}

int CUpdaterApp::ExitInstance()
{
	CWinApp::ExitInstance();
	return m_iResult;
}
