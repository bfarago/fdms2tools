/*Writed by Barna Farago <brown@weblapja.com>
*/
#include "fdms2.h"
#define MYPAGESIZE (2*8*2048)
#define MAXPAGESIZE (2*1024*1024)
#define FIRSTDIRBLOCK (0x6000)
#define FIRSTDATABLOCK (0x50000)
#define LENGTHDIRBLOCK (0x2000)
#define A1OFFSDIRBLOCK (0x0400)
#define A2OFFSDIRBLOCK (0x2000)
#define A3OFFSDIRBLOCK (0x4000)
#define BOFFSDIRBLOCK (0x0800)
#define DOFFSDIRBLOCK (0x1000)
#define EOFFSDIRBLOCK (0x1800)

#ifdef WIN32
#define MAP_FAILED NULL
#else
#include <string.h>
#endif
/**Default constructor.
*/
fdms2::fdms2()
:m_partitionMode(NormalPartitionMode),m_badBlock(NULL),m_pMap(NULL),m_filename(NULL),m_startpos(0),
m_length(0), m_badsector(true),m_eof(false),m_pDirMap(NULL),m_oDirMap(0), m_step(0),
m_oMap(0), m_lMap(0)
{
#ifdef WIN32
 m_fdInput=0;
 m_hMap=0;
 m_fdInput=0;
 m_hDirMap=0;
 m_fdDirectory=0;
#else
 m_flagsInput=0;
 m_fdInput=0;
 m_fdDirectory=0;
#endif
 for(int i=0; i<FOSTEXMAXPROGRAMM; i++){
	 m_PartTable[i]=0;
	 m_PartTableLength[i]=0;
     m_ProgramSampleCount[i]=0;
 }
}
fdms2::~fdms2(){
    stop();
    if (m_filename) free(m_filename);
}
void fdms2::setFileName(const char* s){
    if (m_filename) {
        free(m_filename);
        m_filename=0;
        
    }
    if (s) m_filename= _strdup(s);
 }
fdms2streamerIF* fdms2::duplicate(){
    fdms2* p=new fdms2;
    p->copy(this);
    p->start();
    return p;
}

