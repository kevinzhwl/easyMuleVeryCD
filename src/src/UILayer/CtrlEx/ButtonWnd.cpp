// ButtonWnd.cpp : 实现文件
//

#include "stdafx.h"
#include "ButtonWnd.h"
#include "Util.h"
#include "emuleDlg.h"
// CButtonWnd

IMPLEMENT_DYNAMIC(CButtonWnd, CWnd)

CButtonWnd::CButtonWnd()
{

}

CButtonWnd::~CButtonWnd()
{
}


BEGIN_MESSAGE_MAP(CButtonWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// CButtonWnd 消息处理程序

int CButtonWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	HCURSOR hCursor = NULL;
	hCursor = ::LoadCursor(NULL, IDC_HAND);

	CRect rcClient;
	this->GetClientRect(rcClient);

	m_btIco.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(rcClient.left+37,rcClient.top+5,rcClient.left+59,rcClient.top+26), this, 0);
	m_btIco.SetWindowPos(NULL,37,5,0,0,SWP_NOZORDER | SWP_NOSIZE);//设置按钮在WND中的POS
	m_btIco.SetOwner((CWnd*)(theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_RssWnd.m_pwndRssFeed));
	m_btIco.SetListStyle(1);//设置按钮对应于IcoList
	m_btIco.SetTooltipText( GetResString(IDS_RSS_SHOWSTYLE_POSTER) );
	m_btIco.SetTransparent(RGB(255,0,255));
	m_btIco.SetBtnCursor(hCursor);
	m_btIco.SetBitmaps(_T("RSS_LISTMODE_ICO"), 4, 2);


	m_btItem.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(rcClient.left+15,rcClient.top+5,rcClient.left+37,rcClient.top+26), this, 0);
	m_btItem.SetWindowPos(NULL,15,5,0,0,SWP_NOZORDER | SWP_NOSIZE);
	m_btItem.SetOwner((CWnd*)(theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_RssWnd.m_pwndRssFeed));
	m_btItem.SetListStyle(2);//设置按钮对应于ItemList
	m_btItem.SetTooltipText( GetResString(IDS_RSS_SHOWSTYLE_REPORT) );
	m_btItem.SetTransparent(RGB(255,0,255));
	m_btItem.SetBtnCursor(hCursor);
	m_btItem.SetBitmaps(_T("RSS_LISTMODE_ITEM"), 4, 2);

	return 0;
}

BOOL CButtonWnd::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	RegisterSimpleWndClass();
	return CWnd::Create(_T("SimpleWnd"), NULL, dwStyle, rect, pParentWnd, nID);
}
BOOL CButtonWnd::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	return TRUE;
}
