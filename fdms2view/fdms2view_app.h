/* Written by Barna Farago <brown@weblapja.com> 2006-2018
 * fdms2view_app.h : main header file for the testermfc_fdms2lib application
 */
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "MainFrm.h"
#include "Player.h"

// CFdms2View_App:
// See testermfc_fdms2lib.cpp for the implementation of this class
//

class CFdms2View_App : public CWinApp
{
public:
	CFdms2View_App();
virtual ~CFdms2View_App();
// Overrides
public:
	virtual BOOL InitInstance();
protected:
    CMultiDocTemplate* m_pDocTemplate;
    CMainFrame* m_pMainFrame;
    CPlayer m_player;
// Implementation
	afx_msg void OnAppAbout();
    afx_msg void OnFileNew();
	DECLARE_MESSAGE_MAP()
};

extern CFdms2View_App theApp;

//Global tool functions
bool CStringToCharBuff(const CString& cs, char* &buf, int len);