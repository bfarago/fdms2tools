#include "StdAfx.h"
#include "PeekCache.h"

PeekCache::PeekCache(void):m_bInit(false), m_div(1),m_pFdms2(NULL),m_iPrg(-1),m_cPos(-1)
,m_thHandle(NULL),m_thId(0),m_bRun(false),m_pUser(NULL)
{
}

PeekCache::~PeekCache(void)
{
   stop();
   if (m_pFdms2) delete m_pFdms2;
}
void PeekCache::stop(){
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
void PeekCache::init(){
    if (!m_pFdms2) return;
    if (m_iPrg<0) return;
    m_cPos=0;
    INT64 maxsize= m_pFdms2->getProgramSampleCount(m_iPrg);
    if (maxsize<=0) return;
    ASSERT(maxsize < 0x1FFFFFFFFFFFF); //m_div 32 bites, tehát 32+17 bit lehetne max a maxsize értéke.
    m_div= (unsigned int)(maxsize/MAXPEEKDATA); 
    if (!m_div) return;
    if (!m_thHandle){
        m_bRun=true;
        m_thHandle= CreateThread(NULL, 8192, &PeekCache::ThreadEntry, this, 0, &m_thId);
    }
}
DWORD PeekCache::ThreadEntry(void* ptr){
    PeekCache * p=((PeekCache*)ptr);
    if (!p) return 0;
    DWORD ret=p->worker();
    p->m_thHandle=0;
    p->m_bRun=false;
    return ret;
}
bool PeekCache::read(fdms2stream* pStream){
    if (!m_bRun) return false;
    for (int iCh=0; iCh<8; iCh++){
        short iMax, iMin;
        iMax=0; iMin=0;
        for (int i=0; i< pStream->iSamples; i+=4){
            short val= pStream->pBuff[iCh][i];
            if (val< iMin) iMin=val;
            if (val> iMax) iMax=val;
            if (!m_bRun) return false;
        }
        m_cache[m_cPos][iCh].max=(iMax);
        m_cache[m_cPos][iCh].min=(iMin);
    }
    m_cPos++;
    if (m_cPos>=MAXPEEKDATA) return false;
    return true;
}

DWORD PeekCache::worker(){
    m_pFdms2->reset();
    if (load()){
        return m_cPos;
    }
    m_fdms2reader.registerFdms2(m_pFdms2);
    m_fdms2reader.registerStreaming(this);
    m_fdms2reader.setBlockSizeSample(m_div);
    fdms2pos pLen;
    pLen.setSample(  m_pFdms2->getProgramSampleCount(m_iPrg) );
    m_fdms2reader.setRegion(m_iPrg,0,pLen);
    m_fdms2reader.start();
    while( m_fdms2reader.isRunning()){
        Sleep(200);
    }
    m_fdms2reader.stop();
    if (m_cPos>0) {
        m_bInit=true;
        store();
        if (m_pUser) m_pUser->CacheUpdated();
    }
    return m_cPos;
}
/*
DWORD PeekCache::worker(){
    m_pFdms2->reset();
    if (load()){
        return m_cPos;
    }
    int iIdx;
    fdms2pos pLen;
    fdms2pos pos;
	t1_toffset start;
    m_cPos=0;
    iIdx=0;
    unsigned int uiDiff=m_div;
    try{
        while(!m_pFdms2->getPart(m_iPrg, iIdx, start, pLen)){
                pos.setPos(start);
                pLen.addPos(start);
                while ((pos.m_Sample < pLen.m_Sample)){
                    for (int iCh=0; iCh<8; iCh++){
                        short iMax, iMin;
                        iMax=0; iMin=0;
                        for (unsigned int i=0; i< uiDiff>>2; i+=4){
                            short val=m_pFdms2->getValue(pos.m_Pos + (i>>5), iCh, i & 31);
                            if (val< iMin) iMin=val;
                            if (val> iMax) iMax=val;
                        }
                        if (!m_bRun) return 0;
                        m_cache[m_cPos][iCh].max=(iMax);
                        m_cache[m_cPos][iCh].min=(iMin);
                    }
                    pos.addSample(uiDiff); m_cPos++;
                    if (m_cPos>=MAXPEEKDATA) break;
			    }
                iIdx++;
                if (m_cPos>=MAXPEEKDATA) break;
        }
    }catch(...){
        m_bInit=false;
    }
    if (m_cPos>0) {
        m_bInit=true;
        store();
        if (m_pUser) m_pUser->CacheUpdated();
    }
    return m_cPos;
}
*/
void PeekCache::getPeekFileName(char* s){
    sprintf(s,"%sp%i",m_pFdms2->getFileName(),m_iPrg);
    int len=strlen(s);
    for(int i=0; i<len; i++){
        if (s[i]==' ') s[i]='_';
    }
}
void PeekCache::store(){
    char s[512];
    getPeekFileName(s);
    FILE* f=fopen(s, "wb");
    if (f){
        fwrite(&m_iPrg,sizeof(m_iPrg), 1, f);
        fwrite(&m_div,sizeof(m_div), 1, f);
        fwrite(&m_cPos,sizeof(m_cPos), 1, f);
        int size=MAXPEEKDATA;
        fwrite(&size,sizeof(size), 1, f);
        tPeekDataMultiChannel* p=m_cache;
        int wrsize=0;
        wrsize=fwrite(m_cache, sizeof(tPeekDataMultiChannel),size, f);
        fclose(f);
    }
}
bool PeekCache::load(){
    char s[512];
    getPeekFileName(s);
    FILE* f=fopen(s, "rb");
    if (f){
        bool bError=false;
        int cpos=0;
        fread(&m_iPrg,sizeof(m_iPrg), 1, f);
        fread(&m_div,sizeof(m_div), 1, f);
        fread(&cpos,sizeof(m_cPos), 1, f);
        int size=0;
        fread(&size,sizeof(size), 1, f);
        if (size != MAXPEEKDATA ){
            bError=true;
        }
        if (size>MAXPEEKDATA) size=MAXPEEKDATA;
        int rc=0;
        rc=fread(m_cache, sizeof(tPeekDataMultiChannel), size, f);
        fclose(f);
        m_cPos=cpos;
        m_bInit=true;
        if (m_pUser) m_pUser->CacheUpdated();
        return true;
    }
    return false;
}
#include <math.h>
signed short PeekCache::val2db(signed short val){
    int s=1;
    if (val<0) s=-1; 
    return short(log((double)abs(val))/log((double)2)*64*1024/96*s);
}
fdms2pos PeekCache::getPeekMaxLength(){
    fdms2pos p;
    p.setSample(m_cPos*m_div);
    return p;
}
void PeekCache::setFdms2(fdms2* pFdms2, int iPrg, ICacheUser* pUser){
    if (m_iPrg!= iPrg) {
        stop();
        m_bInit=false;
        m_cPos=-1;
    }
    if (m_pFdms2) delete m_pFdms2;
    m_pUser=pUser;
    m_pFdms2= new fdms2();
    m_pFdms2->copy(pFdms2);
    m_pFdms2->start();
    m_iPrg=iPrg;
}
bool PeekCache::getPeek(INT64 iOffs, short iCh, short &iMax, short &iMin){
    if (m_cPos<0) init();
    if (m_cPos<1) return false;
    if (!m_div) return false;
    int offs= (int) (iOffs / m_div);
    if (offs<0) return true;
    if (offs>m_cPos) return false;
    sPeekData1* pData= &m_cache[offs][iCh];
    sPeekData1* pData2= &m_cache[offs+1][iCh];
    double modulo= (iOffs%m_div);
    iMax= pData->max+ (pData2->max-pData->max)*(modulo)/m_div;
    iMin= pData->min+ (pData2->min-pData->min)*(modulo)/m_div;
    return true;
}