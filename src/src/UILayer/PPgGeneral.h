/*
 * $Id: PPgGeneral.h 19812 2010-06-08 11:34:14Z dgkang $
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
#include "afxwin.h"
#include "afxcmn.h"

class CPPgGeneral : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgGeneral)

public:
	CPPgGeneral();
	virtual ~CPPgGeneral();

// Dialog Data
	enum { IDD = IDD_PPG_GENERAL };

	void Localize(void);

protected:
	UINT m_iFileBufferSize;
	CComboBox m_language;
	void LoadSettings(void);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnSetActive();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSettingsChange()					{ SetModified(); }
	afx_msg void OnBnClickedEd2kfix();
	afx_msg void OnBnClickedEditWebservices();
	afx_msg void OnLangChange();
	afx_msg void OnBnClickedCheck4Update();
	afx_msg void OnCbnCloseupLangs();
	afx_msg void OnHelp();
	afx_msg void OnWebBroswerChange();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
public:
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedDownloadbuf();
	afx_msg void OnDeltaposBufspin(NMHDR *pNMHDR, LRESULT *pResult);
	CComboBox m_CtrlCloseMode;
	afx_msg void OnCbnSelchangeCloseMode();
	afx_msg void OnBnClickedNickFrm();
	CComboBox m_DownloadBuffSizeCtrl;
public:
	afx_msg void OnCbnSelchangeComboBuf();
	afx_msg void OnCbnSelchangeComboRssInterval();
	afx_msg void OnBnClickedCheckRssAutoDownload();

// added by vc-yavey on 20100429 : config rss <begin>
private:
	// init control for config rss
	void InitRssControl();

public:
	// init rss interval combobox
	static void InitRssIntervalComboBox(CComboBox & cboInterval, UINT uCurrValue);

private:
	CComboBox	m_cboRssInterval;		///< select rss update interval
	UINT		m_uRssIntervalSecond;	///< update interval (second)
	CButton		m_btnRssAutoDownload;	///< rss autodownload checkbox
	bool		m_bRssAutoDownload;		///< if the value is true, rss item will be downloaded automatically after rss updated
// added by vc-yavey on 20100429 : config rss <end>

};
