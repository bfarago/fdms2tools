/*Written by Barna Farago <brown@weblapja.com> 2006-2018
*/
#ifndef _FDMS2DISPDATA_H
#define _FDMS2DISPDATA_H

#include "fdms2common.h"
#include "fdms2pos.h"

class fdms2dispdata{
public:
 fdms2dispdata();
 fdms2pos m_Start;
 fdms2pos m_Length;
 int m_XResolution;
 unsigned char** m_Buff;
 bool m_Ready;
 void dumpConsole();

private:
 void dumpfostex(t1_toffset pos);
};

#endif
