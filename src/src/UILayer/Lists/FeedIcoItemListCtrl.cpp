// FeedIcoItemListCtrl.cpp : 实现文件
//

#include "stdafx.h"
#include "FeedIcoItemListCtrl.h"
#include "ximage.h"
#include "OtherFunctions.h"
#include "RssFeedWnd.h"
#include "CmdFuncs.h"
#include "DlgMaintabDownload.h"
#include "emuleDlg.h"
#include "DlgFeedConfig.h"
#include <MemDC.h>

#define ELLIPSIS_STR _T(" ... ")//标题过长 中间的省略号
#define TITLE_DOUBLE_LENGTH 320//标题长度超过此长度就会切割
#define TITLE_SINGLE_LENGTH 160//标题单行长度

#ifndef LVM_GETGROUPINFOBYINDEX
#define LVM_GETGROUPINFOBYINDEX   (LVM_FIRST + 153)
#endif
#ifndef LVM_GETGROUPCOUNT
#define LVM_GETGROUPCOUNT         (LVM_FIRST + 152)
#endif
#ifndef LVM_GETGROUPRECT
#define LVM_GETGROUPRECT          (LVM_FIRST + 98)
#endif
#ifndef LVGGR_HEADER
#define LVGGR_HEADER		      (1)
#endif
// CFeedIcoItemListCtrl

IMPLEMENT_DYNAMIC(CFeedIcoItemListCtrl, CListCtrl)

CFeedIcoItemListCtrl::CFeedIcoItemListCtrl()
{
	nGroupHeight = -1;
	m_nLastHotItemIndex = -1;
}

CFeedIcoItemListCtrl::~CFeedIcoItemListCtrl()
{
}


BEGIN_MESSAGE_MAP(CFeedIcoItemListCtrl, CListCtrl)
	ON_WM_KEYUP()
	//ON_NOTIFY_REFLECT(NM_CLICK, OnNMClick)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclkFeedIcoItemListCtrl)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, &CFeedIcoItemListCtrl::OnLvnItemchanged)
	ON_WM_CONTEXTMENU()
	ON_WM_DRAWITEM()
	ON_WM_MEASUREITEM()
	ON_NOTIFY_REFLECT(LVN_HOTTRACK, &CFeedIcoItemListCtrl::OnLvnHotTrack)
END_MESSAGE_MAP()


namespace {
	LRESULT EnableWindowTheme(HWND hwnd, LPCWSTR classList, LPCWSTR subApp, LPCWSTR idlist)
	{
		LRESULT lResult = S_FALSE;

		HRESULT (__stdcall *pSetWindowTheme)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
		HANDLE (__stdcall *pOpenThemeData)(HWND hwnd, LPCWSTR pszClassList);
		HRESULT (__stdcall *pCloseThemeData)(HANDLE hTheme);

		HMODULE hinstDll = ::LoadLibrary(_T("UxTheme.dll"));
		if (hinstDll)
		{
			(FARPROC&)pOpenThemeData = ::GetProcAddress(hinstDll, "OpenThemeData");
			(FARPROC&)pCloseThemeData = ::GetProcAddress(hinstDll, "CloseThemeData");
			(FARPROC&)pSetWindowTheme = ::GetProcAddress(hinstDll, "SetWindowTheme");
			if (pSetWindowTheme && pOpenThemeData && pCloseThemeData)
			{
				HANDLE theme = pOpenThemeData(hwnd,classList);
				if (theme!=NULL)
				{
					VERIFY(pCloseThemeData(theme)==S_OK);
					lResult = pSetWindowTheme(hwnd, subApp, idlist);
				}
			}
			::FreeLibrary(hinstDll);
		}
		return lResult;
	}

	bool IsCommonControlsEnabled()
	{
		bool commoncontrols = false;

		// Test if application has access to common controls
		HMODULE hinstDll = ::LoadLibrary(_T("comctl32.dll"));
		if (hinstDll)
		{
			DLLGETVERSIONPROC pDllGetVersion = (DLLGETVERSIONPROC)::GetProcAddress(hinstDll, "DllGetVersion");
			if (pDllGetVersion != NULL)
			{
				DLLVERSIONINFO dvi = {0};
				dvi.cbSize = sizeof(dvi);
				HRESULT hRes = pDllGetVersion ((DLLVERSIONINFO *) &dvi);
				if (SUCCEEDED(hRes))
					commoncontrols = dvi.dwMajorVersion >= 6;
			}
			::FreeLibrary(hinstDll);
		}
		return commoncontrols;
	}

	bool IsThemeEnabled()
	{
		bool XPStyle = false;
		bool (__stdcall *pIsAppThemed)();
		bool (__stdcall *pIsThemeActive)();

		// Test if operating system has themes enabled
		HMODULE hinstDll = ::LoadLibrary(_T("UxTheme.dll"));
		if (hinstDll)
		{
			(FARPROC&)pIsAppThemed = ::GetProcAddress(hinstDll, "IsAppThemed");
			(FARPROC&)pIsThemeActive = ::GetProcAddress(hinstDll,"IsThemeActive");
			if (pIsAppThemed != NULL && pIsThemeActive != NULL)
			{
				if (pIsAppThemed() && pIsThemeActive())
				{
					// Test if application has themes enabled by loading the proper DLL
					XPStyle = IsCommonControlsEnabled();
				}
			}
			::FreeLibrary(hinstDll);
		}
		return XPStyle;
	}
}


// CFeedIcoItemListCtrl 消息处理程序

void CFeedIcoItemListCtrl::PreSubclassWindow()
{
//	// Focus retangle is not painted properly without double-buffering
//#if (_WIN32_WINNT >= 0x501)//在已经自绘的情况下再设置双缓冲是画蛇添足
//	//SetExtendedStyle(LVS_EX_DOUBLEBUFFER | GetExtendedStyle());
//#endif
	//if ( thePrefs.GetWindowsVersion() == _WINVER_XP_ )
	//{
	//	SetExtendedStyle(GetExtendedStyle() | LVS_EX_TRACKSELECT);
	//}
	
//
//	// Enable Vista-look if possible
	EnableWindowTheme(GetSafeHwnd(), L"ListView", L"Explorer", NULL);
//
	CListCtrl::PreSubclassWindow();
}

