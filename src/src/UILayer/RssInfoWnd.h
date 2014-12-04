#pragma once

#include "ButtonST.h"
// CRssInfoWnd

class CxImage;

class CRssInfoWnd : public CWnd
{
	DECLARE_DYNAMIC(CRssInfoWnd)

public:
	CRssInfoWnd();
	virtual ~CRssInfoWnd();

public:
	BOOL		Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID = 0);

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();

public:
	CButtonST m_btBack;//返回按钮

	CButtonST m_btPre;//前一个
	CButtonST m_btNext;//后一个

	CString m_strGroupTitle;
	CString m_strInfo;

	int m_nCurrFeedCatalog;//当前在分组的位置
	int m_nFeedCatalogCount;//分组Feed数目

	CxImage*	m_imageRss_Info_Bg;//背景图

	CFont  m_FontBold;
public:
	void SetNumData(CString strGroupTitle, CString strFeedName, int nCurr, int nCount);
	
	//根据资源ID获取CxImage
	CxImage* GetPng(LPCTSTR lpszPngResource);

	void Localize(void);
};


