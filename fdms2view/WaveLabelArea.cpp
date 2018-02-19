//#define _WIN32_WINNT 0x0500
#include "StdAfx.h"
#include "wavelabelarea.h"
#include <math.h>
#include <afxwin.h>
#include "fdms2view_app.h"
#include "fdms2view_doc.h"
#include "fdms2view_view.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CWaveLabelArea::CWaveLabelArea(void)
{
	m_ColorScale=RGB(0, 0, 255);
}

CWaveLabelArea::~CWaveLabelArea(void)
{
}
void CWaveLabelArea::OnLButtonUp(UINT nFlags, CPoint pointUp, CPoint pointDown){
    //m_hide=!m_hide;
}
void CWaveLabelArea::OnRButtonUp(UINT nFlags, CPoint pointUp, CPoint pointDown){
    m_hide=!m_hide;
}
void CWaveLabelArea::Draw(IViewTimeLine *pTl){
}
void CWaveLabelArea::DrawOne(){
    m_fontSmall.CreatePointFont(60, L"Arial", m_pDC);
	int y=m_Rect.CenterPoint().y;
    m_pDC->FillSolidRect(m_Rect.left, m_Rect.top, m_RectWidth, m_Rect.Height(), m_ColorBk);
	COLORREF crPenLine=m_ColorScale;
	int my=m_my;
	CRect r=m_Rect;
    r.bottom=r.top+16;
	int	psPenLine=PS_SOLID;//PS_SOLID;
	CPen penLine(psPenLine, 1, crPenLine);
	CBrush brushLine(RGB(0, 0, 255));
	CPen* pOldPen = m_pDC->SelectObject(&penLine);
	CBrush* pOldBrush = m_pDC->SelectObject(&brushLine);
    CString str;
    str.Format(L"Ch%i", m_iCh);
    m_pDC->DrawText(str, &r, DT_CENTER );
    if (m_lineStrip){
        DrawOneFader(r, L"Vol", m_lineStrip->m_iFader);
        DrawOneFader(r, L"Pan", m_lineStrip->m_iPan+64);
    }
	m_pDC->SelectObject(pOldPen );
	m_pDC->SelectObject(pOldBrush );
    pOldPen->DeleteObject();
    pOldBrush->DeleteObject();
    m_fontSmall.DeleteObject(); //TODO:Init phase?
}
void CWaveLabelArea::DrawOneFader(CRect &r,CString label, int val){
    CFont* pOldFont=m_pDC->SelectObject(&m_fontSmall);
    r.top+=10; r.bottom+=10;
    CString str;
    str.Format(L"%s:%i",label, m_lineStrip->m_iFader);
    CRect r2;
    r2=r;
    m_pDC->DrawText(label, &r2, DT_LEFT);
    r2.top=r2.CenterPoint().y-2; r2.bottom=r2.CenterPoint().y+2;
    r2.left=r.left; //r2.CenterPoint().x;
    r2.right=r.right;
    m_pDC->MoveTo( r2.left, r2.CenterPoint().y);
    m_pDC->LineTo( r2.right, r2.CenterPoint().y);
    int w=r2.Width();
    int p=w*val/127;
    m_pDC->MoveTo( r2.right-p, r2.top);
    m_pDC->LineTo( r2.right-p, r2.bottom);

    m_pDC->SelectObject(pOldFont);
}