void CFeedIcoItemListCtrl::Init()
{
	//SetName(_T("RSSIcoListCtrl"));

	ModifyStyle(0, WS_CLIPCHILDREN);

	m_PosterImageList.Create(100,100,ILC_COLORDDB,1,1);

	m_imageRss_PosterBg = GetPng(_T("PNG_RSS_POSTER_BG"));
	m_imageRss_PosterDefault = GetPng(_T("PNG_RSS_POSTER_DEFAULT"));
	m_imageRss_PosterSelBg = GetPng(_T("PNG_RSS_POSTER_SELECTED_BG"));
	m_imageRss_GroupBg = GetPng(_T("PNG_RSS_GROUP_BG"));

	m_imageRss_Group_Drop_Up_Nor = GetPng(_T("RSS_GROUP_DROP_UP_NORMAL"));
	m_imageRss_Group_Drop_Up_Hov = GetPng(_T("RSS_GROUP_DROP_UP_HOVER"));
	m_imageRss_Group_Drop_Down_Nor = GetPng(_T("RSS_GROUP_DROP_DOWN_NORMAL"));
	m_imageRss_Group_Drop_Down_Hov = GetPng(_T("RSS_GROUP_DROP_DOWN_HOVER"));

	this->SetImageList(&m_PosterImageList,LVSIL_NORMAL);

	//AddImg2List(m_imageRss_PosterDefault);

	SetIconSpacing(150,150);

	m_FontBold.CreateFont(16,0,0,0,FW_BOLD,FALSE,FALSE,0,0,0,0,0,0,_TEXT("tahoma"));//标题 

	m_pMenuXP = NULL;

	EnableGroupView(TRUE);

	if ( thePrefs.GetWindowsVersion() >= _WINVER_VISTA_ )
	{
		SetGroupHeight(38);
	}
	else
	{
		SetGroupHeight(28);
	}
}

void CFeedIcoItemListCtrl::OnDestroy()
{
	if (m_imageRss_PosterBg)
		delete m_imageRss_PosterBg;

	if (m_imageRss_PosterDefault)
		delete m_imageRss_PosterDefault;

	if (m_imageRss_PosterSelBg)
		delete m_imageRss_PosterSelBg;

	if (m_imageRss_GroupBg)
		delete m_imageRss_GroupBg;

	if (m_imageRss_Group_Drop_Up_Nor)
		delete m_imageRss_Group_Drop_Up_Nor;

	if (m_imageRss_Group_Drop_Up_Hov)
		delete m_imageRss_Group_Drop_Up_Hov;

	if (m_imageRss_Group_Drop_Down_Nor)
		delete m_imageRss_Group_Drop_Down_Nor;

	if (m_imageRss_Group_Drop_Down_Hov)
		delete m_imageRss_Group_Drop_Down_Hov;

	if(m_pMenuXP)
		delete m_pMenuXP;

	m_FontBold.DeleteObject();
	CListCtrl::OnDestroy();
}

int	CFeedIcoItemListCtrl::AddImg2List(CxImage* img)
{
	int nRet = -1;

	if (img)
	{
		CImageList* mImgList = GetImageList(LVSIL_NORMAL);

		CDC* hdc = this->GetDC();

		HBITMAP m_bitmap = img->MakeBitmap(hdc->m_hDC);
		nRet = mImgList->Add(CBitmap::FromHandle(m_bitmap),CLR_NONE);

		if (hdc)
			ReleaseDC(hdc);

		if (m_bitmap) 
			DeleteObject(m_bitmap);
	}

	return nRet;
}

LRESULT CFeedIcoItemListCtrl::InsertTypeGroup(int nItem, int nGroupId, const CString& strHeader, DWORD dwState /* = LVGS_NORMAL */, DWORD dwAlign /*= LVGA_HEADER_LEFT*/)
{
	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.iGroupId = nGroupId;
	lg.state = dwState;
	lg.mask = LVGF_GROUPID | LVGF_HEADER | LVGF_STATE | LVGF_ALIGN;
	lg.uAlign = dwAlign;

	// Header-title must be unicode (Convert if necessary)
#ifdef UNICODE
	lg.pszHeader = (LPWSTR)(LPCTSTR)strHeader;
	lg.cchHeader = strHeader.GetLength();
#else
	CComBSTR header = strHeader;
	lg.pszHeader = header;
	lg.cchHeader = header.Length();
#endif

	return InsertGroup(nItem, (PLVGROUP)&lg );
}

void CFeedIcoItemListCtrl::SetGroupHeight(int nHeight)
{
	LVGROUPMETRICS metrics;
	ZeroMemory( &metrics, sizeof(LVGROUPMETRICS) );
	metrics.cbSize = sizeof( LVGROUPMETRICS );
	metrics.mask = LVGMF_BORDERSIZE;
	metrics.Top = nHeight;
	SetGroupMetrics( &metrics );
}

BOOL CFeedIcoItemListCtrl::GroupFeedByType(int nIndex, int nType)
{
	if (!IsCommonControlsEnabled())
		return FALSE;

	SetRedraw(FALSE);

	EnableGroupView(TRUE);
	if (IsGroupViewEnabled())
	{
		CString cellText = GetResString( IDS_UNKNOWN );
		bool	bNew = false;//Is new Group?

		CRssFeed* pFeed = (CRssFeed*)GetItemData(nIndex);
		
		if (pFeed == NULL)
			return FALSE;

		if ( pFeed->m_uCatalogId != FeedCatalog::INVALID_CATALOG_ID )
		{
			FeedCatalog & catalog = CFeedCatalogs::GetInstance()[pFeed->m_uCatalogId];
			ASSERT(catalog.m_uId == pFeed->m_uCatalogId);
			ASSERT(catalog.IsLeaf());

			FeedCatalog * catalogParent;
			if (catalog.m_pParent != NULL)
			{
				catalogParent = catalog.m_pParent;
				cellText = catalogParent->m_strName;
			}
		}

		int nGroupId = m_mapGroups.FindKey(cellText);
		if (nGroupId == -1)
		{
			CSimpleArray<int> nItems;
			m_mapGroups.Add(cellText, nItems);
			nGroupId = m_mapGroups.FindKey(cellText);

			bNew = true;
		}

		//必须添加不重复数据
		if (m_mapGroups.GetValueAt(nGroupId).Find(nIndex) == -1)
		{
			m_mapGroups.GetValueAt(nGroupId).Add(nIndex);
		}

		if (bNew)//new Group
		{
			DWORD dwState = LVGS_NORMAL;

			#ifdef LVGS_COLLAPSIBLE
					if (IsGroupStateEnabled())
						dwState = LVGS_COLLAPSIBLE;
			#endif

			//InsertTypeGroup(nGroupId, nGroupId, cellText, dwState);
			VERIFY(InsertTypeGroup(nGroupId, nGroupId, cellText, dwState) != -1);
		}

		VERIFY( SetItemGroupId(nIndex, nGroupId) );
	}

	SetRedraw(TRUE);
	Invalidate(TRUE);
	

	return FALSE;
}

int CFeedIcoItemListCtrl::GetGroupCounts(CString strGroupCaption)
{
	if ( m_mapGroups.GetSize() > 0 )
	{
		int nGroupId = m_mapGroups.FindKey(strGroupCaption);
		const CSimpleArray<int>& groupRows = m_mapGroups.GetValueAt(nGroupId);

		return groupRows.GetSize();
	}

	return 0;
}

void CFeedIcoItemListCtrl::RemoveAllGroupData()
{
	if ( m_mapGroups.GetSize() > 0 )
	{
		for (int nGroupId = 0; nGroupId < m_mapGroups.GetSize();  nGroupId++)
		{
			m_mapGroups.GetValueAt(nGroupId).RemoveAll();
		}
		m_mapGroups.RemoveAll();
	}
}

BOOL CFeedIcoItemListCtrl::GroupAllFeedsByType(int nType)
{
	if (!IsCommonControlsEnabled())
		return FALSE;

	SetRedraw(FALSE);

	RemoveAllGroups();
	RemoveAllGroupData();
	EnableGroupView( GetItemCount() > 0 );

	if (IsGroupViewEnabled())
	{
		// Loop through all rows and find possible groups
		for(int nItem = 0; nItem < GetItemCount(); nItem++ )
		{
			CString strCatalogName = GetResString( IDS_UNKNOWN );

			CRssFeed* pFeed = (CRssFeed*)GetItemData(nItem);

			if (pFeed == NULL)
				continue;

			if ( pFeed->m_uCatalogId != FeedCatalog::INVALID_CATALOG_ID )
			{
				FeedCatalog & catalog = CFeedCatalogs::GetInstance()[pFeed->m_uCatalogId];
				ASSERT(catalog.m_uId == pFeed->m_uCatalogId);
				ASSERT(catalog.IsLeaf());

				FeedCatalog * catalogParent;
				if (catalog.m_pParent != NULL)
				{
					catalogParent = catalog.m_pParent;
					strCatalogName = catalogParent->m_strName;
				}
			}

			int nGroupId = m_mapGroups.FindKey(strCatalogName);
			if (nGroupId == -1)
			{
				CSimpleArray<int> nItems;
				m_mapGroups.Add(strCatalogName, nItems);
				nGroupId = m_mapGroups.FindKey(strCatalogName);
			}

			//必须添加不重复数据
			if (m_mapGroups.GetValueAt(nGroupId).Find(nItem) == -1)
			{
				m_mapGroups.GetValueAt(nGroupId).Add(nItem);
			}
			
		}

		// Look through all groups and assign rows to group
		for(int nGroupId = 0; nGroupId < m_mapGroups.GetSize(); nGroupId++)
		{
			const CSimpleArray<int>& groupRows = m_mapGroups.GetValueAt(nGroupId);
			DWORD dwState = LVGS_NORMAL;

#ifdef LVGS_COLLAPSIBLE
			if (IsGroupStateEnabled())
				dwState = LVGS_COLLAPSIBLE;
#endif

			VERIFY( InsertTypeGroup(nGroupId, nGroupId, m_mapGroups.GetKeyAt(nGroupId), dwState) != -1);

			for(int groupRow = 0; groupRow < groupRows.GetSize(); ++groupRow)
			{
				VERIFY( SetItemGroupId(groupRows[groupRow], nGroupId) );
			}
		}

	}

	SetRedraw(TRUE);

	Invalidate(TRUE);
	return FALSE;
}

BOOL CFeedIcoItemListCtrl::IsGroupStateEnabled()
{
	if (!IsGroupViewEnabled())
		return FALSE;

	if (thePrefs.GetWindowsVersion() >= _WINVER_VISTA_ )
		return TRUE;

	return FALSE;
}

BOOL CFeedIcoItemListCtrl::SetItemGroupId(int nItem, int nGroupId)
{
	//OBS! Rows not assigned to a group will not show in group-view
	LVITEM lvItem = {0};
	lvItem.mask = LVIF_GROUPID;
	lvItem.iItem = nItem;
	lvItem.iSubItem = 0;
	lvItem.iGroupId = nGroupId;
	return SetItem( &lvItem );
}

int CFeedIcoItemListCtrl::GetGroupIDByItemIndex(int nIndex)
{
	LVITEM lvItem = {0};
	lvItem.mask = LVIF_GROUPID;
	lvItem.iItem = nIndex;
	lvItem.iSubItem = 0;

	int nGroupId = -1;
	if ( GetItem( &lvItem ) )
		nGroupId = lvItem.iGroupId;

	return nGroupId;
}

BOOL CFeedIcoItemListCtrl::SetItemPoster(int nItem, int nImageListId)
{
	//OBS! Rows not assigned to a group will not show in group-view
	LVITEM lvItem = {0};
	lvItem.mask = LVIF_IMAGE;
	lvItem.iItem = nItem;
	lvItem.iSubItem = 0;
	lvItem.iImage = nImageListId;
	return SetItem( &lvItem );
}

BOOL CFeedIcoItemListCtrl::UpdatePoster(CRssFeed* feed)
{
	if (feed->m_pPosterImage)
	{
		//int nItemIndex = GetItemIndex(feed);
		//int nImageId = AddImg2List(feed->m_pPosterImage);

		//SetItemPoster(nItemIndex,nImageId);

		return TRUE;
	}
	else
		return FALSE;
}

int CFeedIcoItemListCtrl::AddFeed(CRssFeed & feed, BOOL bIsNew)
{
	int nRet = -1;

	CString strCaption = GetExtentString( feed.GetDisplayName() );
	//if (feed.m_pPosterImage)
	//{
	//	int nIndex = AddImg2List(feed.m_pPosterImage);//不再使用ImgList
	//	nRet = InsertItem(GetItemCount(), strCaption, nIndex);
	//}

	nRet = InsertItem(GetItemCount(), strCaption);
	m_stringMatcher.AddItem((void*)&feed, feed.GetDisplayName());

	if (nRet >= 0)
	{
		this->SetItemData(nRet, LPARAM(&feed));

		if ( bIsNew )//新加的feed,先在UI上"Default"展示出来
		{
			GroupFeedByType(nRet,0);
		}
	}

	return nRet;
}

