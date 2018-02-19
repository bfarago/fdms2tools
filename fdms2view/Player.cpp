// testermfc_fdms2libDoc.cpp : implementation of the CFdms2View_Doc class
//

#include "stdafx.h"
#include "Player.h"
#include "SoundDeviceEnumeration.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifndef DEFAULTBUFFSIZE
#define DEFAULTBUFFSIZE 2048
#endif


#define PERSECBUFFERSIZE (4)
#define MMM (25)

// CPlayer construction/destruction
CPlayer* g_Player;

CPlayer::CPlayer(): m_fdms2(NULL), m_mixer(NULL), m_VUMeters(NULL),
m_bRun(false), m_thHandle(NULL)
{
	m_bPlayable=false;
	m_bPlayNow=false;
	m_bRecordable=false;
//	m_MixConsole=NULL;
    g_Player=this;
}
CPlayer* CPlayer::getInstance(){
    return g_Player;
}
CPlayer::~CPlayer()
{
    killPlayer();
//    if (m_MixConsole) m_MixConsole->registerMixer(NULL);
	m_bPlayable=false;
	m_bRecordable=false;
	m_bPlayNow=false;
    m_waveFdms2.stop();
}
BOOL CPlayer::OnNewDocument()
{
	m_bPlayable=false;
	m_bPlayNow=false;
	m_bRecordable=false;
	return TRUE;
}
void CPlayer::setSelectedProgram(int iPrg){
    m_SelectedProgram=iPrg;
    //reset
}
void CPlayer::setCursor(fdms2pos pos){
    m_PosEditCursor=pos; //TODO: howto?
    //reset
}
    
#ifdef _DEBUG
void CPlayer::AssertValid() const
{
	//CDocument::AssertValid();
}

void CPlayer::Dump(CDumpContext& dc) const
{
	//CDocument::Dump(dc);
}
#endif //_DEBUG

