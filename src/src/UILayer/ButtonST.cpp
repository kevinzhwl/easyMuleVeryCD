#include "stdafx.h"
#include "ButtonST.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "png.h"

#ifdef __cplusplus
}
#endif // __cplusplus

struct png_io_ptr_struct
{
	BYTE *mem_curpos;
	BYTE *mem_endpos;
};

// prototypes, optionally connect these to whatever you log errors with
void user_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	png_io_ptr_struct *png_io_ptr = (png_io_ptr_struct *)png_ptr->io_ptr;
	if(png_io_ptr->mem_curpos + length <= png_io_ptr->mem_endpos)
	{
		CopyMemory(data, png_io_ptr->mem_curpos, length);
		png_io_ptr->mem_curpos += length;
	}
}

void user_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	png_io_ptr_struct *png_io_ptr = (png_io_ptr_struct *)png_ptr->io_ptr;
	if(png_io_ptr->mem_curpos + length <= png_io_ptr->mem_endpos)
	{
		CopyMemory(png_io_ptr->mem_curpos, data, length);
		png_io_ptr->mem_curpos += length;
	}
}

void user_flush_data(png_structp png_ptr)
{

}

void PNGAPI user_error_fn(png_structp png, png_const_charp sz)
{
}

void PNGAPI user_warning_fn(png_structp png, png_const_charp sz)
{

}
/////////////////////////////////////////////////////////////////////////////
// SkinButtonST

CButtonST::CButtonST()
{
	m_bIsPressed		= FALSE;
	m_bIsDisabled		= FALSE;
	m_bMouseOnButton	= FALSE;
	m_bIsFlashing		= FALSE;
	m_bIsStopFlashingWhenClick	= TRUE;
	m_bIsFlashOn		= FALSE;

	m_byAlign = ST_ALIGN_HORIZ;	// By default icon is aligned horizontally

	m_bIsCheckBox = FALSE;		// By default the button is not a checkbox
	m_nCheck = 0;

	m_ToolTip.m_hWnd = NULL;	// No tooltip created

	m_hCursor = NULL;			// No cursor defined

	int i;
	for(i=0;i<BTNST_MAX;i++)
		m_hBitmap[i] = NULL;
	m_iBackgroundNum = -1;				// 无位图
	m_dwWidth = 0;
	m_dwHeight = 0;

	for(i=0;i<BTNST_MAX;i++)
		m_TextColors[i] = 0;			// 文字色是黑色

	bUseBitmapBack = FALSE;		// 不使用位图背景
	m_bTrans = FALSE;

	m_TextAlign = TEXT_ALIGN_LEFT;

	m_nIndexBitmap = -1;
	m_dwExtendData = 0;
	m_nListStyle = 0;
	m_nListInfoBt = 0;
	m_nSearchClearButton = 0;
} // End of CButtonST

CButtonST::~CButtonST()
{
	// Destroy bitmaps
	for(int i=0;i<BTNST_MAX;i++)
	{
		if(m_hBitmap[i])
		{
			::DeleteObject(m_hBitmap[i]);
			m_hBitmap[i] = NULL;
		}
	}

	// Destroy the cursor (if any)
	if (m_hCursor)
	{
		::DestroyCursor(m_hCursor);
		m_hCursor = NULL;
	}

	m_font.DeleteObject();
	m_dwExtendData = 0;
	m_nListStyle = 0;
	m_nListInfoBt = 0;
	m_nSearchClearButton = 0;
} // End of ~CButtonST

BEGIN_MESSAGE_MAP(CButtonST, CButton)
    //{{AFX_MSG_MAP(CButtonST)
	ON_WM_ERASEBKGND()
	ON_WM_SETCURSOR()
	ON_WM_KILLFOCUS()
	ON_WM_MOUSEMOVE()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_ACTIVATE()
	ON_WM_ENABLE()
	ON_WM_CANCELMODE()
	ON_WM_GETDLGCODE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP

	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_MESSAGE(BM_SETCHECK, OnSetCheck)
	ON_MESSAGE(BM_GETCHECK, OnGetCheck)
	ON_CONTROL_REFLECT(BN_CLICKED, &CButtonST::OnBnClicked)
END_MESSAGE_MAP()

void CButtonST::OnBnClicked()
{
	if ( m_nListStyle > 0 )//模式切换
	{
		GetOwner()->SendMessage(WM_LISTCTRL_SWITCH_SHOWMODE_CLICKED,(WPARAM)m_nListStyle,0);
	}
	else if ( m_nListInfoBt > 0 )//info List按钮事件
	{
		GetOwner()->SendMessage(WM_LISTCTRL_INFO_CLICKED,(WPARAM)m_nListInfoBt,0);
	}
	else if ( m_nSearchClearButton > 0 )//Clear SearchEdit按钮事件
	{
		GetOwner()->SendMessage(WM_SEARCHEDIT_CLEAR,(WPARAM)m_nSearchClearButton,0);
	}
	else//下载按钮
		GetOwner()->SendMessage(WM_LISTCTRLCLICKED,(WPARAM)m_dwExtendData,0);

	//if (m_nListStyle == 1 || m_nListStyle == 2)
	//{
	//	GetOwner()->SendMessage(WM_LISTCTRL_SWITCH_SHOWMODE_CLICKED,(WPARAM)m_nListStyle,0);
	//}
	//else
	//	GetOwner()->SendMessage(WM_LISTCTRLCLICKED,(WPARAM)m_dwExtendData,0);
}


