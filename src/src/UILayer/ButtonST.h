#ifndef _BTNST_H
#define _BTNST_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define WM_LISTCTRLCLICKED WM_USER + 0x548
#define WM_LISTCTRL_SWITCH_SHOWMODE_CLICKED WM_USER + 0x549
#define WM_LISTCTRL_INFO_CLICKED WM_USER + 0x550

//Clear SeachEdit
#define WM_SEARCHEDIT_CLEAR		WM_USER + 0x551

class CButtonST : public CButton
{
public:
    CButtonST();
	~CButtonST();

    enum
	{
		ST_ALIGN_HORIZ	= 0,			// Icon/bitmap on the left, text on the right
		ST_ALIGN_VERT					// Icon/bitmap on the top, text on the bottom
	};

	enum
	{
		TEXT_ALIGN_LEFT = 0,
		TEXT_ALIGN_CENTER,
		TEXT_ALIGN_RIGHT,
		TEXT_ALIGN_DIY
	};

	enum
	{
		BTNST_UP	= 0,	//正常的按钮
		BTNST_UP_ON,		//鼠标移动在按钮上,Move状态
		BTNST_DOWN,			//按钮被按下		
		BTNST_DISABLED,		//按钮被禁止
		BTNST_FLASH,		//按钮闪烁

		BTNST_MAX
	};


	// ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CButtonST)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

private:
	LRESULT OnSetCheck(WPARAM wParam, LPARAM lParam);
	LRESULT OnGetCheck(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);

	void CancelHover();

	void DrawDefaultBackground(CDC* pDC, CRect *itemRect);
	void DrawBitmapBackground(CDC* pDC, CRect *itemRect);
	void DrawTheText(CDC* pDC, CRect *itemRect);

	void InitToolTip();

	int m_iBackgroundNum;

	HBITMAP m_hBitmap[BTNST_MAX];
	HRGN m_hRgn;

	DWORD m_dwWidth;
	DWORD m_dwHeight;

	BOOL		m_bMouseOnButton;	// Is mouse over the button?
	BOOL		m_bIsPressed;		// Is button pressed?
	BOOL		m_bIsDisabled;		// Is button disabled?
	BOOL		m_bIsFlashing;		// Is button flashing?
	BOOL		m_bIsFlashOn;		// Is button flash on?
	BOOL		m_bIsStopFlashingWhenClick;	//按下按钮后停止闪烁
	BOOL		m_bIsCheckBox;		// Is the button a checkbox?
	int			m_nCheck;			// Current value for checkbox
	BYTE		m_byAlign;			// Align mode
	COLORREF	m_TextColors[BTNST_MAX];	// Colors to be used
	HCURSOR		m_hCursor;			// Handle to cursor
	CToolTipCtrl m_ToolTip;			// Tooltip
	BOOL m_bTrans;
	COLORREF	m_TransColor;

	CFont		m_font;

	BYTE		m_TextAlign;
	int			m_nIndexBitmap;

	DWORD_PTR	m_dwExtendData;//这个数据当前是记录RSS List中每个下载按钮的nIndex

	int			m_nListStyle;//这个数据记录RssList显示模式
	int			m_nListInfoBt;//这个数据记录RssList Info模式3个按钮
	int			m_nSearchClearButton;//搜索框"X"按钮

public:
	void SetExtendData(DWORD_PTR dwPtr) {m_dwExtendData = dwPtr;}
	DWORD_PTR GetExtendData()const {return m_dwExtendData;}

	void SetListStyle(int nListStyle) {m_nListStyle = nListStyle;}
	int  GetListStyle()const {return m_nListStyle;}

	void SetListInfoBt(int nListInfoBt) {m_nListInfoBt = nListInfoBt;}
	int  GetListInfoBt()const {return m_nListInfoBt;}

	void SetClearButton(int nSearchClearButton) {m_nSearchClearButton = nSearchClearButton;}
	int  GetClearButton()const {return m_nSearchClearButton;}

	void SetTextColor(COLORREF crColor, BOOL bRepaint = TRUE);
	void SetTextColor(COLORREF crColorUp, COLORREF crColorDown, COLORREF crColorUpOn, COLORREF crColorDisabled, BOOL bRepaint = TRUE);
	void SetTextAlign(BYTE TextAlign, BOOL bRepaint = TRUE);

	void SetCheck(int nCheck, BOOL bRepaint = TRUE);
	int GetCheck();

	void SetTooltipText(int nText, BOOL bActivate = TRUE);
	void SetTooltipText(LPCTSTR lpszText, BOOL bActivate = TRUE);
	void ActivateTooltip(BOOL bEnable = TRUE);

	void SetBtnCursor(int nCursorId);
	void SetBtnCursor(HCURSOR hCursor);

	void SetAlign(BYTE byAlign, BOOL bRepaint = TRUE);

	BOOL SetBitmaps(HMODULE hModule, int nBitmap, int nBitmapNum,int nInterval = 0);
	BOOL SetBitmaps(int nBitmap, int nBitmapNum,int nInterval = 0);
	BOOL SetBitmaps(LPCTSTR lpszRes,int nBitmapNum,int nInterval = 0);
	BOOL SetBitmaps(HBITMAP hBitmap, int nBitmapNum,int nInterval = 0);
	void SetBitmapIndex(int nIndex,BOOL bRepaint = TRUE);

	void SetPos(int x, int y, int cx=0, int cy=0);

	BOOL bUseBitmapBack;

	void SetTransparent(DWORD cTransColor = RGB(0, 0, 0));

	void SetFlashing(BOOL bEnable = TRUE, BOOL bIsStopFlashingWhenClick = TRUE);
	BOOL IsFlashing();

	void SetFont(const LPCTSTR strFont, BOOL bBold, int nSize);


public:
	static HRGN BitmapToRegion (HBITMAP hBmp, COLORREF cTransparentColor, COLORREF cTolerance);
	static HBITMAP ImgPngRes2Bitmap(HMODULE hModule, LPCTSTR szRes);
	static HBITMAP ImgPngMem2Bitmap(BYTE *PngData, int PngDataLength);

public:
		afx_msg void OnBnClicked();

protected:
    //{{AFX_MSG(CButtonST)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnCancelMode();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif
