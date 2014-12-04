/*
 * $Id: DlgAddTask.cpp 20835 2010-11-18 10:36:09Z dgkang $
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
// DlgAddTask.cpp : 实现文件
//

#include "stdafx.h"
#include "DlgAddTask.h"
#include ".\dlgaddtask.h"
#include "otherfunctions.h"
#include "Preferences.h"
#include "UserMsgs.h"
#include "GlobalVariable.h"
#include "Util.h"
#include "CmdGotoPage.h"
#include "CmdFuncs.h"
#include "Util.h"
#include <strsafe.h>

#include "SourceURL.h"
#include "DNSManager.h"
#include "StringConversion.h"
#include "emuleDlg.h"
#include <map>
#include "FileHashKey.h"
#include "AddTaskDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

ULONG CDlgAddTask::m_uAddState = 0;

CDlgAddTask::ItemData::ItemData()
{
	cat			= 0;
	pLink		= NULL;
	pSearchFile	= NULL;
	paused		= FALSE;
}
CDlgAddTask::ItemData::~ItemData()
{
}

// CDlgAddTask 对话框
CDlgAddTask* CDlgAddTask::ms_pInstance = NULL;

IMPLEMENT_DYNAMIC(CDlgAddTask, CDialog)
CDlgAddTask::CDlgAddTask(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CDlgAddTask::IDD, pParent)
	, m_bCheckSaveDirectly(FALSE)
{
	m_bInitList = false;
}

CDlgAddTask::~CDlgAddTask()
{
}

void CDlgAddTask::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_LOCATION, m_cmbLocation);
	DDX_Control(pDX, IDC_LIST_CONTENT, m_lcContent);
	DDX_Control(pDX, IDC_EDIT_LINKS, m_editLinks);
	DDX_Check(pDX, IDC_CHECK_SAVE_DIRECTLY, m_bCheckSaveDirectly);
	DDX_Control(pDX, IDC_CHECK_SELECTALL, m_chkboxSelectAll);
	DDX_Control(pDX, IDC_CHECK_RSS_AUTO, m_chkboxRssAutoDown);
}


BEGIN_MESSAGE_MAP(CDlgAddTask, CResizableDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BN_BROWSE, OnBnClickedBnBrowse)
	ON_WM_NCDESTROY()
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_COMBO_LOCATION, OnCbnSelchangeComboLocation)
	ON_CBN_EDITCHANGE(IDC_COMBO_LOCATION, OnCbnEditchangeComboLocation)
	ON_BN_CLICKED(IDC_CHECK_SELECTALL, OnBnClickedCheckAllselect)
END_MESSAGE_MAP()

void CDlgAddTask::PopBlankTaskDlg(void)   
{
	GetInstance()->PopupDlg(TRUE);

	BOOL	bValidLinks = FALSE;
	CString strText = theApp.CopyTextFromClipboard();
	CString strLeft;
	strLeft = strText.Left(7);
	if (0 == strLeft.CompareNoCase(_T("ed2k://"))
		|| 0 == strLeft.CompareNoCase(_T("http://")))
		bValidLinks = TRUE;
	else
	{
		strLeft = strText.Left(6);
		if (0 == strLeft.CompareNoCase(_T("ftp://")))
			bValidLinks = TRUE;
	}

	if (bValidLinks)
	{
		GetInstance()->AddLinks(strText);

		//CString strRight = strText.Right(2);
		//if (strRight != _T("\r\n"))
		//	strText += _T("\r\n");

		//GetInstance()->m_editLinks.SetWindowText(strText);
		//GetInstance()->m_editLinks.SetSel(0, -1);
		//GetInstance()->m_editLinks.SetSel(-1, -1);
		//GetInstance()->m_editLinks.UpdateLinksByWindowText();
	}
}

void CDlgAddTask::AddNewTask(LPCTSTR lpszLink, int cat)
{
	CED2KFileLink			*pLink = NULL;
	CFileHashKey			key;
	CAddTaskDoc::SItem		item;

	pLink = CreateFileLinkFromUrl(lpszLink);
	if (NULL != pLink)
	{
		key = pLink->GetHashKey();
		item.strLinkText = lpszLink;
		item.bCheck = TRUE;
		item.iCategory = cat;
		GetInstance()->AddTask(key, item);

		SAFE_DELETE(pLink);
	}
}

void CDlgAddTask::AddNewTask(CED2KFileLink* pLink, int cat)
{
	CFileHashKey			key;
	CAddTaskDoc::SItem		item;

	if (NULL != pLink)
	{
		key = pLink->GetHashKey();
		pLink->GetLink(item.strLinkText);
		item.bCheck = TRUE;
		item.iCategory = cat;
		GetInstance()->AddTask(key, item);
	}
}

void CDlgAddTask::AddNewTask(CSearchFile* toadd, uint8 paused, int cat)
{
	CFileHashKey			key;
	CAddTaskDoc::SItem		item;

	if (NULL != toadd)
	{
		key = toadd->GetFileHash();
		item.strLinkText = CreateED2kLink(toadd);
		item.bCheck = TRUE;
		item.iCategory = cat;
		item.uPause = paused;
		GetInstance()->AddTask(key, item);
	}
}

void CDlgAddTask::AddNewTask(LPCTSTR lpszLink, uint8 paused, int cat)
{

	CED2KFileLink			*pLink = NULL;
	CFileHashKey			key;
	CAddTaskDoc::SItem		item;

	pLink = CreateFileLinkFromUrl(lpszLink);
	if (NULL != pLink)
	{
		key = pLink->GetHashKey();
		item.strLinkText = lpszLink;
		item.bCheck = TRUE;
		item.iCategory = cat;
		item.uPause = paused;
		GetInstance()->AddTask(key, item);

		SAFE_DELETE(pLink);
	}
}

void CDlgAddTask::AddNewUrlTask(LPCTSTR lpszUrl)
{
	GetInstance()->AddTask(lpszUrl);
}

void CDlgAddTask::AddMultiLinks(LPCTSTR lpszLinks)
{
	GetInstance()->PopupDlg(FALSE);
	GetInstance()->AddLinks(lpszLinks);
}
void CDlgAddTask::Localize(void)
{
    this->SetWindowText(GetResString(IDS_ADD_TASK));
	GetDlgItem(IDC_BN_BROWSE)->SetWindowText(GetResString(IDS_PW_BROWSE));
	GetDlgItem(IDC_STATIC_SAVE_LOCATION)->SetWindowText(GetResString(IDS_ADDTASKDLG_SAVE_LOCATION));
	GetDlgItem(IDC_STATIC_CONTENT)->SetWindowText(GetResString(IDS_ADDTASKDLG_CONTENTS));
	GetDlgItem(IDC_STATIC_DL_TASK)->SetWindowText(GetResString(IDS_ADDTASKDLG_DL_TASK));
	GetDlgItem(IDC_STATIC_LINKS)->SetWindowText(GetResString(IDS_ADDTASKDLG_LINKS));
	GetDlgItem(IDC_CHECK_SAVE_DIRECTLY)->SetWindowText(GetResString(IDS_ADDTASKDLG_DIRECTLY));
	GetDlgItem(IDC_STATIC_FREE_SPACE)->SetWindowText(GetResString(IDS_ADDTASKDLG_FREE_SPACE));
	GetDlgItem(IDC_CHECK_SELECTALL)->SetWindowText(GetResString(IDS_ADDTASKDLG_SELECTCLEARALL));
	GetDlgItem(IDC_CHECK_RSS_AUTO)->SetWindowText(GetResString(IDS_ADDTASKDLG_RSS_AUTODOWN));

    GetDlgItem(IDOK)->SetWindowText(GetResString(IDS_OK));
	GetDlgItem(IDCANCEL)->SetWindowText(GetResString(IDS_CANCEL));

	m_ttc.UpdateTipText(GetResString(IDS_TIP_NEEDNT_ADDTASKDLG), GetDlgItem(IDC_CHECK_SAVE_DIRECTLY));
}

void CDlgAddTask::AddTask(const CFileHashKey &key, const CAddTaskDoc::SItem &item)
{
	if (!IsDlgPopedUp())
	{
		if (!thePrefs.m_bShowNewTaskDlg)
		{
			int iState = CGlobalVariable::filemgr.GetFileState((const uchar*)&key);
			CString strPrompt;
			CString strFileName;
			m_uAddState = 1;
			switch (iState)
			{
			case FILESTATE_DOWNLOADING:   
				strFileName = CGlobalVariable::filemgr.GetFileName(item.strLinkText);
				strPrompt = GetResString(IDS_TASK_IN_DOWNLOADING);
				//strPrompt += item.strLinkText;
				strPrompt += strFileName;
				CGlobalVariable::ShowNotifier(strPrompt,TBN_IMPORTANTEVENT);
				break;				
			case FILESTATE_COMPLETED:
			case FILESTATE_HASH:
			case FILESTATE_LOCAL_SHARE:
			case FILESTATE_DOWNLOADED_SHARE:	
			case FILESTATE_SHARE_TASK_DELED:
			//case FILESTATE_ZEROSIZE_DOWNLOADED:
				strFileName = CGlobalVariable::filemgr.GetFileName(item.strLinkText);
				strPrompt = GetResString(IDS_ALREADY_DOWNLOAD);
				//strPrompt += item.strLinkText;
				strPrompt += strFileName;
				CGlobalVariable::ShowNotifier(strPrompt,TBN_IMPORTANTEVENT);
				break;			
			case FILESTATE_DELETED: 
				strFileName = CGlobalVariable::filemgr.GetFileName(item.strLinkText);
				strPrompt = GetResString(IDS_DOWN_DELED_LINKS);
				//strPrompt += item.strLinkText;	
				strPrompt += strFileName;
				if(IDNO == MessageBox(strPrompt,GetResString(IDS_CAPTION),MB_YESNO))
					break;
			case FILESTATE_NOT_EXIST:
				CED2KFileLink	*pFilelink = NULL;
				//MODIFIED by VC-yavey on 2010-04-16	<begin>
				pFilelink = CreateFileLinkFromUrl(item.strLinkText);
				if( pFilelink && pFilelink->GetSize()> OLD_MAX_EMULE_FILE_SIZE && !thePrefs.CanFSHandleLargeFiles())
				{					
					CString strMessage = GetResString(IDS_ERR_FSCANTHANDLEFILE);
					strMessage += _T("\n");
					strMessage += pFilelink->GetName();					
					::AfxMessageBox(strMessage);
					break;
				}
				//MODIFIED by VC-yavey on 2010-04-16	<end>
				SAFE_DELETE(pFilelink);
				CGlobalVariable::filemgr.NewDownloadFile(item.strLinkText, thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR), item.iCategory);
				CCmdGotoPage	cmdGotoPage;
				if(thePrefs.bringtoforeground == 1)
				    cmdGotoPage.GotoDownloading();
				break;				            
			}
			return;
		}
		else
		{
			PopupDlg();
		}
	}


	m_doc.SetItem(key, item);
}

// added by vc-yavey 20100427 : 添加订阅 <begin>
void CDlgAddTask::AddNewRssUrl(CED2KRssLink & link)
{
	CDlgAddTask * pDlg = GetInstance();
	if ( pDlg == NULL )
	{
		return;
	}

	if ( !pDlg->IsDlgPopedUp() )
	{
		if (!thePrefs.m_bShowNewTaskDlg)
		{
			BOOL bAutoDown = thePrefs.m_bRssAutoDownload;
			CGlobalVariable::s_wndRssCtrl->AddFeed(link.GetRssUrl(), _T(""), bAutoDown);
		}
		else
		{
			CString  lnk;
			link.GetLink(lnk);
			pDlg->PopupDlg();
			pDlg->m_editLinks.SetWindowText(lnk);
			pDlg->m_editLinks.UpdateLinksByWindowText();
		}
		CCmdGotoPage().GotoSubscribe();
	}
}
// added by vc-yavey 20100427 : 添加订阅 <end>


CPartFile* CDlgAddTask::SilenceAddNewTask(LPCTSTR lpszUrl, LPCTSTR lpszSaveDir /* = NULL */)
{
	CString Link(lpszUrl);
	CString tcsPrefix = Link.Left(Link.Find(_T(':')));
	CPartFile * pPartFile = NULL;
	CDownloadQueue * pQueue = CGlobalVariable::downloadqueue;

	// added by vc-yavey 20100507 : 保存到指定目录  <begin>
	CString strSaveDir = (lpszSaveDir != NULL && lpszSaveDir[0] != NULL ? lpszSaveDir : thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR));
	// added by vc-yavey 20100507 : 保存到指定目录  <begin>

	if (tcsPrefix.CompareNoCase(_T("ed2k")) == 0)
	{
		CED2KFileLink			*pLink = NULL;
		CFileHashKey			key;
		CAddTaskDoc::SItem		item;

		pLink = CreateFileLinkFromUrl(lpszUrl);
		if (NULL != pLink)
		{
			key = pLink->GetHashKey();
			item.strLinkText = lpszUrl;
			item.bCheck = TRUE;
			item.iCategory = 0;
		
			int iState = CGlobalVariable::filemgr.GetFileState((const uchar*)&key);

			if (pQueue)
				pPartFile = pQueue->GetFileByID((const uchar*)&key);

			CString strPrompt;
			CString strFileName;
			m_uAddState = 1;
			switch (iState)
			{
			case FILESTATE_DOWNLOADING:   
				strFileName = CGlobalVariable::filemgr.GetFileName(item.strLinkText);
				strPrompt = GetResString(IDS_TASK_IN_DOWNLOADING);
				strPrompt += strFileName;
				CGlobalVariable::ShowNotifier(strPrompt,TBN_IMPORTANTEVENT);
				break;				
			case FILESTATE_COMPLETED:
			case FILESTATE_HASH:
			case FILESTATE_LOCAL_SHARE:
			case FILESTATE_DOWNLOADED_SHARE:	
			case FILESTATE_SHARE_TASK_DELED:
				strFileName = CGlobalVariable::filemgr.GetFileName(item.strLinkText);
				strPrompt = GetResString(IDS_ALREADY_DOWNLOAD);
				strPrompt += strFileName;
				CGlobalVariable::ShowNotifier(strPrompt,TBN_IMPORTANTEVENT);
				break;			
			case FILESTATE_DELETED: 
				/*
				strFileName = CGlobalVariable::filemgr.GetFileName(item.strLinkText);
				strPrompt = GetResString(IDS_DOWN_DELED_LINKS);
				strPrompt += strFileName;
				break;				
				if(IDNO == MessageBox(strPrompt,GetResString(IDS_CAPTION),MB_YESNO))
					break;
				*/
			case FILESTATE_NOT_EXIST:
				// MODIFIED by VC-yavey on 2010-04-16	<begin>
				// 安全地创建filelink
				CED2KFileLink	*pFilelink = NULL;
				pFilelink = CreateFileLinkFromUrl(item.strLinkText);
				if( pFilelink && pFilelink->GetSize()> OLD_MAX_EMULE_FILE_SIZE && !thePrefs.CanFSHandleLargeFiles())
				{					
					// modifyed by vc-yavey on 2010-5-6 <begin>
					// 既然是SilenceAddNewTask就不要弹框吧?
					/*
					CString strMessage = GetResString(IDS_ERR_FSCANTHANDLEFILE);
					strMessage += _T("\n");
					strMessage += pFilelink->GetName();					
					::AfxMessageBox(strMessage);
					*/

					// pFilelink需要被删除
					SAFE_DELETE(pFilelink);
					// modifyed by vc-yavey on 2010-5-6 <end>

					break;
				}
				// MODIFIED by VC-yavey on 2010-04-16	<end>
				SAFE_DELETE(pFilelink);


				// modified by vc-yavey 20100507 : 保存到指定目录 <begin>
				// CGlobalVariable::filemgr.NewDownloadFile(item.strLinkText, thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR), item.iCategory);
				CGlobalVariable::filemgr.NewDownloadFile(item.strLinkText, strSaveDir, item.iCategory);
				// modified by vc-yavey 20100507 : 保存到指定目录 <end>
				
				// modifyed by vc-yavey on 2010-5-6 <begin>
				// 避免订阅时跳来跳去（此函数只在电驴更新和订阅时使用）
				/*
				CCmdGotoPage	cmdGotoPage;
				if(thePrefs.bringtoforeground == 1)
					cmdGotoPage.GotoDownloading();
				*/
				// modifyed by vc-yavey on 2010-5-6 <end>

				break;				            
			}
			SAFE_DELETE(pLink);
	//		m_doc.SetItem(key, item);
			return pPartFile;			
		}
	}
	else if (tcsPrefix.CollateNoCase(_T("http")) == 0)
	{
		int iState = CGlobalVariable::filemgr.GetUrlTaskState(lpszUrl);
		CString strPrompt;
		CString strFileName;
		m_uAddState = 1;

		for (POSITION pos = pQueue->filelist.GetHeadPosition();pos != 0;)
		{
			CPartFile* cur_file = pQueue->filelist.GetNext(pos);
			if (cur_file->GetPartFileURL().CompareNoCase(lpszUrl) == 0)
			{
				pPartFile = cur_file;
				break;
			}
		}

		switch (iState)
		{
		case FILESTATE_DOWNLOADING:  
			strFileName = CGlobalVariable::filemgr.GetUrlFileName(lpszUrl);
			strPrompt = GetResString(IDS_TASK_IN_DOWNLOADING);		
			strPrompt += strFileName;
			CGlobalVariable::ShowNotifier(strPrompt,TBN_IMPORTANTEVENT);
			break;				
		case FILESTATE_COMPLETED:
		case FILESTATE_HASH:
		case FILESTATE_LOCAL_SHARE:
			strFileName = CGlobalVariable::filemgr.GetUrlFileName(lpszUrl);
			strPrompt = GetResString(IDS_ALREADY_DOWNLOAD);
			strPrompt += strFileName;
			CGlobalVariable::ShowNotifier(strPrompt,TBN_IMPORTANTEVENT);
			break;
		case FILESTATE_DOWNLOADED_SHARE:
		case FILESTATE_SHARE_TASK_DELED:
		case FILESTATE_DELETED: 
		case FILESTATE_ZEROSIZE_DOWNLOADED:
			/*
			strFileName = CGlobalVariable::filemgr.GetUrlFileName(lpszUrl);
			strPrompt = GetResString(IDS_DOWN_DELED_LINKS);
			strPrompt += strFileName;
			break;
			
			if(IDNO == MessageBox(strPrompt,GetResString(IDS_CAPTION),MB_YESNO))			
				break;
			*/
		case FILESTATE_NOT_EXIST:
			// modified by vc-yavey 20100507 : 保存到指定目录 <begin>
			//CmdFuncs::ActualllyAddUrlDownload(lpszUrl,thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR));
			CmdFuncs::ActualllyAddUrlDownload(lpszUrl, strSaveDir);
			// modified by vc-yavey 20100507 : 保存到指定目录 <end>

			// modifyed by vc-yavey on 2010-5-6 <begin>
			// 避免订阅时跳来跳去（此函数只在电驴更新和订阅时使用）
			/*
			CCmdGotoPage	cmdGotoPage;
			if(thePrefs.bringtoforeground == 1)
				cmdGotoPage.GotoDownloading();
			*/
			// modifyed by vc-yavey on 2010-5-6 <end>

			break;				            
		}
		//m_doc.AppendUrl(lpszUrl);
		return pPartFile;
	}
	return pPartFile;
}

