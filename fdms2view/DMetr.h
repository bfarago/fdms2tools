#pragma once
#include "afxwin.h"
#include <fdms2.h>

// DMetr dialog

class DMetr : public CDialog
{
	DECLARE_DYNAMIC(DMetr)

public:
	DMetr(CWnd* pParent = NULL);   // standard constructor
	virtual ~DMetr();
	fdms2* m_fdms2;
	int m_SelectedProgram;
// Dialog Data
	enum { IDD = IDD_DIALOGMetr };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// Listbox for metrums
	CListBox m_lbMetr;
	
	int m_iMBar;
	int m_iMQ;
	int m_iM1;
	int m_iM2;

	int m_iTBar;
	int m_iTQ;
	int m_iTempo;

    int m_iMTCO_H;
    int m_iMTCO_M;
	int m_iMTCO_S;
    int m_iMTCO_F;
    int m_iMTCO_SF;

	CButton m_bnMAdd;
	CButton m_bnTAdd;
	CListBox m_lbTempo;
	virtual BOOL OnInitDialog();
};
