/* Written by Barna Farago <brown@weblapja.com> 2006-2018
*
*/
#include "StdAfx.h"
#include "SoundDeviceEnumeration.h"
CSoundDeviceEnumeration* g_SoundDeviceEnumeration;

CSoundDeviceEnumeration::CSoundDeviceEnumeration(void):m_devOutIndex(0), m_devInIndex(0)
{
    g_SoundDeviceEnumeration=this;
}

CSoundDeviceEnumeration::~CSoundDeviceEnumeration(void)
{
}
CSoundDeviceEnumeration* CSoundDeviceEnumeration::getInstance(){
    if (!g_SoundDeviceEnumeration) g_SoundDeviceEnumeration=new CSoundDeviceEnumeration();
    return g_SoundDeviceEnumeration;
}

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
void CSoundDeviceEnumeration::enumerateSoundOutDevices(LPDSENUMCALLBACK DSoundEnumCallback, HWND hSoundDeviceCombo){

#ifdef USE_DSOUND
    // \dx\samples\VC70\common\$(OutDir)\DXUT.lib
    // dsound.lib
    // dxerr.lib

	int hr;
    // Enumerate the sound devices and place them in the combo box
    if( FAILED( hr = DirectSoundEnumerate( (LPDSENUMCALLBACK)DSoundEnumCallback,
                                           (VOID*)hSoundDeviceCombo ) ) )
	{
       DXTRACE_ERR_MSGBOX( TEXT("DirectSoundEnumerate"), hr );
	   return;
	}
#else
    DSoundEnumCallback(NULL, TEXT("WaveMapper"), TEXT("WaveMapper"), hSoundDeviceCombo);
    WAVEOUTCAPS     woc;
    unsigned long   iNumDevs, i;
    iNumDevs = waveOutGetNumDevs();
    for (i = 0; i < iNumDevs; i++)
    {
        if (!waveOutGetDevCaps(i, &woc, sizeof(WAVEOUTCAPS)))
        {
            TCHAR* sDesc = woc.szPname;
            TCHAR* sName = woc.szPname;
            DSoundEnumCallback(NULL, sDesc, sName, hSoundDeviceCombo);
        }
    }
#endif
    ::SendMessage( hSoundDeviceCombo,   CB_SETCURSEL, m_devOutIndex, 0 );
}
void CSoundDeviceEnumeration::enumerateSoundInDevices(LPDSENUMCALLBACK DSoundEnumCallback, HWND hCaptureDeviceCombo){
#ifdef USE_DSOUND
    // \dx\samples\VC70\common\$(OutDir)\DXUT.lib
    // dsound.lib
    // dxerr.lib

	int hr;
    // Enumerate the capture devices and place them in the combo box
    if( FAILED( hr = DirectSoundCaptureEnumerate( (LPDSENUMCALLBACK)DSoundEnumCallback,
                                                  (VOID*)hCaptureDeviceCombo ) ) )
	{
	   DXTRACE_ERR_MSGBOX( TEXT("DirectSoundCaptureEnumerate"), hr );
	   return;
	}
#else
    WAVEINCAPS      wic;
    unsigned long   iNumDevs, i;
    iNumDevs = waveInGetNumDevs();
    for (i = 0; i < iNumDevs; i++)
    {
        if (!waveInGetDevCaps(i, &wic, sizeof(WAVEINCAPS)))
        {
            TCHAR* sDesc = wic.szPname;
            TCHAR* sName = wic.szPname;
            DSoundEnumCallback(NULL, sDesc, sName, hCaptureDeviceCombo);
        }
    }
#endif
    ::SendMessage( hCaptureDeviceCombo, CB_SETCURSEL, m_devInIndex, 0 );
}
void CSoundDeviceEnumeration::setSoundOutDeviceIndex(int devOutIndex){
    m_devOutIndex=devOutIndex;
}
void CSoundDeviceEnumeration::setSoundInDeviceIndex(int devInIndex){
    m_devInIndex=devInIndex;
}
    