void fdms2::copy(fdms2streamerIF* pIF){
    stop(); //dispose previous state
    fdms2* p=(fdms2*)pIF;
    if (!p) return;
    if (p->m_filename) setFileName(p->m_filename);
    m_eof=false;
    m_badsector=false;
    m_partitionMode=p->m_partitionMode;
}
/**Error logger.
*/
void fdms2::logError(char* err){
 printf("err: fdms2lib : %s \n", err); 
}
t1_toffset fdms2::pagealign(t1_toffset v){
 t1_toffset t=g_pagesize * (t1_toffset)(( v / g_pagesize));
 return  t; 
}
void fdms2::dumphex(t1_toffset pos, long len){
 for (int i = 0 ; i<len; i++){
  unsigned char *b=(unsigned char*)mapPtr(pos+i,0); 
  if (b) printf("%02X ", *b);
 }
}
void fdms2::reset(){
    m_eof=false;
    m_badsector=false;
}
short fdms2::getValue(t1_toffset pos, int channel, int samplenum){
  if (m_eof) return 0;
  if (pos<0) return 0;
  unsigned short v;
  t1_toffset rowPos;
  rowPos=32*( (t1_toffset)(pos/32) ); 
//  printf("getValue row:%i\n", rowPos);
  unsigned char* pch=0;
  pch=(unsigned char*)mapPtr(rowPos, MYPAGESIZE);
  if( (pch != NULL) && ((int)pch != -1)){
//    printf("mapPtr %0x\n", pch);
    pch+= 4*(8*(samplenum/2) + channel);
    v= (unsigned short)((unsigned char)(pch[1+2*(samplenum % 2)]) <<8) | (unsigned char)(pch[ 2*(samplenum %2)]);
  }else{
    v=0;
    m_eof=true;
    m_badsector=true;  //hack
  }
  return (short)v;
}
int fdms2::getValues(t1_toffset pos, int channelmap, int samplenum, short* ptrArray, int lenArray){
  int iCount=0;
  //GET data
  for (int i=0; i< samplenum; i++){
    int chBit=1;
    for(int ch=0; ch < FOSTEXMAXCHANNELS; ch++){
     if (channelmap & chBit){
       short s=0;
       if (!m_badsector) s=getValue(pos, ch, i);
       ptrArray[i*FOSTEXMAXCHANNELS+ch] = s;
       iCount++;
       if (iCount>lenArray) return iCount;	//BUGBUG: we dont know if buffer overruns.
     }
    }
    chBit*=2;
  }
  return iCount;
}
int fdms2::getValueArrays(t1_toffset pos, int samplenum, short** ptrArray){
  int iCount=0;
  //GET data
  for (int i=0; i< samplenum; i++){
      reset();
        for(int ch=0; ch < FOSTEXMAXCHANNELS; ch++){
           short s=0;
	       short *pt=ptrArray[ch];
           if (!m_badsector) s=getValue(pos, ch, i);
           pt[i] = s;
        }
  }
  return samplenum;
}
/**
@param riLastPart Last partition index
*/
int fdms2::getValuesPrg(int iPrg, t1_toffset pos, int channelmap, int samplenum, short* ptrArray, int lenArray, int& riLastPart){
  int iCount=0;
  //GET data
  for (int i=0; i< samplenum; i++){
    int chBit=1;
    for(int ch=0; ch < FOSTEXMAXCHANNELS; ch++){
     if (channelmap & chBit){
       short s=0;
       if (!m_badsector) s=getValue(pos, ch, i);
       ptrArray[i*FOSTEXMAXCHANNELS+ch] = s;
       iCount++;
       if (iCount>lenArray) return iCount;	//BUGBUG: we dont know if buffer overruns.
     }
    }
    chBit*=2;
  }
  return iCount;
}
t1_toffset fdms2::getProgramSampleCount(int iPrg){
    return m_ProgramSampleCount[iPrg];
}
t1_toffset fdms2::getLongestProgramSampleCount(){
    t1_toffset len=0;
    t1_toffset lenMax=0;
    for (int i=0; i< FOSTEXMAXPROGRAMM; i++){
        len=getProgramSampleCount(i);
        if (lenMax<len) lenMax=len;
    }
    return lenMax;
}
int fdms2::convertLogical2Abs(int iPrg, fdms2pos posLogic, t1_toffset& iOffs, t1_toffset& iStart, t1_toffset& iMaxLen, int& iLastPart){
    INT64 pz= 0;
    INT64 pzLogic= posLogic.m_Pos;
    iLastPart=0;
    while (!getPart(iPrg, iLastPart, iStart, iMaxLen)){
        pz+=iMaxLen;
        iLastPart++;
        if (pz>pzLogic) break;
    }
    iOffs=pzLogic+ iMaxLen - pz; iLastPart--; //TODO: ellenorizni miert minusz a pz!
 return 0;
}

 
void fdms2::dumpfostex(t1_toffset pos){
  unsigned short chval1[8];
  unsigned short chval2[8];
  pos=32*( (t1_toffset)(pos/32) ); 
  char* pch=0;
  pch=(char*)mapPtr(pos,8*8);
  if (!pch) return; //overrun
  for (int ch1=0; ch1 < 8 ; ch1++, pch+=4){
        chval1[ch1]= pch[0] <<8 | pch[1];
        chval2[ch1]= pch[2] <<8 | pch[3];
  }
  printf("|");
  for (int ch=0; ch <8 ; ch++){
    long av=abs((short)(chval1[ch]))+ abs((short)(chval1[ch]));
    int v= abs( av /2 / VUMETERDIV );
    for (int j1=0; j1< v; j1++){
     printf("%c", '1'+ch);
    }
    for (int j=v; j< VUMETERMAX+1; j++){
     printf(".");
    }
    printf("|");
  }
}

int fdms2::unmap(){
  if (m_pMap) {
#ifndef WIN32
   munmap(m_pMap, m_lMap);
   printf("after unmap(%x, %Li)\n", m_pMap, m_lMap);
#else
   UnmapViewOfFile(m_pMap);
#endif
   m_pMap=0;
  }
  return 0;
}

