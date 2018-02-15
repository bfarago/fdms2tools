#pragma once
#include <fdms2.h>
#include <sndfile.h>
#include "afxwin.h"
#include "afxcmn.h"
#include "fdms2view_doc.h"

// DExport dialog

class DExport : public CDialog, public fdms2streamingIF
{
	DECLARE_DYNAMIC(DExport)

public:
	DExport(CWnd* pParent = NULL);   // standard constructor
	virtual ~DExport();
    
    virtual bool read(fdms2stream* pStream);
    
    void registerFdms2(fdms2* pFdms2);
    void registerDoc(CFdms2View_Doc* pDoc); //TODO: interfacere

// Dialog Data
	enum { IDD = IDD_DIALOG_EXPORT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    bool UpdateProgressBar(fdms2pos start, fdms2pos pos, fdms2pos len);

	void OpenFiles();
	void CloseFiles();
	void InitDialog();
	DECLARE_MESSAGE_MAP()
    bool m_bCh[FOSTEXMAXCHANNELS];
	CButton m_bnCh[FOSTEXMAXCHANNELS];
	CButton m_bnChAll;
	CString m_sPrefix;
    CString m_sOutLPos;
    CString m_sOutLPosRemain;
    CString m_sOutTimeRemain;
	CComboBox m_cbFormat;
    int m_iFormat;
	afx_msg void OnBnClickedCheckall();
	CProgressCtrl m_progress1;
	afx_msg void OnBnClickedOk();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
private:
    bool m_bRun;
    CFdms2View_Doc* m_pDoc;
    fdms2 m_Fdms2;
    fdms2reader m_fdms2reader;
    DWORD m_timeFirst;
    DWORD m_timeLast;
    SNDFILE* m_files[FOSTEXMAXCHANNELS];
	SF_INFO m_sfinfos[FOSTEXMAXCHANNELS];
};