//---------------------------------------------------------------
//Transport
//---------------------------------------------------------------
void CPlayer::OnUpdateCmdIfPlayable(CCmdUI *pCmdUI){
	BOOL bEn=	m_bPlayable;
	pCmdUI->Enable(bEn);
}
void CPlayer::OnUpdateCmdIfPlayNow(CCmdUI *pCmdUI){
	BOOL bEn=	m_bPlayNow;
	pCmdUI->Enable(bEn);
}
void CPlayer::OnUpdateCmdIfPlayableButNotPlayNow(CCmdUI *pCmdUI){
	BOOL bEn=	m_bPlayable & !m_bPlayNow;
	pCmdUI->Enable(bEn);
}
void CPlayer::InitMixer(){
    UINT nMix= mixerGetNumDevs();
    HMIXER hMix;
    if (MMSYSERR_NOERROR ==mixerOpen(&hMix, (UINT)m_waveOutHandle, NULL, 0, MIXER_OBJECTF_HWAVEOUT)){
        MIXERCAPS mc;
        mixerGetDevCaps((UINT)hMix,&mc,sizeof(mc));
        MIXERLINE ml;
        ml.cbStruct=sizeof(ml);
        ml.dwDestination=0;
        ml.dwSource=0;

        MMRESULT res= mixerGetLineInfo((HMIXEROBJ)hMix, &ml,MIXER_OBJECTF_HMIXER);
        MIXERCONTROL mct[16];
        MIXERLINECONTROLS mlcts;
        mlcts.cbStruct=sizeof(MIXERLINECONTROLS);
        mlcts.cbmxctrl=sizeof(MIXERCONTROL);
        mlcts.cControls=ml.cControls;
        mlcts.pamxctrl=mct;
        mlcts.dwControlID=0;
        mlcts.dwLineID=0;
        res=mixerGetLineControls((HMIXEROBJ)hMix, &mlcts, MIXER_GETLINECONTROLSF_ALL);
        MIXERCONTROLDETAILS mctd;
        MIXERCONTROLDETAILS_LISTTEXT lt[16];
        mctd.paDetails=lt;
        mctd.cbStruct=sizeof(MIXERCONTROLDETAILS);
        mctd.cbDetails=sizeof(MIXERCONTROLDETAILS_LISTTEXT);
        mctd.cChannels=ml.cChannels;
        mctd.cMultipleItems=1;
        mctd.dwControlID=0;
        //mixerGetControlDetails((HMIXEROBJ)hMix, &mctd, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_LISTTEXT);
        mixerClose(hMix);
    }
}
//---------------------------------------------------------------
//Thread
//---------------------------------------------------------------
DWORD CPlayer::ThreadEntry(void* ptr){
    CPlayer * p=((CPlayer*)ptr);
    if (!p) return 0;
    DWORD ret=p->worker();
    p->m_thHandle=0;
    p->m_bRun=false;
    return ret;
}
DWORD CPlayer::worker(){
    SetThreadPriority(m_thHandle, THREAD_PRIORITY_HIGHEST);
    while(m_bRun){
        Sleep(1);
        WaitForSingleObject(m_waveEvent, 10);
        ResetEvent(m_waveEvent);
        if (!(m_waveHdr[m_iMixer].dwFlags & WHDR_DONE)){
            
           continue;
        }
        while(m_waveHdr[m_iMixer].dwFlags & WHDR_DONE){
            if (!m_bPlayNow) break;
            m_waveHdr[m_iMixer].dwBufferLength= m_waveSize;
            DWORD dwReaded= mixWaveData((short*) m_waveHdr[m_iMixer].lpData, m_waveHdr[m_iMixer].dwBufferLength);
            if(dwReaded != m_waveHdr[m_iMixer].dwBufferLength)
		    {
                m_errorCode.flags.mixerFault=1;
                m_waveHdr[m_iMixer].dwBufferLength=dwReaded;
                
            }
            {
                m_waveHdr[m_iMixer].dwFlags &= ~WHDR_DONE;
                m_iMixer++;
                m_iMixer&=MASK_WHSIZE; //Roundrobin
            }
        }
    }
    return 0;
}
//---------------------------------------------------------------
//WaveDev
//---------------------------------------------------------------
void CPlayer::initPlayer(){
    unsigned long result;
    if (m_bPlayNow) return;
    m_waveEvent = CreateEvent(0, FALSE, FALSE, 0);

    m_waveFdms2.copy(m_fdms2);
    m_waveFdms2.start();
    
    /* Initialize the WAVEFORMATEX for 16-bit, 44KHz, stereo */
    m_waveFormat.wFormatTag = WAVE_FORMAT_PCM ; //WAVE_FORMAT_44S16|
    m_waveFormat.nChannels = 2;
    m_waveFormat.nSamplesPerSec = 44100;
    m_waveFormat.wBitsPerSample = 16;
    m_waveFormat.nBlockAlign = m_waveFormat.nChannels * (m_waveFormat.wBitsPerSample/8);
    m_waveFormat.nAvgBytesPerSec = m_waveFormat.nSamplesPerSec * m_waveFormat.nBlockAlign;
    m_waveFormat.cbSize = 0;
    CSoundDeviceEnumeration* pSDE=CSoundDeviceEnumeration::getInstance();
    /* Open the preferred Digital Audio Out device. Note: myWindow is a handle to some open window */
    result = waveOutOpen(&m_waveOutHandle, pSDE->getSoundOutDeviceIndex()-1, &m_waveFormat, (DWORD)&waveCallback, (DWORD)this, CALLBACK_FUNCTION);
    if (result)
    {
        if (result == WAVERR_BADFORMAT){
            //Format not supported.
        }
        printf("There was an error opening the preferred Digital Audio Out device!\r\n");
        return;
    }

    //TODO: buffer meret allithato legyen.

    m_waveSampleMax=m_waveFormat.nSamplesPerSec/PERSECBUFFERSIZE;
    m_waveSize=m_waveFormat.nAvgBytesPerSec/PERSECBUFFERSIZE;
    for (int i=0; i<MAX_WHSIZE; i++){
        m_waveHdr[i].reserved=0;
        //if (i == MASK_WHSIZE)
        //    m_waveHdr[MASK_WHSIZE].lpNext=&m_waveHdr[0];
        //else
        //    m_waveHdr[i].lpNext=&m_waveHdr[i+1];
        m_waveHdr[i].lpNext=0;
        m_waveHdr[i].dwFlags= WHDR_PREPARED|WHDR_DONE;// done means this buffer can be used for mix
        m_waveHdr[i].dwBytesRecorded=0;
        m_waveHdr[i].dwLoops=0;
        m_waveHdr[i].dwUser=0;
        m_waveHdr[i].lpData=(LPSTR)VirtualAlloc(0, m_waveSize*2, MEM_COMMIT, PAGE_READWRITE);
        m_waveHdr[i].dwBufferLength=m_waveSize;
        result = waveOutPrepareHeader(m_waveOutHandle, &m_waveHdr[i], sizeof(WAVEHDR));
    }
  
    m_waveMultiChannel=(short**)malloc(sizeof(short*) * FOSTEXMAXCHANNELS);
	for (int i=0; i<FOSTEXMAXCHANNELS; i++){
		m_waveMultiChannel[i]=(short*)malloc(sizeof(short) * m_waveSampleMax );
	}
    result = waveOutSetVolume(m_waveOutHandle, 0xFFFF );
    m_bPlayNow=true;
    if (!m_thHandle){
        m_bRun=true;
        m_thHandle= CreateThread(NULL, 8192, &CPlayer::ThreadEntry, this, 0, &m_thId);
        SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    }
    SetEvent(m_waveEvent);
    Sleep(200);
    QueueWaveData(&m_waveHdr[0]);
    Sleep(100);
    SetEvent(m_waveEvent);
    //QueueWaveData(&m_waveHdr[1]);

}
void CPlayer::stopWorker(){
   if (m_bRun){
        m_bRun=false;
        DWORD dwTmp=STILL_ACTIVE;
        while (m_thHandle &&(STILL_ACTIVE == dwTmp )){
            m_bRun=false;
            Sleep(50);
            GetExitCodeThread(m_thHandle, &dwTmp);
        }
    }
    if (m_thHandle) CloseHandle(m_thHandle);
 }
