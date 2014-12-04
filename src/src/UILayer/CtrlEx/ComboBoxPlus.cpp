// ComboBoxPlus.cpp : 实现文件
//

#include "stdafx.h"
#include "ComboBoxPlus.h"
#include ".\comboboxplus.h"


// CComboBoxPlus

IMPLEMENT_DYNAMIC(CComboBoxPlus, CComboBox)
CComboBoxPlus::CComboBoxPlus()
{
}

CComboBoxPlus::~CComboBoxPlus()
{
}


BEGIN_MESSAGE_MAP(CComboBoxPlus, CComboBox)
END_MESSAGE_MAP()



// CComboBoxPlus 消息处理程序


LRESULT CComboBoxPlus::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
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

	return CComboBox::WindowProc(message, wParam, lParam);
}

