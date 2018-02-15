//#include "StdAfx.h"
#include "fdms2reader.h"
#ifndef ASSERT
#define ASSERT(x)
#endif

fdms2stream::fdms2stream():iChannels(FOSTEXMAXCHANNELS), iSamples(0), pFdms2Reader(NULL), iPrg(0),tsProduced(0)
,bFilled(FALSE), bProcessed(FALSE), bError(FALSE)
{
    for (int i=0; i<FOSTEXMAXCHANNELS; i++){
        pBuff[i]=NULL;
    }
}
fdms2stream::~fdms2stream(){
    for (int i=0; i<FOSTEXMAXCHANNELS; i++){
        if (pBuff[i]) free(pBuff[i]);
    }
}
void fdms2stream::init(int sampleSize){
    for (int i=0; i<FOSTEXMAXCHANNELS; i++){
        //bCh[i] = m_bnCh[i].GetCheck()== BST_CHECKED;
		pBuff[i]= (short*)malloc(sampleSize*sizeof(short));
	}
    iSamples=sampleSize;
}

//-----------------------------
fdms2reader::fdms2reader(void):
m_bRunning(false),m_pFdms2Streaming(NULL),m_iBlockSizeSample(0),m_pFdms2(NULL), m_thHandle(0), m_thId(0)
{
}

fdms2reader::~fdms2reader(void)
{
    if (m_pFdms2) {
        m_pFdms2->stop();
        delete m_pFdms2;
    }
}
void fdms2reader::registerFdms2(fdms2streamerIF* pFdms2){
    m_pFdms2=pFdms2->duplicate();
}
void fdms2reader::registerStreaming(fdms2streamingIF* pFdms2Streaming){
    m_pFdms2Streaming=pFdms2Streaming;
}
void fdms2reader::setRegion(int iPrg, fdms2pos rStart, fdms2pos rStop){
    m_iPrg=iPrg;
    m_PosRegionStart=rStart;
    m_PosRegionStop=rStop;
}
void fdms2reader::DoOrderRegion(){
    if (m_PosRegionStart.m_Sample < m_PosRegionStop.m_Sample) return; //ok
    fdms2pos tmp= m_PosRegionStart;
    m_PosRegionStart= m_PosRegionStop;
    m_PosRegionStop=tmp;
}

void fdms2reader::setBlockSizeSample(int blockSize){
    m_iBlockSizeSample=blockSize;
}
bool fdms2reader::start()
{
    ASSERT(m_pFdms2Streaming);
    ASSERT(m_iBlockSizeSample);
    if (!m_bRunning){
        m_bRunning=true;
        if (!m_thHandle){
            m_thHandle= CreateThread(NULL, 8192, &fdms2reader::ThreadEntry, this, 0, &m_thId);
        }
    }
    return m_thHandle!=NULL;
}
void fdms2reader::stop(){
    if (m_bRunning){
        m_bRunning=false;
        if (m_thHandle) {
            DWORD dwTmp=STILL_ACTIVE;
            while (m_thHandle &&(STILL_ACTIVE == dwTmp )){
                m_bRunning=false;
                Sleep(50);
                GetExitCodeThread(m_thHandle, &dwTmp);
            }
        }
    }
    CloseHandle(m_thHandle);
    m_thHandle =NULL;
}
DWORD fdms2reader::ThreadEntry(void* ptr){
    fdms2reader * p=((fdms2reader*)ptr);
    if (!p) return 0;
    DWORD ret=p->DoWork();
    p->m_thHandle=0;
    p->m_bRunning=false;
    return ret;
}
DWORD fdms2reader::DoWork(){
    ASSERT(m_pFdms2Streaming);
    m_pFdms2->reset();
    DoOrderRegion();
    bool bOk=true;
    t1_toffset offs, offsStart, partMaxLen;
    fdms2pos posPartMaxLen;
    
    int iPartIndex;
    m_PosRegionMax.setSample( m_PosRegionStop.m_Sample-m_PosRegionStart.m_Sample);
    
    m_pFdms2->convertLogical2Abs(
        m_iPrg,
        m_PosRegionStart,
        offs, offsStart, partMaxLen, iPartIndex);
    m_Stream.init(m_iBlockSizeSample);
    m_Stream.pFdms2Reader=this;
    m_Stream.posAbsolute.setPos( offsStart +offs);
    m_Stream.posLogical=m_PosRegionStart;
    m_Stream.iChannels=FOSTEXMAXCHANNELS;
    m_Stream.iPrg=m_iPrg;
    m_Stream.tsProduced=GetTickCount();
    while(!m_pFdms2->getPart(m_iPrg, iPartIndex, offsStart, posPartMaxLen)){
        posPartMaxLen.addPos(offsStart); //calc absolute end of partition.

        while ((m_Stream.posAbsolute < posPartMaxLen))
        {
            if (m_Stream.posLogical < m_PosRegionStop){
                m_Stream.bFilled=true;
                m_Stream.bProcessed=false;
                m_Stream.bError=false;
                int iRead=0;
                try{
                    m_pFdms2->reset();
	                iRead+=m_pFdms2->getValueArrays(m_Stream.posAbsolute.m_Pos, m_iBlockSizeSample,  m_Stream.pBuff);
                }catch(...){
                    m_Stream.bError=true;   
                };
                m_Stream.iSamples=iRead;
                if (iRead){
                    bOk&=m_pFdms2Streaming->read(&m_Stream);      //callback
                }
                if (!bOk) break;
                m_Stream.posLogical.addSample( m_Stream.iSamples );
                m_Stream.posAbsolute.addSample( m_Stream.iSamples );
            }else{
                bOk=false;
                break;
            }
        }
        if (!bOk) break;
        iPartIndex++;
    }
    return (DWORD)m_Stream.posLogical.m_Sample;       //EXIT !
}