void CPlayer::killPlayer(){
    if (!m_bPlayNow) return;
    ResetEvent(m_waveEvent);
    if (m_bPlayNow){
        m_bPlayNow=false;
   	    WaitForSingleObject(m_waveEvent, 2000); //wait for windows. to finish streaming
    }
    SetEvent(m_waveEvent);  //stop thread if not playing...
    stopWorker();
	long result = 0;
    result = waveOutPause(m_waveOutHandle);
    result = waveOutReset(m_waveOutHandle);
    for (int i=0; i<MAX_WHSIZE; i++){
	    waveOutUnprepareHeader(m_waveOutHandle, &m_waveHdr[i], sizeof(WAVEHDR));
	    VirtualFree(m_waveHdr[i].lpData, m_waveSize<<1, MEM_FREE);
        m_waveHdr[i].lpData=NULL;
    }
	result = waveOutClose(m_waveOutHandle);
    
    if (m_waveMultiChannel){
        for (int i=0; i<FOSTEXMAXCHANNELS; i++){
		    if (m_waveMultiChannel[i]) free(m_waveMultiChannel[i]);
        }
        free(m_waveMultiChannel);
    }
    m_waveMultiChannel=NULL;
}

//inside
DWORD CPlayer::mixWaveData(short* buf, DWORD size){
    DWORD oPtr=0;
    if (!m_waveMultiChannel) return 0;
    short peek[FOSTEXMAXCHANNELS+2];
    for (int i=0; i<MMM; i++){
        try{
            if (!m_bRun) return 0;
            m_waveFdms2.reset();
            int num= m_waveFdms2.getValueArrays(m_PosEditCursor.m_Pos, 2*m_waveSampleMax/MMM, m_waveMultiChannel);
            if (m_waveFdms2.m_badsector){
                m_bPlayNow=false;
                m_waveFdms2.reset();
                return 0;
            }
        }catch(...){
            m_bPlayNow=false;   //Barna
        }
        if (!m_bPlayNow) return 0;
        //TODO:Check
        //BUGBUG: Why isnt wavefdms?
        if (!m_fdms2) return 0;
        if (m_PosEditCursor.m_Sample> m_fdms2->getProgramSampleCount(m_SelectedProgram)) return 0;
        peek[8]=0;
        peek[9]=0;
        for (unsigned int i=0; i<2*m_waveSampleMax/MMM; i++){
            if (!m_bPlayNow) return 0;
       	    m_PosEditCursor.addSample( 1 );
            setCursor(m_PosEditCursor); //TODO: de itt nem kell reset

            long l=0;
            long r=0;
            for (int ch=0; ch<FOSTEXMAXCHANNELS; ch++){
                if (!m_bPlayNow) return 0;
                short *pt=(short *)m_waveMultiChannel[ch];
                short mono=pt[i];
                if (m_mixer) mono=m_mixer->getAmpChannel(mono,ch);
                l+=mono;
                r+=mono;
                if (!i) peek[ch]=0;
                else {
                    if (peek[ch] < mono) peek[ch]=mono; //max
                }
                if (!m_bPlayNow) return 0;
            }
            if (m_mixer) m_mixer->getAmpMaster(l,r);
            l=l/FOSTEXMAXCHANNELS;
            r=r/FOSTEXMAXCHANNELS;
            buf[oPtr++] = l;
            buf[oPtr++] = r;
            if (peek[8] < l) peek[8]=l; //max
            if (peek[9] < r) peek[9]=r; //max
        }
    }
    //TODO: vumeters need to be implemented somehow.
	if (m_VUMeters){
		for (int i=0; i<FOSTEXMAXCHANNELS+2; i++){
			m_VUMeters->m_aVU[i]=abs(peek[i]);
		}
 		//m_MixConsole->DoSetVumeters();
	}
    
	return oPtr<<1; //end
}
long CPlayer::QueueWaveData(WAVEHDR * waveHeader)
{
    if (!m_bRun) goto bad;
    if (!m_bPlayNow) {
        goto bad;
    }
	//if (m_waveDataPlayed)
	//{
	//	if (m_waveDataPlayed < WaveBufSize) //Only a partial block left?
	//	{
	//		//getnext partial frame
	//		if(mixWaveData(waveHeader->lpData, m_waveDataPlayed) != m_waveDataPlayed) goto bad;
	//		waveHeader->dwBufferLength = m_waveDataPlayed;
	//		m_waveLastHdr = waveHeader; //this is the last.
	//		m_waveDataPlayed = 0;
	//		goto good;
	//	}

    m_iStreamer++;
    m_iStreamer&=MASK_WHSIZE;
    if (waveHeader != &m_waveHdr[m_iStreamer]){
   //     DLOG("e");
        waveHeader = &m_waveHdr[m_iStreamer];
    }
    if (!(waveHeader->dwFlags & WHDR_DONE )) goto good;
    int wd=100;
    while(wd--){
        SetEvent(m_waveEvent);
        if (!m_bRun) goto bad;
        Sleep(200);
        if (!(waveHeader->dwFlags & WHDR_DONE )) goto good;
    }
bad:	
		return(-1);
good:
		long result = waveOutWrite(m_waveOutHandle, waveHeader, sizeof(WAVEHDR));
        SetEvent(m_waveEvent);
		return(0); //continue
}
//CALLBACK from windows, static function.
void CPlayer::waveCallback(HWAVEOUT waveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2){
    CPlayer* pPlayer= (CPlayer*)dwInstance;
    if (!pPlayer) return;
    pPlayer->waveHandler(waveOut, uMsg, dwParam1, dwParam2);
}
//Member function.
void CPlayer::waveHandler(HWAVEOUT waveOut, UINT uMsg, DWORD dwParam1, DWORD dwParam2){
    if (!m_bPlayNow) SetEvent(m_waveEvent);
    WAVEHDR* pHdr=(WAVEHDR *)dwParam1;
   /* WAVEHDR* pHdrO=&pDoc->m_waveHdr[0];
    if (pHdr == pHdrO) pHdrO= &pDoc->m_waveHdr[1];*/
	switch(uMsg)
	{
    case MM_WOM_DONE:
		/* Queue another buffer */
		if (QueueWaveData(pHdr))
		{
			/* If no more buffers, notify PlayWave() that the WAVE is done playing */
//			SetEvent(pDoc->m_waveEvent);
		}
        break;
    case MM_WOM_CLOSE:
        break;
    case MM_WOM_OPEN:
        break;
	}
}
/*
UINT64  CFdms2View_Doc::getMaxPos(){
    INT64 len;
	t1_toffset start;
	UINT64 max=0;
	int iIdx=0;
	while(!m_fdms2.getPart(m_SelectedProgram, iIdx, start, len)){
		UINT64 dwTmp= start+len;
		if (max<dwTmp) max=dwTmp;
		iIdx++;
	}
	return max;
}
void CFdms2View_Doc::DoOrderRegion(){
    if (m_PosRegionStart.m_Sample < m_PosRegionStop.m_Sample) return; //ok
    fdms2pos tmp= m_PosRegionStart;
    m_PosRegionStart= m_PosRegionStop;
    m_PosRegionStop=tmp;
}
*/

//---------------------------------------------------------------
//Mix
//---------------------------------------------------------------
