/*Written by Barna Farago <brown@weblapja.com> 2006-2018
*/
#include "fdms2lib.h"
#include <string.h>
#ifdef WIN32
#define LIBSPEC __declspec(dllexport)
#else
#define LIBSPEC
#endif

extern "C" LIBSPEC int getVersionMajor(){                                                                                                                                                           
 return 0;                                                                                                                                                                                                         
}                                                                                                                                                                                                                 
extern "C" LIBSPEC int getVersionMinor(){                                                                                                                                                           
 return 3;                                                                                                                                                                                                         
}

fdms2* g_f2;

extern "C" LIBSPEC int fdms2_init(const char* pszFname, int iStartPos, int iLength, int iStep){
 int iRet=0;
 g_f2=new fdms2();
 g_f2->setFileName(pszFname); 
 g_f2->m_startpos=iStartPos;
 g_f2->m_length=iLength;
 g_f2->m_step=iStep;
 if (!iRet) iRet|= g_f2->start();
 return iRet;
}

extern "C" LIBSPEC int fdms2_done(){
 int iRet=0;
 if (g_f2){
// g_f2=new fdms2();
 }
 return iRet;
}

int main(){
 g_f2=0;
 return 0;
}

extern "C" LIBSPEC BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,  // handle to the DLL module
  DWORD fdwReason,     // reason for calling function
  LPVOID lpvReserved   // reserved
){

 // Perform actions based on the reason for calling.
 switch( fdwReason ) 
 { 
     case DLL_PROCESS_ATTACH:
      // Initialize once for each new process.
      // Return FALSE to fail DLL load.
         break;

     case DLL_THREAD_ATTACH:
      // Do thread-specific initialization.
         break;

     case DLL_THREAD_DETACH:
      // Do thread-specific cleanup.
         break;

     case DLL_PROCESS_DETACH:
      // Perform any necessary cleanup.
         break;
 }
 return TRUE;
}

