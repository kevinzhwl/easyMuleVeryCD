#include "StdAfx.h"
#include "intelligentuploadmanager.h"
#include "GlobalVariable.h"

CIntelligentUploadManager::CIntelligentUploadManager(void)
{
	ZeroMemory( &m_ptMousePosition, sizeof( m_ptMousePosition ) );
	m_dwLastActiveTime = time( NULL );

	m_dwSwitchTime = 1 * 60; // default is 15 minutes

	m_dwStartEveryDay = 22;
	m_dwStopEveryDay = 8;

	// init the key states
	for ( int i = 'a'; i <= 'z'; i++ )
	{
		m_rgnKeyStates[ i ] = GetKeyState( i );
	}

	for ( int i = 'A'; i <= 'Z'; i++ )
	{
		m_rgnKeyStates[ i ] = GetKeyState( i );
	}

	for ( int i = '0'; i <= '9'; i++ )
	{
		m_rgnKeyStates[ i ] = GetKeyState( i );
	}

	m_bEnable = TRUE;

	SetTimer( CGlobalVariable::m_hListenWnd, 0x100, 100, (TIMERPROC)TimerProc );
}

CIntelligentUploadManager::~CIntelligentUploadManager(void)
{
	KillTimer( CGlobalVariable::m_hListenWnd, 0x100 );
}

BOOL CIntelligentUploadManager::Refresh()
{
	if ( IsMouseMove() || IsKeyStateChange() || CGlobalVariable::downloadqueue->GetDownloadingFileCount() > 0 )
	{		
		m_dwLastActiveTime = time( NULL );
	}

	return TRUE;
}

BOOL CIntelligentUploadManager::SetFullSpeedTimeSpan( DWORD dwStart, DWORD dwStop )
{
	ASSERT( dwStart < 24 && dwStop < 24 );

	m_dwStartEveryDay = dwStart;
	m_dwStopEveryDay = dwStop;

	return TRUE;
}

BOOL CIntelligentUploadManager::SetSwitchTime( DWORD dwSeconds )
{
	m_dwSwitchTime = dwSeconds;

	return FALSE;
}

BOOL CIntelligentUploadManager::IsLimitUploadSpeed()
{
	SYSTEMTIME systime;
	
	if ( !IsEnabled() )
	{
		// is not active
		return TRUE;
	}

	GetLocalTime( &systime );

	if ( m_dwStartEveryDay == m_dwStopEveryDay )
	{

	}
	else if ( m_dwStartEveryDay < m_dwStopEveryDay )
	{
		if ( systime.wHour < m_dwStartEveryDay || systime.wHour >= m_dwStopEveryDay )
		{
			return TRUE;
		}
	}
	else
	{
		if ( systime.wHour >= m_dwStopEveryDay && systime.wHour < m_dwStartEveryDay )
		{
			return TRUE;
		}
	}

	return time( NULL ) - m_dwLastActiveTime < m_dwSwitchTime;
}

BOOL CIntelligentUploadManager::IsKeyStateChange()
{
	for ( int i = 'a'; i <= 'z'; i++ )
	{
		SHORT nKeyState = GetKeyState( i );
		if ( nKeyState != m_rgnKeyStates[ i ] )
		{
			// key state changed
			m_rgnKeyStates[ i ] = nKeyState;
			return TRUE;
		}
	}

	for ( int i = 'A'; i <= 'Z'; i++ )
	{
		SHORT nKeyState = GetKeyState( i );
		if ( nKeyState != m_rgnKeyStates[ i ] )
		{
			// key state changed
			m_rgnKeyStates[ i ] = nKeyState;
			return TRUE;
		}
	}

	for ( int i = '0'; i <= '9'; i++ )
	{
		SHORT nKeyState = GetKeyState( i );
		if ( nKeyState != m_rgnKeyStates[ i ] )
		{
			// key state changed
			m_rgnKeyStates[ i ] = nKeyState;
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CIntelligentUploadManager::IsMouseMove()
{
	POINT pt;
	if ( !GetCursorPos( &pt ) || pt.x != m_ptMousePosition.x || pt.y != m_ptMousePosition.y )
	{
		m_ptMousePosition = pt;
		return TRUE;
	}

	return FALSE;
}

void CIntelligentUploadManager::TimerProc( HWND hWnd,UINT nMsg,UINT nTimerid,DWORD dwTime )
{
	if ( CGlobalVariable::s_IntelligentUploadManager != NULL )
	{
		CGlobalVariable::s_IntelligentUploadManager->Refresh();
	}
}

VOID CIntelligentUploadManager::Enable()
{
	m_bEnable = TRUE;
}

VOID CIntelligentUploadManager::Disable()
{
	m_bEnable = FALSE;
}

BOOL CIntelligentUploadManager::IsEnabled()
{
	return m_bEnable;
}

BOOL CIntelligentUploadManager::GetFullSpeedTimeSpan( DWORD &dwStart, DWORD &dwStop )
{
	dwStart = m_dwStartEveryDay;
	dwStop = m_dwStopEveryDay;
	return TRUE;
}