void CDlgAddTask::AddTask(LPCTSTR lpszUrl)
{
	if (!IsDlgPopedUp())
	{
		if (!thePrefs.m_bShowNewTaskDlg)
		{
			int iState = CGlobalVariable::filemgr.GetUrlTaskState(lpszUrl); //< 统一转换后再判断,避免实际是重复的Url
			CString strPrompt;
			CString strFileName;
			m_uAddState = 1;
			switch (iState)
			{
			case FILESTATE_DOWNLOADING:  
				strFileName = CGlobalVariable::filemgr.GetUrlFileName(lpszUrl);
				strPrompt = GetResString(IDS_TASK_IN_DOWNLOADING);
				//strPrompt += CString(lpszUrl);
				strPrompt += strFileName;
				CGlobalVariable::ShowNotifier(strPrompt,TBN_IMPORTANTEVENT);
				break;				
			case FILESTATE_COMPLETED:
			case FILESTATE_HASH:
			case FILESTATE_LOCAL_SHARE:
				strFileName = CGlobalVariable::filemgr.GetUrlFileName(lpszUrl);
				strPrompt = GetResString(IDS_ALREADY_DOWNLOAD);
				//strPrompt += CString(lpszUrl);
				strPrompt += strFileName;
				CGlobalVariable::ShowNotifier(strPrompt,TBN_IMPORTANTEVENT);
				break;
			case FILESTATE_DOWNLOADED_SHARE:
			case FILESTATE_SHARE_TASK_DELED:
			case FILESTATE_DELETED: 
			case FILESTATE_ZEROSIZE_DOWNLOADED:
				strFileName = CGlobalVariable::filemgr.GetUrlFileName(lpszUrl);
				strPrompt = GetResString(IDS_DOWN_DELED_LINKS);
				//strPrompt += CString(lpszUrl);
				strPrompt += strFileName;
				if(IDNO == MessageBox(strPrompt,GetResString(IDS_CAPTION),MB_YESNO))			
					break;
			case FILESTATE_NOT_EXIST:
				CmdFuncs::ActualllyAddUrlDownload(lpszUrl,thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR));
				CCmdGotoPage	cmdGotoPage;
				if(thePrefs.bringtoforeground == 1)
				   cmdGotoPage.GotoDownloading();
				break;				            
			}
			return;
		}
		else
		{
			PopupDlg();
		}
	}

	m_doc.AppendUrl(lpszUrl);
}

