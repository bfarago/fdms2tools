/*Written by Barna Farago <brown@weblapja.com> 2006-2018
*/
#ifndef _FDMS2_H_
#define _FDMS2_H_

#include "fdms2common.h"
#include "fdms2pos.h"
#include "fdms2part.h"
#include "fdms2error.h"
#include "fdms2disk.h"

enum enPartitionMode{
    NormalPartitionMode=0,
    AddUntilTheEnd,
    RawFormat
};
class fdms2streamerIF: public fdms2errorIF{
public:
    virtual void reset()=0;
    virtual void setWriteable(bool bWrite)=0;
    virtual int start()=0;
    virtual int stop()=0;
    virtual fdms2streamerIF* duplicate()=0;
    virtual int getValueArrays(t1_toffset pos, int samplenum, short** ptrArray)=0;
    virtual int convertLogical2Abs(int iPrg, fdms2pos posLogic, t1_toffset& iOffs, t1_toffset& iStart, t1_toffset& iMaxLen, int& iLastPart)=0;
    virtual int getPart(int iPrg, int iIdx, t1_toffset& riStart, fdms2pos& rpLen)=0;
};

class fdms2:public fdms2streamerIF
{
public:
 fdms2()noexcept;
 virtual ~fdms2();
 void setFileName(const char* s);
 const char* getFileName()const;
 virtual void setWriteable(bool bWrite){
     m_bWriteable=bWrite;
 }
 t1_toffset  m_startpos;
 t1_toffset  m_length;
 t1_toffset  m_step; //only for dumping feature
 void dump();
 void logError(const char* err);
 short getValue(t1_toffset pos, int channel, int samplenum);
 int getValues(t1_toffset pos, int channelmap, int samplenum, short* ptrArray, int lenArray);
 t1_toffset getDiskAudioSize();
 t1_toffset getDiskSize();
 t1_toffset getProgramSampleCount(const int iPrg)const noexcept ;
 t1_toffset getLongestProgramSampleCount()const noexcept;
 EDiskType getDiskType();
 void SigBusOccured(int sig) noexcept;
 bool wasBadSector()const noexcept { return m_badsector; }
private:
 long g_pagesize;
 bool m_badsector;
 bool m_eof;
 int getValuesPrg(int iPrg, t1_toffset pos, int channelmap, int samplenum, short* ptrArray, int lenArray, int& riLastPart);
 void* m_badBlock;
 bool m_bWriteable;
 char* m_filename;
 t1_toffset m_endpos;
 t1_toffset m_oMap;
 t1_toffset m_lMap;
 t1_toffset m_required;
 void* m_pMap;
 bool m_bMap;

 //void* m_pDirectMap;
 
 t1_toffset m_oDirMap;
 t1_toffset m_lDirMap;
 void* m_pDirMap;
public:
 void setPartitionMode(enPartitionMode pm);
 enPartitionMode getPartitionMode(){ return m_partitionMode; }
 fdms2parttable m_partTable[FOSTEXMAXPROGRAMM];

 t1_toffset m_ProgramSampleCount[FOSTEXMAXPROGRAMM];
private:
 enPartitionMode m_partitionMode;
#ifdef WIN32
 SYSTEM_INFO m_SysInfo;
 unsigned int getpagesize();
#else
 int m_flagsInput;
 int m_fdInput;
 int m_fdDirectory;
 int openFile(const char* filename, bool bWrite, int &rFD);
 int mapFile(int &rhMap, bool bWrite, void* &rpMap,t1_toffset oMap,t1_toffset lMap);
#endif

 t1_toffset pagealign(t1_toffset v) noexcept;
 void dumphex(t1_toffset pos, long len);
 void dumpfostex(t1_toffset pos);

 /** map. Maps the file to memory mapped pages.
 */
 int map(t1_toffset pos);
 
 /**unmap. Frees the mapped pages.
 */
 int unmap();

 /**
 */
 void* mapPtr(t1_toffset pos, t1_toffset max);
 /**
 */
 int startDirectory();
 void stopDirectory();
 int dumpABlock(unsigned char* pC);
 int dumpBBlock(unsigned char* pC);
 int initPrgPartTable(const int iPrg, unsigned char* pC);
 void killPrgPartTable(const int iPrg);
public:
 virtual void reset();
 virtual int start();
 virtual int stop();
 void quickFormat();
 virtual fdms2streamerIF* duplicate();
 virtual void copy(fdms2streamerIF* p);
 virtual int getValueArrays(t1_toffset pos, int samplenum, short** ptrArray);
 virtual int convertLogical2Abs(const int iPrg, fdms2pos posLogic, t1_toffset& iOffs, t1_toffset& iStart, t1_toffset& iMaxLen, int& iLastPart);
 virtual int getPart(const int iPrg, int iIdx, t1_toffset& riStart, fdms2pos& rpLen);

public:
 int getPart(int iPrg, int iIdx, t1_toffset& riStart, t1_toffset& riLen);
 /**
 */
 int  getMetrum(int iPrg, int iIdx, int& riBar, int& riNumer, int& riDenom);
 void setMetrum(int iPrg, int iIdx, int& riBar, int& riNumer, int& riDenom);
 int  getTempo(int iPrg, int iIdx, int& riBar, int& riBeat, int& riTempo);
 void setTempo(int iPrg, int iIdx, int& riBar, int& riBeat, int& riTempo);
 void getMtcOffset(int iPrg, int& riH, int& riM, int& riS, int& riF, int& riSF);
 void setMtcOffset(int iPrg, int& riH, int& riM, int& riS, int& riF, int& riSF);
 void getClick(int iPrg, bool& rbClick);
 void setClick(int iPrg, bool& rbClick);

fdms2disk* getDisk(){return &m_fdms2disk;}
private:
 fdms2disk m_fdms2disk;
 fdms2diskPtrIF* m_ptr;
 fdms2diskPtrIF* m_ptrDir;
 int  get2BCD2i(unsigned char* p);
 void set2BCD(unsigned char* p, int v);

 int  get4BCD2i(unsigned char* p);
 void set4BCD(unsigned char* p, int v);
 unsigned int getDW(unsigned char* p);
 void setDW(unsigned char* p, unsigned int v);
 int getPartFromDisk(int iPrg, int iIdx, t1_toffset& riStart, fdms2pos& rpLen);
 int setPartOnDisk(int iPrg, int iIdx, t1_toffset& riStart, fdms2pos& rpLen);
};

#endif
