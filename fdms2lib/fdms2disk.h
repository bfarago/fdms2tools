/*Writed by Barna Farago <brown@weblapja.com>
*/
#ifndef _FDMS2DISK_H_
#define _FDMS2DISK_H_

#include "fdms2common.h"

enum EDiskType{
    DTUninitialized=0,
    DTImage,
    DTDisk
};

enum ERWType{
    RWReadOnly=0,
    RWReadWrite
};

enum EErrorCode{
    ErrNone=0,
    ErrInvalidParameter,
    ErrFileNotFound,
    ErrMapping,
    ErrCatch,
    ErrBadSector,
    ErrReadError,
    ErrSeek,
    ErrMemoryFull
};

typedef  long long t_offset;
typedef  long long t_length;

class fdms2diskPtrIF {
public:
    virtual EErrorCode startPtr(void* &rPtr, t_offset offs, t_length len)=0;
    virtual void stopPtr(void* &rPtr)=0;
    virtual void setVerbose(bool verbose)=0;
};


class fdms2disk: public fdms2diskPtrIF {
public:
    fdms2disk(void);
    ~fdms2disk(void);
    fdms2disk* duplicate();
    void setFileName(const char* s);
    char* getFileName(){ return m_pszFileName; }
    char* getInfo(){ return m_pszInfo; }
    int getNumDisk();
    EDiskType getDiskType(){ return m_dtMode; }
    void setDiskId(int iDiskId);
    int getDiskId(){ return m_iDiskId;}
    ERWType getRWType(){ return m_rwType; }
    t_length getDiskSize(){ return m_lDiskSize; }
    EErrorCode start();
    void stop();
    void reset();
    //bool getMap(void* &rpMap, t_offset oMap,t_offset lMap, ERWType rwType);
public: // interface
    virtual EErrorCode startPtr(void* &rPtr, t_offset offs, t_length len);
    virtual void stopPtr(void* &rPtr);
    virtual void setVerbose(bool verbose){
        m_bVerbose=verbose;
    }
protected:
    char*       m_pszFileName;
    char*       m_pszInfo;
    t_length    m_lDiskSize;
    int         m_iDiskId;
    EDiskType   m_dtMode;
    ERWType     m_rwType;
    int         m_iEof;
    int         m_iBadSector;
    HANDLE      m_hFile;
    HANDLE      m_hMap;
    bool        m_bVerbose;

    char *getDiskPath(int iDiskId);
    EErrorCode readDiskInfo();
    EErrorCode startMapOfFile(ERWType rwType);
    EErrorCode startViewMapOfFile(void*& rpMap,t_offset oMap,t_offset lMap, ERWType rwType);
    EErrorCode readViewFromFile(void*& rpMap, t_offset oMap, t_offset lMap, ERWType rwType);
    void stopViewMapOfFile(void*& rpMap);
    void stopMapOfFile();
    DWORD checkLastError();
};

#endif