void CButtonST::PreSubclassWindow() 
{
	UINT nBS;

	nBS = GetButtonStyle();

	// Check if this is a checkbox
	if (nBS & BS_CHECKBOX)
		m_bIsCheckBox = TRUE;

	// Switch to owner-draw
	ModifyStyle(0, BS_OWNERDRAW, SWP_FRAMECHANGED);


	LOGFONT LogFont;
	CFont* pFont = GetFont();
	if(!pFont)
		pFont = AfxGetMainWnd()->GetFont();
	if (!pFont)
	{
		SetFont(_T("宋体"),0,12);
	}
	else
	{
		pFont->GetObject(sizeof(LogFont),&LogFont);
		m_font.CreateFontIndirect(&LogFont);
	}

	CButton::PreSubclassWindow();
} // End of PreSubclassWindow

BOOL CButtonST::PreTranslateMessage(MSG* pMsg) 
{
	if(m_ToolTip.m_hWnd)
	{
		m_ToolTip.RelayEvent(pMsg);	//这个是干什么的，我没搞懂
	}

	if (pMsg->message == WM_LBUTTONDBLCLK)
		pMsg->message = WM_LBUTTONDOWN;
	
	return CButton::PreTranslateMessage(pMsg);
} // End of PreTranslateMessage

LRESULT CButtonST::OnSetCheck(WPARAM wParam, LPARAM lParam)
{
	ASSERT(m_bIsCheckBox);

	switch (wParam)
	{
		case BST_CHECKED:
		case BST_INDETERMINATE:	// Indeterminate state is handled like checked state
			SetCheck(1);
			break;
		default:
			SetCheck(0);
			break;
	} // switch

	return 0;
} // End of OnSetCheck

LRESULT CButtonST::OnGetCheck(WPARAM wParam, LPARAM lParam)
{
	ASSERT(m_bIsCheckBox);
	return GetCheck();
} // End of OnGetCheck

void CButtonST::OnEnable(BOOL bEnable) 
{
	CButton::OnEnable(bEnable);
	
	if (bEnable == FALSE)	
	{
		CWnd*	pWnd = GetParent()->GetNextDlgTabItem(this);
		if (pWnd)
			pWnd->SetFocus();
		else
			GetParent()->SetFocus();

		CancelHover();
	} // if
	else
	{
		//m_ToolTip.Activate(TRUE);		//原来没有这一句话的
	}
} // End of OnEnable

void CButtonST::OnKillFocus(CWnd * pNewWnd)
{
	CButton::OnKillFocus(pNewWnd);
	CancelHover();
} // End of OnKillFocus

void CButtonST::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CButton::OnActivate(nState, pWndOther, bMinimized);

	if (nState == WA_INACTIVE)
		CancelHover();
} // End of OnActivate

void CButtonST::OnCancelMode() 
{
	CButton::OnCancelMode();
	CancelHover();
} // End of OnCancelMode

BOOL CButtonST::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// If a cursor was specified then use it!
	if (m_hCursor != NULL)
	{
		::SetCursor(m_hCursor);
		return TRUE;
	} // if

	return CButton::OnSetCursor(pWnd, nHitTest, message);
} // End of OnSetCursor

void CButtonST::CancelHover()
{
	if (m_bMouseOnButton)
	{
		m_bMouseOnButton = FALSE;
		Invalidate();
	} // if
} // End of CancelHover

void CButtonST::SetBitmapIndex(int nIndex,BOOL bRepaint)
{
	if (nIndex >=0 && nIndex < BTNST_MAX)
		m_nIndexBitmap = nIndex;

	if (bRepaint)
		Invalidate(TRUE);
}

void CButtonST::OnMouseMove(UINT nFlags, CPoint point)
{
	CButton::OnMouseMove(nFlags, point);

	// If the mouse enter the button with the left button pressed then do nothing
	if (nFlags & MK_LBUTTON && m_bMouseOnButton == FALSE)
		return;

	CWnd*				wndUnderMouse = NULL;
	CWnd*				wndActive = this;
	TRACKMOUSEEVENT		csTME;

	ClientToScreen(&point);
	wndUnderMouse = WindowFromPoint(point);

	wndActive = GetActiveWindow();

	if (wndUnderMouse && wndUnderMouse->m_hWnd == m_hWnd && wndActive)
	{
		if (!m_bMouseOnButton)
		{
			m_bMouseOnButton = TRUE;

			Invalidate();

			csTME.cbSize = sizeof(csTME);
			csTME.dwFlags = TME_LEAVE;
			csTME.hwndTrack = m_hWnd;
			::_TrackMouseEvent(&csTME);
		} // if
	} else CancelHover();
} // End of OnMouseMove

// Handler for WM_MOUSELEAVE
LRESULT CButtonST::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	CancelHover();

	return 0;
} // End of OnMouseLeave

void CButtonST::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	CDC*	pDC = CDC::FromHandle(lpDIS->hDC);

	// Checkbox?
	if (m_bIsCheckBox)
		m_bIsPressed  =  (lpDIS->itemState & ODS_SELECTED) || (m_nCheck != 0);
	else	// Normal button OR other button style ...
		m_bIsPressed = (lpDIS->itemState & ODS_SELECTED);

	m_bIsDisabled = (lpDIS->itemState & ODS_DISABLED);

	CRect itemRect = lpDIS->rcItem;

	if(!bUseBitmapBack)	//自己绘制按钮背景
		DrawDefaultBackground(pDC, &itemRect);
	else
		DrawBitmapBackground(pDC, &itemRect);

	DrawTheText(pDC, &itemRect);

} // End of DrawItem

