#pragma once
#include "fdms2.h"
#include "afxcmn.h"

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
    void setFdms2Disk(fdms2disk* p){m_fdms2disk=p;}
protected:
    fdms2disk* m_fdms2disk;
    virtual void OnOK();
    void Refresh();
public:
    CListCtrl m_listDisks;
    afx_msg void OnLvnItemActivateListDisk(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnBnClickedButtonRefresh();
};
