/*Written by Barna Farago <brown@weblapja.com> 2006-2018
*/
#ifndef _FDMS2ERROR_H
#define _FDMS2ERROR_H

#include "fdms2common.h"

class fdms2notifyIF {
public:
    virtual void messageBox(LPCTSTR cat,LPCTSTR msg)=0;
};

class fdms2errorIF {
public:
    fdms2errorIF():m_notify(NULL){}
	virtual ~fdms2errorIF() { m_notify = 0; }
    void setNotify(fdms2notifyIF* notify){m_notify=notify;}
    fdms2notifyIF* getNotify(){return m_notify;}
    void messageBox(LPCTSTR cat,LPCTSTR msg);
private:
    fdms2notifyIF* m_notify;
};

#endif
