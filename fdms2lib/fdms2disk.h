/*Writed by Barna Farago <brown@weblapja.com>
*/
#ifndef _FDMS2DISK_H_
#define _FDMS2DISK_H_

#include "fdms2common.h"
#include "fdms2error.h"

enum EDiskType{
    DTUninitialized=0,
    DTImage,
    DTDisk
};
enum EAccessType{
    ATUninitialized=0,
    ATMap,
    ATSeekRead
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


class fdms2diskPtrIF : public fdms2errorIF {
public:
    virtual EErrorCode startPtr(void* &rPtr, t_offset offs, t_length len, t_length& lRead, ERWType rwType)=0;
    virtual void stopPtr(void* &rPtr)=0;
    virtual void writeBack()=0;
    virtual void setVerbose(bool verbose)=0;
};

class fdms2disk: public fdms2errorIF{
public:
    fdms2disk(void);
    ~fdms2disk(void);
    fdms2disk* duplicate();
    bool isInvalidFdms2Disk(){ return m_diskButNotFdms2!=0;}
    void setFileName(const char* s);
    const char* getFileName()const;
    char* getInfo(){ return m_pszInfo; }
    int getNumDisk(int* ids, int maxnum, bool all=false);
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
    fdms2diskPtrIF* getNewPtr();
    virtual void setVerbose(bool verbose);
    HANDLE getFileHandle(){ return m_hFile;}
    HANDLE getMapHandle(){ return m_hMap;}
protected:
    char*       m_pszFileName;
    char*       m_pszInfo;
    t_length    m_lDiskSize;
    int         m_iDiskId;
    EDiskType   m_dtMode;
    ERWType     m_rwType;
    EAccessType m_atMode;
    int         m_iEof;
    int         m_iBadSector;
    HANDLE      m_hFile;
    HANDLE      m_hMap;
    bool        m_bVerbose;
    unsigned char m_diskButNotFdms2;

#define MAXNUMFDMS2DISKPTRS (8)
    int m_usedPtrs;
    fdms2diskPtrIF* m_ptrs[MAXNUMFDMS2DISKPTRS];

    bool getDiskPath(int iDiskId, char* dst, int len);
    EErrorCode readDiskInfo();
    EErrorCode startMapOfFile(ERWType rwType);
    void stopMapOfFile();
    DWORD checkLastError();
};

class fdms2diskPtr : public fdms2diskPtrIF{
public:
    fdms2diskPtr();
    virtual ~fdms2diskPtr();
    virtual EErrorCode startPtr(void* &rPtr, t_offset offs, t_length len, t_length& lRead, ERWType rwType);
    virtual void stopPtr(void* &rPtr);
    virtual void writeBack();
    virtual void setVerbose(bool verbose);
    void setDisk(fdms2disk* disk);
protected:
    int m_refCount;
    void* m_lastPtr;
    HANDLE      m_hFile;
    HANDLE      m_hMap;
    t_offset    m_oMap;
    t_offset    m_lMap;
    bool        m_bVerbose;
    fdms2disk*  m_disk;
    EErrorCode readViewFromFile(void*& rpMap, t_offset oMap, t_offset lMap,  t_offset& lRead, ERWType rwType);
    DWORD checkLastError();
};


#endif
