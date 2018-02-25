/* Written by Barna Farago <brown@weblapja.com> 2006-2018
 * Player.cpp : implementation of the CPlayer class
 */

#include "stdafx.h"
#include "Player.h"
#include "SoundDeviceEnumeration.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifndef DEFAULTBUFFSIZE
#define DEFAULTBUFFSIZE 2048
#endif


#define PERSECBUFFERSIZE (25)
#define MMM (1)
#define SLEEP_WAIT_DONE (10)
#define WADOG_WAIT_DONE (10)

#define SLEEP_WAIT_EXITCODE (50)
#define SLEEP_WAIT_BEFOREQUEUE (20)
#define SLEEP_WAIT_AFTERQUEUE (1)
#define SLEEP_WORKER_AFTER_ENQUEUE (10)
#define TIME_WAIT_WAVE (400)
// CPlayer construction/destruction
CPlayer* g_Player;

CPlayer::CPlayer(): m_fdms2(NULL), m_mixer(NULL), m_VUMeters(NULL),
m_bRun(false), m_thHandle(NULL), m_requiredBufferedSamples(4410), m_actualWait(0)
{
	m_bPlayable=false;
	m_bPlayNow=false;
	m_bRecordable=false;
    g_Player=this;
}
CPlayer* CPlayer::getInstance(){
    return g_Player;
}
CPlayer::~CPlayer()
{
    killPlayer();
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
void CPlayer::setBufferedSamples(DWORD samplenum) {
	m_requiredBufferedSamples = samplenum;
}
DWORD CPlayer::getBufferedSamples() {
	return m_waveSampleMax;
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
	BOOL bEn=	getPlayable();
	pCmdUI->Enable(bEn);
}
void CPlayer::OnUpdateCmdIfPlayNow(CCmdUI *pCmdUI){
	BOOL bEn=	m_bPlayNow;
	pCmdUI->Enable(bEn);
}
void CPlayer::OnUpdateCmdIfPlayableButNotPlayNow(CCmdUI *pCmdUI){
	BOOL bEn= getPlayable() & !m_bPlayNow;
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
//Started by app, just calls worker loop.
DWORD CPlayer::ThreadEntry(void* ptr){
    CPlayer * p=((CPlayer*)ptr);
    if (!p) return 0;
    DWORD ret=p->worker();
    p->m_thHandle=0;
    p->m_bRun=false;
    return ret;
}
/*worker
The worker thread started by application, gets highest prio, fill play buffer
as mutch as possible, waits for a free buffer in the pool. In case of no free
buffer found, timeout signaled. Syncronization between multimedia thread and
worker stread works by WaitForSingleObject, where the timeout is adapted for
the actual machine. When timeout occures, 10ms added to wait, when syncronization
was successfull, 2ms removed from the wait period. AdaptiveTry number of readjustments
are possible without test of possible queueing. Buffer size must be adecvate size.
Windows tick is more than 14ms by default. But too mutch buffer produce delays
between user interface and playback.
*/
DWORD CPlayer::worker(){
    SetThreadPriority(m_thHandle, THREAD_PRIORITY_HIGHEST);
	int juststarted = 10;
	int adaptivetry = 5;
	DWORD adaptiveWait = TIME_WAIT_WAVE;
	m_iMixer = 0;
    while(m_bRun){
        tryagain:
        DWORD status=WaitForSingleObject(m_waveEvent, adaptiveWait);
		switch (status) {
		case WAIT_TIMEOUT:
			if (!juststarted) {
				adaptiveWait += 10;
				if (adaptivetry) {
					adaptivetry--;
					goto tryagain;
				}
				//Sleep(SLEEP_WORKER_AFTER_ENQUEUE);
			} else {
				//Sleep(SLEEP_WORKER_AFTER_ENQUEUE);
			}
			break;
		case WAIT_OBJECT_0:
			if (adaptiveWait > 50) {
				m_actualWait = adaptiveWait;
				adaptiveWait -= 2;
				adaptivetry = 5;
			}
			if (!ResetEvent(m_waveEvent)) {
				m_errorCode.flags.worker_reset = 1;
			}
			break;
		default:
			m_errorCode.flags.worker_unknown = 1;
			break;
		}	
		
		//if (!(m_waveHdr[m_iMixer].dwFlags & WHDR_DONE)) continue;
		//search the next chunk from pool.
		for (int i=0; i<MAX_WHSIZE; i++){
			if ((m_waveHdr[i].dwFlags & WHDR_DONE)) {
				if (m_iMixer !=i){
					m_iMixer = i;
					m_errorCode.flags.pool_order = 1;
				}
				break;
			}
			if (MAX_WHSIZE - 1 == i) {
				if (!juststarted) 
					m_errorCode.flags.worker_timeout = 1; //adaptive timeout cant solve the issue
			}
        }
		
        while(m_waveHdr[m_iMixer].dwFlags & WHDR_DONE){
            if (!m_bPlayNow) break;
            m_waveHdr[m_iMixer].dwBufferLength= m_waveSize;
			//length in byte
            DWORD dwReaded= mixWaveData((short*) m_waveHdr[m_iMixer].lpData, m_waveHdr[m_iMixer].dwBufferLength);
			if (!dwReaded) break;
			if (juststarted) juststarted--;
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
    m_waveFormat.wBitsPerSample = 16; //todo: per channel or per N channel ?
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

    //TODO: buffer size should be adaptable, design a better config.
	//TODO: there are dependancy between buffersize and timing.
	m_waveSampleMax = m_requiredBufferedSamples; //m_waveFormat.nSamplesPerSec/PERSECBUFFERSIZE;
	m_waveSize = m_waveSampleMax * 4; //m_waveFormat.nAvgBytesPerSec/PERSECBUFFERSIZE;
    for (int i=0; i<MAX_WHSIZE; i++){
        m_waveHdr[i].reserved=0;
        m_waveHdr[i].lpNext=0;
		m_waveHdr[i].dwFlags = 0; // WHDR_PREPARED | WHDR_DONE;// done means this buffer can be used for mix
        m_waveHdr[i].dwBytesRecorded=0;
        m_waveHdr[i].dwLoops=0;
        m_waveHdr[i].dwUser=i;
        m_waveHdr[i].lpData=(LPSTR)VirtualAlloc(0, m_waveSize, MEM_COMMIT, PAGE_READWRITE);
        m_waveHdr[i].dwBufferLength=m_waveSize;
        result = waveOutPrepareHeader(m_waveOutHandle, &m_waveHdr[i], sizeof(WAVEHDR));
    }
  
    m_waveMultiChannel=(short**)malloc(sizeof(short*) * FOSTEXMAXCHANNELS);
	for (int i=0; i<FOSTEXMAXCHANNELS; i++){
		m_waveMultiChannel[i]=(short*)malloc(sizeof(short) * m_waveSampleMax );
	}
    result = waveOutSetVolume(m_waveOutHandle, 0xFFFFffff );
    m_bPlayNow=true;
    if (!m_thHandle){
        m_bRun=true;
        m_thHandle= CreateThread(NULL, 8192, &CPlayer::ThreadEntry, this, 0, &m_thId);
        SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    }
	if (!SetEvent(m_waveEvent)) {
		m_errorCode.flags.worker_set = 1;
	}
    
	Sleep(SLEEP_WAIT_BEFOREQUEUE);
    QueueWaveData(&m_waveHdr[0]);
    //Sleep(SLEEP_WAIT_AFTERQUEUE);
    SetEvent(m_waveEvent);
    //QueueWaveData(&m_waveHdr[1]);

}

void CPlayer::stopWorker(){
   if (m_bRun){
        m_bRun=false;
        DWORD dwTmp=STILL_ACTIVE;
        while (m_thHandle &&(STILL_ACTIVE == dwTmp )){
            m_bRun=false;
            Sleep(SLEEP_WAIT_EXITCODE);
            GetExitCodeThread(m_thHandle, &dwTmp);
        }
    }
    if (m_thHandle) CloseHandle(m_thHandle);
 }

/*KillPlayer
called from main thread
*/
void CPlayer::killPlayer(){
    if (!m_bPlayNow) return;
    ResetEvent(m_waveEvent);
    if (m_bPlayNow){
        m_bPlayNow=false;
   	    WaitForSingleObject(m_waveEvent, 300); //wait for windows. to finish streaming
    }
    SetEvent(m_waveEvent);  //stop thread if not playing...
    stopWorker();
	long result = 0;
    result = waveOutPause(m_waveOutHandle);
    result = waveOutReset(m_waveOutHandle);
    for (int i=0; i<MAX_WHSIZE; i++){
	    waveOutUnprepareHeader(m_waveOutHandle, &m_waveHdr[i], sizeof(WAVEHDR));
	    VirtualFree(m_waveHdr[i].lpData, m_waveSize, MEM_FREE);
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
int CPlayer::readMultiChannel() {
	int num = 0;
	if (m_bRun){
		try {
			if (!m_bRun) return 0;
			m_waveFdms2.reset();
			num=m_waveFdms2.getValueArrays(m_PosEditCursor.m_Pos, m_waveSampleMax / MMM, m_waveMultiChannel);
			if (m_waveFdms2.wasBadSector()) {
				m_bPlayNow = false;
				m_waveFdms2.reset();
				return 0;
			}
		}
		catch (...) {
			m_bPlayNow = false;   //Barna
			m_errorCode.flags.readMultiChannelCatch = 1;
		}
	}
	if (!m_bPlayNow) return 0;
	return num;
}

/* calcMixbus
calculate a stereo mixbus from multichannel, search peaks also.
buf: output buffer, samples
size: buffer size in samples
opos: buffer pos
ipos: multichannel pos
peak: array for the peak values.
*/
int CPlayer::calcMixbus(short* buf, DWORD size, DWORD &opos, int ipos, short* peak) {
	long l=0;
    long r=0;
	if (opos > size) {
		m_errorCode.flags.internalBufferOverrun = 1;
		return 0; //buffer overrun
	}
	if (!ipos) {
		//reset peaks
		for (int ch = 0; ch < FOSTEXMAXCHANNELS + 2; ch++) {
			peak[ch] = 0;
		}
	}
    for (int ch=0; ch<FOSTEXMAXCHANNELS; ch++){
        if (!m_bPlayNow) return 0;
        short *pt=(short *)m_waveMultiChannel[ch];
        short mono=pt[ipos];
        if (m_mixer) mono=(short)m_mixer->getAmpChannel(mono,ch);
        l+=mono;
        r+=mono;
        if (peak[ch] < mono) peak[ch]=mono; //max
        if (!m_bPlayNow) return 0;
    }
    if (m_mixer) m_mixer->getAmpMaster(l,r);

	l >>= FOSTEXSHIFTCH; // l / FOSTEXMAXCHANNELS;
	r >>= FOSTEXSHIFTCH; // r / FOSTEXMAXCHANNELS;
			
	/*
	//TODO: yes, but abs() ? there is a solution in PeakCache
	#define OVER_VUMETER_THRESHOLD (0x7FF0)
	#define OVER_VUMETER_MAX (0x7FFF)
	if (l > OVER_VUMETER_THRESHOLD) {
		//l over;
		if (l > OVER_VUMETER_MAX) {
			l = OVER_VUMETER_MAX;
		}
	}
	if (r > OVER_VUMETER_THRESHOLD) {
		//r over;
		if (r > OVER_VUMETER_MAX) {
			r = OVER_VUMETER_MAX;
		}
	}*/
    buf[opos++] = (short) l;
    buf[opos++] = (short) r;
    if (peak[FOSTEXMAXCHANNELS] < l) peak[FOSTEXMAXCHANNELS]=(short)l; //max
    if (peak[FOSTEXMAXCHANNELS+1] < r) peak[FOSTEXMAXCHANNELS+1]=(short)r; //max
	return 1;
}

/*mixWaveData
Create playable stereo strem from multichannel stream, calculate peaks.
 buf: output audio buffer l,r
 size: buffer size in bytes
*/
DWORD CPlayer::mixWaveData(short* buf, DWORD size)
{
    DWORD oPtr=0; //output audio data index in samples
	DWORD oSize = (size>>1)& ~1; //oSize in saples: +2 = (8channel input 2 channel output)
	int chunkSize = m_waveSampleMax / MMM;	//mustbe divisible without modulus
    if (!m_waveMultiChannel) return 0;
    short peak[FOSTEXMAXCHANNELS+2]; //8+left+right channels
    for (int i=0; i<MMM; i++){
		int num = readMultiChannel();
        if (!num ) return 0;
        
		//Check the active document's fdms also
		if (!m_fdms2) {
			m_errorCode.flags.docref_fdms = 1;
			return 0;
		}

		//play head is over the end of the tape.
        if (m_PosEditCursor.m_Sample > m_fdms2->getProgramSampleCount(m_SelectedProgram)) return 0;

		//process 1/MMM chunk of the buffer
		for (int s=0; s<chunkSize; s++){
            if (!m_bPlayNow) return 0;
			if (!calcMixbus(buf, oSize, oPtr, s, peak)) return 0;
        }
		//move playhead 1/MMM chunk forward.
		m_PosEditCursor.addSample(chunkSize);
		/// setCursor(m_PosEditCursor); //TODO: playhead update on other thread ?
    }
    //TODO: vumeters need to be implemented somehow.
	if (m_VUMeters){
		for (int i=0; i<FOSTEXMAXCHANNELS+2; i++){
			m_VUMeters->m_aVU[i]=abs(peak[i]);
		}
 		//m_MixConsole->DoSetVumeters();
	}
    
	return oPtr<<1; // *sizeof(short) number of bytes.
}
//---------------------------------------------------------------
//Callback from WaveDev thread
//---------------------------------------------------------------
long CPlayer::QueueWaveData(WAVEHDR * waveHeader)
{
	if (!m_bRun) {
		goto bad;
	}
    if (!m_bPlayNow) {
        goto bad;
    }
	
	//play if not yet done, this can be happend only when starts the pool from other thread's initPlayer()
	if (!(waveHeader->dwFlags & WHDR_DONE)) {
		goto good;
	}
	
	//the actual is done, search the next available from the pool
    m_iStreamer++; // This sets the order of the pool's chunks
    m_iStreamer&=MASK_WHSIZE;
    if (waveHeader != &m_waveHdr[m_iStreamer]){
        waveHeader = &m_waveHdr[m_iStreamer];
    }
    if (!(waveHeader->dwFlags & WHDR_DONE )) goto good;
    int wd= WADOG_WAIT_DONE;
    while(wd--){
        SetEvent(m_waveEvent);
		Sleep(SLEEP_WAIT_DONE);
        if (!m_bRun) goto bad;
        if (!(waveHeader->dwFlags & WHDR_DONE )) goto good;
		
    }
	m_errorCode.flags.wdog_wait = 1;
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
