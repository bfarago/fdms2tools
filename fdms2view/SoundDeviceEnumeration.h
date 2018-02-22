/* Written by Barna Farago <brown@weblapja.com> 2006-2018
*/
#pragma once
#include <mmsystem.h>

#ifdef USE_DSOUND
#pragma comment(linker,"\dx\samples\VC70\common\$(OutDir)\DXUT.lib") 
#pragma comment(linker,"dsound.lib") 
#pragma comment(linker,"dxerr.lib") 
#else
typedef INT_PTR (CALLBACK LPDSENUMCALLBACK)(GUID* pGUID, TCHAR* strDesc, TCHAR* strDrvName, VOID* pContext );
#endif

INT_PTR CALLBACK DSoundEnumCallback( GUID* pGUID, TCHAR* strDesc, TCHAR* strDrvName, VOID* pContext );

class CSoundDeviceEnumeration{
public:
    CSoundDeviceEnumeration(void);
    ~CSoundDeviceEnumeration(void);
    static CSoundDeviceEnumeration* getInstance();
    void enumerateSoundInDevices(LPDSENUMCALLBACK DSoundEnumCallback, HWND hCaptureDeviceCombo);
    void enumerateSoundOutDevices(LPDSENUMCALLBACK DSoundEnumCallback, HWND hSoundDeviceCombo);
    void setSoundOutDeviceIndex(int devOutIndex);
    void setSoundInDeviceIndex(int devInIndex);
    int getSoundOutDeviceIndex(){return m_devOutIndex;}
    int getSoundInDeviceIndex(){return m_devInIndex;}
private:
    int m_devOutIndex;
    int m_devInIndex;
};
