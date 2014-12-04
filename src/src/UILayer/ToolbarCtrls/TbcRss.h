/*
 * $Id: TbcRss.h $
 * 
 * this file is part of easyMule
 * Copyright (C)2002-2008 VeryCD Dev Team ( strEmail.Format("%s@%s", "emuledev", "verycd.com") / http: * www.easymule.org )
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#pragma once

// CTbcRss
#include "ToolBarCtrlZ.h"
#include "Localizee.h"

class CTbcRss : public CToolBarCtrlZ, public CLocalizee
{
	DECLARE_DYNAMIC(CTbcRss)
	LOCALIZEE_WND_CANLOCALIZE()

public:
	CTbcRss();
	virtual ~CTbcRss();

	void Localize();

private:
	enum {BUTTON_COUNT = 2};
	void	InitImageList();

protected:
	DECLARE_MESSAGE_MAP()

private:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();

private:
	static CString	GetButtonText(UINT i);

};