void CDlgAddTask::AddLinks(LPCTSTR lpszLinks)
{
	if (NULL == lpszLinks || _T('\0') == lpszLinks[0])
		return;

	CString	strText;
	m_editLinks.GetWindowText(strText);
	
	CString strRight = strText.Right(2);
	if (!strText.IsEmpty()
		&& strRight != _T("\r\n"))
		strText += _T("\r\n");
	
	if (-1 != strText.Find(lpszLinks))	// 如果已经存在则不用加了。
		return;

	strText += lpszLinks;

	strRight = strText.Right(2);
	if (strRight != _T("\r\n"))
		strText += _T("\r\n");

	m_editLinks.SetWindowText(strText);
	m_editLinks.SetSel(0, -1);
	m_editLinks.SetSel(-1, -1);
	m_editLinks.UpdateLinksByWindowText();
}

BOOL CDlgAddTask::IsDlgPopedUp(void)
{
	return (::IsWindow(m_hWnd));
}

void CDlgAddTask::PopupDlg(BOOL bBlank)
{
	if (IsDlgPopedUp())
		return;

	m_uAddState = 2; // Added by Soar Chin 09/06/2007
	Create(CDlgAddTask::IDD);
	ShowWindow(SW_SHOW);

	SetForegroundWindow();
	SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0,SWP_NOMOVE|SWP_NOSIZE);
	SetActiveWindow();
	SetFocus();

	if (!bBlank)
		GetDlgItem(IDOK)->SetFocus();
}