void CButtonST::DrawDefaultBackground(CDC* pDC, CRect *itemRect)
{
	CPen*	pOldPen;

	// Draw pressed button
	if (m_bIsPressed)
	{
		CBrush brBtnShadow(GetSysColor(COLOR_BTNSHADOW));
		pDC->FrameRect(itemRect, &brBtnShadow);
	}
	else // ...else draw non pressed button
	{
		CPen penBtnHiLight(PS_SOLID, 0, GetSysColor(COLOR_BTNHILIGHT)); // White
		CPen pen3DLight(PS_SOLID, 0, GetSysColor(COLOR_3DLIGHT));       // Light gray
		CPen penBtnShadow(PS_SOLID, 0, GetSysColor(COLOR_BTNSHADOW));   // Dark gray
		CPen pen3DDKShadow(PS_SOLID, 0, GetSysColor(COLOR_3DDKSHADOW)); // Black

		// Draw top-left borders
		// White line
		pOldPen = pDC->SelectObject(&penBtnHiLight);
		pDC->MoveTo(itemRect->left, itemRect->bottom-1);
		pDC->LineTo(itemRect->left, itemRect->top);
		pDC->LineTo(itemRect->right, itemRect->top);
		// Light gray line
		pDC->SelectObject(pen3DLight);
		pDC->MoveTo(itemRect->left+1, itemRect->bottom-1);
		pDC->LineTo(itemRect->left+1, itemRect->top+1);
		pDC->LineTo(itemRect->right, itemRect->top+1);
		// Draw bottom-right borders
		// Black line
		pDC->SelectObject(pen3DDKShadow);
		pDC->MoveTo(itemRect->left, itemRect->bottom-1);
		pDC->LineTo(itemRect->right-1, itemRect->bottom-1);
		pDC->LineTo(itemRect->right-1, itemRect->top-1);
		// Dark gray line
		pDC->SelectObject(penBtnShadow);
		pDC->MoveTo(itemRect->left+1, itemRect->bottom-2);
		pDC->LineTo(itemRect->right-2, itemRect->bottom-2);
		pDC->LineTo(itemRect->right-2, itemRect->top);
		//
		pDC->SelectObject(pOldPen);
	} // else
}

void CButtonST::DrawBitmapBackground(CDC* pDC, CRect *itemRect)
{
//	if(!m_hBackground)
//		return;

	HDC			hdcBmpMem	= NULL;
	HBITMAP		hbmOldBmp	= NULL;

	BYTE		byIndex		= BTNST_UP;

	// Select the bitmap to use
	if(m_nIndexBitmap == -1)
	{
		if (m_bIsPressed && m_iBackgroundNum >= 1)//按下
			byIndex = BTNST_DOWN;

		if(m_bMouseOnButton && !m_bIsPressed && m_iBackgroundNum >= 2)//在按钮上面
		{
#if 0
			if((m_nCheck || m_bIsPressed) && m_iBackgroundNum >= 3)
				byIndex = BTNST_DOWN_ON;
			else
#endif
				byIndex = BTNST_UP_ON;
		}

		if(m_bIsDisabled && m_iBackgroundNum >= 3)//不被允许
			byIndex = BTNST_DISABLED;

		if(m_bIsFlashing && m_iBackgroundNum >= 4)	//闪烁状态
		{
			if(m_bIsPressed && m_bIsStopFlashingWhenClick)		//按下按钮后自动关闭FLASHING
				SetFlashing(FALSE);
			else if(m_bIsFlashOn)	//闪烁开/关？	//!m_nCheck && m_bIsFlashOn
				byIndex = BTNST_FLASH;
		}

		if(byIndex > m_iBackgroundNum)
			byIndex = BTNST_UP;
	}
	else
		byIndex = (BYTE)m_nIndexBitmap;

	hdcBmpMem = ::CreateCompatibleDC(pDC->m_hDC);

	hbmOldBmp = (HBITMAP)::SelectObject(hdcBmpMem, m_hBitmap[byIndex]);

	if(m_bTrans)
		::TransparentBlt(pDC->m_hDC, 0, 0, m_dwWidth, m_dwHeight, hdcBmpMem, 0, 0, m_dwWidth, m_dwHeight, m_TransColor);
	else
		::BitBlt(pDC->m_hDC, 0, 0, m_dwWidth, m_dwHeight, hdcBmpMem, 0, 0, SRCCOPY);

	::SelectObject(hdcBmpMem, hbmOldBmp);
	::DeleteDC(hdcBmpMem);
	
} // End of DrawBitmapBackground