CString CFeedIcoItemListCtrl::GetExtentString(const CString & strCaption)
{
	CString strCaptionCopy = strCaption;

	CDC* dc = this->GetDC();
	CSize szCaption = dc->GetTextExtent(strCaptionCopy);

	if (szCaption.cx > TITLE_DOUBLE_LENGTH)//标题长度大于2行总宽度
	{
		int nLength = strCaptionCopy.GetLength();
		int nMidPos = nLength/2;//中点

		//省略号宽度
		CSize szEllipsis = dc->GetTextExtent(ELLIPSIS_STR);

		//将标题等分为2行
		CString strCaptionStart = strCaptionCopy.Mid(0, nMidPos);
		CString strCaptionEnd   = strCaptionCopy.Mid(nMidPos, nLength-1);

		//处理第1行字符串
		for (int i = nMidPos - 1; i > 0; i--)
		{
			strCaptionStart = strCaptionStart.Mid(0, i);
			CSize szMid = dc->GetTextExtent(strCaptionStart);
			if (szMid.cx + szEllipsis.cx <= TITLE_SINGLE_LENGTH)
				break;
		}

		//处理第2行字符串
		int nCaptionEndLength = strCaptionEnd.GetLength();
		for (int i = nCaptionEndLength - 1; i > 0; i--)
		{
			strCaptionEnd = strCaptionEnd.Mid(1, i);//从开头开始截取
			CSize szMid = dc->GetTextExtent(strCaptionEnd);
			if (szMid.cx <= TITLE_SINGLE_LENGTH)
				break;
		}

		strCaptionCopy = strCaptionStart + ELLIPSIS_STR + strCaptionEnd;
	}

	return strCaptionCopy;
}

void CFeedIcoItemListCtrl::RemoveItemforMap(CRssFeed & feed, int nIndex, bool bUpdate/*=false*/)
{
	if ( m_mapGroups.GetSize() > 0 )
	{
		CString strCatalogName = GetResString( IDS_UNKNOWN );//不是更新的话直接查找IDS_UNKNOWN分组
		if ( !bUpdate && feed.m_uCatalogId != FeedCatalog::INVALID_CATALOG_ID )
		{
			//查找分类名称
			FeedCatalog & catalog = CFeedCatalogs::GetInstance()[feed.m_uCatalogId];
			ASSERT(catalog.m_uId == feed.m_uCatalogId);
			ASSERT(catalog.IsLeaf());

			FeedCatalog * catalogParent;
			if (catalog.m_pParent != NULL)
			{
				catalogParent = catalog.m_pParent;
				strCatalogName = catalogParent->m_strName;
			}
		}


		//查找并删除
		int nGroupId = m_mapGroups.FindKey(strCatalogName);
		if (nGroupId == -1)
			return;

		if (m_mapGroups.GetValueAt(nGroupId).Find(nIndex) != -1)
		{
			m_mapGroups.GetValueAt(nGroupId).Remove(nIndex);

			if ( m_mapGroups.GetValueAt(nGroupId).GetSize() <= 0 && bUpdate )
			{//如果此分组中没有item时应该删除, 因为vista+ 使用系统获取的group进行绘制
				RemoveGroup(nGroupId);//从List中删除此Group
			}
		}
	}
}

void CFeedIcoItemListCtrl::RemoveFeed(CRssFeed & feed)
{
	int nIndex = GetItemIndex(&feed);

	m_stringMatcher.RemoveItem((void*)&feed);

	RemoveItemforMap(feed, nIndex);

	DeleteItem(nIndex);
}

int CFeedIcoItemListCtrl::GetItemIndex(CRssFeed* feed)
{
	int nIndex = -1;

	for (int i=0; i < GetItemCount(); i++)
	{
		CRssFeed* pFeed = (CRssFeed*)GetItemData(i);

		if (pFeed != NULL && pFeed->m_strFeedUrl == feed->m_strFeedUrl)
		{
			nIndex = i;
			break;
		}
	}

	return nIndex;
}

CxImage* CFeedIcoItemListCtrl::GetPng(LPCTSTR lpszPngResource)
{
	if (NULL == lpszPngResource)
		return NULL;

	CxImage* image = new CxImage(CXIMAGE_FORMAT_PNG);
	image->LoadResource(FindResource(NULL, lpszPngResource, _T("PNG")), CXIMAGE_FORMAT_PNG);
	return image;
}

// 取得当前选择的index
int CFeedIcoItemListCtrl::GetCurrSelectIndex()
{
	POSITION pos = GetFirstSelectedItemPosition();
	if ( pos != NULL )
	{
		return  GetNextSelectedItem(pos);
	}
	return -1;
}

// 取得feed
CRssFeed * CFeedIcoItemListCtrl::GetFeed(int iItemIndex)
{
	CRssFeed * pFeed = reinterpret_cast<CRssFeed *>(GetItemData(iItemIndex));
	if ( pFeed != NULL )
	{
		return pFeed;
	}
	return NULL;
}

int CFeedIcoItemListCtrl::GetGroupCount()
{
	int groupCount = 0;
	if (thePrefs.GetWindowsVersion() >= _WINVER_VISTA_ )
	{
		groupCount = SNDMSG((m_hWnd), LVM_GETGROUPCOUNT, (WPARAM)0, (LPARAM)0);
	}
	else
	{
		CFeedCatalogs & feedCatalogs = CFeedCatalogs::GetInstance();

		for (	CFeedCatalogs::iterator it = feedCatalogs.GetBegin();
				it != feedCatalogs.GetEnd();
				++it	)
		{
			FeedCatalog & feedCatlog = it->second;
			if ( feedCatlog.IsTop() && feedCatlog.GetFeedCount() > 0 )
			{
				groupCount++;
			}
		}
	}

	return groupCount;
}

LVGROUP CFeedIcoItemListCtrl::GetGroupInfoByIndex(int iGroupId, UINT mask)
{
	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.mask = mask;//这里只需要ID

	VERIFY( SNDMSG((m_hWnd), LVM_GETGROUPINFOBYINDEX, (WPARAM)(iGroupId), (LPARAM)(&lg)) );

	return lg;
}

// Vista SDK - ListView_GetGroupState / LVM_GETGROUPSTATE
BOOL CFeedIcoItemListCtrl::HasGroupState(int nGroupId, DWORD dwState)
{
	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.mask = LVGF_STATE;
	lg.stateMask = dwState;
	if ( GetGroupInfo(nGroupId, (PLVGROUP)&lg) == -1)
		return FALSE;

	return lg.state == dwState;
}

CRect CFeedIcoItemListCtrl::GetRectbyGroupID(int nGroupID, UINT mask)
{
	if (nGroupID >= GetGroupCount() || nGroupID < 0)
	{
		return CRect(0,0,0,0);
	}

	CRect rect(0,mask,0,0);
	VERIFY( SNDMSG((m_hWnd), LVM_GETGROUPRECT, (WPARAM)(nGroupID), (LPARAM)(RECT*)(&rect)) );

	return rect;
}