CDlgAddTask* CDlgAddTask::GetInstance()
{
	if (NULL == ms_pInstance)
	{
		ms_pInstance = new CDlgAddTask;
		//ms_pInstance->Create(CDlgAddTask::IDD);
		//ms_pInstance->ShowWindow(SW_SHOW);
	}
	return ms_pInstance;
}

void CDlgAddTask::LoadHistoryLocs()
{
	m_cmbLocation.ResetContent();

	int		i;
	int		iIndex;
	BOOL	bHasIncomingDir;
	BOOL	bFirstItem;
	CString	str;

	bFirstItem = TRUE;
	bHasIncomingDir = FALSE;
	int	iCount = thePrefs.GetSaveLocationsCount();
	for (i = 0; i < iCount; i++)
	{
		str  = thePrefs.GetSaveLocation(i);
		if (!str.IsEmpty())
		{
			if (str == thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR))
				bHasIncomingDir = TRUE;

			iIndex = m_cmbLocation.InsertString(-1, str);
			m_cmbLocation.SetItemData(iIndex, 0);
			if (bFirstItem)
			{
				m_cmbLocation.SetCurSel(iIndex);
				bFirstItem = FALSE;
			}
		}
	}

	if (! bHasIncomingDir)
	{
		iIndex = m_cmbLocation.InsertString(-1, thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR));
		m_cmbLocation.SetItemData(iIndex, 0);
		if (bFirstItem)
		{
			m_cmbLocation.SetCurSel(iIndex);
			bFirstItem = FALSE;
		}
	}

	iIndex = m_cmbLocation.InsertString(-1, GetResString(IDS_DELETE_HISTORY));
	m_cmbLocation.SetItemData(iIndex, 1);

}

