#pragma once
#include "afxwin.h"
//#include "fdms2view_doc.h"
#include "afxcmn.h"

#ifdef USE_DSOUND
#pragma comment(linker,"\dx\samples\VC70\common\$(OutDir)\DXUT.lib") 
#pragma comment(linker,"dsound.lib") 
#pragma comment(linker,"dxerr.lib") 
#endif

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
};