CString CFeedIcoItemListCtrl::GetGroupCaption(int nGroupId)
{
	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.iGroupId = nGroupId;
	lg.mask = LVGF_HEADER | LVGF_GROUPID;
	VERIFY( GetGroupInfo(nGroupId, (PLVGROUP)&lg) != -1 );

#ifdef UNICODE
	return lg.pszHeader;
#else
	CComBSTR header( lg.pszHeader );
	return (LPCTSTR)COLE2T(header);
#endif
}
//
//CString CFeedIcoItemListCtrl::GetGroupFooter(int nGroupId)
//{
//	LVGROUP lg = {0};
//	lg.cbSize = sizeof(lg);
//	lg.iGroupId = nGroupId;
//	lg.mask = LVGF_SUBTITLE | LVGF_GROUPID;
//	VERIFY( GetGroupInfo(nGroupId, (PLVGROUP)&lg) != -1 );
//
//#ifdef UNICODE
//	return lg.pszSubsetTitle;
//#else
//	CComBSTR header( lg.pszSubsetTitle );
//	return (LPCTSTR)COLE2T(header);
//#endif
//}
//
//BOOL CFeedIcoItemListCtrl::SetGroupFooter(int nGroupId, CString & strFooter)
//{
//	if (!IsGroupStateEnabled())
//		return FALSE;
//
//#if _WIN32_WINNT >= 0x0600
//	LVGROUP lg = {0};
//	lg.cbSize = sizeof(lg);
//	lg.mask = LVGF_SUBTITLE;
//#ifdef UNICODE
//	lg.pszSubtitle = (LPWSTR)(LPCTSTR)strFooter;
//	lg.cchSubtitle = strFooter.GetLength();
//#else
//	CComBSTR bstrSubtitle = strFooter;
//	lg.pszSubtitle = bstrSubtitle;
//	lg.cchSubtitle = bstrSubtitle.Length();
//#endif
//
//	if (SetGroupInfo(nGroupId, (PLVGROUP)&lg)==-1)
//		return FALSE;
//
//	return TRUE;
//#else
//	return FALSE;
//#endif
//
////	LVGROUP lg = {0};
////	lg.cbSize = sizeof(lg);
////	lg.mask = LVGF_FOOTER | LVGF_ALIGN;
////	lg.uAlign = LVGA_FOOTER_CENTER;
////
////#ifdef UNICODE
////	lg.pszFooter = (LPWSTR)(LPCTSTR)strFooter;
////	lg.cchFooter = strFooter.GetLength();
////#else
////	CComBSTR bstrFooter = strFooter;
////	lg.pszFooter = bstrFooter;
////	lg.cchFooter = bstrFooter.Length();
////#endif
////
////	if( SetGroupInfo(nGroupId, (PLVGROUP)&lg) != -1 )
////		return TRUE;
////
////	return FALSE;
//}

void CFeedIcoItemListCtrl::OnNMDblclkFeedIcoItemListCtrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	ASSERT(pNMHDR != NULL);
	ASSERT(pResult != 0);
	*pResult = 0;

	// Version 4.71. Pointer to an NMITEMACTIVATE structure 
	LPNMITEMACTIVATE lpnmitem = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	if ( lpnmitem->iItem == -1 )
	{
		return;
	}

	CRssFeed * pFeed = reinterpret_cast<CRssFeed *>(GetItemData(lpnmitem->iItem));
	if ( pFeed == NULL )
	{
		return;
	}

	((CRssFeedWnd*)GetParent())->JumpListInfo(pFeed, ListShowMode::LISTSTYLE_SINGLE);
}

BOOL CFeedIcoItemListCtrl::OnEraseBkgnd(CDC* pDC)
{
	return TRUE; 
}

void CFeedIcoItemListCtrl::DrawGroupInfo(CDC & dcMem, const CRect & rectClip, const CRect & rectClient)
{
	dcMem.SelectObject(&m_FontBold);

	if (thePrefs.GetWindowsVersion() >= _WINVER_VISTA_ )
	{
		int n = GetGroupCount();
		for(int i = 0 ; i < n; ++i)
		{
			CString  strGroupCaption = GetGroupCaption( i );

			CString strGroupTotals;
			
			if ( strGroupCaption == m_mapGroups.GetKeyAt(i) )
			{
				const CSimpleArray<int>& groupRows = m_mapGroups.GetValueAt(i);
				strGroupTotals.Format(_T("(共%d个)"), groupRows.GetSize());
			}

			LVGROUP  lg = GetGroupInfoByIndex(i, LVGF_GROUPID);
			CRect    rcGroup = GetRectbyGroupID(lg.iGroupId, LVGGR_HEADER);
			
			CRect rcIntersect;
			rcIntersect.IntersectRect(rectClip, rcGroup);

			//rcGroup.DeflateRect(0, 7);//缩小7像素高度
			//rcGroup.OffsetRect(0, -7);//向上偏移7像素

			if ( !rcIntersect.IsRectEmpty() )
			{
				DrawGroup(dcMem, rcGroup, strGroupCaption, strGroupTotals);
			}

#ifdef LVGS_COLLAPSIBLE
			// Maintain LVGS_COLLAPSIBLE state
			CRect rcDropDown(rcGroup.right - 20, rcGroup.top + 10, rcGroup.right - 5, rcGroup.bottom - 3);
			if (HasGroupState(i, LVGS_COLLAPSED))
			{
				if (m_imageRss_Group_Drop_Up_Nor)
				{
					m_imageRss_Group_Drop_Up_Nor->Draw(dcMem, rcDropDown.left, rcDropDown.top);
				}
			}
			else if (HasGroupState(i, LVGS_NORMAL))
			{
				if (m_imageRss_Group_Drop_Down_Nor)
				{
					m_imageRss_Group_Drop_Down_Nor->Draw(dcMem, rcDropDown.left, rcDropDown.top);
				}
			}
#endif

			//CRect rcDropDown(rcGroup.right - 20, rcGroup.top + 7, rcGroup.right - 5, rcGroup.bottom - 3);
			//CRgn rgDropDown;//画圆角矩形
			//rgDropDown.CreateRoundRectRgn( rcDropDown.left, rcDropDown.top, rcDropDown.right, rcDropDown.bottom, 3, 3 );
			//
			//CBrush* brushFrame; 
			//brushFrame = new CBrush( GetSysColor(COLOR_GRAYTEXT) );
			//dcMem.FrameRgn(&rgDropDown,brushFrame,1,1);//画焦点边框
			//delete brushFrame;
		}
	}
	else
	{
		if ( nGroupHeight == -1 )
		{
			CRect rcFirstItem;//算法有待优化
			GetItemRect(0, rcFirstItem, LVIR_BOUNDS);
			if ( !rcFirstItem.IsRectEmpty() )
			{
				nGroupHeight = rcFirstItem.top;
			}
		}//计算group 高度

		CFeedCatalogs & feedCatalogs = CFeedCatalogs::GetInstance();
		//遍历Catlogs 获得每个一级分类的Start RssData[只要用第一个就行了....第一个向上偏移绘制]
		for (	CFeedCatalogs::iterator it = feedCatalogs.GetBegin();
				it != feedCatalogs.GetEnd();
				++it	)
		{
			FeedCatalog & feedCatlog = it->second;
			if ( feedCatlog.IsTop() && feedCatlog.GetFeedCount() > 0 )
			{
				CRssFeedBase* feedStart = feedCatlog.GetFirstFeed();

				int nStartIndex = GetItemIndex( (CRssFeed*)feedStart );//确定第一个item位置
				
				if ( nStartIndex > -1 )
				{
					int nGroupID = GetGroupIDByItemIndex( nStartIndex );
					CString strGroupCaption = GetGroupCaption( nGroupID );

					CString strGroupTotals;
					const CSimpleArray<int>& groupRows = m_mapGroups.GetValueAt(nGroupID);
					strGroupTotals.Format(_T("(共%d个)"), groupRows.GetSize());
					//strGroupTotals.Format(_T("(共 %n 个)"), GetGroupCount());

					CRect itemRect;
					GetItemRect(nStartIndex, &itemRect, LVIR_BOUNDS);

					//根据item rect获得group rect
					CRect rcGroup(rectClient.left, itemRect.top - nGroupHeight, rectClient.right, itemRect.top - 6);
					
					CRect rtIntersect;
					rtIntersect.IntersectRect(rectClip, rcGroup);

					if ( !rtIntersect.IsRectEmpty() )
					{
						DrawGroup(dcMem, rcGroup, strGroupCaption, strGroupTotals);
					}
				}
			}
		}

	}
}