void CDlgAddTask::SaveHistoryLocs()
{
	CString		strCurLoc;

	m_cmbLocation.GetWindowText(strCurLoc);
	thePrefs.SetSaveLocation(0, strCurLoc);

	int		iCmbIndex;
	int		iPrefIndex;
	DWORD	dwData;
	CString	strLoc;

	iPrefIndex = 1;
	iCmbIndex = 0;

	while (iPrefIndex < thePrefs.GetSaveLocationsCount()
		&& iCmbIndex < m_cmbLocation.GetCount())
	{

		dwData = m_cmbLocation.GetItemData(iCmbIndex);
		if (0 == dwData)
		{
			m_cmbLocation.GetLBText(iCmbIndex, strLoc);
			if (strLoc != strCurLoc)
			{
				thePrefs.SetSaveLocation(iPrefIndex, strLoc);
				iPrefIndex++;
			}
		}

		iCmbIndex++;
	}

	thePrefs.SaveSaveLocations();
}

int CDlgAddTask::AddLocToCmb(LPCTSTR lpszLoc)
{
	if (NULL == lpszLoc || _T('\0') == lpszLoc[0])
		return -1;

	int		i;
	int		iCount = m_cmbLocation.GetCount();
	DWORD	dwData;
	CString	strItemText;

	for (i = 0; i < iCount; i++)
	{
		dwData = m_cmbLocation.GetItemData(i);
		if (0 == dwData)
		{
			m_cmbLocation.GetLBText(i, strItemText);
			if (strItemText == lpszLoc)
				return i;
		}
	}

	return m_cmbLocation.InsertString(0, lpszLoc);
}

BOOL CDlgAddTask::CheckLocation(const CString &strLocation)
{
	if (strLocation.IsEmpty())
	{
		::AfxMessageBox(GetResString(IDS_LOC_CANNT_BE_EMPTY));
		return FALSE;
	}

	if (PathFileExists(strLocation))
	{
		return TRUE;
	}

	int iResult = SHCreateDirectoryEx(NULL, strLocation, NULL);
	if (ERROR_SUCCESS != iResult && ERROR_ALREADY_EXISTS != iResult)
	{
		::AfxMessageBox(GetResString(IDS_CANNT_CREATE_THIS_DIR));
		return FALSE;
	}

	return TRUE;
}


CString UrlConvert( CString strUrl )
{
	CString urlConverted;
	//this is another way to process ftp url Decode	
	if( (strUrl.Left(7).CompareNoCase(_T("http://")) != 0) ) //ftp...
	{

		ParseUrlString( strUrl );
		strUrl.Replace(_T('\\'),_T('/'));
	}
	else
	{		
		strUrl = URLDecode(strUrl);
		strUrl.Replace(_T('\\'),_T('/'));
		strUrl = EncodeUrlUtf8(strUrl);		
	}	
	
	return strUrl; //for http
}

