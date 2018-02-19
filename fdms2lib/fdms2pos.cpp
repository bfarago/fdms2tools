/*Writed by Barna Farago <brown@weblapja.com>
*/
#include "fdms2pos.h"
#include <string.h>
#define strdup _strdup
fdms2pos::fdms2pos(){
    setPos(0);
}

fdms2pos::fdms2pos(t1_toffset pos){
    setPos(pos);
}
void fdms2pos::setPos(t1_toffset pos){
    if (pos<0) pos=0;
    m_Pos=pos;
    m_Sample= pos /2 /FOSTEXMAXCHANNELS;
    m_Hour=(int)(m_Sample/3600/FOSTEXSAMPLERATE);
    m_Min=(int)(m_Sample/60/FOSTEXSAMPLERATE - m_Hour*60);
    m_Sec=(int)(m_Sample/FOSTEXSAMPLERATE- m_Hour*3600 -m_Min*60);
    m_Frame=(int)((m_Sample/1764) % FOSTEXFRAMERATE);
}
void fdms2pos::setSample(t1_toffset sample){
    if (sample<0) sample=0;
    m_Pos=sample*2*FOSTEXMAXCHANNELS;
    m_Sample= sample;
    m_Hour=(int)(m_Sample/3600/FOSTEXSAMPLERATE);
    m_Min=(int)(m_Sample/60/FOSTEXSAMPLERATE - m_Hour*60);
    m_Sec=(int)(m_Sample/FOSTEXSAMPLERATE- m_Hour*3600 -m_Min*60);
    m_Frame=(int)((m_Sample/1764) % FOSTEXFRAMERATE);
}
void fdms2pos::setTime(int h, int m, int s, int f, int sf){
    t1_toffset sample;
    sample=3600*FOSTEXSAMPLERATE*h;
    sample+=60*FOSTEXSAMPLERATE*m;
    sample+=FOSTEXSAMPLERATE*s;
    sample+=1764*f;
    sample+=sf;
    setSample(sample);
}
t1_toffset fdms2pos::addPos(t1_toffset pos){
	setPos(m_Pos+pos);
	return m_Pos;
}
fdms2pos fdms2pos::operator -(const fdms2pos &pos2) const{
    return m_Pos - pos2.m_Pos;
}
void fdms2pos::addSample(t1_toffset sample){
    addPos(sample *2*FOSTEXMAXCHANNELS);
}
void fdms2pos::operator +=(const fdms2pos& pos){
	addPos(pos);
}
bool fdms2pos::operator ==(const fdms2pos& pos)const{
    return m_Pos == pos.m_Pos;
}
bool fdms2pos::operator <(const fdms2pos& pos)const{
    return m_Pos < pos.m_Pos;
}	
t1_toffset fdms2pos::addPos(fdms2pos pos){
	setPos(m_Pos + pos.m_Pos);
	return m_Pos;
}
void fdms2pos::dump(){
    printf("%02ih %02im %02is %02if", m_Hour, m_Min, m_Sec, m_Frame);
}
#define TXTSIZE (255)
void fdms2pos::dumpTimeStr(char*& rpcStr){
	char* s=(char*)malloc(TXTSIZE);
    snprintf(s, TXTSIZE, "%02ih %02im %02is %02if", m_Hour, m_Min, m_Sec, m_Frame);
	if (rpcStr) free(rpcStr);
    rpcStr=strdup(s);
    free(s);
}
#define HMSSIZE (20)
void fdms2pos::dumpTimeStrHMS(char*& rpcStr){
	char* s=(char*)malloc(HMSSIZE);  //TODO: atirni stackesre.
	snprintf(s, HMSSIZE, "%02i:%02i:%02i", m_Hour, m_Min, m_Sec);
	if (rpcStr) free(rpcStr);
    rpcStr=strdup(s);
    free(s);
}
void fdms2pos::dumpTimeStrHMSF(char*& rpcStr){
	char* s=(char*)malloc(HMSSIZE);
	snprintf(s, HMSSIZE, "%02i:%02i:%02i.%02i", m_Hour, m_Min, m_Sec, m_Frame);
	if (rpcStr) free(rpcStr);
    rpcStr=strdup(s);
    free(s);
}
void fdms2pos::dumpTimeStrSF(char*& rpcStr){
	char* s=(char*)malloc(MAXTIMESTR);
	sprintf_s(s,MAXTIMESTR,"%02i.%02i", m_Sec, m_Frame);
	if (rpcStr) free(rpcStr);
    rpcStr=fdms2_strdup(s);
    free(s);
}
void fdms2pos::dumpByte(){
    if (m_Pos<1024){
        printf("%lli", m_Pos);
    }else{
        if (m_Pos<1024*1024){
            printf("%.3fk", (float)m_Pos/(1024));
        }else{
            if(m_Pos<1024*1024*1024){
                printf("%.3fM", (float)m_Pos/(1024*1024));
            }else{
                printf("%.3fG", (float)m_Pos/(1024*1024*1024));
            }
        }
    }
}
void fdms2pos::dumpByteStr(char*& rpcStr){
    char* s=(char*)malloc(TXTSIZE);
    if (m_Pos<1024){
        snprintf(s, TXTSIZE, "%lli", m_Pos);
    }else{
        if (m_Pos<1024*1024){
            snprintf(s, TXTSIZE, "%.3fk", (float)m_Pos/(1024));
        }else{
            if(m_Pos<1024*1024*1024){
                snprintf(s, TXTSIZE, "%.3fM", (float)m_Pos/(1024*1024));
            }else{
                snprintf(s, TXTSIZE, "%.3fG", (float)m_Pos/(1024*1024*1024));
            }
        }
    }
    if (rpcStr) free(rpcStr);
    rpcStr=strdup(s);
    free(s);
}
