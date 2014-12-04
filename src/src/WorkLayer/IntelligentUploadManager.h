#pragma once

class CIntelligentUploadManager
{
public:
	CIntelligentUploadManager(void);
	virtual ~CIntelligentUploadManager(void);


	// refresh and check current state
	BOOL Refresh();

	// set every day full speed upload time span ( 0 ~ 24 )
	BOOL SetFullSpeedTimeSpan( DWORD dwStart, DWORD dwStop );
	BOOL GetFullSpeedTimeSpan( DWORD &dwStart, DWORD &dwStop );

	// set how long time static to switch computer full speed upload state 
	BOOL SetSwitchTime( DWORD dwSeconds );


	// test current limit state
	BOOL IsLimitUploadSpeed();

	BOOL IsKeyStateChange();
	BOOL IsMouseMove();

	VOID Enable();
	VOID Disable();

	BOOL IsEnabled();

	static void TimerProc(HWND hWnd,UINT nMsg,UINT nTimerid,DWORD dwTime);

private:
	DWORD m_dwStartEveryDay;
	DWORD m_dwStopEveryDay;

	// max switch to rest mode time (sec)
	DWORD m_dwSwitchTime;

	// user last press key or move mouse time
	time_t m_dwLastActiveTime;

	// last mouse position and key states
	POINT m_ptMousePosition;
	SHORT m_rgnKeyStates[ 256 ];

	BOOL m_bEnable;

};