BOOL CDlgAddTask::ApplyDocToTaskMgr(LPCTSTR lpszLocation, BOOL & bFeedAdded, BOOL bAutoDown)
{
	// MODIFIED by VC-yavey on 2010-04-16	<begin>
	// 处理RSS订阅url
	bFeedAdded = FALSE;
	//CAddTaskDoc::RssUrlSet & setRssUrl = m_doc.GetRssUrlSet();
	CString	strCurLoc;
	m_cmbLocation.GetWindowText(strCurLoc);
	if ( !PathFileExists(strCurLoc) )
	{
		strCurLoc.Empty();
	}

	/*for ( CAddTaskDoc::RssUrlSet::const_iterator it = setRssUrl.begin();
		  it != setRssUrl.end();
		  ++it
		)
	{
 		TRACE(TEXT("\nRssUrlSet str %s\n"), *it);	
	}*/

	set<CRssFeed*>& setLastRssFeed = m_editLinks.GetLastRssFeedSet();
	for (set<CRssFeed*>::iterator it = setLastRssFeed.begin(); it != setLastRssFeed.end(); ++it )
	{
		//TRACE(TEXT("m_editLinks.m_setLastRssfee str %s\n"), (*it)->m_strFeedUrl);
		CGlobalVariable::s_wndRssCtrl->AddFeed(*it, strCurLoc, bAutoDown);
		bFeedAdded = TRUE;
	}
	// MODIFIED by VC-yavey on 2010-04-16	<end>


	//	ed2k tasks	<begin>
	const map<CFileHashKey, CAddTaskDoc::SItem>			*pMap;
	map<CFileHashKey, CAddTaskDoc::SItem>::const_iterator		it;

	CArray<SSpDownLink*, SSpDownLink*>	arrSpDownLinks;		//special download links
	CArray<SSpDownLink*, SSpDownLink*>	arrDowningLinks;	//downloading links
	CArray<SSpDownLink*, SSpDownLink*>	arrReDownLinks;		//ask whether redownload links
	CArray<SSpDownLink*, SSpDownLink*>	arrTooBigDownLinks;
	int	iState;
	CString strFileName;

	pMap = m_doc.GetData();
	for (it = pMap->begin();
		it != pMap->end();
		it++)
	{
		if (it->second.bCheck)
		{
			CED2KFileLink	*pLink = NULL;
			// MODIFIED by VC-yavey on 2010-04-16 : 安全地创建filelink
			pLink = CreateFileLinkFromUrl(it->second.strLinkText);
			if( pLink && pLink->GetSize()> OLD_MAX_EMULE_FILE_SIZE && !thePrefs.CanFSHandleLargeFiles(lpszLocation))
			{
				strFileName = CGlobalVariable::filemgr.GetFileName(it->second.strLinkText);
				SSpDownLink*	psdl = new SSpDownLink;
				psdl->iLinkType = 0;
				psdl->strLink = it->second.strLinkText;
				psdl->iCat = it->second.iCategory;
				psdl->strName = strFileName;
				arrTooBigDownLinks.Add(psdl);
				continue;
			}
			SAFE_DELETE(pLink);

			iState = CGlobalVariable::filemgr.GetFileState(it->second.strLinkText); 
			switch (iState)
			{
			case FILESTATE_DOWNLOADING:
				{
					strFileName = CGlobalVariable::filemgr.GetFileName(it->second.strLinkText);
					SSpDownLink*	psdl = new SSpDownLink;
					psdl->iLinkType = 0;
					psdl->strLink = it->second.strLinkText;
					psdl->iCat = it->second.iCategory;
					psdl->iState = iState;
					psdl->strName = strFileName;
					arrDowningLinks.Add(psdl);					
				}
				break;
			case FILESTATE_COMPLETED:
			case FILESTATE_HASH:
			case FILESTATE_DOWNLOADED_SHARE:
			case FILESTATE_LOCAL_SHARE:
			case FILESTATE_SHARE_TASK_DELED:
				{
					strFileName = CGlobalVariable::filemgr.GetFileName(it->second.strLinkText);
					SSpDownLink*	psdl = new SSpDownLink;
					psdl->iLinkType = 0;
					psdl->strLink = it->second.strLinkText;
					psdl->iCat = it->second.iCategory;
					psdl->iState = iState;
					psdl->strName = strFileName;
					arrSpDownLinks.Add(psdl);
				}
				break;
			case FILESTATE_DELETED:
				{
					strFileName = CGlobalVariable::filemgr.GetFileName(it->second.strLinkText);
					SSpDownLink*	psdl = new SSpDownLink;
					psdl->iLinkType = 0;
					psdl->strLink = it->second.strLinkText;
					psdl->iCat = it->second.iCategory;
					psdl->iState = iState;
					psdl->strName = strFileName;
					arrReDownLinks.Add(psdl);
				}
				break;
			default:
				CGlobalVariable::filemgr.NewDownloadFile(it->second.strLinkText, lpszLocation, it->second.iCategory);
				break;
			}
		}
	}
	//	ed2k tasks	<end>

	//	Url tasks	<begin>
	CString			strUrl;
	BOOL			bCheck;

	const CMapStringToPtr*	pUrlData = m_doc.GetUrlData();
	POSITION	pos = pUrlData->GetStartPosition();
	
	while (NULL != pos)
	{
		pUrlData->GetNextAssoc(pos, strUrl, (void*&)bCheck);
        
		strUrl.Trim();

		if( _tcsrchr((LPCTSTR)strUrl+7,_T('/'))==NULL )
			strUrl += _T('/');
/*
		int len=strUrl.GetLength();
		bool state=false;

		int index = strUrl.Find(_T("//"),0);
		for(int i = index + 2;i < len;i++)
		{
			if(strUrl[i] == _T('/'))
			{
				state = true;
				break;
			}
		}
		if(state ==false)
			  strUrl+='/';*/

		if (bCheck)
		{   
			iState = CGlobalVariable::filemgr.GetUrlTaskState(strUrl); //< 统一转换后再判断,避免实际是重复的Url
			switch (iState)
			{
			case FILESTATE_DOWNLOADING:   
				{
					strFileName = CGlobalVariable::filemgr.GetUrlFileName(strUrl);
					SSpDownLink*	psdl = new SSpDownLink;
					psdl->iLinkType = 1;
					psdl->strLink = strUrl;
					psdl->iCat = 0;
					psdl->iState = iState;
					psdl->strName = strFileName;
					arrDowningLinks.Add(psdl);
				}
				break;
			case FILESTATE_COMPLETED:
			case FILESTATE_HASH:
			case FILESTATE_LOCAL_SHARE:
				{
					strFileName = CGlobalVariable::filemgr.GetUrlFileName(strUrl);
					SSpDownLink*	psdl = new SSpDownLink;
					psdl->iLinkType = 1;
					psdl->strLink = strUrl;
					psdl->iCat = 0;
					psdl->iState = iState;
					psdl->strName = strFileName;
					arrSpDownLinks.Add(psdl);
				}
				break;
			case FILESTATE_DOWNLOADED_SHARE:
			case FILESTATE_SHARE_TASK_DELED:
			case FILESTATE_DELETED: 
			case FILESTATE_ZEROSIZE_DOWNLOADED:
				{
					strFileName = CGlobalVariable::filemgr.GetUrlFileName(strUrl);
					SSpDownLink*	psdl = new SSpDownLink;
					psdl->iLinkType = 1;
					psdl->strLink = strUrl;
					psdl->iCat = 0;
					psdl->iState = iState;
					psdl->strName = strFileName;
					arrReDownLinks.Add(psdl);
				}
				break;
			default:
				CmdFuncs::ActualllyAddUrlDownload(strUrl,lpszLocation);
				break;
			}
		}
	}
	//	URL tasks	<end>

	int i;

	CString strPrompt;

	if (!arrSpDownLinks.IsEmpty()) 
	{
		strPrompt = GetResString(IDS_ALREADY_DOWNLOAD);

		for (i = 0; i < arrSpDownLinks.GetCount(); i++)
		{
			if(arrSpDownLinks[i]->iLinkType==1)
				strPrompt += arrSpDownLinks[i]->strLink;
			else
				strPrompt += arrSpDownLinks[i]->strName;
			strPrompt += _T("\n");
		}
		
		for (i = 0; i < arrSpDownLinks.GetCount(); i++)
			delete arrSpDownLinks[i];
		arrSpDownLinks.RemoveAll();
	}
	
	UINT iTooBigDownLinks = 0;
	if (!arrTooBigDownLinks.IsEmpty()) 
	{
		iTooBigDownLinks =arrTooBigDownLinks.GetCount();
		if (!strPrompt.IsEmpty())
			strPrompt += _T("\n\n");
		strPrompt = GetResString(IDS_ERR_FSCANTHANDLEFILE);
		strPrompt += _T("\n");

		for (i = 0; i < arrTooBigDownLinks.GetCount(); i++)
		{
			if(arrTooBigDownLinks[i]->iLinkType==1)
				strPrompt += arrTooBigDownLinks[i]->strLink;
			else
				strPrompt += arrTooBigDownLinks[i]->strName;
			strPrompt += _T("\n");
		}

		for (i = 0; i < arrTooBigDownLinks.GetCount(); i++)
			delete arrTooBigDownLinks[i];
		arrTooBigDownLinks.RemoveAll();
	}

	if (!arrDowningLinks.IsEmpty())   
	{
		if (!strPrompt.IsEmpty())
			strPrompt += _T("\n\n");

		strPrompt += GetResString(IDS_TASK_IN_DOWNLOADING);
		for (i = 0; i < arrDowningLinks.GetCount(); i++)
		{
			if(arrDowningLinks[i]->iLinkType==1)
				strPrompt += arrDowningLinks[i]->strLink;
			else
				strPrompt += arrDowningLinks[i]->strName;
			strPrompt += _T("\n");
		}

		for (i = 0; i < arrDowningLinks.GetCount(); i++)
			delete arrDowningLinks[i];
		arrDowningLinks.RemoveAll();
	}

	if (!strPrompt.IsEmpty())
		 MessageBox(strPrompt,GetResString(IDS_CAPTION),MB_OK|MB_ICONWARNING);
	//if (IDYES == ::AfxMessageBox(strPrompt, MB_YESNO))
	//{
	//	for (i = 0; i < arrDledLink.GetCount(); i++)
	//		CGlobalVariable::filemgr.NewDownloadFile(arrDledLink[i], lpszLocation, arrDledLinkCat[i]);
	//	for (i = 0; i < arrDledUrl.GetCount(); i++)
	//		ActualllyAddUrlDownload(arrDledUrl[i], lpszLocation);
	//}

	
	if (!arrReDownLinks.IsEmpty())
	{
		strPrompt = GetResString(IDS_DOWN_DELED_LINKS);

		for (i = 0; i < arrReDownLinks.GetCount(); i++)
		{
			if(arrReDownLinks[i]->iLinkType==1)
				strPrompt += arrReDownLinks[i]->strLink;
			else
				strPrompt += arrReDownLinks[i]->strName;
			strPrompt += _T("\n");           
		}
      
		if(IDYES == MessageBox(strPrompt,GetResString(IDS_CAPTION),MB_YESNO|MB_ICONWARNING))
		{
			for (i = 0; i < arrReDownLinks.GetCount(); i++)
			{
				switch (arrReDownLinks[i]->iLinkType)
				{
				case 0:
					CGlobalVariable::filemgr.NewDownloadFile(arrReDownLinks[i]->strLink, lpszLocation, arrReDownLinks[i]->iCat);
					break;
				case 1:
					//ActualllyAddUrlDownload(arrReDownLinks[i]->strLink, lpszLocation);
					CmdFuncs::ActualllyAddUrlDownload(arrReDownLinks[i]->strLink,lpszLocation);
					break;
				default:
					break;
				}
			}
		}

		for (i = 0; i < arrReDownLinks.GetCount(); i++)
			delete arrReDownLinks[i];
		arrReDownLinks.RemoveAll();
	}
	
	if (!pMap->size() && !iTooBigDownLinks)
		return TRUE;

	return iTooBigDownLinks != pMap->size(); 
}

