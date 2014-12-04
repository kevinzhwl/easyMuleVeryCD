// RssFilterWnd.cpp : 实现文件
//

#include "stdafx.h"
#include "emule.h"
#include "resource.h"
#include "RssFilterWnd.h"
#include ".\rssfilterwnd.h"
#include "rss\RssFilter.h"


// CRssFilterWnd 对话框

IMPLEMENT_DYNAMIC(CRssFilterWnd, CDialog)
CRssFilterWnd::CRssFilterWnd(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CRssFilterWnd::IDD, pParent)
{
}

CRssFilterWnd::~CRssFilterWnd()
{
}

// 添加过滤器到列表
void CRssFilterWnd::AddFilterToList(CRssFilter & filter)
{
	int nNextIndex = m_listFilter.GetItemCount();
	m_listFilter.InsertItem(nNextIndex, 
							filter.m_strName, 
							0
						   );
	m_listFilter.SetItemData(nNextIndex, reinterpret_cast<DWORD_PTR>(&filter));
	m_listFilter.SetCheck(nNextIndex, filter.m_bEnable);
}

// 显示一个过滤器内容
void CRssFilterWnd::DrawFilter(const CRssFilter * pFilter)
{
	if ( pFilter == NULL )
	{
		return;
	}

	m_edtFilterIn.SetWindowText(pFilter->m_strFilter);
	m_edtFilterNot.SetWindowText(pFilter->m_strNot);
	m_cboQuality.SetWindowText(pFilter->m_strQuality);
	m_edtFileType.SetWindowText(pFilter->m_strFileType);
	//m_edtSaveDir.SetWindowText(pFilter->m_strSaveDir);

	CString str;
	str.Format(_T("%u"), pFilter->m_uMinSizeKB / 1024);
	m_edtMinSize.SetWindowText(str);
	str.Format(_T("%u"), pFilter->m_uMaxSizeKB / 1024);
	m_edtMaxSize.SetWindowText(str);

	m_cboApplyFeed.SetWindowText(pFilter->m_pApplyFeed ? pFilter->m_pApplyFeed->GetDisplayName() : _T(""));
	m_btnNotAutoStart.SetCheck(pFilter->m_bAutoStart ? BST_UNCHECKED : BST_CHECKED);
}

// 保存一个过滤器内容
CRssFilter * CRssFilterWnd::SaveFilterContent(CRssFilter * pFilter)
{
	if ( pFilter == NULL )
	{
		return pFilter;
	}

	CString str;
	m_edtFilterIn.GetWindowText(str);
	pFilter->m_strFilter = str;

	m_edtFilterNot.GetWindowText(str);
	pFilter->m_strNot = str;

	m_cboQuality.GetWindowText(str);
	pFilter->m_strQuality = str;

	m_edtFileType.GetWindowText(str);
	pFilter->m_strFileType = str;

	m_edtSaveDir.GetWindowText(str);
	//pFilter->m_strSaveDir = str;

	m_edtMinSize.GetWindowText(str);
	pFilter->m_uMinSizeKB = _ttoi(str) * 1024;
	
	m_edtMaxSize.GetWindowText(str);
	pFilter->m_uMaxSizeKB = _ttoi(str) * 1024;

	//m_cboApplyFeed.SetWindowText(pFilter->m_pApplyFeed ? pFilter->m_pApplyFeed->GetDisplayName() : _T(""));

	pFilter->m_bAutoStart = (m_btnNotAutoStart.GetCheck() == BST_UNCHECKED);

	return pFilter;
}

// 保存一个过滤器是否启用状态
CRssFilter * CRssFilterWnd::SaveFilterEnable(CRssFilter * pFilter, BOOL bEnable)
{
	if ( pFilter != NULL )
	{
		pFilter->m_bEnable = bEnable;
	}
	return pFilter;
}

// 保存一个过滤器名字
CRssFilter * CRssFilterWnd::SaveFilterName(CRssFilter * pFilter, LPCTSTR lpszName)
{
	if ( pFilter != NULL )
	{
		pFilter->m_strName = lpszName;
	}
	return pFilter;
}


