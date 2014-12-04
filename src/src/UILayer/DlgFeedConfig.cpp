// DlgFeedConfig.cpp : 实现文件
//

#include "stdafx.h"
#include "resource.h"
#include "DlgFeedConfig.h"
#include "rss/RssFeed.h"
#include "otherfunctions.h"
#include ".\dlgfeedconfig.h"
#include "PPgGeneral.h"
#include "Preferences.h"

// CDlgFeedConfig 对话框

IMPLEMENT_DYNAMIC(CDlgFeedConfig, CDialog)
CDlgFeedConfig::CDlgFeedConfig(CRssFeed & feed, CWnd* pParent /*=NULL*/)
	: CDialog(CDlgFeedConfig::IDD, pParent),
	m_feed(feed),
	m_uUpdateInterval(0),
	m_uAutoDownload(0),
	m_pFontSaveDir(NULL)
{
}

CDlgFeedConfig::~CDlgFeedConfig()
{
	if ( m_pFontSaveDir != NULL )
	{
		m_pFontSaveDir->DeleteObject();
		delete m_pFontSaveDir;
		m_pFontSaveDir = NULL;
	}
}

void CDlgFeedConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_RSS_INTERVAL, m_cboInterval);
	DDX_Control(pDX, IDC_CHECK_RSS_AUTO_DOWNLOAD, m_btnAutoDownload);
	DDX_Control(pDX, IDC_CHECK_RW_AUTO_REFRESH, m_btnAutoRefresh);
	DDX_Control(pDX, IDC_STATIC_RW_SHOW_FEED_NAME, m_stcShowFeedName);
}

// 更新ui上的feed大小信息
void CDlgFeedConfig::UpdateFeedSizeInfo()
{
	// 磁盘剩余空间
	CString strSaveDir;
	GetDlgItemText(IDC_EDIT_RW_SAVE_DIR, strSaveDir);
	uint64 uFreeSpace;
	if ( strSaveDir.GetLength() < 3 || strSaveDir.GetAt(1) != _T(':') || strSaveDir.GetAt(2) != _T('\\') )
	{
		uFreeSpace = 0;
	}
	else
	{
		uFreeSpace = GetFreeDiskSpaceX(strSaveDir.Left(3));
	}

	// 订阅大小（磁盘剩余空间）
	CString strText;
	strText.Format(_T("%s (%s%s)"),
				   m_strFeedSize,
				   GetResString(IDS_ADDTASKDLG_FREE_SPACE),
				   CastItoXBytes(uFreeSpace)
				  );
	SetDlgItemText(IDC_EDIT_RW_FEED_SIZE, strText);

}