int fdms2::map(t1_toffset offset){
  m_bMap=true;
  printf("map(%lli)\n",offset);
  unmap();
  m_lMap= MAPPAGELENGTH;
  m_badsector=false;
  m_eof=false;
  m_oMap=pagealign(offset);
  printf("m_oMap: %lli\n",m_oMap);
  //printf("map\n");
  //Map
  int iRet=0;
  t1_toffset offs=m_oMap+FIRSTDATABLOCK;
  printf("offs: %lli  m_length: %lli m_lMap: %lli\n",offs, m_length, m_lMap);

  if ((m_lMap+offs) > m_length) m_lMap= m_length-offs;
  if (m_lMap<=1){
    m_lMap=MAPPAGELENGTH;	//hack
  }
  if (m_lMap>MAXPAGESIZE){
    m_lMap=MAXPAGESIZE;	//hack
  }
  m_lMap=pagealign(m_lMap);
  printf("m_lMap: %lli\n", m_lMap);

  if (m_lMap<=1) return -1;
#ifndef WIN32
  printf("call mapFile(%0x, false, %0x, %lli, %lli)\n", m_fdInput, m_pMap, offs, m_lMap);
  iRet=mapFile(m_fdInput, false, m_pMap, offs, m_lMap);
#else
  iRet=mapFile(m_hMap, false, m_pMap, offs, m_lMap);
#endif
  return iRet;
}
/**
*/
void* fdms2::mapPtr(t1_toffset pos, t1_toffset max){
  void* p=0;  
  int iRet=0;
//  if (m_badsector) throw 1;  //test it
  try{
   if (pos < m_oMap) {
    m_eof=true; //try again
    iRet=map(pos);
   }
   if ((pos+max) >= (m_oMap+m_lMap)) {
    iRet|=map(pos);
   }
   if (iRet){
    printf("map() returned by: %i\n", iRet);
   }
   if (m_eof) return m_badBlock;
   if (pos-m_oMap<0) {
     logError("negative offset ?");
   }
   if (iRet) return NULL;
   if (m_pMap && !m_badsector){
    p=(char*)m_pMap + (pos-m_oMap);
   }else{
    p=m_badBlock;
    printf("mapPtr(%lli, %lli)\n", pos,max);
   }
  }catch(...){
    logError("catch\n");
    m_badsector=true;
    p= m_badBlock;
  }
  return p;
}
#ifdef WIN32
unsigned int fdms2::getpagesize(){
	 // Get the system allocation granularity.
  GetSystemInfo(&m_SysInfo);
  return m_SysInfo.dwAllocationGranularity;
}
#endif

