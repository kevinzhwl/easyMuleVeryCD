#include "stdafx.h"
#include "emule.h"
#include "emuleDlg.h"
#include "CmdFuncs.h"
#include "TransferWnd.h"

#ifdef HAVE_WIN7_SDK_H

#include "JumpList.h"
#include <psapi.h>
#include <shlwapi.h>

#include <objectarray.h>
#include <shobjidl.h>
#include <propkey.h>
#include <propvarutil.h>
#include <knownfolders.h>
#include <shlobj.h>
#include "../CxImage/ximage.h"
#include "langids.h"
#include "Version.h"

BOOL g_OptionsParam	[OP_END];
CString g_szParam	[OP_END] = {sz_STOPALL,sz_STARTALL,sz_EXIT,sz_NEWTASK,sz_OPENVERYCD};


static CString GetFolderPath(DWORD Flags)
{
	TCHAR szPath[MAX_PATH];
	CString temp = _T("");

	if(SUCCEEDED(SHGetFolderPath(NULL, Flags | CSIDL_FLAG_CREATE, NULL, 0, szPath))) 
		temp = szPath;
	return temp;
}

static BOOL IsExistDirectory(LPCTSTR strPath)
{
	BOOL   bExist;   
	WIN32_FIND_DATA   wfd;   
	HANDLE hFind = FindFirstFile(strPath, &wfd);     
	if(hFind == INVALID_HANDLE_VALUE)
	{
		bExist = FALSE;     
	}   
	else
	{
		if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)   
			bExist = TRUE;
		else   
			bExist = FALSE;
		FindClose(hFind);   
	}   
	return   bExist;
}


static BOOL CreateMultiFolder(LPCTSTR strPath)   
{   
	BOOL bRet = TRUE;
	LPCTSTR   lpcstrParent; 

	CString cstrPath = strPath;

	CString cstrParent;   
	int iPos = 0;   
	int iLen;   

	if(cstrPath.IsEmpty())
		return FALSE;   

	iLen		= cstrPath.GetLength();   
	iPos		= cstrPath.ReverseFind('\\');   
	cstrParent  = cstrPath.Left(iPos);   

	if(cstrParent.IsEmpty())
		return FALSE;
	lpcstrParent = cstrParent.Left(cstrParent.GetLength());   
	if(cstrParent.GetLength() > 3)
		bRet = IsExistDirectory(lpcstrParent);
	if(!bRet)     
		bRet = CreateMultiFolder(lpcstrParent);
	if(bRet)
		bRet = CreateDirectory(cstrPath,   NULL);   
	return bRet;   
}



JumpList g_Jumplist;


JumpList::JumpList()
{
	m_bLoaded = FALSE;
}

JumpList::~JumpList()
{

}


BOOL JumpList::ParseCmd()
{
	for(int i = 0; i < OP_END; i++)
	{
		g_OptionsParam[i] = FALSE;
	}

	if (__argc > 1 &&  __targv[1])
	{
		if(!_tcscmp(__targv[1],sz_EXIT))
		{
			g_OptionsParam[OP_EXIT] = TRUE;
			return TRUE;
		}
		else if (!_tcscmp(__targv[1],sz_STARTALL))
		{
			g_OptionsParam[OP_STARTALL] = TRUE;
			return TRUE;
		}
		else if (!_tcscmp(__targv[1],sz_STOPALL))
		{
			g_OptionsParam[OP_STOPALL] = TRUE;
			return TRUE;
		}
		else if (!_tcscmp(__targv[1],sz_NEWTASK))
		{
			g_OptionsParam[OP_NEWTASK] = TRUE;
			return TRUE;
		}
		else if (!_tcscmp(__targv[1],sz_OPENVERYCD))
		{
			g_OptionsParam[OP_OPENVERYCD] = TRUE;
			return TRUE;
		}	
		else
		{
			return FALSE;
		}
	}
	return FALSE;
}


