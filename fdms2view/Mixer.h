#pragma once
#include "linestrip.h"
#include <fdms2lib.h>

class CMixer
{
public:
    CMixer(void);
    ~CMixer(void);
    long getAmpChannel(long v, int ch);
    void setFaderChannel(int f, int ch);
    int getFaderChannel(int ch);
    void getAmpMaster(long& l, long& r);
    void setFaderMaster(int f);
    int getFaderMaster();
    void Serialize(CArchive &ar);
private:
    CLineStrip m_Lines[FOSTEXMAXCHANNELS];
    CLineStrip m_Master;
};
