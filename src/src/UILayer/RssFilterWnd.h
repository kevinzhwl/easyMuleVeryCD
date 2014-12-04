#pragma once
#include "afxcmn.h"
#include "afxwin.h"

#include "ResizableLib\ResizableDialog.h"


// CRssFilterWnd 对话框
class CRssFilter;
class CRssFilterWnd : public CResizableDialog
{
	DECLARE_DYNAMIC(CRssFilterWnd)

public:
	CRssFilterWnd(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CRssFilterWnd();

// 对话框数据
	enum { IDD = IDD_RW_FILTER };

public:
	// 添加过滤器到列表
	void AddFilterToList(CRssFilter & filter);

	// 更新开始
	void BeginUpdateFilter()
	{
		m_listFilter.SetRedraw(FALSE);
	}

	// 更新结束
	void EndUpdateFilter()
	{
		m_listFilter.SetRedraw(TRUE);
	}

	// 取得过滤器
	CRssFilter * GetFilter(int iItemIndex)
	{
		return reinterpret_cast<CRssFilter *>(m_listFilter.GetItemData(iItemIndex));
	}

private:
	// 显示一个过滤器内容
	void DrawFilter(const CRssFilter * pFilter);

	// 保存一个过滤器内容
	CRssFilter * SaveFilterContent(CRssFilter * pFilter);

	// 保存一个过滤器是否启用状态
	CRssFilter * SaveFilterEnable(CRssFilter * pFilter, BOOL bEnable);

	// 保存一个过滤器名字
	CRssFilter * SaveFilterName(CRssFilter * pFilter, LPCTSTR lpszName);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
	// 过滤器列表
	CListCtrl m_listFilter;
	// 需要包含的子串
	CEdit m_edtFilterIn;
	// 需要排除的子串
	CEdit m_edtFilterNot;
	// 资源质量
	CComboBox m_cboQuality;
	// 文件类型
	CEdit m_edtFileType;
	// 资源保存目录
	CEdit m_edtSaveDir;
	// 选择资源保存目录
	CButton m_btnSelSaveDir;
	// 资源最小尺寸(MB)
	CEdit m_edtMinSize;
	// 资源最大尺寸(MB)
	CEdit m_edtMaxSize;
	// 应用到指定的订阅
	CComboBox m_cboApplyFeed;
	// 指定是否自动开始下载
	CButton m_btnNotAutoStart;

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnLvnItemchangedRwFilterList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnEndlabeleditRwFilterList(NMHDR *pNMHDR, LRESULT *pResult);
};
