#include "StdAfx.h"
#include "Area.h"

CArea::CArea(void):m_pDC(NULL),m_ColorBk(RGB(0,0,0)),m_pDoc(NULL),m_pFdms2(NULL),m_iCh(0)
{
}

CArea::~CArea(void)
{
}
 void CArea::registerDoc(CFdms2View_Doc* pDoc){
        m_pDoc=pDoc;
        m_pFdms2=&(pDoc->m_fdms2);
}