/*Written by Barna Farago <brown@weblapja.com> 2006-2018
*/
#include "fdms2.h"
#include "fdms2disk.h"

#ifdef WIN32
#include <WinIoCtl.h>
#endif
#ifndef ASSERT
#define ASSERT(cond) //if (!(conf)) {}
#endif


fdms2disk::fdms2disk(void): m_dtMode(DTUninitialized), m_pszFileName(NULL),m_pszInfo(NULL),m_iDiskId(0),
m_iEof(0), m_iBadSector(0),m_rwType(RWReadOnly), m_hFile(NULL), m_hMap(NULL),m_bVerbose(false),
m_usedPtrs(0), m_lDiskSize(0)
{
    for(int i=0; i<MAXNUMFDMS2DISKPTRS; i++) m_ptrs[i]=NULL;
}

fdms2disk::~fdms2disk(void)
{
    stop();
    setFileName(NULL);
    if (m_pszInfo) free( m_pszInfo);
}
void fdms2disk::setVerbose(bool verbose){
    m_bVerbose=verbose;
}
fdms2disk* fdms2disk::duplicate(){
    fdms2disk* p=new fdms2disk;
    switch(m_dtMode){
        case DTImage:
            p->setFileName( getFileName() );
            break;
        case DTDisk:
            p->setDiskId( getDiskId() );
            break;
    };
    p->start();
    return p;
}
const char* fdms2disk::getFileName()const { return m_pszFileName; }
void fdms2disk::setFileName(const char* s){
    if (m_pszFileName) {
        free(m_pszFileName);
        m_pszFileName=0;
        m_dtMode= DTUninitialized;
    }
    if (s){
        m_pszFileName= _strdup(s);
        m_dtMode= DTImage;
    }
}
int fdms2disk::getNumDisk(int* ids, int maxnum, bool all){
    if (!ids) return 0;
    EErrorCode ec;
    int iDisk=0;
    int iDetectedDisks=0;
    do{
        if (iDetectedDisks > maxnum) break;
        setDiskId(iDisk);
        ec=start();
        if (ec == ErrNone){
            if (!m_diskButNotFdms2 ||all){
                ids[iDetectedDisks]=iDisk;
                iDetectedDisks++;
            }
        }
        stop();
        iDisk++;
    }while (ec != ErrFileNotFound);
    return iDetectedDisks;
}
void fdms2disk::setDiskId(int iDiskId){
    m_iDiskId= iDiskId;
    char szTmp[100];
    szTmp[0]=0;
    if (getDiskPath(iDiskId, szTmp, 100)){
        setFileName( szTmp );
        m_dtMode= DTDisk;
        m_atMode= ATUninitialized;
    }
}
bool fdms2disk::getDiskPath(int iDiskId, char* dst, int len){
    if (iDiskId<0) return false;
    if (iDiskId>20) return false;
#ifdef WIN32
    sprintf_s(dst, len, TEXT("\\\\.\\PHYSICALDRIVE%i"), iDiskId);
#else
    sprintf(dst, "/dev/hd%c", 'a'+iDiskId);
#endif
    return true;
}
EErrorCode fdms2disk::start(){
    return startMapOfFile(RWReadWrite);
}
void fdms2disk::stop(){
    stopMapOfFile();
}
void fdms2disk::reset(){
    m_iEof=0;
    m_iBadSector=0;
}
EErrorCode fdms2disk::readDiskInfo(){
    if (!m_pszFileName) return ErrInvalidParameter;
    EErrorCode ret= ErrNone;
    m_diskButNotFdms2=0;
    if (m_pszInfo){
        free(m_pszInfo);
        m_pszInfo=NULL;
    }
	t_length s=0; 
#ifndef WIN32
    struct stat64 st64;
    stat64(m_filename, &st64);
    
    if (S_ISREG(st64.st_mode)){
        s= st64.st_size;
		printf("size64 : %lld\n", s);
    }
    if (S_ISBLK(st64.st_mode)){
		s= st64.st_size;
		printf("blocks : %lld\n", st64.st_blocks);
		printf("blocksize : %lld\n", st64.st_blksize);
		printf("size : %lld\n", s);
    }
#else
 
	HANDLE hFile= CreateFile(m_pszFileName, 
                     GENERIC_READ,
                     FILE_SHARE_READ | FILE_SHARE_WRITE,
                     NULL,
                     OPEN_EXISTING, 
                     0,//FILE_ATTRIBUTE_NORMAL, 
                     NULL);
	DWORD high=0;
    if (hFile == INVALID_HANDLE_VALUE ) return ErrFileNotFound;
    if (m_dtMode == DTImage){
	    s = GetFileSize(hFile,  &high);
	    s+= (((UINT64)high)<<32);
        char tmp[128];
        sprintf_s(tmp, 128, TEXT("Image (%iMb)"), (int)(s/1024/1024));
        m_pszInfo=_strdup(tmp);
    }else{
        DISK_GEOMETRY dg;
        DWORD junk;
        BOOL bResult = DeviceIoControl(hFile,  // device to be queried
            IOCTL_DISK_GET_DRIVE_GEOMETRY,  // operation to perform
                                   NULL, 0, // no input buffer
                            &dg, sizeof(dg),     // output buffer
                            &junk,                 // # bytes returned
                            (LPOVERLAPPED) NULL);  // synchronous I/O
        if (bResult){
        s=dg.Cylinders.QuadPart * (ULONG)dg.TracksPerCylinder *
        (ULONG)dg.SectorsPerTrack * (ULONG)dg.BytesPerSector;
            
        }else{
            s=0;
            ret=ErrFileNotFound;
            m_diskButNotFdms2 = 1;
        }
#define TMPSIZE (1024)
        char tmp[TMPSIZE];
        sprintf_s(tmp, TMPSIZE, TEXT("Disk%i (%iGb)"), m_iDiskId, (int) s/1024/1024/1024);
        m_pszInfo=fdms2_strdup(tmp);
        
        DRIVE_LAYOUT_INFORMATION_EX* liex=(DRIVE_LAYOUT_INFORMATION_EX*)tmp;
        bResult = DeviceIoControl(hFile,
            IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
            NULL, 0,
            liex, TMPSIZE,
            &junk, (LPOVERLAPPED)NULL);
        char tmp2[128];
        if (bResult){
            DLOG("Disk%i (%iMb, %i part)", m_iDiskId,(int) s/1024/1024, liex->PartitionCount);   
            switch (liex->PartitionStyle){
                case PARTITION_STYLE_RAW:
                    DLOG("Raw");
                    break;
                case PARTITION_STYLE_MBR:
                    DLOG("Mbr(0x%04x)", liex->Mbr.Signature);
                    break;
                case PARTITION_STYLE_GPT:
                    DLOG("Gpt");
                    break;
            }
            sprintf_s(tmp2, 128, ("Pn:%i)"), liex->PartitionCount);
            PARTITION_INFORMATION_EX* piex= liex->PartitionEntry;
            if (!liex->PartitionCount){
                LARGE_INTEGER li;
                li.QuadPart = 2*512;
                li.LowPart = SetFilePointer(hFile, li.LowPart, &li.HighPart, FILE_BEGIN);
                if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
                { 
                    DLOG("SeekError");
                }
                DWORD dwByteRead=0;
                DWORD ret=0;
                char buf[1024];
                int count=1024;

                ret=ReadFile(hFile, buf, count,&dwByteRead,NULL);
                checkLastError();

            }
            else //partitioned
            for (DWORD i=0; i < liex->PartitionCount; i++){
                char * ptname="Unknown";
                BOOL recognized=false;
                LARGE_INTEGER size={0};
                switch (piex->PartitionStyle){
                    case PARTITION_STYLE_RAW:
                        //sprintf_s(tmp2, 128, TEXT("Raw") );
                        DLOG("Raw");
                        break;
                    case PARTITION_STYLE_MBR:
                        {
                            //piex[i].Mbr.PartitionType
                            recognized= piex[i].Mbr.RecognizedPartition;
                            size= piex[i].PartitionLength;
                            
                            #define CASEPTNAME(x) case x: ptname= #x; break;
                            switch (piex[i].Mbr.PartitionType){
                                CASEPTNAME(PARTITION_EXTENDED);
                                CASEPTNAME(PARTITION_ENTRY_UNUSED);
                                CASEPTNAME(PARTITION_FAT_12);
                                CASEPTNAME(PARTITION_FAT_16);
                                CASEPTNAME(PARTITION_FAT32);
                                CASEPTNAME(PARTITION_IFS);
                                CASEPTNAME(PARTITION_LDM);
                                CASEPTNAME(PARTITION_NTFT);
                                CASEPTNAME(VALID_NTFT);
                            }
                            
                            if(IsRecognizedPartition(piex[i].Mbr.PartitionType)){
                                m_diskButNotFdms2=1;
                            }
                        }
                        break;
                    case PARTITION_STYLE_GPT:
                        break;
                }
                DLOG("%i: %s %iMb", i, ptname,(int) size.QuadPart/(1<<20));
            }//for
        }else{ //result
            m_bVerbose=true;
            DWORD le= checkLastError();
            DLOG("Disk%i error#%i", m_iDiskId, le);
            m_diskButNotFdms2=1;
        }
    }

	CloseHandle(hFile);

#endif 
    m_lDiskSize=s;
    return ret;
}
EErrorCode fdms2disk::startMapOfFile(ERWType rwType){
    int mode=0;
    int modemap=0;
    int modeshare=0;
    int attr=0;
	if (!m_pszFileName) return ErrFileNotFound;
    if (rwType == RWReadWrite){
        mode= GENERIC_READ | GENERIC_WRITE;
        modeshare=FILE_SHARE_READ | FILE_SHARE_WRITE;
        modemap=PAGE_READWRITE;
        
    }else{
        mode= GENERIC_READ;
        modeshare=FILE_SHARE_READ| FILE_SHARE_WRITE;
        modemap=PAGE_READONLY;
        attr|= FILE_ATTRIBUTE_READONLY;
    }
    if ((m_pszFileName[0] == '\\')&&(m_pszFileName[1] == '\\')){
        if (m_pszFileName[2] =='.'){
            m_iDiskId= m_pszFileName[strlen(m_pszFileName)-1]-'0';
            m_dtMode=DTDisk;
        }
    }
    if (m_dtMode == DTDisk){
        //attr|= FILE_ATTRIBUTE_DEVICE;
        // mode= GENERIC_READ;
        modeshare=FILE_SHARE_READ| FILE_SHARE_WRITE;
        //attr=FILE_ATTRIBUTE_READONLY;
        attr=0;
        mode=0;
    }else{
        attr|= FILE_ATTRIBUTE_NORMAL;
    }
    if (!m_pszFileName) return ErrInvalidParameter;
    HANDLE rHFile= CreateFile(m_pszFileName, 
                     mode ,
                     modeshare, 
                     NULL,
                     OPEN_EXISTING, 
                     attr, 
                     NULL);
    readDiskInfo(); //TODO: hova tegyük
    if (INVALID_HANDLE_VALUE == rHFile ){
        checkLastError();
        return ErrFileNotFound;
    }else{
        if (m_dtMode == DTDisk){
        UINT uiType=GetDriveType(m_pszFileName);
            //getDiskSize(); //ez csak egy get-er már. Nincs értelme meghivni...

        }
    }
    char * mapname=NULL;
    HANDLE rHMap= CreateFileMapping(
           rHFile,	// use file ?
           NULL,	// no security attributes
           modemap,	// read/write access
           0,		// size: high 32-bits
           0,		// size: low 32-bits
           mapname);	// name of map object //mapname
   /* if (rHMap ==0){
      setVerbose(true);
      DWORD la= checkLastError();
      CloseHandle(rHFile);
      rHMap= OpenFileMapping(FILE_MAP_READ, FALSE, m_pszFileName);
      
    }*/
    if (!rHMap ){ //&& la==5
        //Reopen for Seek/read
        mode= GENERIC_READ;
        CloseHandle(rHFile);
        rHFile= CreateFile(m_pszFileName, 
                     mode ,
                     modeshare, 
                     NULL,
                     OPEN_EXISTING, 
                     FILE_FLAG_NO_BUFFERING |attr, 
                     NULL);
        if (rHFile) m_atMode=ATSeekRead;
    }else{
        m_atMode=ATMap;
    }
    m_hFile=rHFile;
    m_hMap= rHMap;
    if (m_dtMode == DTImage){
        if (rHMap == 0){
          m_atMode=ATSeekRead;
        }
    }
    if (m_atMode==ATUninitialized)   return ErrMapping;
    for (int i=0; i< m_usedPtrs; i++){
        ((fdms2diskPtr*)m_ptrs[i])->setDisk(this);
    }
    return ErrNone;
}

