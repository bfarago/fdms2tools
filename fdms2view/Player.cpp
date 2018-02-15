// testermfc_fdms2libDoc.cpp : implementation of the CFdms2View_Doc class
//

#include "stdafx.h"
/*
#include "fdms2view_app.h"
#include "fdms2view_doc.h"
#include "fdms2view_view.h"
#include "DFdmsProp.h"
#include "DMetr.h"
#include "DFrag.h"
#include "DExport.h"
#include "DAudioDeviceSetup.h"
*/
#include "Player.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifndef DEFAULTBUFFSIZE
#define DEFAULTBUFFSIZE 2048
#endif

// CPlayer construction/destruction
CPlayer* g_Player;

CPlayer::CPlayer(): m_fdms2(NULL), m_mixer(NULL), m_VUMeters(NULL)
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
    m_waveFormat.wFormatTag = WAVE_FORMAT_PCM ;
    m_waveFormat.nChannels = 2;
    m_waveFormat.nSamplesPerSec = 44100;
    m_waveFormat.wBitsPerSample = 16;
    m_waveFormat.nBlockAlign = m_waveFormat.nChannels * (m_waveFormat.wBitsPerSample/8);
    m_waveFormat.nAvgBytesPerSec = m_waveFormat.nSamplesPerSec * m_waveFormat.nBlockAlign;
    m_waveFormat.cbSize = 0;

    /* Open the preferred Digital Audio Out device. Note: myWindow is a handle to some open window */
    result = waveOutOpen(&m_waveOutHandle, WAVE_MAPPER, &m_waveFormat, (DWORD)&waveCallback, (DWORD)this, CALLBACK_FUNCTION);
    if (result)
    {
       printf("There was an error opening the preferred Digital Audio Out device!\r\n");
       return;
    }
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
    m_waveSampleMax=m_waveFormat.nSamplesPerSec/4;
    m_waveSize=m_waveFormat.nAvgBytesPerSec/4;
    m_waveHdr[0].dwBytesRecorded=0;
    m_waveHdr[0].dwLoops=0;
    m_waveHdr[0].dwUser=0;
    m_waveHdr[0].dwBytesRecorded=0;
    m_waveHdr[0].lpNext=0;//&m_waveHdr[1];
    m_waveHdr[0].reserved=0;
    m_waveHdr[1].dwBytesRecorded=0;
    m_waveHdr[1].dwLoops=0;
    m_waveHdr[1].dwUser=0;
    m_waveHdr[1].dwBytesRecorded=0;
    m_waveHdr[1].lpNext=0;//&m_waveHdr[0];
    m_waveHdr[1].reserved=0;
    m_waveHdr[0].dwFlags= WHDR_PREPARED|WHDR_BEGINLOOP;
    m_waveHdr[1].dwFlags= WHDR_PREPARED|WHDR_ENDLOOP;
    m_waveHdr[0].lpData=(LPSTR)VirtualAlloc(0, m_waveSize, MEM_COMMIT, PAGE_READWRITE);
    m_waveHdr[1].lpData=(LPSTR)VirtualAlloc(0, m_waveSize, MEM_COMMIT, PAGE_READWRITE);
    m_waveMultiChannel=(short**)malloc(sizeof(short*) * FOSTEXMAXCHANNELS);
	for (int i=0; i<FOSTEXMAXCHANNELS; i++){
		m_waveMultiChannel[i]=(short*)malloc(sizeof(short) * m_waveSampleMax );
	}
    m_waveHdr[0].dwBufferLength=m_waveHdr[1].dwBufferLength=m_waveSize;
    result = waveOutPrepareHeader(m_waveOutHandle, &m_waveHdr[0], sizeof(WAVEHDR));
    result = waveOutPrepareHeader(m_waveOutHandle, &m_waveHdr[1], sizeof(WAVEHDR));
    result = waveOutSetVolume(m_waveOutHandle, 0xFFFF );
    m_bPlayNow=true;
    QueueWaveData(&m_waveHdr[0]);
    QueueWaveData(&m_waveHdr[1]);

}
void CPlayer::killPlayer(){
    if (!m_bPlayNow) return;
    ResetEvent(m_waveEvent);
    m_bPlayNow=false;
   	WaitForSingleObject(m_waveEvent, 2000);
	long result = 0;
    result = waveOutPause(m_waveOutHandle);
    result = waveOutReset(m_waveOutHandle);
	waveOutUnprepareHeader(m_waveOutHandle, &m_waveHdr[1], sizeof(WAVEHDR));
	waveOutUnprepareHeader(m_waveOutHandle, &m_waveHdr[0], sizeof(WAVEHDR));
	VirtualFree(m_waveHdr[0].lpData, m_waveSize<<1, MEM_FREE);
    VirtualFree(m_waveHdr[1].lpData, m_waveSize<<1, MEM_FREE);
    m_waveHdr[0].lpData=NULL;
    m_waveHdr[1].lpData=NULL;
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
#define MMM 25
    short peek[FOSTEXMAXCHANNELS+2];
    for (int i=0; i<MMM; i++){
        try{
            m_waveFdms2.reset();
            m_waveFdms2.getValueArrays(m_PosEditCursor.m_Pos, m_waveSampleMax/MMM, m_waveMultiChannel);
        }catch(...){

        }
        if (!m_bPlayNow) return 0;
        //TODO:Check
        //BUGBUG: Why isnt wavefdms?
        if (!m_fdms2) return 0;
        if (m_PosEditCursor.m_Sample> m_fdms2->getProgramSampleCount(m_SelectedProgram)) return 0;
        peek[8]=0;
        peek[9]=0;
        for (int i=0; i<m_waveSampleMax/MMM; i++){
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

        if(mixWaveData((short*) waveHeader->lpData, m_waveSize) != m_waveSize)
		{
bad:	
           //waveOutPause(m_waveOutHandle); //Stop playback
			return(-1);
		}
		//m_waveDataPlayed -= m_waveSize;

good:
        waveHeader->dwFlags &= ~WHDR_DONE;
        //waveHeader->dwFlags |= WHDR_DONE;
		long result = waveOutWrite(m_waveOutHandle, waveHeader, sizeof(WAVEHDR));
        SetEvent(m_waveEvent);
		return(0); //continue
	//}

	if (waveHeader == m_waveLastHdr)
	{
		return(1); //finish
	}

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