// 更新ui上的feed信息
void CDlgFeedConfig::UpdateFeedInfo()
{
	// 保存订阅大小信息
	m_strFeedSize = CastItoXBytes(m_feed.GetRssTotalSize(), false, false);

	// 显示订阅名称
	m_strFeedName = m_feed.GetDisplayName();
	m_stcShowFeedName.SetWindowText(m_strFeedName);

	//初始化tooltip
	m_tooltipShowFeedName.Create(this, TTS_NOPREFIX);
	m_tooltipShowFeedName.AddTool(&m_stcShowFeedName, m_strFeedName);
	//m_tooltipShowFeedName.SetTipTextColor(RGB(0, 0, 255));
	//m_tooltipShowFeedName.UpdateTipText(m_feed.GetDisplayName(), &m_stcShowFeedName);
	m_tooltipShowFeedName.SetDelayTime(500);
	m_tooltipShowFeedName.Activate(TRUE);

	// 订阅保存目录
	SetDlgItemText(IDC_STATIC_RW_SHOW_SAVE_DIR, m_feed.m_strSaveDir);

	// 更新显示的订阅大小信息
	UpdateFeedSizeInfo();

	// 已/未下载项目
	unsigned int uFilmCount;
	unsigned int uDownloadedCount = m_feed.GetDownloadedFilmCount(&uFilmCount);
	CString strText;
	strText.Format(_T("%u/%u"), uDownloadedCount, uFilmCount - uDownloadedCount);
	SetDlgItemText(IDC_EDIT_RW_FEED_DOWNLOAD_INFO, strText);

	// 保存刷新间隔时间
	m_uUpdateInterval = m_feed.GetUpdateInterval();

	// 若有定义，则使用feed的值；否则，按全局设置来显示
	if ( m_feed.UpdateIntervalIsDefined() )
	{
		CPPgGeneral::InitRssIntervalComboBox(m_cboInterval, m_uUpdateInterval);
		if ( m_feed.IsManualRefresh() )
		{
			// 手动刷新，禁用选择框
			m_btnAutoRefresh.SetCheck(BST_UNCHECKED);
			m_cboInterval.SetCurSel(0);
			m_cboInterval.EnableWindow(FALSE);
		}
		else
		{
			// 自动刷新， 启用选择框
			m_btnAutoRefresh.SetCheck(BST_CHECKED);
			m_cboInterval.EnableWindow(TRUE);
		}
	}
	else
	{
		// 使用全局设置
		CPPgGeneral::InitRssIntervalComboBox(m_cboInterval, thePrefs.m_uRssUpdateInterval);
		m_btnAutoRefresh.SetCheck(BST_CHECKED);
		m_cboInterval.EnableWindow(TRUE);
	}

	// 保存自动下载
	m_uAutoDownload = m_feed.GetAutoDownload();
	
	// 若有定义，则使用feed的值；否则，按全局设置来显示
	if ( m_feed.AutoDownloadIsDefined() )
	{
		m_btnAutoDownload.SetCheck(m_uAutoDownload ? BST_CHECKED : BST_UNCHECKED);
	}
	else
	{
		m_btnAutoDownload.SetCheck(thePrefs.m_bRssAutoDownload ? BST_CHECKED : BST_UNCHECKED);
	}
}

// 本地化
void CDlgFeedConfig::Localize()
{
	// 标题
	SetWindowText(GetResString(IDS_RW_RSS_CONFIG));

	// 订阅信息
	SetDlgItemText(IDC_STATIC_RW_FEED_INFO, GetResString(IDS_RW_FEED_INFO));
	SetDlgItemText(IDC_STATIC_RW_FEED_NAME, GetResString(IDS_RW_CONFIG_RSS_NAME) + CString(":"));
	SetDlgItemText(IDC_STATIC_RW_FEED_SIZE, GetResString(IDS_RSSLIST_LENGTH) + CString(":"));
	SetDlgItemText(IDC_STATIC_RW_FEED_DOWNLOAD_INFO, GetResString(IDS_RW_FEED_DOWNLOAD_INFO) + CString(":"));
	SetDlgItemText(IDC_STATIC_RW_FEED_SAVE_DIR, GetResString(IDS_ADDTASKDLG_SAVE_LOCATION) + CString(":"));

	// 更新
	SetDlgItemText(IDC_STATIC_RW_FEED_CONFIG, GetResString(IDS_RW_FEED_CONFIG));
	m_btnAutoRefresh.SetWindowText(GetResString(IDS_RW_AUTO_REFRESH));
	SetDlgItemText(IDC_STATIC_RSS_INTERVAL, GetResString(IDS_PPGRSS_INTERVAL));
	m_btnAutoDownload.SetWindowText(GetResString(IDS_PPGRSS_AUTO_DOWNLOAD));
}

BEGIN_MESSAGE_MAP(CDlgFeedConfig, CDialog)
	ON_EN_CHANGE(IDC_EDIT_RW_SAVE_DIR, OnEnChangeEditRwSaveDir)
	ON_BN_CLICKED(IDC_CHECK_RW_AUTO_REFRESH, OnBnClickedCheckRwAutoRefresh)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_STN_CLICKED(IDC_STATIC_RW_SHOW_SAVE_DIR, OnStnClickedStaticRwShowSaveDir)
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()


// CDlgFeedConfig 消息处理程序


void CDlgFeedConfig::OnEnChangeEditRwSaveDir()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	UpdateFeedSizeInfo();
}