void fdms2disk::stopMapOfFile(){
    for (int i=0; i<m_usedPtrs; i++){
       delete m_ptrs[i];
       m_ptrs[i]=0;
    }
    m_usedPtrs=0;
    if (m_hMap) CloseHandle(m_hMap);
    m_hMap=NULL;
    //if (m_dtMode == DTImage){
        if (m_hFile) CloseHandle(m_hFile);
        m_hFile=NULL;
        //TODO: Disket nem lehet bezarni? Miért?
   // }
    
}
DWORD fdms2disk::checkLastError(){
    DWORD dwError=  GetLastError();
    if (!m_bVerbose) return dwError;
    if (!dwError) return 0;
	LPVOID lpMsgBuf;
	FormatMessage( 
	    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dwError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL 
	);
    messageBox(TEXT("disk"),(LPTSTR)lpMsgBuf);
	LocalFree( lpMsgBuf );
    return dwError;
}
fdms2diskPtrIF* fdms2disk::getNewPtr(){
    m_usedPtrs&= (MAXNUMFDMS2DISKPTRS-1);
    if (m_ptrs[m_usedPtrs] == NULL){
        fdms2diskPtr * p=new fdms2diskPtr();
        p->setDisk(this);
        m_ptrs[m_usedPtrs]= p;
    }
    return m_ptrs[m_usedPtrs++];
}
//---
fdms2diskPtr::fdms2diskPtr(): m_hFile(NULL), m_hMap(NULL), m_disk(NULL), m_bVerbose(false),
m_refCount(0), m_oMap(NULL), m_lMap(0), m_lastPtr(0)
{
}
fdms2diskPtr::~fdms2diskPtr(){
  ASSERT( m_refCount == 0);
  stopPtr(m_lastPtr);
}
EErrorCode fdms2diskPtr::readViewFromFile(void*& rpMap, t_offset oMap, t_offset lMap, t_offset& lRead, ERWType rwType){
//    if (!m_disk) return ErrInvalidParameter;
    if(!m_hFile) return ErrMapping;
    if (rpMap) free(rpMap);
    m_lastPtr=0;
    m_lMap=0;
    lMap=((lMap/512)+1)*512;
    if (lMap >= MAXUINT_PTR) return ErrInvalidParameter;
    rpMap=malloc((size_t)lMap);
    if (!rpMap) return ErrMemoryFull;
    DWORD dwRead=0;
    LARGE_INTEGER li;
    li.QuadPart=oMap;
    m_oMap=oMap;
    DWORD dwPos = SetFilePointer ( m_hFile, li.LowPart, &li.HighPart, FILE_BEGIN);
    if (dwPos == INVALID_SET_FILE_POINTER){
        DWORD dwE=GetLastError();
        if (dwE) {
            if (rpMap) {
                free(rpMap);
                rpMap=NULL;
            }
            checkLastError();
            return ErrSeek;
        }
    }
    if (!ReadFile(m_hFile, rpMap, (DWORD)lMap, &dwRead, NULL)){
        checkLastError();
    }
    lRead= dwRead;
    if (!dwRead){
        free(rpMap);
        rpMap=0;
        return ErrMapping;
    }
    if (lMap != dwRead) 
        return ErrInvalidParameter; //TODO: vagy read error?
    m_lastPtr=rpMap;
    m_lMap=lRead;
    return ErrNone;
}
void fdms2diskPtr::writeBack(){
    if (m_hMap) return; //already written
    if (!m_lMap) return; //zerosize
    LARGE_INTEGER li;
    li.QuadPart=m_oMap;
    DWORD dwPos = SetFilePointer ( m_hFile, li.LowPart, &li.HighPart, FILE_BEGIN);
    if (dwPos == INVALID_SET_FILE_POINTER){
        if (checkLastError()){
            return ;
       }
    }
    DWORD dwWritten;
    if (!WriteFile(m_hFile, m_lastPtr, (DWORD)m_lMap, &dwWritten, NULL)){
        checkLastError();
    }
    if (dwWritten!= m_lMap){
        DLOG("ErrorWriteingBack");
    }
}
EErrorCode fdms2diskPtr::startPtr(void* &rPtr, t_offset offs, t_length len, t_length& lRead, ERWType rwType){
    ASSERT(m_refCount==0);
    m_refCount++;
    if (!m_hMap) return readViewFromFile(rPtr, offs, len, lRead, rwType);
    int mode=0;
    if (rwType == RWReadWrite){
        mode= FILE_MAP_ALL_ACCESS;
    }else{
        mode= FILE_MAP_READ;
    }
    //if (!m_hMap) startMapOfFile(rwType); //TODO: ez nem biztos
    lRead=0;

    try{
	    // Get a pointer to the file-mapped shared memory.
		rPtr = MapViewOfFile( 
            m_hMap,				// object to map view of
			mode,				// read/write access
			(DWORD)((unsigned long)(offs>>32) & (unsigned long)0xFFFFFFFF),	// high offset:  map from
			(DWORD)(unsigned long)offs & (unsigned long)0xFFFFFFFF,			// low offset:   beginning
			(unsigned long)len & (unsigned long)0xFFFFFFFF);				// default: map entire file
		if (rPtr == NULL) {
            checkLastError();
            return ErrInvalidParameter;
		}
        if (rPtr == MAP_FAILED){
         return ErrMapping;
        }
    }catch(...){
        return ErrCatch;
    }
    lRead=len;
    m_lastPtr= rPtr;
    return ErrNone;
}
void fdms2diskPtr::stopPtr(void* &rPtr){
    m_refCount--;
    if (rPtr) {
        if (m_hMap){
            BOOL ret=UnmapViewOfFile(rPtr);
        }else{
            //if (m_refCount>=0) free(rPtr);	//TODO: BUG:error when m_hMap is null.
        }
        rPtr=NULL;
    }
}
void fdms2diskPtr::setVerbose(bool verbose){
    m_bVerbose=verbose;
}
void fdms2diskPtr::setDisk(fdms2disk* disk){
    m_disk= disk;
	if (disk){
     m_hFile=m_disk->getFileHandle();
     m_hMap= m_disk->getMapHandle();
	} else {
		m_hFile = 0;
		m_hMap = NULL;
	}
}
DWORD fdms2diskPtr::checkLastError(){
    DWORD dwError=  GetLastError();
    if (!m_bVerbose) return dwError;
    if (!dwError) return 0;
	LPVOID lpMsgBuf;
	FormatMessage( 
	    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dwError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL 
	);
    messageBox(TEXT("disk"),(LPTSTR)lpMsgBuf);
	LocalFree( lpMsgBuf );
    return dwError;
}
/*
EErrorCode fdms2disk::startPtr(void* &rPtr, t_offset offs, t_length len){
    return startViewMapOfFile(rPtr, offs, len, RWReadOnly);
}
void fdms2disk::stopPtr(void* &rPtr){
    stopViewMapOfFile(rPtr);
}*/