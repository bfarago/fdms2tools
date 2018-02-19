// fdms2view_doc.h : interface of the CFdms2View_Doc class
//
#pragma once
#include <fdms2lib.h>
#include <mmsystem.h>
#include "Mixer.h"
#include "VUMeters.h"
/*
#include "peekcache.h"
#include "DMix1.h"
*/

class CPlayer //: public CDocument, public ICacheUser
{
//Interface
public:
    typedef union{ 
        struct {
            unsigned char mixerFault:1;
        } flags;
        DWORD dw;
    }tErrorCode;
    tErrorCode m_errorCode;
    CPlayer();
    static CPlayer* getInstance();
    // TODO: doc valtas miatt egy struktúrába tegyük? vagy hogy?
    void setFdms2(fdms2* fdms2){
        m_fdms2=fdms2;
        m_bPlayable= m_fdms2->getDiskAudioSize()>10000;
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
    //m_MixConsole; //TODO: vumeter tema
    int m_SelectedProgram; //nem lehetne atvenni a disktol?

//inside    
    fdms2 m_waveFdms2;
	bool m_bPlayable;
	bool m_bPlayNow;
	bool m_bRecordable;
    //TODO:refactoring needed
    fdms2pos m_PosEditCursor;
	
//    DMix1 *m_MixConsole;
//TODO:inside must be protected soon.
    HWAVEOUT      m_waveOutHandle;
    WAVEFORMATEX  m_waveFormat;
    HANDLE  m_waveEvent;
    DWORD   m_waveSize;
    DWORD   m_waveSampleMax;
    short** m_waveMultiChannel;

    #define MAX_WHSIZE (1<<3)
    #define MASK_WHSIZE (MAX_WHSIZE-1)
    WAVEHDR m_waveHdr[MAX_WHSIZE];
    WAVEHDR* m_waveLastHdr;
    HANDLE m_thHandle;
    DWORD m_thId;
    unsigned char m_iMixer;
    unsigned char m_iStreamer;
    bool m_bRun;
    DWORD worker();
    static DWORD __stdcall ThreadEntry(void* ptr);
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