#ifndef WIN32
int fdms2::openFile(const char* filename, bool bWrite, int &rFD){
  int flags=0;
  if (bWrite){
    flags= O_LARGEFILE ;//  | O_DIRECT
  }else{
    flags= O_RDONLY | O_LARGEFILE; //  | O_DIRECT
  }
  rFD=open(filename, flags);
  if (rFD == 0){
    logError("we can not open the specified file!");
    return -1;
  }
  return 0;
}
int fdms2::mapFile(int &rhMap, bool bWrite, void* &rpMap,t1_toffset oMap,t1_toffset lMap){
  try{
    printf("mapFile: lMap:%lli, rhMap:%0x, oMap: %lli\n",lMap, rhMap, oMap);
    errno=0;
//    rpMap= mmap(0, lMap, PROT_READ, MAP_PRIVATE | MAP_FILE, rhMap, oMap);
    rpMap= mmap64(0, lMap, PROT_READ, MAP_PRIVATE | MAP_FILE, rhMap, oMap);
    printf("mapFile ret: %0x, errno:%i\n", rpMap, errno);
    if (errno !=0){
      char* ers=strerror(errno);
      printf("error: %s\n", ers);
      free(ers);
      m_eof=true;
      m_badsector=true; //hack
    }else{
      if (rpMap){
        mlock(rpMap, lMap);
      }
    }
  }catch(...){
    logError("catch map()");
    m_badsector=true; //checkit
    return -1;
  }
  return 0;
}
#else
int fdms2::openFile(const char* filename, bool bWrite, HANDLE &rHFile, HANDLE &rHMap, const char* mapname){
  int mode=0;
  int modemap=0;
  if (bWrite){
    mode= GENERIC_READ | GENERIC_WRITE;
    modemap=PAGE_READONLY;
  }else{
    mode= GENERIC_READ;
    modemap=PAGE_READONLY;
  }
  if (!filename) return -1;
  rHFile= CreateFile(filename, 
                     mode ,
                     FILE_SHARE_READ, 
                     NULL,
                     OPEN_EXISTING, 
                     FILE_ATTRIBUTE_NORMAL, 
                     NULL);

  if (rHFile == INVALID_HANDLE_VALUE){
    logError("File not found.\n");
    return -1;
  }

  rHMap= CreateFileMapping(
		   rHFile,	// use file ?
		   NULL,	// no security attributes
		   modemap,	// read/write access
		   0,		// size: high 32-bits
		   0,		// size: low 32-bits
		   mapname);	// name of map object //mapname
  if (rHMap ==0){
      CloseHandle(rHFile);
      rHMap= OpenFileMapping(FILE_MAP_READ, FALSE, filename);
  }
  if (rHMap == 0){
    DWORD dwError=  GetLastError();
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
	// Process any inserts in lpMsgBuf.
	// ...
	// Display the string.
	MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
	// Free the buffer.
	LocalFree( lpMsgBuf );
	return -1;
  }
  return 0;
}
int fdms2::mapFile(HANDLE &rhMap, bool bWrite, void* &rpMap,t1_toffset oMap,t1_toffset lMap){
  int mode=0;
  if (bWrite){
    mode= FILE_MAP_ALL_ACCESS;
  }else{
    mode= FILE_MAP_READ;
  }
  try{
	if (rhMap != NULL) {
	// Get a pointer to the file-mapped shared memory.
		rpMap = MapViewOfFile( 
			rhMap,				// object to map view of
			mode,				// read/write access
			(DWORD)((unsigned long)(oMap>>32) & (unsigned long)0xFFFFFFFF),	// high offset:  map from
			(DWORD)(unsigned long)oMap & (unsigned long)0xFFFFFFFF,			// low offset:   beginning
			(unsigned long)lMap & (unsigned long)0xFFFFFFFF);				// default: map entire file
		if (rpMap == NULL) {
			LPVOID lpMsgBuf;
			FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL 
			);
			// Process any inserts in lpMsgBuf.
			// ...
			// Display the string.
//			MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
			// Free the buffer.
			LocalFree( lpMsgBuf );
			//CloseHandle(rhMap);
		}
	}
    if (rpMap == MAP_FAILED){
     int tmperrno= errno;
     logError("We can not mmap the specified file!");
     return -1;
    }
  }catch(...){
    logError("catch map()");
    return -1;
  }
  return 0;
}
int fdms2::directMapFile(void* &rpMap,t1_toffset oMap,t1_toffset lMap){
    return 0;
}
#endif

int fdms2::start(){
 m_pMap=0; m_lMap=0;
 m_badBlock=malloc(MYPAGESIZE);
 
 t1_toffset mdisksize = getDiskSize();
 if (mdisksize){
   m_length=mdisksize;
 }
 g_pagesize = getpagesize();
 m_endpos=m_startpos+m_length;
 printf("log: tester1.start: %s\n %016llXh - %016llXh (%016llXh)\n", m_filename, m_startpos, m_endpos , m_length);
 
 m_length= pagealign(m_length);
 m_endpos=m_startpos+ m_length;
 printf("log: after align : %s\n %016llXh - %016llXh (%016llXh)\n", m_filename, m_startpos, m_endpos , m_length);
 m_badsector=false;
 m_eof=false;
#ifndef WIN32
 if (openFile(m_filename, false, m_fdInput)) return -1;
#else
 if (openFile(m_filename, false, m_fdInput, m_hMap, NULL)) return -1; //"mapfilefdms2"
#endif

//map
  map(0);
  startDirectory();
  dump();
  map(0);
    
  return 0;
}

