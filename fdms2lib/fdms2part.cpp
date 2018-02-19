/*Writed by Barna Farago <brown@weblapja.com>
*/
#include "fdms2part.h"

fdms2part::fdms2part(){
	fdms2pos logical;
    setData(0,0, logical);
}

fdms2part::fdms2part(t1_toffset PhisicalPos, fdms2pos len, fdms2pos& LogicalPos){
    setData(PhisicalPos, len, LogicalPos);
}

void fdms2part::setData(t1_toffset PhisicalPos, fdms2pos len, fdms2pos& LogicalPos){
    m_PhisicalPos=PhisicalPos;
    m_Length=len;
	m_LogicalPos=LogicalPos;
	LogicalPos+=m_Length;
}
void fdms2part::getStartLength(t1_toffset &PhisicalPos, fdms2pos &len){
    PhisicalPos= m_PhisicalPos;
    len=m_Length;
}
#define TXTBUFSIZE (255)
void fdms2part::dumpStr(char*& rpcStr){
	char* s=(char*)malloc(TXTBUFSIZE);
	char* sLP=NULL;
	char* sSP=NULL;
	char* sLN=NULL;
	char* sLS=NULL;
	m_LogicalPos.dumpTimeStr(sLP);
	m_Length.dumpTimeStr(sLN);
	m_Length.dumpByteStr(sLS);
	snprintf(s, TXTBUFSIZE, "%s +%s (%sbyte)", sLP, sLN, sLS);
	if (rpcStr) free(rpcStr);
    rpcStr=_strdup(s);
    free(s);
}

fdms2parttable::fdms2parttable():
m_length(0), m_parts(NULL)
{
}
void fdms2parttable::init(int n){
 m_parts=(fdms2part*)malloc(sizeof(fdms2part)*(n+1)); // +1 a könyveletlen particio miatt.
 m_length=0; //meg nulla
}
void fdms2parttable::kill(){
 //for (int i=0; i<m_length; i++){
 //        delete( m_parts[i]);
 //}
 m_length=0;
 if (m_parts) free(m_parts);
 m_parts=0;
}
