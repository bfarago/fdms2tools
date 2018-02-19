#pragma once
#include "afxwin.h"
#include "afxcmn.h"

// DAudioDeviceSetup dialog
class CFdms2View_Doc;

class DAudioDeviceSetup : public CDialog
{
	DECLARE_DYNAMIC(DAudioDeviceSetup)

public:
	DAudioDeviceSetup(CWnd* pParent = NULL);   // standard constructor
	virtual ~DAudioDeviceSetup();
	CFdms2View_Doc* m_pDoc;
// Dialog Data
	enum { IDD = IDD_DIALOG_AUDIODEVICE };
	
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void InitSound();
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CComboBox m_cbSoundDevice;
    CComboBox m_cbCaptureDevice;
    CComboBox m_cbBufferOutSize;
    afx_msg void OnCbnSelchangeSoundDeviceCombo();
    
    afx_msg void OnCbnSelchangeCaptureDeviceCombo();
};