int fdms2::stop(){
 stopDirectory();
 if (m_badBlock){
    free(m_badBlock);
    m_badBlock=0;
 }
#ifndef WIN32
 unmap();
// munmap(m_pMap, m_length);
 close(m_fdInput);
 m_fdInput=0;
#else
 UnmapViewOfFile(m_pMap);
 m_pMap=0;
 CloseHandle(m_hMap);
 m_hMap=0;
#endif
 return 0;
}

void fdms2::SigBusOccured(int sig){
 m_badsector=true;
}

/**
*/
void fdms2::dump(){
 t1_toffset d=m_step; //division 
 t1_toffset pos=m_startpos; 
 for (t1_toffset p= 0; p< d; p++){
    pos=m_startpos+ p*m_length/d;
    printf("%016llXh ",pos+FIRSTDATABLOCK);
    dumpfostex(pos);
    printf(" ");
    fdms2pos xpos(pos);
    xpos.dump();
    printf(" \t");
    xpos.dumpByte();
    printf(" %lli", xpos.m_Sample);
#ifdef VERBOSE_MAPPING    
    if (m_bMap) printf(" MAP(%016LXh)", m_oMap);
#endif
    m_bMap=false;
    printf("\n");
 } 
}
t1_toffset fdms2::getDiskAudioSize(){
	return getDiskSize()-FIRSTDATABLOCK;
}
/** Returns By size of the specified Disk or file.
@return 64bit wide size.
*/
t1_toffset fdms2::getDiskSize(){
	if (!m_filename) return 0;
	t1_toffset s=0; 
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
 
	HANDLE hFile= CreateFile(m_filename, 
                     0 ,
                     FILE_SHARE_READ, 
                     NULL,
                     OPEN_EXISTING, 
                     FILE_ATTRIBUTE_NORMAL, 
                     NULL);
	DWORD high=0;
	s = GetFileSize(hFile,  &high);
	s+= (((UINT64)high)<<32);

	CloseHandle(hFile);


#endif 
    return s;
}

int fdms2::dumpABlock(unsigned char* pC){
 unsigned int vDW=0;
 unsigned int vDW2=0;
 unsigned int iIndx=0;
 unsigned int iMaxIndx=0;
 iMaxIndx=pC[2] | pC[1]<<8 | pC[0]<<16;
 pC+=4;
 do{
    vDW=pC[3] | pC[2]<<8 | pC[1]<<16 | pC[0]<<24;
    vDW2=pC[7] | pC[6]<<8 | pC[5]<<16 | pC[4]<<24;
    if (vDW) printf(" %i: 0x%08X 0x%08X\n", iIndx++, vDW, vDW2);
//    if (vDW) printf("%i,%d,%d\n", iIndx++, vDW, vDW2);
    pC+=8;
 }while ((vDW!=0)&&(vDW!=0xFFFFFFFF)&&(iIndx<iMaxIndx));
 return iIndx;
}

