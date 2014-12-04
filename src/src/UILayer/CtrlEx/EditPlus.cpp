// EditPlus.cpp : 实现文件
//

#include "stdafx.h"
#include "EditPlus.h"
#include ".\editplus.h"


// CEditPlus

IMPLEMENT_DYNAMIC(CEditPlus, CEdit)
CEditPlus::CEditPlus()
{
}

CEditPlus::~CEditPlus()
{
}


BEGIN_MESSAGE_MAP(CEditPlus, CEdit)
END_MESSAGE_MAP()



// CEditPlus 消息处理程序


LRESULT CEditPlus::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if ( message == WM_INPUTLANGCHANGEREQUEST )
	{
		if(GetKeyboardLayout(0) == (HKL)lParam)
			return 0;
		UINT count = GetKeyboardLayoutList(0,NULL);
		if(count == 0) return 0;
		HKL* lpList = new HKL[count];

		count = GetKeyboardLayoutList(count,lpList);
		BOOL bFound = FALSE;
		for(int i=0;i<count;i++)
		{
			if((HKL)lParam == lpList[i])
			{
				bFound = TRUE;
				break;
			}
		}
		if(lpList)
		{
			delete[] lpList;
			lpList = NULL;
		}
		if(!bFound)
		{
			CString sID;
			sID.Format( L"%08x",lParam );
			LoadKeyboardLayout(sID,KLF_ACTIVATE);
		}
		else
		{
			ActivateKeyboardLayout((HKL)lParam,KLF_REORDER);
		}

		return 0;
	}	

	return CEdit::WindowProc(message, wParam, lParam);
}
