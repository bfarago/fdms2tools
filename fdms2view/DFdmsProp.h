#pragma once
#include "fdms2.h"

// DFdmsProp dialog

class DFdmsProp : public CDialog
{
	DECLARE_DYNAMIC(DFdmsProp)

public:
	DFdmsProp(CWnd* pParent = NULL);   // standard constructor
	virtual ~DFdmsProp();

// Dialog Data
	enum { IDD = IDD_DIALOG_FDMSPROP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_csFileName;
    enPartitionMode m_pm;
    int m_iRadioPM;
	afx_msg void OnBnClickedButtonFnBrowse();
    virtual BOOL OnInitDialog();
protected:
    virtual void OnOK();
};
