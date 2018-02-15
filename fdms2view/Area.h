#pragma once

#include "fdms2view_app.h"
#include "fdms2view_doc.h"
#include "iviewtimeline.h"


class CArea
{
public:
    CArea(void);
    ~CArea(void);
    void setDC(CDC* pDC){m_pDC=pDC;}
    void setColorBkg(COLORREF bk){ m_ColorBk = bk; }
    void setChannel(int ch){ m_iCh=ch;}
    void registerDoc(CFdms2View_Doc* pDoc);
    void setRect(CRect * lpRectCh){
        m_Rect.CopyRect( lpRectCh );
        m_RectWidth= m_Rect.Width();
        m_my= (m_Rect.Height())/2-1;
    }
    virtual void Draw(IViewTimeLine *pTl)=0;
protected:
   	COLORREF m_ColorBk;
	CFdms2View_Doc* m_pDoc;
    fdms2* m_pFdms2;
    CDC* m_pDC;
    int m_iCh;
    CRect m_Rect;
    int m_RectWidth;
	int m_my;
};
