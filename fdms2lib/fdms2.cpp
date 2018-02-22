/*Written by Barna Farago <brown@weblapja.com> 2006-2018
*/
#include "fdms2.h"
#include "fdms2disk.h"
//#define MYPAGESIZE (2*8*2048)
#define MYPAGESIZE  (MAPPAGELENGTH>>5)
//TODO: Page poolt kéne csinálni mert igy nem optimalis.

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

#ifndef ASSERT
#define ASSERT(x) if(!x) return;
#define ASSERT1(x,y) if(!x) return y;
#endif

#define ROUND_UP_SIZE(Value,Pow2) \
   ((SIZE_T) ((((ULONG)(Value)) + (Pow2) - 1) \
   & (~(((LONG)(Pow2)) - 1))))

#define ROUND_UP_PTR(Ptr,Pow2) \
   ((void *) ((((ULONG_PTR)(Ptr)) + (Pow2) - 1) \
   & (~(((LONG_PTR)(Pow2)) - 1))))


/**Default constructor.
*/
fdms2::fdms2()
:m_partitionMode(NormalPartitionMode),m_badBlock(NULL),m_pMap(NULL),m_filename(NULL),m_startpos(0),
m_length(0), m_badsector(true),m_eof(false),m_pDirMap(NULL),m_oDirMap(0),m_bWriteable(false),
m_oMap(0), m_lMap(0), m_lDirMap(0), m_required(0), 
g_pagesize(DEFAULT_PAGESIZE), m_endpos(0),m_bMap(false), m_step(0)
// m_pDirectMap(NULL),
{
 m_ptr=NULL;
 m_ptrDir=NULL;
#ifdef WIN32
#else
 m_flagsInput=0;
 m_fdInput=0;
 m_fdDirectory=0;
#endif
 for(int i=0; i<FOSTEXMAXPROGRAMM; i++){
     m_partTable[i].kill();
	 m_ProgramSampleCount[i]=0;
 }
}
fdms2::~fdms2(){
    m_fdms2disk.stop();
}
void fdms2::setFileName(const char* s){
    m_fdms2disk.setFileName(s);
 }
