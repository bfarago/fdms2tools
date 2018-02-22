/*Written by Barna Farago <brown@weblapja.com> 2006-2018
*/
#ifndef _FDMS2POS_H
#define _FDMS2POS_H

#include "fdms2common.h"
#define MAXTIMEDUMP (255)
#define MAXTIMESTR (20)
class fdms2pos{
public:
 fdms2pos();
 fdms2pos(t1_toffset pos);
 void setPos(t1_toffset pos);
 void setSample(t1_toffset sample);
 void setTime(int h, int m, int s, int f, int sf);
 t1_toffset m_Pos;
 t1_toffset m_Sample;
 int m_Hour;
 int m_Min;
 int m_Sec;
 int m_Frame;
 void dump();
 void dumpByte();
 void dumpByteStr(char*& rpcStr);
 void dumpTimeStr(char*& rpcStr);
 void dumpTimeStrHMS(char*& rpcStr);
 void dumpTimeStrHMSF(char*& rpcStr);
 void dumpTimeStrSF(char*& rpcStr);
 t1_toffset addPos(t1_toffset pos);
 t1_toffset addPos(fdms2pos pos);
 void addSample(t1_toffset sample);
 void operator +=(const fdms2pos& pos);
 bool operator <(const fdms2pos& pos)const;
 bool operator ==(const fdms2pos& pos)const;
 fdms2pos operator -(const fdms2pos &pos2) const;
};

#endif