void CButtonST::DrawTheText(CDC* pDC, CRect *itemRect)
{
	CString sTitle;
	GetWindowText(sTitle);
	if(sTitle.IsEmpty())// Write the button title (if any)
		return;

	pDC->SetBkMode(TRANSPARENT);
	
	CFont *pOldFont = pDC->SelectObject(&m_font);

	// Center text
	CRect rpCaption = *itemRect;
	CRect centerRect = *itemRect;
	pDC->DrawText(sTitle, -1, rpCaption, DT_WORDBREAK | DT_CENTER | DT_CALCRECT);
	
	if(m_TextAlign == TEXT_ALIGN_LEFT)
	{
	}
	else if (m_TextAlign == TEXT_ALIGN_DIY)
	{
		rpCaption.OffsetRect((centerRect.Width() - rpCaption.Width())/2+3, (centerRect.Height() - rpCaption.Height())/2);
	}
	else if(m_TextAlign == TEXT_ALIGN_CENTER)
		rpCaption.OffsetRect((centerRect.Width() - rpCaption.Width())/2, (centerRect.Height() - rpCaption.Height())/2);
	else
		rpCaption.OffsetRect(centerRect.Width() - rpCaption.Width(), centerRect.Height() - rpCaption.Height());

	if (m_bIsDisabled)
	{
		rpCaption.OffsetRect(1, 1);
		pDC->SetTextColor(::GetSysColor(COLOR_3DHILIGHT));
		pDC->DrawText(sTitle, -1, &rpCaption, DT_WORDBREAK | DT_CENTER);
		rpCaption.OffsetRect(-1, -1);
		pDC->SetTextColor(::GetSysColor(COLOR_3DSHADOW));
		pDC->DrawText(sTitle, -1, &rpCaption, DT_WORDBREAK | DT_CENTER);
	} // if
	else
	{
		if (m_bIsPressed)
		{
			//rpCaption.OffsetRect(1, 0);
			pDC->SetTextColor(m_TextColors[BTNST_DOWN]);
		}
		else if(m_bMouseOnButton)
		{
			if(m_nCheck)
			{
				//rpCaption.OffsetRect(1, 0);
				pDC->SetTextColor(m_TextColors[BTNST_UP_ON]);
			}
#if 0
			else
			{
				//rpCaption.OffsetRect(1, 0);
				pDC->SetTextColor(m_TextColors[BTNST_DOWN_ON]);
			}
#endif
		}
		else
			pDC->SetTextColor(m_TextColors[BTNST_UP]);
		pDC->DrawText(sTitle, -1, &rpCaption, DT_WORDBREAK | DT_CENTER);
	} // if

	pDC->SelectObject(pOldFont);

} // End of DrawTheText

BOOL CButtonST::SetBitmaps(HMODULE hModule, int nBitmap, int nBitmapNum,int nInterval)
{
	HBITMAP		hBitmap		= NULL;

	// Load bitmap In

	hBitmap = ImgPngRes2Bitmap(hModule, MAKEINTRESOURCE(nBitmap));
	if(!hBitmap)
	{
		hBitmap = (HBITMAP)::LoadImage(hModule, MAKEINTRESOURCE(nBitmap), IMAGE_BITMAP, 0, 0, 0);
	}

	BOOL result = SetBitmaps(hBitmap, nBitmapNum,nInterval);

	DeleteObject(hBitmap);

	return result;
} // End of SetBitmaps

BOOL CButtonST::SetBitmaps(LPCTSTR lpszRes,int nBitmapNum,int nInterval)
{
	HBITMAP		hBitmap		= NULL;
	HINSTANCE	hInstResource	= NULL;


	hInstResource = AfxFindResourceHandle(lpszRes, _T("PNG"));
	hBitmap = ImgPngRes2Bitmap(hInstResource,lpszRes);
	if(!hBitmap)
	{
		// Find correct resource handle
		hInstResource = AfxFindResourceHandle(lpszRes, RT_BITMAP);
		hBitmap = (HBITMAP)::LoadImage(hInstResource, lpszRes, IMAGE_BITMAP, 0, 0, 0);		
	}

	BOOL result = SetBitmaps(hBitmap, nBitmapNum,nInterval);

	DeleteObject(hBitmap);

	return result;
}

BOOL CButtonST::SetBitmaps(int nBitmap, int nBitmapNum,int nInterval)
{
	HBITMAP		hBitmap		= NULL;
	HINSTANCE	hInstResource	= NULL;
	

	hInstResource = AfxFindResourceHandle(MAKEINTRESOURCE(nBitmap), _T("PNG"));
	hBitmap = ImgPngRes2Bitmap(hInstResource, MAKEINTRESOURCE(nBitmap));
	if(!hBitmap)
	{
		// Find correct resource handle
		hInstResource = AfxFindResourceHandle(MAKEINTRESOURCE(nBitmap), RT_BITMAP);
		hBitmap = (HBITMAP)::LoadImage(hInstResource, MAKEINTRESOURCE(nBitmap), IMAGE_BITMAP, 0, 0, 0);		
	}

	BOOL result = SetBitmaps(hBitmap, nBitmapNum,nInterval);

	DeleteObject(hBitmap);

	return result;
} // End of SetBitmaps