void CFeedIcoItemListCtrl::DrawGroup(CDC & dcMem, const CRect & rectGroup, const CString & strGroupCaption, const CString & strGroupTotals)
{
	CRgn rgnFrame;//画圆角矩形
	rgnFrame.CreateRoundRectRgn( rectGroup.left, rectGroup.top, rectGroup.right, rectGroup.bottom, 3, 3 );

	if (m_imageRss_GroupBg)
	{
		m_imageRss_GroupBg->Draw(dcMem.m_hDC, rectGroup.left,  rectGroup.top, rectGroup.Width(),  24);
	}

	//CBrush* brushFrame; 
	//brushFrame = new CBrush( GetSysColor(COLOR_GRAYTEXT) );
	//dcMem.FrameRgn(&rgnFrame,brushFrame,1,1);//画焦点边框
	//delete brushFrame;

	CRect rectCaption  ( rectGroup.left + 7, rectGroup.top + 4, rectGroup.left + 40, rectGroup.bottom - 3   ) ;

	dcMem.SetTextColor( RGB(0,0,0) );
	dcMem.DrawText(strGroupCaption, -1, &rectCaption, DT_SINGLELINE|DT_LEFT/*|DT_CENTER*/ );

	CRect rectTotals  ( rectCaption.right + 4, rectGroup.top + 5, rectCaption.right + 80, rectGroup.bottom - 3   ) ;

	CFont * pListFont = GetFont();
	CFont* pOld = dcMem.SelectObject(pListFont);
	dcMem.SetTextColor( RGB(169,47,47) );
	dcMem.DrawText(strGroupTotals, -1, &rectTotals, DT_SINGLELINE|DT_LEFT/*|DT_CENTER*/ );
	dcMem.SelectObject(pOld);
}

void CFeedIcoItemListCtrl::DrawItems(CDC & dcMem, const CRect & rectClip, const CRect & rectClient)
{
	//CImageList* pImgList = GetImageList(LVSIL_NORMAL);

	CFont * pListFont = GetFont();
	dcMem.SelectObject(pListFont);
	dcMem.SetTextColor(RGB(0,0,0));

	CRect itemRect(0,0,0,0);

	CString strTaskName;

	int nItemCount  = GetItemCount() - 1;
	int nHoverIndex = GetHotItem();//Hover
	for ( ; nItemCount >= 0; nItemCount-- )
	{
		int nItemIndex = nItemCount;
		GetItemRect(nItemIndex, &itemRect, LVIR_BOUNDS);

		CRect rtIntersect;
		rtIntersect.IntersectRect(rectClip,itemRect);
		if (rtIntersect.IsRectEmpty())//判断此ITEM是否在重绘区域内,不在的话则直接continue
			continue;

		CRect rectPoster;//封面RECT
		GetItemRect(nItemIndex, &rectPoster, LVIR_ICON);

		//绘制海报
		if( nHoverIndex == nItemIndex || GetItemState(nItemIndex, ODA_SELECT) == ODA_SELECT )
		{//Hover
			if (m_imageRss_PosterSelBg)
				m_imageRss_PosterSelBg->Draw(dcMem.m_hDC, rectPoster.left + 11, rectPoster.top - 8);
		}
		else
		{//Normal
			if (m_imageRss_PosterBg)
				m_imageRss_PosterBg->Draw(dcMem.m_hDC, rectPoster.left + 15, rectPoster.top - 4);
		}

		CRssFeed* pFeed = (CRssFeed*)GetItemData(nItemIndex);

		if (pFeed->m_pPosterImage)//绘制海报
			pFeed->m_pPosterImage->Draw(dcMem.m_hDC, rectPoster.left + 20, rectPoster.top + 1);
		else if (m_imageRss_PosterDefault)//绘制默认海报
				m_imageRss_PosterDefault->Draw(dcMem.m_hDC, rectPoster.left + 20, rectPoster.top + 1);

		//strTaskName = pFeed->m_strTitle;
		strTaskName = GetItemText(nItemCount, 0);

		CRect rectNameStr;//宽度146
		GetItemRect(nItemIndex, &rectNameStr, LVIR_LABEL);
		rectNameStr.OffsetRect(0,5);//拉开标题和Poster的距离 

		dcMem.DrawText(strTaskName, -1, &rectNameStr, DT_CENTER | DT_WORDBREAK );

		//if (pImgList)
		//	pImgList->Draw(&MemDC,nItem,CPoint(itemRect.left+5,itemRect.top+5),ILD_TRANSPARENT);
	}
}

