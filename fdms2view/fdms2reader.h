/*Written by Barna Farago <brown@weblapja.com> 2006-2018
*/
#pragma once
#include "fdms2.h"

class fdms2reader;

class fdms2stream{
public:
    fdms2stream();
    ~fdms2stream();
    void init(int sampleSize); //malloc
	void close(); //free
    ///Streaming source
    fdms2reader* pFdms2Reader; 
    ///Actual program number
    int iPrg;
    ///Actual logical position
    fdms2pos posLogical;
    ///Actual relative position
    fdms2pos posAbsolute;
    ///Pointer of the data stream
    short* pBuff[FOSTEXMAXCHANNELS];
    ///Number of used samples
    int iSamples;
    ///Number of used channels
    int iChannels;
    ///Sender will set this, if ready
    bool bFilled;
    ///Receiver must set this
    bool bProcessed;
    ///Bad sector or similar. Receiver can check this, if interested in the state.
    bool bError;
    ///TimeStamp of the buffer creation
    DWORD tsProduced;
private:
	int iMaxSamples;
};

class fdms2streamingIF{
public:
    virtual bool read(fdms2stream* pStream)=0;
};


class fdms2reader
{
public:
    fdms2reader(void);
    ~fdms2reader(void);
    void registerFdms2(fdms2streamerIF* pFdms2);
    void registerStreaming(fdms2streamingIF* pFdms2Streaming);
    bool start();
    void stop();
    bool isRunning(){ return m_bRunning;}
    void setRegion(int iPrg, fdms2pos rStart, fdms2pos rStop);
    void setBlockSizeSample(int blockSize);
    fdms2pos getRegionMax(){ return m_PosRegionMax; }
    fdms2pos getActualPos(){ return m_Stream.posLogical; }
private:
    bool m_bRunning;
    fdms2streamingIF* m_pFdms2Streaming;
    fdms2streamerIF* m_pFdms2;
    DWORD worker();
    HANDLE m_thHandle;
    DWORD m_thId;
    int m_iPrg;
    fdms2pos m_PosRegionStart;
    fdms2pos m_PosRegionStop;
    fdms2pos m_PosRegionMax;
    fdms2pos m_PosActual;
    fdms2stream m_Stream; //would be pool
    int m_iBlockSizeSample;
    void DoOrderRegion();
    DWORD DoWork();
    static DWORD __stdcall ThreadEntry(void* ptr);
};
