/*Written by Barna Farago <brown@weblapja.com> 2006-2018
*/
#include "fdms2error.h"

void fdms2errorIF::messageBox(LPCTSTR cat, LPCTSTR msg){
    if (m_notify) m_notify->messageBox(cat, msg);
    DLOG("Error %s:%s", cat, msg);
    //MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
 }
