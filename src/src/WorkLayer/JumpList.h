#pragma once
#ifdef HAVE_WIN7_SDK_H

#define OP_STOPALL		0
#define OP_STARTALL		1
#define OP_EXIT			2
#define OP_NEWTASK		3
#define OP_OPENVERYCD	4
#define OP_END			5

#define sz_STOPALL		_T("-stopall")
#define sz_STARTALL		_T("-startall")
#define sz_EXIT			_T("-exit")
#define sz_NEWTASK		_T("-newtask")
#define sz_OPENVERYCD	_T("-openverycd")

#define WM_JUMPLIST_APP WM_USER + 5421


class JumpList
{

public:
	static const UINT MAX_RECENT_NUM = 20;

	JumpList();
	virtual ~JumpList();

public:
	void UpdateJumpList();
	void OnDownloadComplete(LPBYTE lpFileID);
	HRESULT AddCategoryToList(ICustomDestinationList *pcdl, IObjectArray *poaRemoved);
	HRESULT AddTasksToList(ICustomDestinationList *pcdl);
	BOOL Load();
	BOOL Save();
	CString GetIconFromRes(LPCWSTR lpRes,LPCWSTR lpIconName);
	void Remove(CString strMD4);
private:
	void Init();
private:
	static HRESULT CreateSeparatorLink(IShellLink **ppsl);
	static HRESULT CreateDestination(PCWSTR pszFilePath,PCWSTR pszIconFile,PCWSTR pszTitle,LPCTSTR lpszDescription,IShellLink **ppsl);
	static HRESULT CreateTaskLink(PCWSTR pszArguments, PCWSTR pszTitle, IShellLink **ppsl,PCWSTR pszIconFile = NULL,int nIndexIcon = 0,LPCTSTR lpszDescription = NULL);	
	static BOOL	   GetIconFile(PCWSTR lpszFilePath,PCWSTR lpszIconFile);

public:
	static BOOL PerformCommand(TCHAR* pCmd);
	static BOOL CheckCmdAndSendMsg(HWND hWnd);
	static BOOL ParseCmd();
	static void PauseAllTasks();
	static void StartAllTasks();
	static void Localize();

public:
	CStringList m_lsRecentCompleteTask;
	CString		m_szRecentFile;
	CString     m_szJumpIconFolder;
	BOOL		m_bLoaded;
};
extern JumpList g_Jumplist;

#endif