BOOL CButtonST::SetBitmaps(HBITMAP hBitmap, int nBitmapNum,int nInterval)
{
	// Free any loaded resource
	for(int i= 0;i <BTNST_MAX; i++)
	{
		if(m_hBitmap[i])
		{
			::DeleteObject(m_hBitmap[i]);
			m_hBitmap[i] = NULL;
		}
	}
	
	if (hBitmap && nBitmapNum > BTNST_UP && nBitmapNum <= BTNST_MAX)
	{
		m_iBackgroundNum = nBitmapNum - 1;
		
		{
			// Get bitmap size
			BITMAP	csBitmapSize;
			if(::GetObject(hBitmap, sizeof(csBitmapSize), &csBitmapSize) == 0)
				return FALSE;

			m_dwWidth  = (DWORD)(csBitmapSize.bmWidth - nInterval * (nBitmapNum - 1)) / nBitmapNum;
			m_dwHeight = (DWORD)csBitmapSize.bmHeight;
			
			{				
				CDC *pDC = GetDC();
				HDC hdcSrc = ::CreateCompatibleDC(pDC->m_hDC);
				HBITMAP hbmSrcOld = (HBITMAP)::SelectObject(hdcSrc, hBitmap);
				
				for(int i=0; i < nBitmapNum; i++)
				{
					m_hBitmap[i] = ::CreateCompatibleBitmap(pDC->m_hDC, csBitmapSize.bmWidth, csBitmapSize.bmHeight);
					HDC hdcDest = ::CreateCompatibleDC(pDC->m_hDC);
					HBITMAP hbmDestOld = (HBITMAP)::SelectObject(hdcDest, m_hBitmap[i]);
					
					BitBlt(hdcDest, 0, 0, m_dwWidth, m_dwHeight, hdcSrc, m_dwWidth * i + nInterval * i, 0, SRCCOPY);
					
					::SelectObject(hdcDest, hbmDestOld);
					DeleteDC(hdcDest);
				}
			
				::SelectObject(hdcSrc, hbmSrcOld);
				DeleteDC(hdcSrc);	
				ReleaseDC(pDC);
			}
			
			SetWindowPos(NULL, 0, 0, m_dwWidth, m_dwHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE);
		}
		
	} // if
	
	bUseBitmapBack = TRUE;
	
	Invalidate();

	if(hBitmap)
		DeleteObject(hBitmap);

	return TRUE;
} // End of SetBitmaps

void CButtonST::SetAlign(BYTE byAlign, BOOL bRepaint)
{
	switch (byAlign)
	{    
		case ST_ALIGN_HORIZ:
		case ST_ALIGN_VERT:
			m_byAlign = byAlign;
			if (bRepaint)
				Invalidate();
			break;
	} // switch
} // End of SetAlign

void CButtonST::SetCheck(int nCheck, BOOL bRepaint)
{
	if (m_bIsCheckBox)
	{
		if (nCheck == 0)
			m_nCheck = 0;
		else
			m_nCheck = 1;

		if (bRepaint)
			Invalidate();
	} // if
} // End of SetCheck

int CButtonST::GetCheck()
{
	return m_nCheck;
} // End of GetCheck

void CButtonST::SetTextColor(COLORREF crColor, BOOL bRepaint)
{
	m_TextColors[BTNST_UP] = crColor;
	m_TextColors[BTNST_DOWN] = crColor;
	m_TextColors[BTNST_UP_ON] = crColor;
	m_TextColors[BTNST_DISABLED] = crColor;
//	m_TextColors[BTNST_DOWN_ON] = crColor;

	if (bRepaint)
		Invalidate();
}


void CButtonST::SetTextColor(COLORREF crColorUp, COLORREF crColorDown, COLORREF crColorUpOn, COLORREF crColorDisabled, BOOL bRepaint)
{
	m_TextColors[BTNST_UP] = crColorUp;
	m_TextColors[BTNST_DOWN] = crColorDown;
	m_TextColors[BTNST_UP_ON] = crColorUpOn;
	m_TextColors[BTNST_DISABLED] = crColorDisabled;
//	m_TextColors[BTNST_DOWN_ON] = crColorDownOn;

	if (bRepaint)
		Invalidate();
} // End of SetColor

void CButtonST::SetTextAlign(BYTE TextAlign, BOOL bRepaint)
{
	m_TextAlign = TextAlign;

	if (bRepaint)
		Invalidate();
}

void CButtonST::SetBtnCursor(int nCursorId)
{
	// Destroy any previous cursor
	if (m_hCursor)
	{
		::DestroyCursor(m_hCursor);
		m_hCursor = NULL;
	} // if

	// Load cursor
	if (nCursorId)
	{
		HINSTANCE hInstResource = AfxFindResourceHandle(MAKEINTRESOURCE(nCursorId), RT_GROUP_CURSOR);	//就必须是这个，而不是RT_CURSOR
		// Load cursor resource
		m_hCursor = (HCURSOR)::LoadImage(hInstResource, MAKEINTRESOURCE(nCursorId), IMAGE_CURSOR, 0, 0, 0);
	} // if
} // End of SetBtnCursor

void CButtonST::SetBtnCursor(HCURSOR hCursor)
{
	if (m_hCursor)
	{
		::DestroyCursor(m_hCursor);
		m_hCursor = NULL;
	} // if

	if(hCursor)
		m_hCursor = hCursor;
}

void CButtonST::InitToolTip()
{
	if (m_ToolTip.m_hWnd == NULL)
	{
		// Create ToolTip control
		m_ToolTip.Create(this);
		// Create inactive
		m_ToolTip.Activate(FALSE);
		// Enable multiline
		m_ToolTip.SendMessage(TTM_SETMAXTIPWIDTH, 0, 400);
	} // if
} // End of InitToolTip

// This function sets the text to show in the button tooltip.
//
// Parameters:
//		[IN]	nText
//				ID number of the string resource containing the text to show.
//		[IN]	bActivate
//				If TRUE the tooltip will be created active.
//
void CButtonST::SetTooltipText(int nText, BOOL bActivate)
{
	CString sText;

	// Load string resource
	sText.LoadString(nText);
	// If string resource is not empty
	if (sText.IsEmpty() == FALSE) SetTooltipText((LPCTSTR)sText, bActivate);
} // End of SetTooltipText

