// DAudioDeviceSetup.cpp : implementation file
//

#include "stdafx.h"
#include "fdms2view_app.h"
#include "DAudioDeviceSetup.h"
#include "SoundDeviceEnumeration.h"

// DAudioDeviceSetup dialog

IMPLEMENT_DYNAMIC(DAudioDeviceSetup, CDialog)
DAudioDeviceSetup::DAudioDeviceSetup(CWnd* pParent /*=NULL*/)
	: CDialog(DAudioDeviceSetup::IDD, pParent)
{
}

DAudioDeviceSetup::~DAudioDeviceSetup()
{
}

void DAudioDeviceSetup::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SOUND_DEVICE_COMBO, m_cbSoundDevice);
    DDX_Control(pDX, IDC_CAPTURE_DEVICE_COMBO, m_cbCaptureDevice);
    DDX_Control(pDX, IDC_COMBO_BUFFEROUTSIZE, m_cbBufferOutSize);
}


BEGIN_MESSAGE_MAP(DAudioDeviceSetup, CDialog)
    ON_CBN_SELCHANGE(IDC_SOUND_DEVICE_COMBO, &DAudioDeviceSetup::OnCbnSelchangeSoundDeviceCombo)
    ON_CBN_SELCHANGE(IDC_CAPTURE_DEVICE_COMBO, &DAudioDeviceSetup::OnCbnSelchangeCaptureDeviceCombo)
END_MESSAGE_MAP()


// DAudioDeviceSetup message handlers
void DAudioDeviceSetup::InitSound(){
    CWnd* pcwSoundDeviceCombo = GetDlgItem(  IDC_SOUND_DEVICE_COMBO );
	HWND hSoundDeviceCombo = pcwSoundDeviceCombo->m_hWnd;
    CWnd* pcwCaptureDeviceCombo = GetDlgItem( IDC_CAPTURE_DEVICE_COMBO );
	HWND hCaptureDeviceCombo = pcwCaptureDeviceCombo->m_hWnd;
    
    CSoundDeviceEnumeration* pSDE=CSoundDeviceEnumeration::getInstance();
    pSDE->enumerateSoundOutDevices(DSoundEnumCallback, hSoundDeviceCombo);
    pSDE->enumerateSoundInDevices(DSoundEnumCallback, hCaptureDeviceCombo);
    m_cbBufferOutSize.AddString(TEXT("4096"));
    m_cbBufferOutSize.AddString(TEXT("8192"));
    m_cbBufferOutSize.AddString(TEXT("16384"));
}


BOOL DAudioDeviceSetup::OnInitDialog()
{
	CDialog::OnInitDialog();
	InitSound();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void DAudioDeviceSetup::OnCbnSelchangeSoundDeviceCombo()
{
    int csel=m_cbSoundDevice.GetCurSel();
    CSoundDeviceEnumeration* pSDE=CSoundDeviceEnumeration::getInstance();
    pSDE->setSoundOutDeviceIndex(csel);
}

void DAudioDeviceSetup::OnCbnSelchangeCaptureDeviceCombo()
{
    int csel=m_cbCaptureDevice.GetCurSel();
    CSoundDeviceEnumeration* pSDE=CSoundDeviceEnumeration::getInstance();
    pSDE->setSoundInDeviceIndex(csel);

}