void CDlgAddTask::UpdateFreeSpaceValue(void)
{
	CString	strText;
	m_cmbLocation.GetWindowText(strText);

	uint64 uFreeSpace;
	if (strText.GetLength()<3 || strText.GetAt(1)!=_T(':') || strText.GetAt(2)!=_T('\\'))
		uFreeSpace = 0;
	else
		uFreeSpace = GetFreeDiskSpaceX(strText.Left(3));

	CString strSize = CastItoXBytes(uFreeSpace);
	GetDlgItem(IDC_STATIC_SPACE_VALUE)->SetWindowText(strSize);
}

// CDlgAddTask 消息处理程序
BOOL CDlgAddTask::OnInitDialog()
{
	CResizableDialog::OnInitDialog();

	AddAnchor(IDC_STATIC_DL_TASK, TOP_LEFT,MIDDLE_RIGHT);
	AddAnchor(IDC_STATIC_LINKS, TOP_LEFT);
	AddAnchor(IDC_EDIT_LINKS, TOP_LEFT,MIDDLE_RIGHT);
	AddAnchor(IDC_STATIC_SAVE_LOCATION, MIDDLE_LEFT);
	AddAnchor(IDC_COMBO_LOCATION, MIDDLE_LEFT,MIDDLE_RIGHT);
	AddAnchor(IDC_BN_BROWSE, MIDDLE_RIGHT);

	AddAnchor(IDC_STATIC_CONTENT, MIDDLE_LEFT,BOTTOM_RIGHT);
	AddAnchor(IDC_STATIC_FREE_SPACE, MIDDLE_LEFT);
	AddAnchor(IDC_STATIC_SPACE_VALUE, MIDDLE_LEFT);
	AddAnchor(IDC_LIST_CONTENT, MIDDLE_LEFT,BOTTOM_RIGHT);
	AddAnchor(IDC_CHECK_SELECTALL, BOTTOM_LEFT);
	AddAnchor(IDC_CHECK_RSS_AUTO, BOTTOM_LEFT);
	AddAnchor(IDC_CHECK_SAVE_DIRECTLY, BOTTOM_LEFT);

	AddAnchor(IDCANCEL, BOTTOM_RIGHT);
	AddAnchor(IDOK, BOTTOM_RIGHT);


	m_cmbLocation.LimitText(MAX_PATH);
	m_editLinks.SetLimitText(MAX_PATH * 256);
	m_editLinks.SetDoc(&m_doc);
	m_lcContent.SetDoc(&m_doc);
	m_doc.RegisterWnd(m_editLinks.GetSafeHwnd());
	m_doc.RegisterWnd(m_lcContent.GetSafeHwnd());

	CenterWindow();
	m_lcContent.SetExtendedStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

	m_ttc.Create(this);
	m_ttc.SetDelayTime(TTDT_AUTOPOP, INFINITE);
	m_ttc.SetDelayTime(TTDT_INITIAL, 100);
	m_ttc.AddTool(GetDlgItem(IDC_CHECK_SAVE_DIRECTLY), GetResString(IDS_TIP_NEEDNT_ADDTASKDLG));

	Localize();

	LoadHistoryLocs();

	m_lcContent.InsertColumn(0, _T(""), LVCFMT_CENTER, 22);
	m_lcContent.InsertColumn(1, GetResString(IDS_ADDTASKDLG_FILENAME), LVCFMT_LEFT, 300);
	m_lcContent.InsertColumn(2, GetResString(IDS_ADDTASKDLG_FILESIZE), LVCFMT_LEFT, 70);

	m_bInitList = true;

	m_bCheckSaveDirectly = !thePrefs.m_bShowNewTaskDlg;

	if (thePrefs.m_bRssAutoDownload)
		m_chkboxRssAutoDown.SetCheck(TRUE);
	else
		m_chkboxRssAutoDown.SetCheck(FALSE);

	//初始化隐藏显示,待edit中change再设置显示与否
	m_chkboxRssAutoDown.ShowWindow(SW_HIDE);

	UpdateData(FALSE);

	UpdateFreeSpaceValue();

	BringWindowToTopExtremely(m_hWnd);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CDlgAddTask::OnBnClickedBnBrowse()
{
	// TODO: 在此添加控件通知处理程序代码

	CString	str;
	m_cmbLocation.GetWindowText(str);
	

	TCHAR buffer[MAX_PATH] = {0};
	::StringCchCopy(buffer, MAX_PATH, str);
	if(SelectDir(GetSafeHwnd(),buffer,GetResString(IDS_CHOOSE_SAVE_LOCATION)))
	{
		m_cmbLocation.SetWindowText(buffer);

		//m_cmbLocation.SetCurSel(AddLocToCmb(buffer));
		UpdateFreeSpaceValue();
	}
}

void CDlgAddTask::OnNcDestroy()
{
	CDialog::OnNcDestroy();

	// TODO: 在此处添加消息处理程序代码
	ms_pInstance = NULL;
	m_uAddState &= ~2; // Added by Soar Chin 09/06/2007	

	delete this; // modify by nightsuns 2007/11/22: 从上面移下来
}

void CDlgAddTask::OnOK()
{
	CString	strLocation;
	m_cmbLocation.GetWindowText(strLocation);
	if (!CheckLocation(strLocation))
		return;

	UpdateData();
	thePrefs.m_bShowNewTaskDlg = !m_bCheckSaveDirectly;
	SaveHistoryLocs();

	BOOL bAutoDown = m_chkboxRssAutoDown.GetCheck();

	BOOL bFeedAdded;

	if ( GetEnvironmentVariable( L"EDIT_LINK_TEXT", NULL, 0 ) > 1 )
	{
		// quick task dlg

		( (CemuleDlg*)theApp.GetMainWnd() )->m_sDelayTaskLocation = strLocation;
		
		std::map<CFileHashKey, CAddTaskDoc::SItem>::const_iterator it = m_doc.GetData()->begin();

		for ( ; it != m_doc.GetData()->end(); it++ )
		{
			if ( it->second.bCheck )
			{
				( (CemuleDlg*)theApp.GetMainWnd() )->m_vDelayTaskLinks.push_back( it->second.strLinkText.GetString() );
			}
		}
	}
	else
	{
		if( !ApplyDocToTaskMgr(strLocation, bFeedAdded, bAutoDown) )
			return;


		// modified by vc-yavey 20100426 begin: 有订阅则移动到订阅
		if( thePrefs.bringtoforeground )
		{
			if ( bFeedAdded )
			{
				CCmdGotoPage().GotoSubscribe();
			}
			else
			{
				CCmdGotoPage().GotoDownloading();
			}
		}
		// modified by vc-yavey 20100426 end
	}

	//CDialog::OnOK();
	DestroyWindow();
	
	m_uAddState |= 1; // Added by Soar Chin 09/06/2007

	SetEnvironmentVariable( L"EDIT_LINK_TEXT", L"" );
}

void CDlgAddTask::OnCancel()
{
	// TODO: 在此添加专用代码和/或调用基类

	//CDialog::OnCancel();
	DestroyWindow();
	m_uAddState &= ~1; // Added by Soar Chin 09/06/2007

	SetEnvironmentVariable( L"EDIT_LINK_TEXT", L"" );
}

void CDlgAddTask::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
	m_uAddState &= ~2; // Added by Soar Chin 09/06/2007

	SetEnvironmentVariable( L"EDIT_LINK_TEXT", L"" );
}