int fdms2::dumpBBlock(unsigned char* pC){
 unsigned int vDW=0;
 unsigned int vDW2=0;
 unsigned int vQW=0;
 unsigned int iIndx=0;
 
 do{
    vDW=pC[3] | pC[2]<<8 | pC[1]<<16 | pC[0]<<24;
    vDW2=pC[7] | pC[6]<<8 | pC[5]<<16 | pC[4]<<24;
    fdms2pos pos(vDW2*2);
    if (vDW) {
	printf(" %i: 0x%08X 0x%08X ", iIndx++, vDW, vDW2);
	vQW+=vDW2;
	pos.dump();
	printf("\n");
    }
    pC+=8;
 }while ((vDW!=0)&&(vDW!=0xFFFFFFFF)&&(iIndx<256));
 printf(" summ: %08X ", vQW);
 fdms2pos(vQW*2).dump();
 printf("\n");
 return iIndx;
}
void fdms2::killPrgPartTable(int iPrg){
 if (m_PartTable[iPrg]) {
     for (int i=0; i<m_PartTableLength[iPrg]; i++){
         delete( m_PartTable[iPrg][i]);
     }
     m_PartTableLength[iPrg]=0;
     free(m_PartTable[iPrg]);
     m_PartTable[iPrg]=NULL;
 }
}
int fdms2::initPrgPartTable(int iPrg, unsigned char* pC){
 unsigned int vDW=0;
 unsigned int vDW2=0;
 unsigned int vQW=0;
 unsigned int iIdx=0;
 t1_toffset Start=0;
 t1_toffset sampleCount=0;
 fdms2pos Len(0);
 fdms2pos Logical(0);
 
 killPrgPartTable(iPrg);

 while(!getPartFromDisk(iPrg, iIdx, Start, Len)){
	iIdx++;
 }
 m_PartTable[iPrg]=(fdms2part**)malloc(sizeof(fdms2part*)*(iIdx+1)); // +1 a könyveletlen particio miatt.
 m_PartTableLength[iPrg]=0; //meg nulla
 
 iIdx=0;
 if ((m_partitionMode==NormalPartitionMode) || (m_partitionMode == AddUntilTheEnd)){
     while(!getPartFromDisk(iPrg, iIdx, Start, Len)){
	     m_PartTable[iPrg][iIdx]=new fdms2part(Start,Len, Logical);
	     iIdx++;
         sampleCount+=Len.m_Sample;
         m_PartTableLength[iPrg]=iIdx;
     }
     if (m_partitionMode == AddUntilTheEnd){
         fdms2pos diskSize;
         diskSize.setPos(getDiskAudioSize());

         if (sampleCount < diskSize.m_Sample){
             Start+=Len.m_Pos;
             Len.setSample(diskSize.m_Sample-sampleCount);
             m_PartTable[iPrg][iIdx]=new fdms2part(Start,Len, Logical);
	         iIdx++;
             sampleCount+=Len.m_Sample;
             m_PartTableLength[iPrg]++;
         }
     }
 }else{ //RawFormat
    Start=0;
    sampleCount=getDiskAudioSize()/16;
    Len.setSample(sampleCount);
    Logical.setPos(0);
    m_PartTable[iPrg][0]=new fdms2part(Start, Len, Logical);
    m_PartTableLength[iPrg]=1;
 }
 m_ProgramSampleCount[iPrg]=sampleCount;
 return iIdx;
}
void fdms2::setPartitionMode(enPartitionMode pm){
    m_partitionMode=pm;
    stopDirectory();
    startDirectory();
}
int fdms2::get2BCD2i(unsigned char* p){
    return (p[0]&0x0F) +
    ((int)(p[0]&0xF0) >>4)*10;
}
void fdms2::set2BCD(unsigned char* p, int v){
    p[0]= (v%10) | ((v/10)%10 <<4);
}

int fdms2::get4BCD2i(unsigned char* p){
    return (p[0]&0x0F) +
    ((int)(p[0]&0xF0) >>4)*10 +
    ((int)(p[1]&0x0F)*100) +
    ((int)(p[1]&0xF0) >>4)*1000;
}

void fdms2::set4BCD(unsigned char* p, int v){
    p[0]= (v%10) | ((v/10)%10 <<4);
    p[1]= ((v/100)%10) | ((v/1000)%10 <<4);
}
unsigned int fdms2::getDW(unsigned char* p){
 return p[3] | p[2]<<8 | p[1]<<16 | p[0]<<24;
}
void fdms2::setDW(unsigned char* p, unsigned int v){
  p[3]= v & 0xff;
  p[2]= (v >>8) & 0xff;
  p[1]= (v >>16) & 0xff;
  p[0]= (v >>24) & 0xff;
}