void JumpList::StartAllTasks()
{
	if (CGlobalVariable::IsRunning() && CGlobalVariable::downloadqueue)
	{
		CTypedPtrList<CPtrList, CPartFile*> filelist;
		for (POSITION pos = CGlobalVariable::downloadqueue->filelist.GetHeadPosition();pos != 0;)
		{
			CPartFile* cur_file = CGlobalVariable::downloadqueue->filelist.GetNext(pos);
			filelist.AddTail(cur_file);
		}

		for (POSITION pos = filelist.GetHeadPosition();pos != 0;)
		{
			CPartFile* cur_file = filelist.GetNext(pos);
			if (cur_file)
			{
				if (cur_file->GetStatus() == PS_INSUFFICIENT)
					cur_file->ResumeFileInsufficient();
				else
					cur_file->ResumeFile();	
			}
		}
		filelist.RemoveAll();
	}	
}

void JumpList::PauseAllTasks()
{
	if (CGlobalVariable::IsRunning() && CGlobalVariable::downloadqueue)
	{	
		CTypedPtrList<CPtrList, CPartFile*> filelist;
		for (POSITION pos = CGlobalVariable::downloadqueue->filelist.GetHeadPosition();pos != 0;)
		{
			CPartFile* cur_file = CGlobalVariable::downloadqueue->filelist.GetNext(pos);
			filelist.AddTail(cur_file);
		}

		for (POSITION pos = filelist.GetHeadPosition();pos != 0;)
		{
			CPartFile* cur_file = filelist.GetNext(pos);
			if (cur_file && cur_file->CanPauseFile())
				cur_file->PauseFile();			
		}
		filelist.RemoveAll();
	}
}

