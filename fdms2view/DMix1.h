#pragma once
#include "afxcmn.h"
#include "Mixer.h"
#include "VUMeters.h"



// DMix1 dialog

class DMix1 : public CDialog, public IVUMeters
{
	DECLARE_DYNAMIC(DMix1)

public:
	DMix1(CWnd* pParent = NULL);   // standard constructor
	virtual ~DMix1();
    void registerMixer(CMixer* mixer);
    //long getAmp(long v, int ch);
// Dialog Data
	enum { IDD = IDD_DIALOG_MIX1 };
	virtual void DoReset();
	void DoGet();
	void DoSetVumeters();
    void DoSetFaders();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    bool m_bRun;
    CMixer* m_pMixer;
	DECLARE_MESSAGE_MAP()
public:
	CProgressCtrl m_pr[10];
	CSliderCtrl m_sl[9];
    afx_msg void OnNMReleasedcaptureSliders(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    virtual BOOL OnInitDialog();
};