// This function sets the text to show in the button tooltip.
//
// Parameters:
//		[IN]	lpszText
//				Pointer to a null-terminated string containing the text to show.
//		[IN]	bActivate
//				If TRUE the tooltip will be created active.
//
void CButtonST::SetTooltipText(LPCTSTR lpszText, BOOL bActivate)
{
	// We cannot accept NULL pointer
	if (lpszText == NULL) return;

	// Initialize ToolTip
	InitToolTip();

	// If there is no tooltip defined then add it
	if (m_ToolTip.GetToolCount() == 0)
	{
		CRect rectBtn; 
		GetClientRect(rectBtn);
		m_ToolTip.AddTool(this, lpszText, rectBtn, 1);
	} // if

	// Set text for tooltip
	m_ToolTip.UpdateTipText(lpszText, this, 1);
	m_ToolTip.Activate(bActivate);
} // End of SetTooltipText

// This function enables or disables the button tooltip.
//
// Parameters:
//		[IN]	bActivate
//				If TRUE the tooltip will be activated.
//
void CButtonST::ActivateTooltip(BOOL bActivate)
{
	// If there is no tooltip then do nothing
	if (m_ToolTip.GetToolCount() == 0) return;

	// Activate tooltip
	m_ToolTip.Activate(bActivate);
} // End of EnableTooltip

//设置位置
void CButtonST::SetPos(int x, int y, int cx, int cy)
{
	if(cx == 0 && cy == 0)
		SetWindowPos(NULL, x, y, m_dwWidth, m_dwHeight, SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE);
	else
		SetWindowPos(NULL, x, y, cx, cy, SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE);
}

void CButtonST::SetTransparent(DWORD cTransColor)
{
	m_bTrans = TRUE;
	m_TransColor = cTransColor;

	m_hRgn = BitmapToRegion(m_hBitmap[0], m_TransColor, 0);
	int iResult = SetWindowRgn(m_hRgn, TRUE);
}

BOOL CButtonST::OnEraseBkgnd(CDC* pDC) 
{
	if(bUseBitmapBack)
		return TRUE;
	else
		return CButton::OnEraseBkgnd(pDC);
}

void CButtonST::OnTimer(UINT nIDEvent) 
{

	m_bIsFlashOn = !m_bIsFlashOn;

	RedrawWindow();
	
	CButton::OnTimer(nIDEvent);
}

void CButtonST::SetFlashing(BOOL bEnable, BOOL bIsStopFlashingWhenClick)
{
	if(bEnable)
	{
		m_bIsFlashing = TRUE;
		m_bIsStopFlashingWhenClick = bIsStopFlashingWhenClick;
		SetTimer(0, 500, NULL);
	}
	else
	{
		m_bIsFlashing = FALSE;
		m_bIsStopFlashingWhenClick = bIsStopFlashingWhenClick;
		KillTimer(0);
	}

	RedrawWindow();
}

BOOL CButtonST::IsFlashing()
{
	return m_bIsFlashing;
}

void CButtonST::SetFont(const LPCTSTR strFont, BOOL bBold, int nSize)
{
	//LOGFONT LogFont;

	//GetFont()->GetObject(sizeof(LogFont),&LogFont);

	//if(strFont && _tcslen(strFont) > 0)
	//{
	//	LogFont.lfCharSet = ANSI_CHARSET;
	//	_tcsncpy(LogFont.lfFaceName, strFont, _tcslen(strFont));
	//}

	//LogFont.lfWeight = bBold ? FW_BOLD : FW_NORMAL;

	//CFont cf;	LOGFONT lf;
	//cf.CreatePointFont(nSize * 10, lf1.lfFaceName);
	////DeleteObject(lf);
	//cf.GetLogFont(&lf);
	//LogFont.lfHeight = lf.lfHeight;
	//LogFont.lfWidth  = lf.lfWidth;

	m_font.DeleteObject();
	//m_font.CreateFontIndirect(&LogFont);

	m_font.CreateFont(nSize,0,0,0,bBold,FALSE,FALSE,0,0,0,0,0,0,_TEXT(""));
}

