/*Writed by Barna Farago <brown@weblapja.com>
*/
#include "fdms2dispdata.h"

fdms2dispdata::fdms2dispdata(){
}
/**
*/
void fdms2dispdata::dumpConsole(){
  /*
	t1_toffset d=m_XResolution; //division
	t1_toffset pos=m_Start;
	for (t1_toffset p= 0; p< d; p++){
		pos=m_Start+ p*m_length/d;
		printf("%016LXh ",pos+FIRSTDATABLOCK);
		dumpfostex(pos);
		printf(" ");
		fdms2pos xpos(pos);
		xpos.dump();
		printf(" \t");
		xpos.dumpByte();
		printf(" %Li", xpos.m_Sample);
	#ifdef VERBOSE_MAPPING
		if (m_bMap) printf(" MAP(%016LXh)", m_oMap);
	#endif
		m_bMap=false;
		printf("\n");
	}
 */
}
/**
*/
void fdms2dispdata::dumpfostex(t1_toffset pos){
 /*
  unsigned short chval1[8];
  unsigned short chval2[8];
  pos=32*( (t1_toffset)(pos/32) );
  char* pch=0;
  pch=(char*)mapPtr(pos,8*8);
  for (int ch1=0; ch1 < 8 ; ch1++, pch+=4){
        chval1[ch1]= pch[0] <<8 | pch[1];
        chval2[ch1]= pch[2] <<8 | pch[3];
  }
  printf("|");
  for (int ch=0; ch <8 ; ch++){
    long av=abs((short)(chval1[ch]))+ abs((short)(chval1[ch]));
    int v= abs( av /2 / VUMETERDIV );
    for (int j1=0; j1< v; j1++){
     printf("%c", '1'+ch);
    }
    for (int j=v; j< VUMETERMAX+1; j++){
     printf(".");
    }
    printf("|");
  }
 */
}
