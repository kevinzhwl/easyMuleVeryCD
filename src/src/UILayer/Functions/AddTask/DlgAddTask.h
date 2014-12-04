/*
 * $Id: DlgAddTask.h 20479 2010-08-27 10:19:36Z huby $
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
#pragma once

// CDlgAddTask 对话框
#include <set>
using namespace std;

#include "resource.h"
#include "afxcmn.h"
#include "ED2KLink.h"
#include "SearchFile.h"
#include "FileHashKey.h"
#include "AddTaskDoc.h"
#include "AddTaskListCtrl.h"
#include "AddTaskLinksEdit.h"
#include "ToolTipCtrlZ.h"
#include "ResizableLib\ResizableDialog.h"
#include "afxwin.h"

class CDlgAddTask : public CResizableDialog
{
	DECLARE_DYNAMIC(CDlgAddTask)

private:
	CDlgAddTask(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgAddTask();
public:
// 对话框数据
	enum { IDD = IDD_ADDTASK };

	
	// modified by vc-yavey 20100507 : 增加指定保存目录的参数 <begin>
	static CPartFile* SilenceAddNewTask(LPCTSTR lpszUrlLink, LPCTSTR lpszSaveDir = NULL);
	// modified by vc-yavey 20100507 : 增加指定保存目录的参数 <begin>

	static void PopBlankTaskDlg(void);
	static void AddNewTask(LPCTSTR lpszLink, int cat = 0);
	static void AddNewTask(CED2KFileLink* pLink, int cat = 0);
	static void AddNewTask(CSearchFile* toadd, uint8 paused = 2, int cat = 0);
	static void AddNewTask(LPCTSTR lpszLink, uint8 paused = 2, int cat = 0);
	static void AddNewUrlTask(LPCTSTR lpszUrl);
	static void AddMultiLinks(LPCTSTR lpszLinks);

	// added by vc-yavey 20100427 : 添加订阅 <begin>
	static void	AddNewRssUrl(CED2KRssLink & link);
	// added by vc-yavey 20100427 : 添加订阅 <end>

	static BOOL GetAddState(void); // Added by Soar Chin 09/06/2007

	void	Localize(void);
	void	AddTask(const CFileHashKey &key, const CAddTaskDoc::SItem &item);
	void	AddTask(LPCTSTR lpszUrl);
	void	AddLinks(LPCTSTR lpszLinks);
	BOOL	IsDlgPopedUp(void);
	void	PopupDlg(BOOL bBlank = FALSE);

	static  void FreeInstance() { if(ms_pInstance) {delete ms_pInstance; ms_pInstance=NULL;} }

	
protected:
	static CDlgAddTask	*ms_pInstance;
	

	CToolTipCtrlZ	m_ttc;
	
	CAddTaskDoc		m_doc;

	static ULONG	m_uAddState; // Added by Soar Chin 09/06/2007
	struct ItemData
	{
		ItemData();
		~ItemData();

		int				cat;
		CED2KFileLink	*pLink;
		CSearchFile		*pSearchFile;
		uint8			paused;
		CString			strLink;
	};

	static CDlgAddTask*	GetInstance();

	void	LoadHistoryLocs();
	void	SaveHistoryLocs();
	int		AddLocToCmb(LPCTSTR lpszLoc);

	BOOL	CheckLocation(const CString &strLocation);
	BOOL	ApplyDocToTaskMgr(LPCTSTR lpszLocation, BOOL & bFeedAdded, BOOL bAutoDown);
	//void	ActualllyAddUrlDownload(const CString &strUrl, const CString & strLocation);
	
	void	UpdateFreeSpaceValue(void);

	struct SSpDownLink	//special download link (links in the states of downloading, downloaded, ...)
	{
		int			iLinkType;	// 0-ed2k, 1-url
		CString		strLink;
		int			iCat;		// Category
		int			iState;
		CString     strName;    
	};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CComboBox			m_cmbLocation;
	CAddTaskListCtrl	m_lcContent;
	CAddTaskLinksEdit	m_editLinks;
	CButton m_chkboxSelectAll;
	CButton m_chkboxRssAutoDown;

	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBnBrowse();
	afx_msg void OnNcDestroy();
protected:
	virtual void OnOK();
	virtual void OnCancel();
public:
	afx_msg void OnDestroy();
	afx_msg void OnCbnSelchangeComboLocation();
	BOOL m_bCheckSaveDirectly;
	afx_msg void OnCbnEditchangeComboLocation();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	void	DeletedDownloadedFile(void);
public:
	afx_msg void OnSize(UINT nType,int cx,int cy);
	afx_msg void OnBnClickedCheckAllselect();

private:
	bool m_bInitList;
};