int fdms2::getPart(int iPrg, int iIdx, t1_toffset& riStart, t1_toffset& riLen){
    fdms2pos len;
    int iRet=getPart(iPrg, iIdx, riStart, len);
    riLen=len.m_Pos;    //TODO: nem sok ertelme van hogy byteposban adjuk vissza??? megnézni.
    return iRet;
}
int fdms2::getPart(int iPrg, int iIdx, t1_toffset& riStart, fdms2pos& rpLen){
    if (!m_pDirMap) return -1;
    if (iPrg>=5) return -3;
    if (iIdx>=m_PartTableLength[iPrg]) return -2;
    fdms2part*p=m_PartTable[iPrg][iIdx];
    if (!p) return -4;
    p->getStartLength(riStart, rpLen);
    return 0;
}
int fdms2::getPartFromDisk(int iPrg, int iIdx, t1_toffset& riStart, fdms2pos& rpLen){
	if (!m_pDirMap) return -1;
    unsigned char* pC=(unsigned char*)m_pDirMap + FIRSTDIRBLOCK+BOFFSDIRBLOCK +
    iPrg*LENGTHDIRBLOCK + iIdx*8;
    unsigned long vDW=getDW(pC);
	unsigned long vDW2=getDW(pC+4);
    if (!vDW) return -1;
    if (vDW==0xFFFFFFFF) return -2;
    riStart= vDW;
	rpLen.setPos(vDW2);
    return 0;
}
int fdms2::getMetrum(int iPrg, int iIdx, int& riBar, int& riNumer, int& riDenom){
	if (!m_pDirMap) return -1;
    unsigned char* pC=(unsigned char*)m_pDirMap + FIRSTDIRBLOCK+DOFFSDIRBLOCK +
    iPrg*LENGTHDIRBLOCK + iIdx*4;
    unsigned long vDW=pC[3] | pC[2]<<8 | pC[1]<<16 | pC[0]<<24;
    if (!vDW) return -1;
    if (vDW==0xFFFFFFFF) return -2;
    riBar= get4BCD2i(pC);
    riDenom= pC[2];
    riNumer= pC[3];
    return 0;
}

void fdms2::setMetrum(int iPrg, int iIdx, int& riBar, int& riNumer, int& riDenom){
	if (!m_pDirMap) return ;
    unsigned char* pC=(unsigned char*)m_pDirMap + FIRSTDIRBLOCK+DOFFSDIRBLOCK +
    iPrg*LENGTHDIRBLOCK + iIdx*4;
    set4BCD(pC, riBar);
    pC[2]= riDenom;
    pC[3]= riNumer;
}
int fdms2::getTempo(int iPrg, int iIdx, int& riBar, int& riBeat, int& riTempo){
	if (!m_pDirMap) return -1;
    unsigned char* pC=(unsigned char*)m_pDirMap + FIRSTDIRBLOCK+EOFFSDIRBLOCK +
    iPrg*LENGTHDIRBLOCK + iIdx*8;
    unsigned long vDW=pC[3] | pC[2]<<8 | pC[1]<<16 | pC[0]<<24;
    if (!vDW) return -1;
    if (vDW==0xFFFFFFFF) return -2;
    riBar=   get4BCD2i(pC);
    riBeat=  pC[3];
    riTempo= get4BCD2i(&pC[4]);
    return 0;
}

