/* Written by Barna Farago <brown@weblapja.com> 2006-2018
*/
#include "StdAfx.h"
#include "Mixer.h"

CMixer::CMixer(void)
{
}

CMixer::~CMixer(void)
{
}

long CMixer::getAmpChannel(long v, int ch){
    return m_Lines[ch].getAmp(v);
}
void CMixer::setFaderChannel(int f, int ch){
    m_Lines[ch].m_iFader=f;
}
int CMixer::getFaderChannel(int ch){
    return m_Lines[ch].m_iFader;
}
void CMixer::getAmpMaster(long& l, long& r){
    m_Master.getAmpStereo(l,r);
}
void CMixer::setFaderMaster(int f){
    m_Master.m_iFader=f;
}
int CMixer::getFaderMaster(){
    return m_Master.m_iFader;
}
void CMixer::Serialize(CArchive &ar){
    for (int i=0; i< FOSTEXMAXCHANNELS; i++){
        m_Lines[i].Serialize(ar);
    }

}