BOOL CDlgFeedConfig::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	EnableToolTips(TRUE);
	GetDlgItem(IDC_EDIT_RW_FEED_NAME)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_RW_SAVE_DIR)->ShowWindow(SW_HIDE);
	
	if ( CWnd *pWndSaveDir = GetDlgItem(IDC_STATIC_RW_SHOW_SAVE_DIR) )
	{
		if ( CFont * pFont = pWndSaveDir->GetFont() )
		{
			LOGFONT logfont;
			pFont->GetLogFont(&logfont);
			logfont.lfUnderline = TRUE;
			
			m_pFontSaveDir = new CFont();
			m_pFontSaveDir->CreateFontIndirect(&logfont);
			pWndSaveDir->SetFont(m_pFontSaveDir);
		}
	}

	// 更新ui上的feed信息
	UpdateFeedInfo();

	// 本地化
	Localize();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CDlgFeedConfig::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	// 保存更新时间间隔
	switch ( m_btnAutoRefresh.GetCheck() )
	{
		case BST_UNCHECKED:
			// 取消自动刷新，即为手动刷新
			m_uUpdateInterval = CRssFeed::MANUAL_REFRESH_VALUE;
			break;

		case BST_CHECKED:
			// 选择自动刷新，使用选择框内的值
			{
				int iCurSel = m_cboInterval.GetCurSel();
				if ( iCurSel >= 0 )
				{
					m_uUpdateInterval = m_cboInterval.GetItemData(iCurSel);
				}
			}
			break;
	}

	// 保存是否自动更新
	m_uAutoDownload = (m_btnAutoDownload.GetCheck() == BST_CHECKED);

	CDialog::OnOK();
}

void CDlgFeedConfig::OnBnClickedCheckRwAutoRefresh()
{
	// TODO: 在此添加控件通知处理程序代码

	// 选择自动刷新，启用选择，取消自动刷新，禁用选择
	m_cboInterval.EnableWindow(m_btnAutoRefresh.GetCheck() == BST_CHECKED);
}

void CDlgFeedConfig::OnPaint()
{
	CDialog::OnPaint();

	//CPaintDC dc(this); // device context for painting
	//// TODO: 在此处添加消息处理程序代码
	//// 不为绘图消息调用 CDialog::OnPaint()

	//CRect rectFeedName;
	//GetDlgItem(IDC_STATIC_RW_SHOW_FEED_NAME)->GetWindowRect(rectFeedName);
	//ScreenToClient(rectFeedName);
	//dc.SetBkMode(TRANSPARENT);
	//CFont * pOrgFont = dc.SelectObject(GetFont());

	//dc.DrawText(m_feed.GetDisplayName(), 
	//			rectFeedName, 
	//			DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_TOP | DT_NOPREFIX
	//		   );
	//if ( pOrgFont != NULL )
	//{
	//	dc.SelectObject(pOrgFont);
	//}
	
}

BOOL CDlgFeedConfig::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类

	if( m_tooltipShowFeedName.m_hWnd != NULL ) 
	{
		m_tooltipShowFeedName.RelayEvent(pMsg);
	}

	return CDialog::PreTranslateMessage(pMsg);
}

HBRUSH CDlgFeedConfig::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何属性
	if ( nCtlColor == CTLCOLOR_STATIC )
	{
		ASSERT(pDC != NULL);
		ASSERT(pWnd != NULL);
		if ( pWnd->GetDlgCtrlID() == IDC_STATIC_RW_SHOW_SAVE_DIR )
		{
			pDC->SetTextColor(RGB(0, 0, 255));
		}
	}

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

void CDlgFeedConfig::OnStnClickedStaticRwShowSaveDir()
{
	// TODO: 在此添加控件通知处理程序代码
	ShellExecute(GetSafeHwnd(), 
				 _T("open"),
				 m_feed.m_strSaveDir,
				 NULL,
				 NULL,
				 SW_SHOWNORMAL
				);
}

BOOL CDlgFeedConfig::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	ASSERT(pWnd != NULL);

	if ( pWnd->GetDlgCtrlID() != IDC_STATIC_RW_SHOW_SAVE_DIR )
	{
		return CDialog::OnSetCursor(pWnd, nHitTest, message);
	}
	else
	{
		::SetCursor(::LoadCursor(NULL, IDC_HAND));
		return TRUE;
	}
}
