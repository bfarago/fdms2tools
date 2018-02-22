/* Written by Barna Farago <brown@weblapja.com> 2006-2018
*/
#include "StdAfx.h"
#include "LineStrip.h"
#include "math.h"

CLineStrip::CLineStrip(void):m_iFader(16), m_iPan(0), m_bMute(false), m_bSolo(false)
{
}

CLineStrip::~CLineStrip(void)
{
}
long CLineStrip::getAmp(long v){
    long ret=v;
    double s=pow(1.04,double(16-m_iFader )) ;
    ret=v*s;
    return ret;
}
void CLineStrip::getAmpStereo(long& l, long& r){
    double s=pow(1.04,double(16-m_iFader )) ;
    l=l*s;
    r=r*s;
}
void CLineStrip::Serialize(CArchive &ar){
    if (ar.IsStoring())
	{
        ar<< m_iFader;
        ar<< m_iPan;
        ar<< m_bMute;
        ar<< m_bSolo;
    }else{
        ar>> m_iFader;
        ar>> m_iPan;
        ar>> m_bMute;
        ar>> m_bSolo;
    }
}