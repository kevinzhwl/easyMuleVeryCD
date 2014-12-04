// RssInfoWnd.cpp : 实现文件
//

#include "stdafx.h"
#include "RssInfoWnd.h"
#include "Util.h"
#include "emuleDlg.h"

// CRssInfoWnd

IMPLEMENT_DYNAMIC(CRssInfoWnd, CWnd)

CRssInfoWnd::CRssInfoWnd()
{
	m_nCurrFeedCatalog = 0;
	m_nFeedCatalogCount = 0;

	m_strInfo = _T("");

	m_FontBold.CreateFont(14,0,0,0,FW_BOLD,FALSE,FALSE,0,0,0,0,0,0,_TEXT("tahoma"));//标题 

	m_imageRss_Info_Bg = GetPng(_T("RSS_INFOWND_BG"));
}

CRssInfoWnd::~CRssInfoWnd()
{
}


BEGIN_MESSAGE_MAP(CRssInfoWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_DESTROY()
END_MESSAGE_MAP()



// CRssInfoWnd 消息处理程序

void CRssInfoWnd::Localize()
{
	if (m_btBack && m_btPre && m_btNext)
	{
		m_btBack.SetTooltipText( GetResString(IDS_RSSINFOWND_BACK) );
		m_btPre.SetTooltipText( GetResString(IDS_RSSINFOWND_PRE) );
		m_btNext.SetTooltipText( GetResString(IDS_RSSINFOWND_NEXT) );
	}
}

int CRssInfoWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	ModifyStyle(0, WS_CLIPCHILDREN, 0);

	HCURSOR hCursor = NULL;
	hCursor = ::LoadCursor(NULL, IDC_HAND);

	CRect rcClient;
	this->GetClientRect(rcClient);

	m_btBack.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(rcClient.left,rcClient.top,rcClient.left+49,rcClient.bottom-1), this, 0);
	m_btBack.SetOwner((CWnd*)(theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_RssWnd.m_pwndRssFeed));
	m_btBack.SetListInfoBt(1);
	m_btBack.SetTransparent(RGB(255,0,255));
	m_btBack.SetTooltipText( GetResString(IDS_RSSINFOWND_BACK) );
	m_btBack.SetBtnCursor(hCursor);
	m_btBack.SetBitmaps(_T("RSS_INFOWND_BACK"), 4, 2);

	m_btPre.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(rcClient.right-108,rcClient.top,rcClient.right-59,rcClient.bottom-1), this, 0);
	m_btPre.SetOwner((CWnd*)(theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_RssWnd.m_pwndRssFeed));
	m_btPre.SetListInfoBt(2);
	m_btPre.SetTooltipText( GetResString(IDS_RSSINFOWND_PRE) );
	m_btPre.SetTransparent(RGB(255,0,255));
	m_btPre.SetBtnCursor(hCursor);
	m_btPre.SetBitmaps(_T("RSS_INFOWND_PRE"), 4, 2);

	m_btNext.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(rcClient.right-59, rcClient.top,rcClient.right-10,rcClient.bottom-1), this, 0);
	m_btNext.SetOwner((CWnd*)(theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_RssWnd.m_pwndRssFeed));
	m_btNext.SetListInfoBt(3);
	m_btNext.SetTooltipText( GetResString(IDS_RSSINFOWND_NEXT) );
	m_btNext.SetTransparent(RGB(255,0,255));
	m_btNext.SetBtnCursor(hCursor);
	m_btNext.SetBitmaps(_T("RSS_INFOWND_NEXT"), 4, 2);

	return 0;
}

void CRssInfoWnd::SetNumData(CString strGroupTitle,  CString strFeedName, int nCurr, int nCount)
{
	m_strGroupTitle = strGroupTitle;

	m_nCurrFeedCatalog = nCurr;
	m_nFeedCatalogCount = nCount;

	m_strInfo = _T("");
	m_strInfo.Format(_T("[%s][%d/%d]%s"), strGroupTitle, nCurr, nCount, strFeedName);
}

BOOL CRssInfoWnd::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	RegisterSimpleWndClass();
	return CWnd::Create(_T("SimpleWnd"), NULL, dwStyle, rect, pParentWnd, nID);
}

CxImage* CRssInfoWnd::GetPng(LPCTSTR lpszPngResource)
{
	if (NULL == lpszPngResource)
		return NULL;

	CxImage* image = new CxImage(CXIMAGE_FORMAT_PNG);
	image->LoadResource(FindResource(NULL, lpszPngResource, _T("PNG")), CXIMAGE_FORMAT_PNG);
	return image;
}

void CRssInfoWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	CRect rcClient;
	this->GetClientRect(rcClient);

	m_btPre.MoveWindow(rcClient.right-108, rcClient.top, 49, rcClient.Height());
	m_btNext.MoveWindow(rcClient.right-59, rcClient.top, 49, rcClient.Height());
	// TODO: 在此处添加消息处理程序代码
}

BOOL CRssInfoWnd::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	return TRUE;
	//return CWnd::OnEraseBkgnd(pDC);
}

void CRssInfoWnd::OnPaint()
{
	CRect rectClip;
	CPaintDC dcc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CWnd::OnPaint()

	CRect rectClient; 
	GetClientRect(&rectClient); //获取客户区大小

	CClientDC dc(this); //背景图 获取客户区DC
	dc.GetClipBox(rectClip);

	CDC dcMem; //定义一个显示设备对象
	dcMem.CreateCompatibleDC(NULL);//随后建立与屏幕显示兼容的内存显示设备

	CBitmap MemBitmap;//定义一个位图对象(画布)
	MemBitmap.CreateCompatibleBitmap(&dc,rectClient.Width(),rectClient.Height());

	//将位图选入到内存显示设备中
	CBitmap *pOldBit = dcMem.SelectObject(&MemBitmap);//相当于将此位图设置为画布
	dcMem.SetBkMode(TRANSPARENT);

	//CBrush *pBKBrush = new CBrush( RGB(0xf3,0xf3,0xf3/*246, 252, 251*/) );
	//dcMem.FillRect( rectClient, pBKBrush );//填充客户区背景色
	//delete pBKBrush;

	if (m_imageRss_Info_Bg)
	{
		m_imageRss_Info_Bg->Draw(dcMem, rectClient.left,  rectClient.top, rectClient.Width(),  25);
	}

	dcMem.SelectObject(&m_FontBold);

	CRect rectCaption  ( rectClient.left + 56, rectClient.top + 5, rectClient.right - 110, rectClient.bottom - 3   ) ;

	dcMem.SetTextColor( RGB(0,0,0) );
	dcMem.DrawText(m_strInfo, -1, &rectCaption, DT_SINGLELINE | DT_CENTER | DT_END_ELLIPSIS );


	dc.BitBlt( rectClient.left, rectClient.top, rectClient.Width(),rectClient.Height(), &dcMem, 0,0, SRCCOPY );//最终贴图

}

void CRssInfoWnd::OnDestroy()
{
	m_FontBold.DeleteObject();

	if (m_imageRss_Info_Bg)
		delete m_imageRss_Info_Bg;

	CWnd::OnDestroy();
}
