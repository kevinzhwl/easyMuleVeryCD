#pragma once


// CComboBoxPlus

class CComboBoxPlus : public CComboBox
{
	DECLARE_DYNAMIC(CComboBoxPlus)

public:
	CComboBoxPlus();
	virtual ~CComboBoxPlus();

protected:
	DECLARE_MESSAGE_MAP()


	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
};


