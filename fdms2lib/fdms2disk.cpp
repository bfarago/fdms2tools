#include "fdms2.h"
#include "fdms2disk.h"

fdms2disk::fdms2disk(void): m_dtMode(DTUninitialized), m_pszFileName(NULL),m_pszInfo(NULL),m_iDiskId(0),
m_iEof(0), m_iBadSector(0),m_rwType(RWReadOnly), m_hFile(NULL), m_hMap(NULL),m_bVerbose(false)
{
}

fdms2disk::~fdms2disk(void)
{
    stop();
    setFileName(NULL);
    if (m_pszInfo) free( m_pszInfo);
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

void fdms2disk::setFileName(const char* s){
    if (m_pszFileName) {
        free(m_pszFileName);
        m_pszFileName=0;
        m_dtMode= DTUninitialized;
    }
    if (s){
        m_pszFileName= strdup(s);
        m_dtMode= DTImage;
    }
}
int fdms2disk::getNumDisk(){
    EErrorCode ec;
    int iDisk=-1;
    do{
        iDisk++;
        setDiskId(iDisk);
        ec=start();
        stop();
    }while (ec != ErrFileNotFound);
    return iDisk;
}
void fdms2disk::setDiskId(int iDiskId){
    m_iDiskId= iDiskId;
    char * pszTmp=getDiskPath(iDiskId);
    setFileName( pszTmp );
    if (pszTmp) free(pszTmp);
    m_dtMode= DTDisk;
}

char *fdms2disk::getDiskPath(int iDiskId){
    char* pszTmp=(char*)malloc(64);
#ifdef WIN32
    sprintf_s(pszTmp,64, TEXT("\\\\.\\PHYSICALDRIVE%i"), iDiskId);
#else
    sprintf(pszTmp, "/dev/hd%c", 'a'+iDiskId);
#endif
    return pszTmp;
}
EErrorCode fdms2disk::start(){
    return startMapOfFile(RWReadOnly);
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
                     0 ,
                     FILE_SHARE_READ, 
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
        sprintf_s(tmp, 128, TEXT("Image (%iMb)"), s/1024/1024);
        m_pszInfo=strdup(tmp);
    }else{
        DISK_GEOMETRY dg;
        DWORD junk;
        BOOL bResult = DeviceIoControl(hFile,  // device to be queried
            IOCTL_DISK_GET_DRIVE_GEOMETRY,  // operation to perform
                                   NULL, 0, // no input buffer
                            &dg, sizeof(dg),     // output buffer
                            &junk,                 // # bytes returned
                            (LPOVERLAPPED) NULL);  // synchronous I/O
        s=dg.Cylinders.QuadPart * (ULONG)dg.TracksPerCylinder *
        (ULONG)dg.SectorsPerTrack * (ULONG)dg.BytesPerSector;
        char tmp[128];
        sprintf_s(tmp, 128, TEXT("Disk%i (%iMb)"), m_iDiskId, s/1024/1024);
        m_pszInfo=strdup(tmp);
    }

	CloseHandle(hFile);

#endif 
    m_lDiskSize=s;
    return ErrNone;
}
EErrorCode fdms2disk::startMapOfFile(ERWType rwType){
    int mode=0;
    int modemap=0;
    int modeshare=0;
    int attr=0;
    if (rwType == RWReadWrite){
        mode= GENERIC_READ | GENERIC_WRITE;
        modeshare=FILE_SHARE_READ | FILE_SHARE_WRITE;
        modemap=PAGE_READONLY;
        
    }else{
        mode= GENERIC_READ;
        modeshare=FILE_SHARE_READ| FILE_SHARE_WRITE;
        modemap=PAGE_READONLY;
        attr|= FILE_ATTRIBUTE_READONLY;
    }
    if (m_dtMode == DTDisk){
        //attr|= FILE_ATTRIBUTE_DEVICE;
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
    if (rHFile == INVALID_HANDLE_VALUE){
        checkLastError();
        return ErrFileNotFound;
    }else{
        if (m_dtMode == DTDisk){
        UINT uiType=GetDriveType(m_pszFileName);
        getDiskSize();

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
    if (rHMap ==0){
      CloseHandle(rHFile);
      rHMap= OpenFileMapping(FILE_MAP_READ, FALSE, m_pszFileName);
    }
    m_hFile=rHFile;
    m_hMap= rHMap;
    if (m_dtMode == DTImage){
        if (rHMap == 0){
          //checkLastError();
          return ErrMapping;
        }
    }
    return ErrNone;
}
EErrorCode fdms2disk::readViewFromFile(void*& rpMap, t_offset oMap, t_offset lMap, ERWType rwType){
    if(!m_hFile) return ErrMapping;
    if (rpMap) free(rpMap);
    lMap=((lMap/512)+1)*512;
    rpMap=malloc(lMap);
    if (!rpMap) return ErrMemoryFull;
    DWORD dwRead=0;
    LARGE_INTEGER li;
    li.QuadPart=oMap;
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
    if (!ReadFile(m_hFile, rpMap, lMap, &dwRead, NULL)){
        checkLastError();
    }
    if (lMap != dwRead) return ErrInvalidParameter; //TODO: vagy read error?
    return ErrNone;
}
EErrorCode fdms2disk::startViewMapOfFile(void*& rpMap, t_offset oMap, t_offset lMap, ERWType rwType){
    if (!m_hMap) return readViewFromFile(rpMap, oMap, lMap, rwType);
    int mode=0;
    if (rwType == RWReadWrite){
        mode= FILE_MAP_ALL_ACCESS;
    }else{
        mode= FILE_MAP_READ;
    }
    //if (!m_hMap) startMapOfFile(rwType); //TODO: ez nem biztos
    

    try{
	    // Get a pointer to the file-mapped shared memory.
		rpMap = MapViewOfFile( 
            m_hMap,				// object to map view of
			mode,				// read/write access
			(DWORD)((unsigned long)(oMap>>32) & (unsigned long)0xFFFFFFFF),	// high offset:  map from
			(DWORD)(unsigned long)oMap & (unsigned long)0xFFFFFFFF,			// low offset:   beginning
			(unsigned long)lMap & (unsigned long)0xFFFFFFFF);				// default: map entire file
		if (rpMap == NULL) {
            checkLastError();
            return ErrInvalidParameter;
		}
        if (rpMap == MAP_FAILED){
         return ErrMapping;
        }
    }catch(...){
        return ErrCatch;
    }
    return ErrNone;
}

void fdms2disk::stopViewMapOfFile(void*& rpMap){
    if (rpMap) {
        if (m_hMap){
            UnmapViewOfFile(rpMap);
        }else{
            free(rpMap);
        }
        rpMap=NULL;
    }
}
void fdms2disk::stopMapOfFile(){
    if (m_hMap) CloseHandle(m_hMap);
    m_hMap=NULL;
    if (m_dtMode == DTImage){
        if (m_hFile) CloseHandle(m_hFile);
        m_hFile=NULL;
        //TODO: Disket nem lehet bezarni? Miért?
    }
    
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
	MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
	LocalFree( lpMsgBuf );
    return dwError;
}

EErrorCode fdms2disk::startPtr(void* &rPtr, t_offset offs, t_length len){
    return startViewMapOfFile(rPtr, offs, len, RWReadOnly);
}
void fdms2disk::stopPtr(void* &rPtr){
    stopViewMapOfFile(rPtr);
}