void CFeedIcoItemListCtrl::OnPaint()
{
	CRect rectClip;
	{	
		CPaintDC dcPaint(this); // device context for painting
		//dcPaint.GetClipBox(rectClip);
	}

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

	CBrush *pBKBrush = new CBrush( RGB(0xf3,0xf3,0xf3/*246, 252, 251*/) );
	dcMem.FillRect( rectClient, pBKBrush );//填充客户区背景色
	delete pBKBrush;

	
	//CMemDC dcMem(&dc, rectClient);
	//dcMem.BitBlt(0, 0, rectClient.Width(), rectClient.Height(), &dc, 0, 0, SRCCOPY);

	//CFont * pListFont = GetFont();
	//CFont * pOrgMemFont = NULL;
	//if ( pListFont != NULL )
	//{
	//	pOrgMemFont = dcMem.SelectObject(pListFont);
	//}
	//dcMem.SetBkMode(TRANSPARENT);

	if ( GetItemCount() > 0 )
	{
		DrawGroupInfo(dcMem, rectClip, rectClient);
		DrawItems(dcMem, rectClip, rectClient);
	}

	//if ( pOrgMemFont != NULL )
	//{
	//	dcMem.SelectObject(pOrgMemFont);
	//}
	//dcMem->Flush();
	dc.BitBlt( rectClient.left, rectClient.top, rectClient.Width(),rectClient.Height(), &dcMem, 0,0, SRCCOPY );//最终贴图
}

void CFeedIcoItemListCtrl::OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	// 更新工具条
	CDownloadTabWnd & wd = theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_DownloadTabWnd;
	UpdateToolBarState(wd.m_Toolbar);
}

// 更新toolbar按钮状态
void CFeedIcoItemListCtrl::UpdateToolBarState(CToolBarCtrl & toolbar)
{
	if( !CGlobalVariable::IsRunning() )
	{
		return;
	}

	toolbar.EnableButton(MP_PAUSE, FALSE);
	toolbar.EnableButton(MP_RESUME, FALSE);
	toolbar.EnableButton(MP_STOP, FALSE);
	toolbar.EnableButton(MP_OPENFOLDER, FALSE);
	toolbar.EnableButton(MP_CANCEL, FALSE);

	int iSelectedItems = GetSelectedCount();
	if(iSelectedItems && iSelectedItems == 1)
	{
		int nIndex = GetCurrSelectIndex();
		if ( nIndex >= 0 )
		{
			if ( CRssFeed* pFeed = GetFeed(nIndex) )
			{
				toolbar.EnableButton(MP_OPENFOLDER, TRUE);

				//下载所有||暂停
				if (pFeed->ExistDownloadingItem())
				{
					toolbar.EnableButton(MP_PAUSE, TRUE);
				}

				if (pFeed->ExistCanResumeItem())
				{
					toolbar.EnableButton(MP_RESUME, TRUE);
				}

				toolbar.EnableButton(MP_CANCEL, TRUE);
			}
		}
	}
}

BOOL CFeedIcoItemListCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类

	if (  /*LOWORD(wParam) == MP_STOP
		||*/LOWORD(wParam) == MP_DOWNLOAD_ALL
		||LOWORD(wParam) == MP_OPEN
		||LOWORD(wParam) == MP_OPENFOLDER
		||LOWORD(wParam) == MP_RESUME
		||LOWORD(wParam) == MP_PAUSE
		||LOWORD(wParam) == MP_CANCEL
		||LOWORD(wParam) == MP_OPTIONS)
	{
		POSITION Pos = this->GetFirstSelectedItemPosition();
		if (Pos == NULL)
		{
			return 0;
		}
		int nIndex = this->GetNextSelectedItem(Pos);

		if( CRssFeed* pFeed = this->GetFeed(nIndex) )
		{
			if ( LOWORD(wParam) == MP_OPENFOLDER)
			{
				if( CRssFeed* pFeed = this->GetFeed(nIndex) )
				{
					if(PathFileExists(pFeed->m_strSaveDir))
						ShellExecute(NULL, _T("open"), _T("explorer.exe"), pFeed->m_strSaveDir, NULL, SW_SHOW);
				}
			}

			if ( LOWORD(wParam) == MP_DOWNLOAD_ALL )
			{
				((CRssFeedWnd*)GetParent())->DownloadAllItem(nIndex);
			}
			if ( LOWORD(wParam) == MP_RESUME )
			{
				((CRssFeedWnd*)GetParent())->ResumeAllItem(nIndex);
			}
			if ( LOWORD(wParam) == MP_PAUSE)
			{
				for ( CRssFeed::ItemIterator it = pFeed->GetItemIterBegin();
					it != pFeed->GetItemIterEnd();
					++it
					)
				{
					CPartFile* pPartFile = it->second.GetPartFile();
					if (pPartFile != NULL)
					{
						if (pPartFile->CanPauseFile())
							pPartFile->PauseFile();		
					}
				}
			}

			if ( LOWORD(wParam) == MP_CANCEL)
			{
				::SendMessage(CGlobalVariable::s_wndRssCtrl->GetSafeHwnd(), WM_COMMAND, wParam, lParam);
			}

			if ( LOWORD(wParam) == MP_OPTIONS)
			{
				CDlgFeedConfig dlgFeedConfig(*pFeed);
				if ( dlgFeedConfig.DoModal() == IDOK )
				{
					// 保存设置
					pFeed->SetUpdateInterval(dlgFeedConfig.m_uUpdateInterval);
					pFeed->SetAutoDownload(dlgFeedConfig.m_uAutoDownload);
					CGlobalVariable::s_wndRssCtrl->SaveFeed(*pFeed);
					//RedrawItems(nIndex, nIndex);
				}
			}

			CDownloadTabWnd & wd = theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_DownloadTabWnd;
			UpdateToolBarState(wd.m_Toolbar);

			return 0;
		}
	}

	::SendMessage(CGlobalVariable::s_wndRssCtrl->GetSafeHwnd(), WM_COMMAND, wParam, lParam);
	
	return CListCtrl::OnCommand(wParam, lParam);
}

void CFeedIcoItemListCtrl::UpdateMenuState(CTitleMenu	& menuRSS)
{
	if( !CGlobalVariable::IsRunning() )
	{
		return;
	}

	//设置菜单状态
	int iSelectedItems = GetSelectedCount();
	if(iSelectedItems && iSelectedItems == 1)
	{
		int nIndex = GetCurrSelectIndex();
		if ( nIndex >= 0 )
		{
			if ( CRssFeed* pFeed = GetFeed(nIndex) )
			{
				//删除不用的menu项
				menuRSS.DeleteMenu(MP_OPEN,MF_STRING);
				menuRSS.DeleteMenu(MP_RESUME,MF_STRING);

				//下载所有||暂停
				if (pFeed->ExistDownloadingItem())
				{
					menuRSS.EnableMenuItem(MP_PAUSE, MF_ENABLED);
				}

				if (pFeed->ExistUnloadItem())
				{
					menuRSS.EnableMenuItem(MP_DOWNLOAD_ALL, MF_ENABLED);
				}

				menuRSS.EnableMenuItem(MP_CANCEL, MF_ENABLED);
				menuRSS.EnableMenuItem(MP_REFRESH, MF_ENABLED);
				menuRSS.EnableMenuItem(MP_OPTIONS, MF_ENABLED);
				menuRSS.EnableMenuItem(MP_OPENFOLDER, MF_ENABLED);
			}
		}
	}
	else//空白处右键
	{
		menuRSS.DeleteMenu(MP_OPEN,MF_STRING);
		menuRSS.DeleteMenu(MP_OPENFOLDER,MF_STRING);
		menuRSS.DeleteMenu(MP_DOWNLOAD_ALL,MF_STRING);
		menuRSS.DeleteMenu(MP_RESUME,MF_STRING);
		menuRSS.DeleteMenu(MP_PAUSE,MF_STRING);
		menuRSS.DeleteMenu(MP_CANCEL,MF_STRING);
		menuRSS.DeleteMenu(MP_OPTIONS,MF_STRING);
		//menuRSS.DeleteMenu(MF_SEPARATOR,MF_STRING);

		if ( this->GetItemCount() > 0 )
		{
			menuRSS.EnableMenuItem(MP_REFRESH, MF_ENABLED);
		}
		else
		{
			menuRSS.DeleteMenu(MP_REFRESH,MF_STRING);
		}
	}
}


void CFeedIcoItemListCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	// TODO: 在此处添加消息处理程序代码
	CTitleMenu	menuRSS;
	menuRSS.CreatePopupMenu();

	menuRSS.AppendMenu(MF_STRING,MP_OPEN,GetResString(IDS_OPEN),_T("OPEN"));
	menuRSS.AppendMenu(MF_STRING,MP_OPENFOLDER, GetResString(IDS_OPENFOLDER), _T("OPENFOLDER"));
	//menuRSS.AppendMenu(MF_STRING|MF_SEPARATOR);
	menuRSS.AppendMenu(MF_STRING,MP_DOWNLOAD_ALL, GetResString(IDS_RSSBTN_ALLDL), _T("RESUME"));//下载所有
	menuRSS.AppendMenu(MF_STRING, MP_RESUME, GetResString(IDS_DOWNLOAD), _T("RESUME"));
	menuRSS.AppendMenu(MF_STRING, MP_PAUSE, GetResString(IDS_PAUSE), _T("PAUSE"));
	menuRSS.AppendMenu(MF_STRING,MP_CANCEL, GetResString(IDS_DELETE_FILE), _T("DELETE"));
	//menuRSS.AppendMenu(MF_STRING|MF_SEPARATOR);
	menuRSS.AppendMenu(MF_STRING,MP_REFRESH, GetResString(IDS_SV_UPDATE), NULL );
	menuRSS.AppendMenu(MF_STRING,MP_OPTIONS, GetResString(IDS_RW_RSS_CONFIG), NULL );
	//menuRSS.AppendMenu(MF_STRING, MP_STOP, GetResString(IDS_DL_STOP), _T("STOP"));

	menuRSS.EnableMenuItem(MP_OPEN, MF_GRAYED);
	menuRSS.EnableMenuItem(MP_OPENFOLDER, MF_GRAYED);
	menuRSS.EnableMenuItem(MP_DOWNLOAD_ALL, MF_GRAYED);
	menuRSS.EnableMenuItem(MP_RESUME, MF_GRAYED);
	menuRSS.EnableMenuItem(MP_PAUSE, MF_GRAYED);
	menuRSS.EnableMenuItem(MP_CANCEL, MF_GRAYED);
	menuRSS.EnableMenuItem(MP_REFRESH, MF_GRAYED);
	menuRSS.EnableMenuItem(MP_OPTIONS, MF_GRAYED);
	//menuRSS.EnableMenuItem(MP_STOP, MF_GRAYED);

	UpdateMenuState(menuRSS);

	m_pMenuXP = new CMenuXP();
	m_pMenuXP->AddMenu(&menuRSS, TRUE);
	menuRSS.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
	menuRSS.DestroyMenu();

	delete m_pMenuXP;
	m_pMenuXP = NULL;
}

void CFeedIcoItemListCtrl::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (lpDrawItemStruct->CtlType == ODT_MENU)
		m_pMenuXP->DrawItem(lpDrawItemStruct);
	else
		CListCtrl::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

void CFeedIcoItemListCtrl::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (lpMeasureItemStruct->CtlType == ODT_MENU)
		m_pMenuXP->MeasureItem(lpMeasureItemStruct);
	else
		CListCtrl::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}

void CFeedIcoItemListCtrl::OnLvnHotTrack(NMHDR *pNMHDR, LRESULT *pResult)
{
	// vista+ 系统的经典主题和vista-系统的 必须要加此消息
	if (thePrefs.GetWindowsVersion() >= _WINVER_VISTA_ && IsThemeEnabled())
		return;

	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	int nCurrentHotItemIndex = GetHotItem();

	if (nCurrentHotItemIndex == m_nLastHotItemIndex)
		return;

	if (-1 != nCurrentHotItemIndex)
	{// move into the hot item
		m_nLastHotItemIndex = nCurrentHotItemIndex;
	}
	else
	{// move out of the hot item
		nCurrentHotItemIndex = m_nLastHotItemIndex;
		m_nLastHotItemIndex = -1;
	}

	CRect itemRect(0,0,0,0);
	GetItemRect(nCurrentHotItemIndex, &itemRect, LVIR_BOUNDS);

	InvalidateRect(&itemRect);
}

void CFeedIcoItemListCtrl::FilterItemsBySearchKey(CString* pSearchKey)
{
	if (!IsWindowVisible())
		return;

	if (pSearchKey == NULL || pSearchKey->IsEmpty())
	{
		if (m_stringMatcher.IsOriginal())
			return;
	}

	const StringMatcher::ListItemsNeedShow& listItemsNeedShow = m_stringMatcher.GetMatchResult(pSearchKey);

	SetRedraw(FALSE);

	RemoveAllGroups();//删除分组
	RemoveAllGroupData();//删除m_mapGroups
	DeleteAllItems();//删除items

	for (StringMatcher::ListItemsNeedShow::const_iterator ix = listItemsNeedShow.begin(); ix != listItemsNeedShow.end(); ++ix)
	{
		CRssFeed* listKey = (CRssFeed*)*ix;
		CString strCaption = GetExtentString( listKey->GetDisplayName() );

		int nRet = InsertItem(GetItemCount(), strCaption);
		this->SetItemData(nRet, LPARAM(listKey));
	}
	GroupAllFeedsByType(0);

	SetRedraw(TRUE);
}
BOOL CFeedIcoItemListCtrl::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类

	if (pMsg->message == WM_KEYDOWN)
	{
		if ( VK_ESCAPE == pMsg->wParam )
		{
			FilterItemsBySearchKey(NULL);
		}
	}

	return CListCtrl::PreTranslateMessage(pMsg);
}
