/* Written by Barna Farago <brown@weblapja.com> 2006-2018
*/
#pragma once

class CLineStrip
{
public:
    int m_iFader;
    int m_iPan;
    bool m_bMute;
    bool m_bSolo;
    CLineStrip(void);
    ~CLineStrip(void);
    long getAmp(long v);
    void getAmpStereo(long& l, long& r);
    void Serialize(CArchive &ar);
};

