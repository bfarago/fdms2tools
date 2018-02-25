/* Written by Barna Farago <brown@weblapja.com> 2006-2018
* Player.h : interface of the CPlayer class
*/
#pragma once
#include <fdms2lib.h>
#include <mmsystem.h>
#include "Mixer.h"
#include "VUMeters.h"
/*
#include "peekcache.h"
#include "DMix1.h"
*/

#define MAX_WHSIZE (1<<1)
#define MASK_WHSIZE (MAX_WHSIZE-1)

class CPlayer //: public Cparent
{
//Interface
public:
    typedef union{ 
        struct {
            unsigned char mixerFault : 1;
			unsigned char internalBufferOverrun : 1;
			unsigned char docref_fdms : 1;
			unsigned char wdog_wait : 1;
			unsigned char worker_unknown : 1;
			unsigned char worker_timeout : 1;
			unsigned char readMultiChannelCatch : 1;
			unsigned char worker_set : 1;
			unsigned char worker_reset : 1;
			unsigned char pool_order : 1;
        } flags;
        DWORD dw;
    }tErrorCode;
    tErrorCode m_errorCode;
    CPlayer();
    static CPlayer* getInstance();
    // TODO: doc valtas miatt egy struktúrába tegyük? vagy hogy?
	void updatePlayable() {
		if (m_fdms2) 
			m_bPlayable = m_fdms2->getDiskAudioSize()>10000;
		else 
			m_bPlayable = false;
	}
    void setFdms2(fdms2* fdms2){
        m_fdms2=fdms2;
		updatePlayable();
	}
    void setMixer(CMixer* mixer){
        m_mixer=mixer;
    }
    void setVUMeters(IVUMeters* vu){
        m_VUMeters=vu;
    }
    void setCursor(fdms2pos pos);
    fdms2pos getCursor(){ return m_PosEditCursor; }//TODO: howto?
    void setSelectedProgram(int iPrg);
    int getSelectedProgram(){return m_SelectedProgram;}

    void initPlayer();
    void killPlayer();
    
    bool getPlayNow(){ return m_bPlayNow; }
	bool getPlayable() { updatePlayable(); return m_bPlayable; }
	void setBufferedSamples(DWORD samplenum);
	DWORD getBufferedSamples();
	DWORD getAdaptiveWait() { return m_actualWait; }
// Operations
public:
    BOOL OnNewDocument();
    void OnUpdateCmdIfPlayable(CCmdUI *pCmdUI);
    void OnUpdateCmdIfPlayNow(CCmdUI *pCmdUI);
    void OnUpdateCmdIfPlayableButNotPlayNow(CCmdUI *pCmdUI);

protected:
// Outside 
	fdms2* m_fdms2;
    CMixer* m_mixer;
    IVUMeters* m_VUMeters;
    int m_SelectedProgram; //get from the doc/disk ?

//inside    
    fdms2 m_waveFdms2;
	bool m_bPlayable;
	bool m_bPlayNow;
	bool m_bRecordable;
    //TODO:refactoring needed
    fdms2pos m_PosEditCursor;
	DWORD m_actualWait;
	DWORD m_requiredBufferedSamples;
	
protected:
//TODO:inside must be protected soon.
    HWAVEOUT      m_waveOutHandle;
    WAVEFORMATEX  m_waveFormat;
    HANDLE  m_waveEvent;
    DWORD   m_waveSize;
    DWORD   m_waveSampleMax;
    short** m_waveMultiChannel;
    WAVEHDR m_waveHdr[MAX_WHSIZE];
    WAVEHDR* m_waveLastHdr;
    HANDLE m_thHandle;
    DWORD m_thId;
    unsigned char m_iMixer;
    unsigned char m_iStreamer;
    bool m_bRun;
    DWORD worker();
    static DWORD __stdcall ThreadEntry(void* ptr);
	int readMultiChannel();
	int calcMixbus(short * buf, DWORD size, DWORD &opos, int ipos, short* peak);
	DWORD mixWaveData(short* buf, DWORD size);
    long QueueWaveData(WAVEHDR * waveHeader);
    void stopWorker();
    void waveHandler(HWAVEOUT waveOut, UINT uMsg, DWORD dwParam1, DWORD dwParam2);
    static void CALLBACK waveCallback(HWAVEOUT waveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
    void InitMixer();
// Implementation
public:
	virtual ~CPlayer();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};
