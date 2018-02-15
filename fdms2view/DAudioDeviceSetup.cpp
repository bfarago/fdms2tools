// DAudioDeviceSetup.cpp : implementation file
//

#include "stdafx.h"
#include "fdms2view_app.h"
//#include "fdms2view_doc.h"
#include "DAudioDeviceSetup.h"
#include <mmsystem.h>

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
}


BEGIN_MESSAGE_MAP(DAudioDeviceSetup, CDialog)
END_MESSAGE_MAP()



//-----------------------------------------------------------------------------
// Name: DSoundEnumCallback()
// Desc: Enumeration callback called by DirectSoundEnumerate
//-----------------------------------------------------------------------------
INT_PTR CALLBACK DSoundEnumCallback( GUID* pGUID, TCHAR* strDesc, TCHAR* strDrvName,
                                  VOID* pContext )
{
    // Set aside static storage space for 20 audio drivers
    static GUID  AudioDriverGUIDs[20];
    static DWORD dwAudioDriverIndex = 0;

    GUID* pTemp  = NULL;

    if( pGUID )
    {
        if( dwAudioDriverIndex >= 20 )
            return TRUE;

        pTemp = &AudioDriverGUIDs[dwAudioDriverIndex++];
        memcpy( pTemp, pGUID, sizeof(GUID) );
    }

    HWND hSoundDeviceCombo = (HWND)pContext;

    // Add the string to the combo box
    SendMessage( hSoundDeviceCombo, CB_ADDSTRING, 
                 0, (LPARAM) (LPCTSTR) strDesc );

    // Get the index of the string in the combo box
    INT nIndex = (INT)SendMessage( hSoundDeviceCombo, CB_FINDSTRING, 
                                   0, (LPARAM) (LPCTSTR) strDesc );

    // Set the item data to a pointer to the static guid stored in AudioDriverGUIDs
    SendMessage( hSoundDeviceCombo, CB_SETITEMDATA, 
                 nIndex, (LPARAM) pTemp );

    return TRUE;
}

// DAudioDeviceSetup message handlers
void DAudioDeviceSetup::InitSound(){
#ifdef USE_DSOUND
    // \dx\samples\VC70\common\$(OutDir)\DXUT.lib
    // dsound.lib
    // dxerr.lib

	int hr;
    // Enumerate the sound devices and place them in the combo box
    CWnd* pcwSoundDeviceCombo = GetDlgItem(  IDC_SOUND_DEVICE_COMBO );
	HWND hSoundDeviceCombo = pcwSoundDeviceCombo->m_hWnd;
    if( FAILED( hr = DirectSoundEnumerate( (LPDSENUMCALLBACK)DSoundEnumCallback,
                                           (VOID*)hSoundDeviceCombo ) ) )
	{
       DXTRACE_ERR_MSGBOX( TEXT("DirectSoundEnumerate"), hr );
	   return;
	}

    // Enumerate the capture devices and place them in the combo box
    CWnd* pcwCaptureDeviceCombo = GetDlgItem( IDC_CAPTURE_DEVICE_COMBO );
	HWND hCaptureDeviceCombo = pcwCaptureDeviceCombo->m_hWnd;
    if( FAILED( hr = DirectSoundCaptureEnumerate( (LPDSENUMCALLBACK)DSoundEnumCallback,
                                                  (VOID*)hCaptureDeviceCombo ) ) )
	{
	   DXTRACE_ERR_MSGBOX( TEXT("DirectSoundCaptureEnumerate"), hr );
	   return;
	}

    // Select the first device in the combo box
	::SendMessage( hSoundDeviceCombo,   CB_SETCURSEL, 0, 0 );
	::SendMessage( hCaptureDeviceCombo, CB_SETCURSEL, 0, 0 );
#else
    WAVEOUTCAPS     woc;
    WAVEINCAPS      wic;
    unsigned long   iNumDevs, i;
    iNumDevs = waveOutGetNumDevs();
    CWnd* pcwSoundDeviceCombo = GetDlgItem(  IDC_SOUND_DEVICE_COMBO );
	HWND hSoundDeviceCombo = pcwSoundDeviceCombo->m_hWnd;
    for (i = 0; i < iNumDevs; i++)
    {
        if (!waveOutGetDevCaps(i, &woc, sizeof(WAVEOUTCAPS)))
        {
           
            //printf("Device ID #%u: %s\r\n", i, woc.szPname);
            //woc.
            TCHAR* sDesc = woc.szPname;
            TCHAR* sName = woc.szPname;
            DSoundEnumCallback(NULL, sDesc, sName, hSoundDeviceCombo);
        }
    }
    iNumDevs = waveInGetNumDevs();
    CWnd* pcwCaptureDeviceCombo = GetDlgItem(  IDC_CAPTURE_DEVICE_COMBO );
	HWND hCaptureDeviceCombo = pcwCaptureDeviceCombo->m_hWnd;
    for (i = 0; i < iNumDevs; i++)
    {
        if (!waveInGetDevCaps(i, &wic, sizeof(WAVEINCAPS)))
        {
           
            //printf("Device ID #%u: %s\r\n", i, woc.szPname);
            //woc.
            TCHAR* sDesc = wic.szPname;
            TCHAR* sName = wic.szPname;
            DSoundEnumCallback(NULL, sDesc, sName, hCaptureDeviceCombo);
        }
    }
#endif
}


BOOL DAudioDeviceSetup::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitSound();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
