#include "StdAfx.h"
#include "MapArea.h"

CMapArea::CMapArea(void)
{
}

CMapArea::~CMapArea(void)
{
}

void CMapArea::Draw(IViewTimeLine *pTl){
    t1_toffset maxSample=m_pFdms2->getLongestProgramSampleCount();
    t1_toffset s=pTl->getDisplayStart().m_Sample; //*16?
	if (m_RectWidth < 1) m_RectWidth = 1;
    if (maxSample < s) maxSample=s+m_RectWidth*m_pDoc->m_DisplayXMul;

    int ddx= maxSample / m_RectWidth;
    if (!ddx) return;
    int iPrg= m_pDoc->getSelectedProgram();
    int y= m_Rect.top + iPrg*5;
    CBrush brushLine(RGB(255, 0, 0));
	CBrush* pOldBrush = m_pDC->SelectObject(&brushLine);
    int	psPenLine=PS_SOLID;//PS_SOLID;
    COLORREF crPenLine=RGB(0, 0, 0);
	CPen penLine1(psPenLine, 1, crPenLine);
	CPen* pOldPen = m_pDC->SelectObject(&penLine1);
    for (int i=0; i<5; i++){
        int iIdx=0;
         if (iPrg == i){
            m_pDC->FillSolidRect(m_Rect.left + s / ddx -1, y,
                m_RectWidth*m_pDoc->m_DisplayXMul / ddx +1 ,3, RGB(80, 80, 255));
            m_pDC->FillSolidRect(m_Rect.left, y+3,
                pTl->getPeekMaxLength().m_Sample / ddx, 3, RGB(0, 255, 0));
            
        }
        int x=m_Rect.left;
        fdms2pos len=0;
        INT64 start=0;
        while(!m_pFdms2->getPart(i, iIdx, start, len)){
            int x2=x+(len.m_Sample)/ddx;
		    m_pDC->MoveTo(x2, 0+y);
		    m_pDC->LineTo(x2,5+y);
		    m_pDC->MoveTo(x, 1+y+(iIdx%4));
		    m_pDC->LineTo(x2,1+y+(iIdx%4));
		    m_pDC->MoveTo(x, 0+y);
		    m_pDC->LineTo(x,5+y);
		    iIdx++; x=x2; //+1
	    }
        y+=5;
    }
    
    CPen penLine(PS_DOT, 2, RGB(255, 0, 0));
	m_pDC->SelectObject(&penLine);

    int xEC=m_Rect.left+m_pDoc->m_PosEditCursor.m_Sample /ddx;
    y=m_Rect.top+ iPrg*5;
    m_pDC->MoveTo(xEC, y);
    m_pDC->LineTo(xEC, y+5);
    m_pDC->SelectObject(pOldPen);
    m_pDC->SelectObject(pOldBrush );
    pOldPen->DeleteObject();
    pOldBrush->DeleteObject();
}
void CMapArea::OnLButtonUp(UINT nFlags, CPoint pointUp, CPoint pointDown){

}
void CMapArea::OnRButtonUp(UINT nFlags, CPoint pointUp, CPoint pointDown){

}