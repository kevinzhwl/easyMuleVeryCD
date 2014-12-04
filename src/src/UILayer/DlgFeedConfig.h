#pragma once
#include "afxwin.h"


// CDlgFeedConfig 对话框

class CRssFeed;

class CDlgFeedConfig : public CDialog
{
	DECLARE_DYNAMIC(CDlgFeedConfig)

public:
	CDlgFeedConfig(CRssFeed & feed, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgFeedConfig();

// 对话框数据
	enum { IDD = IDD_RW_FEED_CONFIG };

private:
	// 更新ui上的feed信息
	void UpdateFeedInfo();

	// 本地化
	void Localize();

private:
	// 更新ui上的feed大小信息
	void UpdateFeedSizeInfo();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

private:
	CRssFeed &		m_feed;
	CString			m_strFeedSize;
	CString			m_strFeedName;
	CComboBox		m_cboInterval;
	CButton			m_btnAutoDownload;
	CButton			m_btnAutoRefresh;
	CStatic			m_stcShowFeedName;
	CToolTipCtrl	m_tooltipShowFeedName;
	CFont *			m_pFontSaveDir;

public:
	UINT		m_uUpdateInterval;
	UINT		m_uAutoDownload;

public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnEnChangeEditRwSaveDir();
	afx_msg void OnBnClickedCheckRwAutoRefresh();
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnStnClickedStaticRwShowSaveDir();

protected:
	virtual void OnOK();	
	
public:
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
};