void CDlgAddTask::OnCbnSelchangeComboLocation()
{
	// TODO: 在此添加控件通知处理程序代码
	DWORD dwData = m_cmbLocation.GetItemData(m_cmbLocation.GetCurSel());
	if (1 == dwData)
	{
		thePrefs.DeleteSaveLocations();
		LoadHistoryLocs();
	}
	UpdateFreeSpaceValue();
}

void CDlgAddTask::OnCbnEditchangeComboLocation()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateFreeSpaceValue();
}

BOOL CDlgAddTask::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	m_ttc.RelayEvent(pMsg);

	return CDialog::PreTranslateMessage(pMsg);
}

void CDlgAddTask::DeletedDownloadedFile(void)
{
	
}

// Added by Soar Chin 09/06/2007
BOOL CDlgAddTask::GetAddState(void)
{
	return GetInstance()->m_uAddState;
}

void CDlgAddTask::OnBnClickedCheckAllselect()
{
	if ( m_lcContent && m_lcContent.GetItemCount() > 0 )
	{
		if ( m_lcContent.CheckAllSelected() )
		{
			m_lcContent.CheckAllItems(FALSE);
		}
		else
			m_lcContent.CheckAllItems(TRUE);
	}
	else
		m_chkboxSelectAll.SetCheck(FALSE);
	// TODO: Add your control notification handler code here
}

void CDlgAddTask::OnSize(UINT nType,int cx,int cy)
{
	CResizableDialog::OnSize(nType,cx,cy);

	if (m_bInitList)
	{
		CRect rcList;
		m_lcContent.GetClientRect(&rcList);

		if (!rcList.IsRectEmpty())
		{
			int nColWidth0 = m_lcContent.GetColumnWidth(0);//取得"大小"宽度
			int nColWidth2 = m_lcContent.GetColumnWidth(2);//取得"大小"宽度
			m_lcContent.SetColumnWidth(1,rcList.Width()-nColWidth0-nColWidth2);//根据拖放自适应宽度
		}
	}
}