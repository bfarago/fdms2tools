#pragma once
#include <fdms2lib.h>
#include "fdms2reader.h"

typedef struct sPeekData1{
    signed short min; 
    signed short max;
} stPeekData1;

typedef sPeekData1 tPeekDataMultiChannel[8];
#define MAXPEEKDATA (512*1024)
class ICacheUser{
public:
 virtual  void CacheUpdated()=0;
};

class PeekCache: public fdms2streamingIF
{
    tPeekDataMultiChannel m_cache[MAXPEEKDATA];
    bool m_bInit;
    unsigned int m_div;
    fdms2* m_pFdms2;
    fdms2reader m_fdms2reader;
    int m_iPrg;
    int m_cPos;
    ICacheUser* m_pUser;
public:
    PeekCache(void);
    ~PeekCache(void);
    virtual bool read(fdms2stream* pStream);
    void setFdms2(fdms2* pFdms2, int iPrg, ICacheUser* pUser);
    void init();
    void stop();
    bool isRunning(){
        return (m_cPos>1) && !m_bInit;
    }
    bool getPeek(INT64 iOffs, short iCh, short &iMax, short &iMin);
    fdms2pos getPeekMaxLength();
private:
    DWORD worker();
    HANDLE m_thHandle;
    DWORD m_thId;
    bool m_bRun;
    signed short val2db(signed short val);
    static DWORD __stdcall ThreadEntry(void* ptr);
    void store();
    bool load();
    void getPeekFileName(char* s);
};