HRGN CButtonST::BitmapToRegion (HBITMAP hBmp, COLORREF cTransparentColor, COLORREF cTolerance)
{
	HRGN hRgn = NULL;

	if (hBmp)
	{
		// Create a memory DC inside which we will scan the bitmap content
		HDC hMemDC = CreateCompatibleDC(NULL);
		if (hMemDC)
		{
			// Get bitmap size
			BITMAP bm;
			GetObject(hBmp, sizeof(bm), &bm);

			// Create a 32 bits depth bitmap and select it into the memory DC 
			BITMAPINFOHEADER RGB32BITSBITMAPINFO = {	
				sizeof(BITMAPINFOHEADER),	// biSize 
					bm.bmWidth,					// biWidth; 
					bm.bmHeight,				// biHeight; 
					1,							// biPlanes; 
					32,							// biBitCount 
					BI_RGB,						// biCompression; 
					0,							// biSizeImage; 
					0,							// biXPelsPerMeter; 
					0,							// biYPelsPerMeter; 
					0,							// biClrUsed; 
					0							// biClrImportant; 
			};
			VOID * pbits32; 
			HBITMAP hbm32 = CreateDIBSection(hMemDC, (BITMAPINFO *)&RGB32BITSBITMAPINFO, DIB_RGB_COLORS, &pbits32, NULL, 0);
			if (hbm32)
			{
				HBITMAP holdBmp = (HBITMAP)SelectObject(hMemDC, hbm32);

				// Create a DC just to copy the bitmap into the memory DC
				HDC hDC = CreateCompatibleDC(hMemDC);
				if (hDC)
				{
					// Get how many bytes per row we have for the bitmap bits (rounded up to 32 bits)
					BITMAP bm32;
					GetObject(hbm32, sizeof(bm32), &bm32);
					while (bm32.bmWidthBytes % 4)
						bm32.bmWidthBytes++;

					// Copy the bitmap into the memory DC
					HBITMAP holdBmp = (HBITMAP)SelectObject(hDC, hBmp);
					BitBlt(hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, hDC, 0, 0, SRCCOPY);

					// For better performances, we will use the ExtCreateRegion() function to create the
					// region. This function take a RGNDATA structure on entry. We will add rectangles by
					// amount of ALLOC_UNIT number in this structure.
#define ALLOC_UNIT	100
					DWORD maxRects = ALLOC_UNIT;
					HANDLE hData = GlobalAlloc(GMEM_MOVEABLE, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects));
					RGNDATA *pData = (RGNDATA *)GlobalLock(hData);
					pData->rdh.dwSize = sizeof(RGNDATAHEADER);
					pData->rdh.iType = RDH_RECTANGLES;
					pData->rdh.nCount = pData->rdh.nRgnSize = 0;
					SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);

					// Keep on hand highest and lowest values for the "transparent" pixels
					BYTE lr = GetRValue(cTransparentColor);
					BYTE lg = GetGValue(cTransparentColor);
					BYTE lb = GetBValue(cTransparentColor);
					BYTE hr = min(0xff, lr + GetRValue(cTolerance));
					BYTE hg = min(0xff, lg + GetGValue(cTolerance));
					BYTE hb = min(0xff, lb + GetBValue(cTolerance));

					// Scan each bitmap row from bottom to top (the bitmap is inverted vertically)
					BYTE *p32 = (BYTE *)bm32.bmBits + (bm32.bmHeight - 1) * bm32.bmWidthBytes;
					for (int y = 0; y < bm.bmHeight; y++)
					{
						// Scan each bitmap pixel from left to right
						for (int x = 0; x < bm.bmWidth; x++)
						{
							// Search for a continuous range of "non transparent pixels"
							int x0 = x;
							LONG *p = (LONG *)p32 + x;
							while (x < bm.bmWidth)
							{
								BYTE b = GetRValue(*p);
								if (b >= lr && b <= hr)
								{
									b = GetGValue(*p);
									if (b >= lg && b <= hg)
									{
										b = GetBValue(*p);
										if (b >= lb && b <= hb)
											// This pixel is "transparent"
											break;
									}
								}
								p++;
								x++;
							}

							if (x > x0)
							{
								// Add the pixels (x0, y) to (x, y+1) as a new rectangle in the region
								if (pData->rdh.nCount >= maxRects)
								{
									GlobalUnlock(hData);
									maxRects += ALLOC_UNIT;
									hData = GlobalReAlloc(hData, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), GMEM_MOVEABLE);
									pData = (RGNDATA *)GlobalLock(hData);
								}
								RECT *pr = (RECT *)&pData->Buffer;
								SetRect(&pr[pData->rdh.nCount], x0, y, x, y+1);
								if (x0 < pData->rdh.rcBound.left)
									pData->rdh.rcBound.left = x0;
								if (y < pData->rdh.rcBound.top)
									pData->rdh.rcBound.top = y;
								if (x > pData->rdh.rcBound.right)
									pData->rdh.rcBound.right = x;
								if (y+1 > pData->rdh.rcBound.bottom)
									pData->rdh.rcBound.bottom = y+1;
								pData->rdh.nCount++;

								// On Windows98, ExtCreateRegion() may fail if the number of rectangles is too
								// large (ie: > 4000). Therefore, we have to create the region by multiple steps.
								if (pData->rdh.nCount == 2000)
								{
									HRGN h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);
									if (hRgn)
									{
										CombineRgn(hRgn, hRgn, h, RGN_OR);
										DeleteObject(h);
									}
									else
										hRgn = h;
									pData->rdh.nCount = 0;
									SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);
								}
							}
						}

						// Go to next row (remember, the bitmap is inverted vertically)
						p32 -= bm32.bmWidthBytes;
					}

					// Create or extend the region with the remaining rectangles
					HRGN h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);
					if (hRgn)
					{
						CombineRgn(hRgn, hRgn, h, RGN_OR);
						DeleteObject(h);
					}
					else
						hRgn = h;

					// Clean up
					GlobalUnlock(hData);
					GlobalFree(hData);
					SelectObject(hDC, holdBmp);
					DeleteDC(hDC);
				}

				DeleteObject(SelectObject(hMemDC, holdBmp));
			}

			DeleteDC(hMemDC);
		}	
	}

	return hRgn;
}