BOOL JumpList::PerformCommand(TCHAR* pCmd)
{
	if(!_tcscmp(pCmd,sz_EXIT))
	{
		if (theApp.emuledlg)
			theApp.emuledlg->OnClose();
		return TRUE;
	}
	else if (!_tcscmp(pCmd,sz_STARTALL))
	{
		JumpList::StartAllTasks();
		return TRUE;
	}
	else if (!_tcscmp(pCmd,sz_STOPALL))
	{
		JumpList::PauseAllTasks();
		return TRUE;
	}
	else if (!_tcscmp(pCmd,sz_NEWTASK))
	{
		CmdFuncs::PopupNewTaskDlg();
		return TRUE;
	}
	else if (!_tcscmp(pCmd,sz_OPENVERYCD))
	{
#if 0
		CString strPalam = _T("http://www.verycd.com");
		ShellExecute(NULL, _T("open"), _T("iexplore.exe"),strPalam, NULL, SW_SHOW);
#endif
		if (theApp.emuledlg && ::IsWindow(theApp.emuledlg->m_hWnd))
		{
			ForceBringWindowToTop(theApp.emuledlg->m_hWnd);
			if (thePrefs.m_bStartShowHomePage)
				CmdFuncs::SetMainActiveTab(CMainTabWnd::TI_RESOURCE);
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL JumpList::CheckCmdAndSendMsg(HWND hWnd)
{
	int nIndex = -1;
	for(int i = 0; i < OP_END; i++ )
	{
		if (g_OptionsParam[i])
		{
			nIndex = i;
			break;
		}
	}
	if (nIndex == -1)
	{
		return FALSE;
	}

	ASSERT(nIndex < OP_END);

	int nByte = 0; TCHAR* pData = NULL;

	CString szParams = g_szParam[nIndex];
	
	nByte = (szParams.GetLength() + 1) * sizeof(TCHAR);
	pData = (TCHAR*)malloc(nByte);
	_tcscpy(pData,(LPCTSTR)szParams);
	pData[szParams.GetLength()] = _T('\0');

	COPYDATASTRUCT	cds;
	ZeroMemory(&cds, sizeof(cds));
	cds.cbData = nByte;
	cds.dwData = OP_JUMPLIST;
	cds.lpData = pData;
	::SendMessage(hWnd, WM_COPYDATA, 0, (LPARAM)&cds);
	if (pData)
		free(pData);		  
	return TRUE;
}

void JumpList::Init()
{
	CString strEnvDir;
	 strEnvDir = GetFolderPath(CSIDL_COMMON_DOCUMENTS);
	if (strEnvDir.GetLength() > 0 && strEnvDir.Right(1) == _T("\\"))
		strEnvDir.SetAt(strEnvDir.GetLength() - 1,'\0');

	strEnvDir+= L"\\easyMule1.0";
	CreateMultiFolder(strEnvDir);

	m_szJumpIconFolder = strEnvDir + L"\\JumpListIco";
	CreateMultiFolder(m_szJumpIconFolder);

	m_szRecentFile = strEnvDir + L"\\RecentComplete.dat";
}

HRESULT JumpList::CreateSeparatorLink(IShellLink **ppsl)
{
	IPropertyStore *pps;
	HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pps));
	if (SUCCEEDED(hr))
	{
		PROPVARIANT propvar;
		hr = InitPropVariantFromBoolean(TRUE, &propvar);
		if (SUCCEEDED(hr))
		{
			hr = pps->SetValue(PKEY_AppUserModel_IsDestListSeparator, propvar);
			if (SUCCEEDED(hr))
			{
				hr = pps->Commit();
				if (SUCCEEDED(hr))
				{
					hr = pps->QueryInterface(IID_PPV_ARGS(ppsl));
				}
			}
			PropVariantClear(&propvar);
		}
		pps->Release();
	}
	return hr;
}

BOOL JumpList::GetIconFile(PCWSTR lpszFilePath,PCWSTR lpszIconFile)
{
	HICON hIcon = NULL;

	SHFILEINFO sfi;
	DWORD_PTR Ret = SHGetFileInfo(lpszFilePath,FILE_ATTRIBUTE_NORMAL,&sfi,sizeof(sfi),SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_SMALLICON| SHGFI_ICON );
	if (!Ret)
		return FALSE;

	HIMAGELIST hImageList = (HIMAGELIST)Ret;		
	hIcon = ImageList_GetIcon(hImageList,sfi.iIcon,ILD_NORMAL | ILD_TRANSPARENT);

	if (sfi.hIcon)
		DestroyIcon(sfi.hIcon);
	
	CxImage image(CXIMAGE_FORMAT_ICO);
	image.CreateFromHICON(hIcon);

	image.Save(lpszIconFile,CXIMAGE_FORMAT_ICO);

	return TRUE;
}


HRESULT JumpList::CreateDestination(PCWSTR pszFilePath,PCWSTR pszIconFile,PCWSTR pszTitle,LPCTSTR lpszDescription,IShellLink **ppsl)
{
	IShellLink *psl;
	HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&psl));
	if (SUCCEEDED(hr))
	{
		if (TRUE)
		{
			hr = psl->SetPath(pszFilePath);
			if (SUCCEEDED(hr))
			{
				if (lpszDescription)
					hr = psl->SetDescription(lpszDescription);
				
				if (!PathFileExists(pszIconFile))
					GetIconFile(pszFilePath,pszIconFile);
				psl->SetIconLocation(pszIconFile,0);

				if (SUCCEEDED(hr))
				{
					IPropertyStore *pps;
					hr = psl->QueryInterface(IID_PPV_ARGS(&pps));
					if (SUCCEEDED(hr))
					{
						PROPVARIANT propvar;
						hr = InitPropVariantFromString(pszTitle, &propvar);
						if (SUCCEEDED(hr))
						{
							hr = pps->SetValue(PKEY_Title, propvar);
							if (SUCCEEDED(hr))
							{
								hr = pps->Commit();
								if (SUCCEEDED(hr))
								{
									hr = psl->QueryInterface(IID_PPV_ARGS(ppsl));
								}
							}
							PropVariantClear(&propvar);
						}
						pps->Release();
					}
				}
			}
		}
		else
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
		}
		psl->Release();
	}
	return hr;
}


