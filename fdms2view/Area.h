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
    void setHide(bool hide){m_hide=hide;}
    bool getHide(){return m_hide;}
    void setHeight(int height);
    int getHeight(){return m_RectHeight;}

    void setRect(CRect * lpRectCh){
        m_Rect.CopyRect( lpRectCh );
        m_RectWidth= m_Rect.Width();
        m_RectHeight=m_Rect.Height();
        m_my= (m_Rect.Height())/2-1;
    }
    CRect* getRect(){ return &m_Rect;}
    virtual void Draw(IViewTimeLine *pTl)=0;
    virtual void OnLButtonUp(UINT nFlags, CPoint pointUp, CPoint pointDown)=0;
    virtual void OnRButtonUp(UINT nFlags, CPoint pointUp, CPoint pointDown)=0;
protected:
   	COLORREF m_ColorBk;
	CFdms2View_Doc* m_pDoc;
    fdms2* m_pFdms2;
    bool m_hide;
    CDC* m_pDC;
    int m_iCh;
    int m_RectHeight;
    CRect m_Rect;
    int m_RectWidth;
	int m_my;
};