const char* fdms2::getFileName()const{
    return m_fdms2disk.getFileName();
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
    m_bWriteable=p->m_bWriteable;
    setFileName(p->getFileName());
    m_eof=false;
    m_badsector=false;
    m_partitionMode=p->m_partitionMode;
    setNotify(p->getNotify());
}
/**Error logger.
*/
void fdms2::logError(char* err){
    messageBox("Err", err);
    //printf("err: fdms2lib : %s \n", err); 
}
t1_toffset fdms2::pagealign(t1_toffset v){
	t1_toffset t;
	if (g_pagesize)t=g_pagesize * (t1_toffset)(( v / g_pagesize));
	else t = v;
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
  pch=(unsigned char*)mapPtr(rowPos, m_required);
  if( (pch != NULL) && ((int)pch != -1)){
    unsigned int indxH=4*(8*(samplenum/2) + channel);
    if (indxH >= m_lMap){
       // DLOG("overread");
        return 0;
    }
    pch+= indxH;
    unsigned int indx=2*(samplenum % 2);
    v= (unsigned short)((unsigned char)(pch[1+indx]) <<8) | (unsigned char)(pch[ indx]);
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
   m_required= (samplenum+1)*2*8; //byte
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
  if (!ptrArray) return 0;
  int iCount=0;
  //GET data
  reset();
  m_required= (samplenum+1)*2*8; //byte
  for (int i=0; i< samplenum; i++){
        if (!m_badsector) reset();
        for(int ch=0; ch < FOSTEXMAXCHANNELS; ch++){
           short s=0;
	       short *pt=ptrArray[ch];
           if (!m_badsector) s=getValue(pos, ch, i);
           pt[i] = s;
        }
        if (m_eof) return i;
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
   if (m_ptr) m_ptr->stopPtr(m_pMap);
   return 0;
}

int fdms2::map(t1_toffset offset){
  int iRet=0;
  m_bMap=true;
  printf("map(%lli)\n",offset);
  unmap();
  m_lMap= MAPPAGELENGTH;
  m_badsector=false;
  m_eof=false;
  m_oMap=pagealign(offset);
  t1_toffset offs=m_oMap+FIRSTDATABLOCK;
//  printf("offs: %lli  m_length: %lli m_lMap: %lli\n",offs, m_length, m_lMap);

  if ((m_lMap+offs) >= (m_length))
      m_lMap= m_length-offs;
  if (m_lMap<=1){
    m_lMap=MAPPAGELENGTH;	//hack
  }
  if (m_lMap>MAXPAGESIZE){
    m_lMap=MAXPAGESIZE;	//hack
  }
  if (m_lMap>MAPPAGELENGTH){
    m_lMap=MAPPAGELENGTH;	//hack
  }
  m_lMap=pagealign(m_lMap);
  printf("m_lMap: %lli\n", m_lMap);

  if (m_lMap<=1) m_lMap=g_pagesize;
#ifndef WIN32
  printf("call mapFile(%0x, false, %0x, %lli, %lli)\n", m_fdInput, m_pMap, offs, m_lMap);
  iRet=mapFile(m_fdInput, false, m_pMap, offs, m_lMap);
#else
  iRet=(int)m_ptr->startPtr(m_pMap, offs, m_lMap, m_lMap, RWReadWrite);
  //iRet=mapFile(m_hMap, false, m_pMap, offs, m_lMap);
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
   if ((pos+max)>=m_length){
       DLOG("eof");
       return 0;
   }
   if (iRet){
       return NULL;
       //DLOG("map() returned by: %i\n", iRet);
   }
   if (m_eof) 
       return m_badBlock;
   if (pos-m_oMap<0) {
     DLOG("negative offset ?");
   }
   if (iRet) return NULL;
   if (m_pMap && !m_badsector){
       if ((pos-m_oMap+max) >= m_lMap){
           m_eof=true;
           return 0;
       }
    p=(char*)m_pMap + (pos-m_oMap);
    static t1_toffset posprev=0;
   // if (posprev!=pos) DLOG("mapPtr(%lli, %lli, %lli, %lli)\n", pos,m_oMap, max, m_lMap);
    posprev=pos;
   }else{
    p=m_badBlock;
    DLOG("mapPtr(%lli, %lli)\n", pos,max);
   }
  }catch(...){
    DLOG("catch\n");
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

#endif

int fdms2::start(){
    m_pMap=0; m_lMap=0;
    m_badBlock=malloc(MYPAGESIZE);
    if (m_fdms2disk.start() != ErrNone) return -1;
    m_ptr = m_fdms2disk.getNewPtr();
    m_ptrDir=m_fdms2disk.getNewPtr();
    t1_toffset mdisksize = getDiskSize();
    if (mdisksize){
        m_length=mdisksize;
    }
    g_pagesize = getpagesize();
    m_endpos=m_startpos+m_length;
    //printf("log: tester1.start: %s\n %016LXh - %016LXh (%016LXh)\n", m_filename, m_startpos, m_endpos , m_length);

    m_length= pagealign(m_length);
    m_endpos=m_startpos+ m_length;
    //printf("log: after align : %s\n %016LXh - %016LXh (%016LXh)\n", m_filename, m_startpos, m_endpos , m_length);
    m_badsector=false;
    m_eof=false;
    
    map(0);
    startDirectory();
    map(0);
    return 0;
}

int fdms2::stop(){
 stopDirectory();
 if (m_badBlock){
    free(m_badBlock);
    m_badBlock=0;
 }
 if (m_pMap) if (m_ptr) m_ptr->stopPtr(m_pMap);
 m_fdms2disk.stop();
 m_pMap=0;
 return 0;
}

void fdms2::SigBusOccured(int sig){
 m_badsector=true;
}

/** debug dump
*/
void fdms2::dump(){
 if (m_step < 1) m_step = 1;
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
	return getDiskSize()-FIRSTDATABLOCK; //TODO: check if disk size is the image file size or the audio block size
}

/** Returns By size of the specified Disk or file.
@return 64bit wide size.
*/
t1_toffset fdms2::getDiskSize(){
    t1_toffset s= m_fdms2disk.getDiskSize();
    return s;
}

EDiskType fdms2::getDiskType(){
    return m_fdms2disk.getDiskType();
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
    m_partTable[iPrg].kill();
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

 m_partTable[iPrg].init(iIdx);
 
 iIdx=0;
 if ((m_partitionMode==NormalPartitionMode) || (m_partitionMode == AddUntilTheEnd)){
     while(!getPartFromDisk(iPrg, iIdx, Start, Len)){
	     m_partTable[iPrg][iIdx].setData(Start,Len, Logical);
	     iIdx++;
         sampleCount+=Len.m_Sample;
         m_partTable[iPrg].m_length=iIdx;
     }
     if (m_partitionMode == AddUntilTheEnd){
         fdms2pos diskSize;
         diskSize.setPos(getDiskAudioSize());

         if (sampleCount < diskSize.m_Sample){
             Start+=Len.m_Pos;
             Len.setSample(diskSize.m_Sample-sampleCount);
             m_partTable[iPrg][iIdx].setData(Start,Len, Logical);
	         iIdx++;
             sampleCount+=Len.m_Sample;
             m_partTable[iPrg].m_length++;
         }
     }
 }else{ //RawFormat
    Start=0;
    sampleCount=getDiskAudioSize()/16;
    Len.setSample(sampleCount);
    Logical.setPos(0);
    m_partTable[iPrg][0].setData(Start, Len, Logical);
    m_partTable[iPrg].m_length=1;
 }
 m_ProgramSampleCount[iPrg]=sampleCount;
 return iIdx;
}

void fdms2::setPartitionMode(enPartitionMode pm){
    m_partitionMode=pm;
    if (m_filename){
        stopDirectory();
        startDirectory();
    }
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
  return p[2] | p[3]<<8 | p[1]<<24 | p[0]<<16;
 //return p[3] | p[2]<<8 | p[1]<<16 | p[0]<<24;
}

void fdms2::setDW(unsigned char* p, unsigned int v){
  p[2]= v & 0xff;
  p[3]= (v >>8) & 0xff;
  p[0]= (v >>16) & 0xff;
  p[1]= (v >>24) & 0xff;
  /*p[3]= v & 0xff;
  p[2]= (v >>8) & 0xff;
  p[1]= (v >>16) & 0xff;
  p[0]= (v >>24) & 0xff;*/
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
    if (iIdx >= m_partTable[iPrg].m_length) return -2;
    fdms2part* p=&m_partTable[iPrg][iIdx];
    if (!p) return -4;
    p->getStartLength(riStart, rpLen);
    return 0;
}

int fdms2::getPartFromDisk(int iPrg, int iIdx, t1_toffset& riStart, fdms2pos& rpLen){
	if (!m_pDirMap) return -1;
    unsigned char* pC=(unsigned char*)m_pDirMap + FIRSTDIRBLOCK+BOFFSDIRBLOCK +
    iPrg*LENGTHDIRBLOCK + iIdx*8 + 8;
    unsigned long vDW=getDW(pC);
	unsigned long vDW2=getDW(pC+4);
    if (!vDW) return -1;
    if (vDW==0xFFFFFFFF) return -2;
    riStart= vDW*512; //sample vs word
	rpLen.setPos(vDW2*512);
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
    iPrg*LENGTHDIRBLOCK + iIdx*8;
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

int fdms2::setPartOnDisk(int iPrg, int iIdx, t1_toffset& riStart, fdms2pos& rpLen){
	if (!m_pDirMap) return -1;
    unsigned char* pC=(unsigned char*)m_pDirMap + FIRSTDIRBLOCK+BOFFSDIRBLOCK +
    iPrg*LENGTHDIRBLOCK + iIdx*8 + 8;

    unsigned long vDW= riStart/512;
    unsigned long vDW2=rpLen.m_Pos/512;
    setDW(pC, vDW);
    setDW(pC+4,vDW2);
    return 0;
}

void fdms2::quickFormat(){
    stop();
    start();
    for (int iProg=0; iProg<5; iProg++){
        int iH, iM, iS, iF, iSF;
        iH=0; iM=59; iS=57; iSF=0;
        setMtcOffset(iProg, iH, iM, iS, iF, iSF);
        bool bClick=false;
        setClick(iProg, bClick);

       // pC=(unsigned char*)m_pDirMap + FIRSTDIRBLOCK+BOFFSDIRBLOCK + iProg*LENGTHDIRBLOCK;
        t1_toffset iStart=0;
        fdms2pos len=0;
        setPartOnDisk(iProg, 0,iStart, len);
        

        //vDW=0;
        //iIndx=0;
        int iRet=0;
        int iBar, iN, iD;
        iBar=1; iN=1; iD=4;
        setMetrum(iProg, 0, iBar, iN, iD);
        iBar=0; iN=0; iD=0;
        setMetrum(iProg, 1, iBar, iN, iD);

        //TEMPO
        int iBeat, iTempo;
        iBeat=1;
        iTempo=120;
        iBar=1;
        getTempo(iProg, 0, iBar, iBeat, iTempo);
        iBar=0; iBeat=0; iTempo=0;
        getTempo(iProg, 1, iBar, iBeat, iTempo);
              
    }//for iProg
	ASSERT(m_ptrDir);
    m_ptrDir->writeBack();
    stop();
    start();
}

int fdms2::startDirectory(){
 int iRet=0;
 m_lDirMap=0x020000;
 m_oDirMap=0;
 ASSERT1(m_ptrDir, -1);
 iRet= (int) m_ptrDir->startPtr(m_pDirMap, m_oDirMap, m_lDirMap, m_lDirMap, RWReadWrite);
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
//   printf("Program%i:\n", iProg);
   int iH, iM, iS, iF, iSF;
   getMtcOffset(iProg, iH, iM, iS, iF, iSF);
//   printf(" MTC offset: %02i:%02i:%02i(h:m:s)  %02if %02isf\n", iH, iM, iS, iF, iSF);
   bool bClick;
   getClick(iProg, bClick);
//   printf(" Click: %i\n", bClick);
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
  //  if (!iRet) printf(" %i: %ibar Metrum: %d/%d\n", iIndx++, iBar, iN, iD);
    iIndx++;
   }while (!iRet);

 //TEMPO
   int iBeat, iTempo;
   iIndx=0;
   do{
    iRet=getTempo(iProg, iIndx, iBar, iBeat, iTempo);
    //if (!iRet) printf(" %i: %ibar %ibeat tempo: %d\n", iIndx++, iBar, iBeat, iTempo);
    iIndx++;
   }while (!iRet);
              
 }//for iProg

 return iRet;    
}

void fdms2::stopDirectory(){
 if (m_pDirMap) {
     for (int iPrg=0; iPrg< FOSTEXMAXPROGRAMM; iPrg++){
         killPrgPartTable(iPrg);
     }
	 if (m_ptrDir) m_ptrDir->stopPtr(m_pDirMap); //TODO: I got an exception here  in destruction phase. (referenced obj missing?)
 }
}

