/* Written by Barna Farago <brown@weblapja.com> 2006-2018
*/
#include "StdAfx.h"
#include "LineStrip.h"
#include "math.h"

CLineStrip::CLineStrip(void):m_iFader(16), m_iPan(0), m_bMute(false), m_bSolo(false)
{
	setFader(m_iFader); //calc scale
}

CLineStrip::~CLineStrip(void)
{
}
void CLineStrip::setFader(int f) {
	m_iFader = f;
	m_s = pow(1.04, double(16 - m_iFader));
}
long CLineStrip::getAmp(long v){
    long ret=v;
    ret=v*m_s;
    return ret;
}
void CLineStrip::getAmpStereo(long& l, long& r){
    l=l* m_s;
    r=r* m_s;
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
		setFader(m_iFader); //calc scale
    }
}