HRESULT JumpList::CreateTaskLink(PCWSTR pszArguments, PCWSTR pszTitle, IShellLink **ppsl,PCWSTR pszIconFile,int nIndexIcon,LPCTSTR lpszDescription)
{
	IShellLink *psl;
	HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&psl));
	if (SUCCEEDED(hr))
	{
		WCHAR szAppPath[MAX_PATH];
		if (GetModuleFileName(NULL, szAppPath, ARRAYSIZE(szAppPath)))
		{
			hr = psl->SetPath(szAppPath);
			if (SUCCEEDED(hr))
			{
				hr = psl->SetArguments(pszArguments);
				if (pszIconFile)
					hr = psl->SetIconLocation(pszIconFile,nIndexIcon);
				else
					hr = psl->SetIconLocation(szAppPath,nIndexIcon);

				if (lpszDescription)
					hr = psl->SetDescription(lpszDescription);
				if (SUCCEEDED(hr))
				{
					IPropertyStore *pps;
					hr = psl->QueryInterface(IID_PPV_ARGS(&pps));
					if (SUCCEEDED(hr))
					{
						PROPVARIANT propvar;
						hr = InitPropVariantFromString(pszTitle, &propvar);
						if (SUCCEEDED(hr))
						{
							hr = pps->SetValue(PKEY_Title, propvar);
							if (SUCCEEDED(hr))
							{
								hr = pps->Commit();
								if (SUCCEEDED(hr))
								{
									hr = psl->QueryInterface(IID_PPV_ARGS(ppsl));
								}
							}
							PropVariantClear(&propvar);
						}
						pps->Release();
					}
				}
			}
		}
		else
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
		}
		psl->Release();
	}
	return hr;
}


CString JumpList::GetIconFromRes(LPCWSTR lpRes,LPCWSTR lpIconName)
{
	CString strIconFile = m_szJumpIconFolder + L"\\" + lpIconName;
	if (!PathFileExists(strIconFile))
	{
		HICON hIcon = theApp.LoadIcon(lpRes,16,16);
		CxImage image(CXIMAGE_FORMAT_ICO);
		image.CreateFromHICON(hIcon);
		image.Save(strIconFile,CXIMAGE_FORMAT_ICO);
	}
	return strIconFile;
}


