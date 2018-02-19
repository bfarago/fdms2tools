#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "resource.h"
#include "iipccom.h"
#include "ipccom.h"


// DGrab dialog

class DGrab : public CDialog
{
	DECLARE_DYNAMIC(DGrab)

public:
	DGrab(CWnd* pParent = NULL);   // standard constructor
	virtual ~DGrab();
    bool getFileName(CString &s){
        s=m_csFileName;
        return true;
    }
// Dialog Data
	enum { IDD = IDD_DIALOG_GRAB };
    enum eState { sInit, sListed, sGrabbing,sFinished, sFinishedFail, sCancel} ;
    virtual BOOL OnInitDialog();
protected:
    long m_diskSize;
    CIpcComSE m_ipcList;
    CIpcComSE m_ipcGrab;
    CString m_csFileName;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    void EnterState(DGrab::eState state);
    void OnGrabbingTimer();
	DECLARE_MESSAGE_MAP()
public:
    
    // List of input devices
    CComboBox m_cbInDevices;
    CEdit m_editOutputFile;
    CButton m_bnStop;
    CButton m_bnGrab;
    // List devices
    CButton m_bnList;
    CButton m_bnOk;
    CButton m_bnOutBrowse;
    CProgressCtrl m_progress1;
    afx_msg void OnBnClickedButtonStop();
    afx_msg void OnBnClickedButtonGrab();
    afx_msg void OnBnClickedButtonList();
    afx_msg void OnBnClickedButtonOutBrowse();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    CStatic m_sProgress;
};
