#pragma once
#include <fdms2.h>
#include "afxwin.h"

// DFrag dialog

class DFrag : public CDialog
{
	DECLARE_DYNAMIC(DFrag)

public:
	DFrag(CWnd* pParent = NULL);   // standard constructor
	virtual ~DFrag();

// Dialog Data
	enum { IDD = IDD_DIALOG_FRAG };
	fdms2* m_Fdms2;
	int	   m_SelectedProgram;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CListBox m_lbFrag;
};