HRESULT JumpList::AddTasksToList(ICustomDestinationList *pcdl)
{
	IObjectCollection *poc;
	HRESULT hr = CoCreateInstance(CLSID_EnumerableObjectCollection, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&poc));
	if (SUCCEEDED(hr))
	{
		IShellLink * psl;

		hr = JumpList::CreateTaskLink(sz_OPENVERYCD, GetResString(IDS_VERYCD) +L" " + GetResString(IDS_RSSLIST_RES), &psl,GetIconFromRes(MAKEINTRESOURCE(IDI_ICON_VERYCD),L"VeryCDSit.ico"));
		if (SUCCEEDED(hr))
		{
			hr = poc->AddObject(psl);
			psl->Release();
		}

		if (SUCCEEDED(hr))
		{
			hr =JumpList::CreateTaskLink(sz_NEWTASK,GetResString(IDS_NEWTASK), &psl,GetIconFromRes(_T("JLNEWTASK"),L"JumplistNewTask.ico"));
			if (SUCCEEDED(hr))
			{
				hr = poc->AddObject(psl);
				psl->Release();
			}
		}
#if 0
		if (SUCCEEDED(hr))
		{
			hr = JumpList::CreateSeparatorLink(&psl);
			hr = poc->AddObject(psl);
			psl->Release();
		}
#endif

		if (SUCCEEDED(hr))
		{
			if(thePrefs.GetLanguageID() == LANGID_EN_US)
				hr = JumpList::CreateTaskLink(sz_STARTALL,  GetResString(IDS_SW_START) + L" " + GetResString(IDS_ALL), &psl,GetIconFromRes(_T("JLSTART"),L"JumpListStart.ico"));
			else
				hr = JumpList::CreateTaskLink(sz_STARTALL, GetResString(IDS_ALL) + GetResString(IDS_SW_START), &psl,GetIconFromRes(_T("JLSTART"),L"JumpListStart.ico"));
			if (SUCCEEDED(hr))
			{
				hr = poc->AddObject(psl);
				psl->Release();
			}
		}

		if (SUCCEEDED(hr))
		{
			if(thePrefs.GetLanguageID() == LANGID_EN_US)
				hr = JumpList::CreateTaskLink(sz_STOPALL,  GetResString(IDS_PAUSE) + L" " + GetResString(IDS_ALL), &psl,GetIconFromRes(_T("JLPAUSE"),L"JumpListPause.ico"));
			else
				hr = JumpList::CreateTaskLink(sz_STOPALL, GetResString(IDS_ALL) + GetResString(IDS_PAUSE), &psl,GetIconFromRes(_T("JLPAUSE"),L"JumpListPause.ico"));
			if (SUCCEEDED(hr))
			{
				hr = poc->AddObject(psl);
				psl->Release();
			}
		}

		if (SUCCEEDED(hr))
		{
			IObjectArray * poa;
			hr = poc->QueryInterface(IID_PPV_ARGS(&poa));
			if (SUCCEEDED(hr))
			{
				hr = pcdl->AddUserTasks(poa);
				poa->Release();
			}
		}
		poc->Release();
	}
	return hr;
}

HRESULT JumpList::AddCategoryToList(ICustomDestinationList *pcdl, IObjectArray *poaRemoved)
{
	IObjectCollection *poc;
	HRESULT hr = CoCreateInstance(CLSID_EnumerableObjectCollection, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&poc));
	if (SUCCEEDED(hr))
	{
		IShellLink * psl;
		uchar byFileID[16];
		POSITION Pos = m_lsRecentCompleteTask.GetTailPosition();
		BOOL bHas = FALSE;

		CStringArray strArray;
		for(; Pos != NULL && CGlobalVariable::sharedfiles;)
		{
			CString szMD4 = m_lsRecentCompleteTask.GetPrev(Pos);
			if (!szMD4.IsEmpty())
			{
				strmd4(szMD4,byFileID);
				CKnownFile* pPartFile =  CGlobalVariable::sharedfiles->GetFileByID(byFileID);
				CString strFilePath;
				CString strFileName;
				CString strIconFile;
				if (pPartFile)
				{
					strFilePath = pPartFile->GetFilePath();
					strFileName = pPartFile->GetFileName();
					strIconFile.Format(L"%s\\%s.ico",m_szJumpIconFolder,szMD4);
					hr = JumpList::CreateDestination(strFilePath,strIconFile,strFileName,strFileName,&psl);
					if (SUCCEEDED(hr))
					{
						hr = poc->AddObject(psl);
						psl->Release();
						bHas = TRUE;
					}
				}
				else
				{
					strArray.Add(szMD4);
				}
			}

		}

		if (bHas)
		{
			IObjectArray *poa;
			hr = poc->QueryInterface(IID_PPV_ARGS(&poa));
			if (SUCCEEDED(hr))
			{
				if ( ::GetUserDefaultUILanguage() == 0x0804)
					hr = pcdl->AppendCategory(L"最新完成", poa);
				else if (::GetUserDefaultUILanguage() == 0x0404 || ::GetUserDefaultUILanguage() == 0x0c0)
				{
					hr = pcdl->AppendCategory(L"最新完成",poa);
				}
				else
				{
					hr = pcdl->AppendCategory(L"Recent Complete",poa);
				}
				poa->Release();
			}
		}
		poc->Release();


		for (int i = 0; i < strArray.GetCount(); i++)
		{
			POSITION Pos = m_lsRecentCompleteTask.Find(strArray[i]);
			if (Pos != NULL)
				m_lsRecentCompleteTask.RemoveAt(Pos);
			Remove(strArray[i]);
		}
		if(strArray.GetCount() > 0)
			Save();
	}
	return hr;
}