void CRssFilterWnd::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RW_FILTER_LIST, m_listFilter);
	DDX_Control(pDX, IDC_RW_EDIT_FILTER, m_edtFilterIn);
	DDX_Control(pDX, IDC_RW_EDIT_FILTER_NOT, m_edtFilterNot);
	DDX_Control(pDX, IDC_RW_COMBO_QUALITY, m_cboQuality);
	DDX_Control(pDX, IDC_RW_EDIT_FILE_TYPE, m_edtFileType);
	DDX_Control(pDX, IDC_RW_EDIT_SAVE_DIR, m_edtSaveDir);
	DDX_Control(pDX, IDC_RW_SEL_SAVE_DIR_BUTTON, m_btnSelSaveDir);
	DDX_Control(pDX, IDC_RW_EDIT_MIN_SIZE, m_edtMinSize);
	DDX_Control(pDX, IDC_RW_EDIT_MAX_SIZE, m_edtMaxSize);
	DDX_Control(pDX, IDC_RW_COMBO_APPLY, m_cboApplyFeed);
	DDX_Control(pDX, IDC_RW_CHECK_NO_AUTO_START, m_btnNotAutoStart);
}


BEGIN_MESSAGE_MAP(CRssFilterWnd, CResizableDialog)
ON_NOTIFY(LVN_ITEMCHANGED, IDC_RW_FILTER_LIST, OnLvnItemchangedRwFilterList)
ON_NOTIFY(LVN_ENDLABELEDIT, IDC_RW_FILTER_LIST, OnLvnEndlabeleditRwFilterList)
//ON_WM_MEASUREITEM()
END_MESSAGE_MAP()


// CRssFilterWnd 消息处理程序

BOOL CRssFilterWnd::OnInitDialog()
{
	CResizableDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化

	AddAnchor(IDC_RW_FILTER_LIST, TOP_LEFT, BOTTOM_LEFT);
	AddAnchor(IDC_RW_GROUP_FILTER, TOP_LEFT, BOTTOM_RIGHT);

	DWORD dwExStyle = m_listFilter.GetExtendedStyle();
	dwExStyle |= LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_FLATSB /*| LVS_EX_ONECLICKACTIVATE*/;
	m_listFilter.SetExtendedStyle(dwExStyle);

	CRect rectFilterList;
	m_listFilter.GetClientRect(rectFilterList);
	m_listFilter.InsertColumn(0, _T(""), LVCFMT_LEFT, rectFilterList.Width());

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CRssFilterWnd::OnLvnItemchangedRwFilterList(NMHDR *pNMHDR, LRESULT *pResult)
{
	ASSERT(pNMHDR != NULL);
	ASSERT(pResult != NULL);

	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	if ( pNMLV->iItem == -1 )
	{
		return;
	}

    BOOL bNewIsSelected = pNMLV->uNewState & LVIS_SELECTED;
	BOOL bOldIsSelected = pNMLV->uOldState & LVIS_SELECTED;
	CRssFilter * pFilter = GetFilter(pNMLV->iItem);

	if ( bNewIsSelected && !bOldIsSelected )
	{
		// 由未选中到选中
		DrawFilter(pFilter);
		return;
	}

	if ( !bNewIsSelected && bOldIsSelected )
	{
		// 由选中到未选中
		//CString strNewName = m_listFilter.GetItemText(pNMLV->iItem, 0);
		SaveFilterContent(pFilter);
		return;
	}

	if ( !bNewIsSelected && !bOldIsSelected )
	{
		// 保存启用状态
		//SaveFilterEnable(pFilter, m_listFilter.GetCheck(pNMLV->iItem));
		return;
	}

}

void CRssFilterWnd::OnLvnEndlabeleditRwFilterList(NMHDR *pNMHDR, LRESULT *pResult)
{
	ASSERT(pNMHDR != NULL);
	ASSERT(pResult != NULL);

	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = FALSE;

	if ( pDispInfo->item.iItem == -1 )
	{
		return;
	}

	if ( pDispInfo->item.pszText != NULL )
	{
		CRssFilter * pFilter = GetFilter(pDispInfo->item.iItem);
		SaveFilterName(pFilter, pDispInfo->item.pszText);
		*pResult = TRUE;
	}
}