void fdms2::setTempo(int iPrg, int iIdx, int& riBar, int& riBeat, int& riTempo){
	if (!m_pDirMap) return;
    unsigned char* pC=(unsigned char*)m_pDirMap + FIRSTDIRBLOCK+EOFFSDIRBLOCK +
    iPrg*LENGTHDIRBLOCK + iIdx*4;
    set4BCD(pC, riBar);
    pC[3]= riBeat;
    set4BCD(&pC[4], riTempo);
}
void fdms2::getMtcOffset(int iPrg, int& riH, int& riM, int& riS, int& riF, int& riSF){
	if (!m_pDirMap) return ;
    unsigned char* pC=(unsigned char*)m_pDirMap + FIRSTDIRBLOCK +
    iPrg*LENGTHDIRBLOCK+ 4;
    riH=  get2BCD2i(&pC[1]);
    riM=  get2BCD2i(&pC[0]);
    riS=  get2BCD2i(&pC[3]);
    riF=  get2BCD2i(&pC[4]);
    riSF=  get2BCD2i(&pC[5]);
}
void fdms2::setMtcOffset(int iPrg,  int& riH, int& riM, int& riS, int& riF, int& riSF){
	if (!m_pDirMap) return ;
    unsigned char* pC=(unsigned char*)m_pDirMap + FIRSTDIRBLOCK +
    iPrg*LENGTHDIRBLOCK + 4;
    set2BCD(&pC[1], riH);
    set2BCD(&pC[0], riM);
    set2BCD(&pC[2], 0);
    set2BCD(&pC[3], riS);
    set2BCD(&pC[4], riF);
    set2BCD(&pC[5], riSF);
    set2BCD(&pC[6], 0);
    set2BCD(&pC[7], 0);
}
void fdms2::getClick(int iPrg, bool& rbClick){
	if (!m_pDirMap) return ;
    unsigned char* pC=(unsigned char*)m_pDirMap + FIRSTDIRBLOCK +
    iPrg*LENGTHDIRBLOCK+ 16;
    rbClick=  (pC[0] & 0x08) !=0;
}
void fdms2::setClick(int iPrg, bool& rbClick){
	if (!m_pDirMap) return ;
    unsigned char* pC=(unsigned char*)m_pDirMap + FIRSTDIRBLOCK +
    iPrg*LENGTHDIRBLOCK+ 16;
    if (rbClick) pC[0]|= 0x08;
    else pC[0]&=0xFB;
}
int fdms2::startDirectory(){
 int iRet=0;
 m_lDirMap=0x020000;
 m_oDirMap=0;
#ifndef WIN32
 if (openFile(m_filename, false, m_fdDirectory)) return -1;
 iRet=mapFile(m_fdInput, false, m_pDirMap, m_oDirMap, m_lDirMap);
#else
 if (openFile(m_filename, false, m_fdDirectory, m_hDirMap, NULL)) return -1; //"mapdirfdms2"
 iRet=mapFile(m_hDirMap, false, m_pDirMap, m_oDirMap, m_lDirMap);
#endif
 if (iRet) return iRet;
 if (!m_pDirMap) return -1;
 unsigned char * pC=(unsigned char*)m_pDirMap +A1OFFSDIRBLOCK;
 unsigned int vDW=0;
 unsigned int vDW2=0;
 unsigned int iIndx=0;
 /*
 printf("A tipusu ismeretlen:\n");
 dumpABlock((unsigned char*)m_pDirMap +A1OFFSDIRBLOCK);
 printf("A tipusu ismeretlen:\n");
 dumpABlock((unsigned char*)m_pDirMap +A2OFFSDIRBLOCK);
 printf("A tipusu ismeretlen:\n");
 dumpABlock((unsigned char*)m_pDirMap +A3OFFSDIRBLOCK);
 printf("Deleted:\n");
 dumpBBlock((unsigned char*)m_pDirMap +BOFFSDIRBLOCK);
 */

 for (int iProg=0; iProg<5; iProg++){
   printf("Program%i:\n", iProg);
   int iH, iM, iS, iF, iSF;
   getMtcOffset(iProg, iH, iM, iS, iF, iSF);
   printf(" MTC offset: %02i:%02i:%02i(h:m:s)  %02if %02isf\n", iH, iM, iS, iF, iSF);
   bool bClick;
   getClick(iProg, bClick);
   printf(" Click: %i\n", bClick);
  //INDEX?
   pC=(unsigned char*)m_pDirMap + FIRSTDIRBLOCK+BOFFSDIRBLOCK + iProg*LENGTHDIRBLOCK;
//   dumpBBlock(pC);
   initPrgPartTable(iProg, pC);

  //METRUM
   vDW=0;
   iIndx=0;
   int iRet=0;
   int iBar, iN, iD;
   do{
    iRet=getMetrum(iProg, iIndx, iBar, iN, iD);
    if (!iRet) printf(" %i: %ibar Metrum: %d/%d\n", iIndx++, iBar, iN, iD);
   }while (!iRet);

 //TEMPO
   int iBeat, iTempo;
   iIndx=0;
   do{
    iRet=getTempo(iProg, iIndx, iBar, iBeat, iTempo);
    if (!iRet) printf(" %i: %ibar %ibeat tempo: %d\n", iIndx++, iBar, iBeat, iTempo);
   }while (!iRet);
              
 }//for iProg

 return iRet;    
}

void fdms2::stopDirectory(){
 if (m_pDirMap) {
     for (int iPrg=0; iPrg< FOSTEXMAXPROGRAMM; iPrg++){
         killPrgPartTable(iPrg);
     }
#ifndef WIN32
  munmap(m_pDirMap, m_lDirMap);
#else
  UnmapViewOfFile(m_pDirMap);
#endif
 }
}