void JumpList::UpdateJumpList()
{
	ICustomDestinationList *pcdl;
	HRESULT hr = CoCreateInstance(CLSID_DestinationList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pcdl));
	if (SUCCEEDED(hr))
	{

#if 0	
		CString strProID;
		strProID.Format(L"VeryCD.easyMule.%s",SZ_VERSION_NAME);
		CComPtr<IApplicationDestinations> pDests;
		hr = pDests.CoCreateInstance (CLSID_ApplicationDestinations,NULL, CLSCTX_INPROC_SERVER );
		if ( SUCCEEDED(hr) )
		{
			hr = pDests->SetAppID ( strProID );
			if ( SUCCEEDED(hr) )
				pDests->RemoveAllDestinations();
		}
	//	hr = pcdl->SetAppID(strProID);
#endif

		UINT cMinSlots;
		IObjectArray *poaRemoved;
		hr = pcdl->BeginList(&cMinSlots, IID_PPV_ARGS(&poaRemoved));
		if (SUCCEEDED(hr))
		{
			hr = AddCategoryToList(pcdl,poaRemoved);
			if (SUCCEEDED(hr))
			{
				hr = AddTasksToList(pcdl);
				if (SUCCEEDED(hr))
				{
					hr = pcdl->CommitList();
				}
			}
		}
		pcdl->Release();		
	}
}

void JumpList::Localize()
{
	if (g_Jumplist.m_bLoaded)
		g_Jumplist.UpdateJumpList();
}


void JumpList::OnDownloadComplete(LPBYTE lpFileID)
{
	CString strMD4;
	strMD4 = md4str(lpFileID);

	while (m_lsRecentCompleteTask.GetCount() >= JumpList::MAX_RECENT_NUM)
	{
		Remove(m_lsRecentCompleteTask.RemoveHead());
	}

	if (!m_lsRecentCompleteTask.Find(strMD4))
		m_lsRecentCompleteTask.AddTail(strMD4);
}

void JumpList::Remove(CString strMD4)
{
	CString strDelete;
	strDelete.Format(L"%s\\%s.ico",m_szJumpIconFolder,strMD4);
	::DeleteFile(strDelete);
}

BOOL JumpList::Load()
{
	Init();

	m_lsRecentCompleteTask.RemoveAll();
	CStdioFile File;
	CFileException e;
	CString strDelete;

	BOOL bChanged  = FALSE;
	if(File.Open(m_szRecentFile,CFile::modeRead, &e))
	{
		CString str;
		while(File.ReadString(str))
		{
			if (m_lsRecentCompleteTask.GetCount() <= JumpList::MAX_RECENT_NUM)
				m_lsRecentCompleteTask.AddTail(str);
			else
			{
				strDelete.Format(L"%s\\%s.ico",m_szJumpIconFolder,str);
				::DeleteFile(strDelete);
				bChanged = TRUE;
			}
		}
	}

	if (bChanged)
	{
		Save();
	}
	m_bLoaded = TRUE;
	return TRUE;
}

BOOL JumpList::Save()
{
	if (!m_lsRecentCompleteTask.IsEmpty())
	{
		CStdioFile File;
		CFileException e;
		if(!File.Open(m_szRecentFile,CFile::modeCreate | CFile::modeWrite, &e))
		{
			return FALSE;
		}

		POSITION Pos = m_lsRecentCompleteTask.GetHeadPosition();
		for(; Pos != NULL;)
		{
			CString str = m_lsRecentCompleteTask.GetNext(Pos);
			str += L"\n";
			File.WriteString(str);
		}
		File.Flush();
	}
	return TRUE;
}

#endif