HBITMAP CButtonST::ImgPngMem2Bitmap(BYTE *PngData, int PngDataLength)
{
	HBITMAP hbm = NULL;

	BYTE header[8];
	CopyMemory(header, PngData, 8);

	if (png_sig_cmp(header, 0, 8))
		return FALSE;

	// now allocate stuff

	png_structp png_ptr = 
		png_create_read_struct(PNG_LIBPNG_VER_STRING, 
		NULL, user_error_fn, user_warning_fn);

	if (!png_ptr)
		return FALSE;

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_read_struct(&png_ptr,
			(png_infopp)NULL, (png_infopp)NULL);
		return FALSE;
	}

	png_infop end_info = png_create_info_struct(png_ptr);
	if (!end_info)
	{
		png_destroy_read_struct(&png_ptr, &info_ptr,
			(png_infopp)NULL);
		return FALSE;
	}

	if (PngData)
	{
		png_io_ptr_struct png_io_ptr;
		png_io_ptr.mem_curpos = (BYTE *)PngData;
		png_io_ptr.mem_endpos = (BYTE *)PngData + PngDataLength;

		//png_init_io(png_ptr, fp);
		png_set_read_fn(png_ptr, &png_io_ptr, (png_rw_ptr)user_read_data);
		//		png_set_error_fn(png_ptr,info.szLastError,(png_error_ptr)user_error_fn,NULL);

		// should really use png_set_rows() to allocate space first, rather than doubling up

		png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_BGR | PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING, NULL);

		png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);//new png_bytep[info_ptr->height];

		// now for a tonne of ugly DIB setup crap

		int width = info_ptr->width;
		int height = info_ptr->height;
		int bpp = info_ptr->channels * 8;
		int memWidth = (width * (bpp >> 3) + 3) & ~3;

		LPBITMAPINFO lpbi = (LPBITMAPINFO) new char[sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD))];

		// create a greyscale palette 
		for (int a_i = 0; a_i < 256; a_i++)
		{
			lpbi->bmiColors[a_i].rgbRed = (BYTE)a_i;
			lpbi->bmiColors[a_i].rgbGreen = (BYTE)a_i;
			lpbi->bmiColors[a_i].rgbBlue = (BYTE)a_i;
			lpbi->bmiColors[a_i].rgbReserved = 0;
		}

		lpbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		lpbi->bmiHeader.biWidth = width;
		lpbi->bmiHeader.biHeight = -height;	// must be negative for top down
		lpbi->bmiHeader.biPlanes = 1;
		lpbi->bmiHeader.biBitCount = bpp;
		lpbi->bmiHeader.biCompression = BI_RGB;
		lpbi->bmiHeader.biSizeImage = memWidth * height;
		lpbi->bmiHeader.biXPelsPerMeter = 0;
		lpbi->bmiHeader.biYPelsPerMeter = 0;
		lpbi->bmiHeader.biClrUsed = 0;
		lpbi->bmiHeader.biClrImportant = 0;

		BYTE * pixelData;
		hbm = CreateDIBSection(NULL, lpbi, DIB_RGB_COLORS, (void **)&pixelData, NULL, 0 );
		if (hbm && pixelData)
		{
			// now copy the rows
			for (int i = 0; i < height; i++)
				memcpy(pixelData + memWidth * i, row_pointers[i], width * info_ptr->channels);
		}

		delete (char*) lpbi;
	}

	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

	return hbm;
}

HBITMAP CButtonST::ImgPngRes2Bitmap(HMODULE hModule, LPCTSTR szRes)
{
	// check the header first
	HRSRC hRes = FindResource(hModule, szRes, _T("PNG"));
	HGLOBAL hGlobalMem = LoadResource(hModule, hRes); 

	LPVOID ResData = (LPVOID)LockResource(hGlobalMem); 
	int ResLen = SizeofResource(hModule, hRes);
	if(ResLen <= 8)
		return FALSE;

	HBITMAP hbm = ImgPngMem2Bitmap((BYTE *)ResData, ResLen);

	FreeResource(hGlobalMem);

	return hbm;
}

/*
void CButtonST::OnSkinChange(CSkin*pSkin ,CXMLElement* pElement)
{
	ISKinControl::OnSkinChange(pSkin,pElement);

	SKIN_BUTTON SkinButton;
	if(!GetButton(pElement, &SkinButton))
		return;

	if(SkinButton.eCursor == CURSOR_HAND)
		SetBtnCursor(pSkin->GetCursorHand());
	else if(SkinButton.eCursor == CURSOR_MOVE)
		SetBtnCursor(pSkin->GetCursorMove());

	if(SkinButton.strImage.GetLength() > 0)
	{
		HBITMAP image = NULL;
		image = pSkin->GetHBitmap(SkinButton.strImage);
		if(image)
		{
			SetBitmaps(image, SkinButton.nNum);

			if(SkinButton.eTrans == TRANS_BLACK)
				SetTransparent();
			else if(SkinButton.eTrans == TRANS_PINK)
				SetTransparent(RGB(255, 0, 255));
		}
		
		SetPos(SkinButton.spPos.nPosx, SkinButton.spPos.nPosy);
	}
	else
	{
		if(SkinButton.srRect.nWidth > 0 && SkinButton.srRect.nHeight > 0)
			SetPos(SkinButton.srRect.nPosx, SkinButton.srRect.nPosy, SkinButton.srRect.nWidth, SkinButton.srRect.nHeight);
	}

	SetWindowText(SkinButton.strTitle);

	if(SkinButton.strTip.GetLength() > 0)
		SetTooltipText(SkinButton.strTip);
}
*/
