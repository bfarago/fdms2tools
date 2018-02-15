/*Writed by Barna Farago <brown@weblapja.com>
*/
#include "fdms2pos.h"
#include <string.h>
fdms2pos::fdms2pos(){
    setPos(0);
}

fdms2pos::fdms2pos(t1_toffset pos){
    setPos(pos);
}
void fdms2pos::setPos(t1_toffset pos){
    m_Pos=pos;
    m_Sample= pos /2 /FOSTEXMAXCHANNELS;
    m_Hour=(int)(m_Sample/3600/FOSTEXSAMPLERATE);
    m_Min=(int)(m_Sample/60/FOSTEXSAMPLERATE - m_Hour*60);
    m_Sec=(int)(m_Sample/FOSTEXSAMPLERATE- m_Hour*3600 -m_Min*60);
    m_Frame=(int)((m_Sample/1764) % FOSTEXFRAMERATE);
}
void fdms2pos::setSample(t1_toffset sample){
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
void fdms2pos::dumpTimeStr(char*& rpcStr){
	char* s=(char*)malloc(255);
    sprintf(s,"%02ih %02im %02is %02if", m_Hour, m_Min, m_Sec, m_Frame);
	if (rpcStr) free(rpcStr);
    rpcStr=strdup(s);
    free(s);
}
void fdms2pos::dumpTimeStrHMS(char*& rpcStr){
	char* s=(char*)malloc(20);  //TODO: atirni stackesre.
	sprintf(s,"%02i:%02i:%02i", m_Hour, m_Min, m_Sec);
	if (rpcStr) free(rpcStr);
    rpcStr=strdup(s);
    free(s);
}
void fdms2pos::dumpTimeStrHMSF(char*& rpcStr){
	char* s=(char*)malloc(20);
	sprintf(s,"%02i:%02i:%02i.%02i", m_Hour, m_Min, m_Sec, m_Frame);
	if (rpcStr) free(rpcStr);
    rpcStr=strdup(s);
    free(s);
}
void fdms2pos::dumpByte(){
    if (m_Pos<1024){
        printf("%i", m_Pos);
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
    char* s=(char*)malloc(255);
    if (m_Pos<1024){
        sprintf(s,"%i", m_Pos);
    }else{
        if (m_Pos<1024*1024){
            sprintf(s,"%.3fk", (float)m_Pos/(1024));
        }else{
            if(m_Pos<1024*1024*1024){
                sprintf(s,"%.3fM", (float)m_Pos/(1024*1024));
            }else{
                sprintf(s,"%.3fG", (float)m_Pos/(1024*1024*1024));
            }
        }
    }
    if (rpcStr) free(rpcStr);
    rpcStr=strdup(s);
    free(s);
}