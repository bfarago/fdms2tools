//#define _WIN32_WINNT 0x0500
#include "StdAfx.h"
#include "wavearea.h"
#include <math.h>
#include <afxwin.h>
#include "fdms2view_app.h"
#include "fdms2view_doc.h"
#include "fdms2view_view.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class CPeekExc {
public:
	CPeekExc() {
	}
	CPeekExc(const CPeekExc &) {

	}
};

CWaveArea::CWaveArea(void)
{
	m_ColorScale=RGB(0, 0, 255);
}

CWaveArea::~CWaveArea(void)
{
}
void CWaveArea::Draw(IViewTimeLine *pTl){
}
void CWaveArea::DrawYMarker(unsigned short valueY, const CString & str,
							bool bTopLeft=true, bool bBottomLeft=false,
							bool bTopRight=false, bool bBottomRight=false)
{
	int width=50;
	int height2=8;
    int y=m_Rect.CenterPoint().y;
	int ny= valueY * m_my / m_pDoc->m_DisplayLevelValue;
    
	if (bTopLeft){
		CRect r(m_Rect.left, y-ny-height2, m_Rect.left + width, y-ny+height2);
		m_pDC->DrawText(str, &r, DT_CENTER );
	}
	if (bBottomLeft){
		CRect r(m_Rect.left, y+ny-height2, m_Rect.left + width, y+ny+height2);
		m_pDC->DrawText(str, &r, DT_CENTER );
	}
	if (bTopRight){
		CRect r(m_Rect.right-width, y-ny-height2, m_Rect.right, y-ny+height2);
		m_pDC->DrawText(str, &r, DT_CENTER );
	}
	if (bBottomRight){
		CRect r(m_Rect.right-width, y+ny-height2, m_Rect.right, y+ny+height2);
		m_pDC->DrawText(str, &r, DT_CENTER );
	}
	m_pDC->MoveTo(m_Rect.left, y+ny);
	m_pDC->LineTo(m_Rect.right, y+ny);
	m_pDC->MoveTo(m_Rect.left, y-ny);
	m_pDC->LineTo(m_Rect.right, y-ny);
	
}
void CWaveArea::DrawSniped(int px, int py, short val){
    int ny=0;
    int my=m_my;
    ny= val*my/m_pDoc->m_DisplayLevelValue;
	if (ny>my){
		ny=my;
	}
	if (-ny>my){
		ny=-my;
	}
	if (ny==0){
		m_pDC->SetPixel(px,py, 0);
	}else{
        m_pDC->MoveTo(px,py);
		m_pDC->LineTo(px,py+ny);
	}
}


void CWaveArea::DrawPeek(int px, int py, short iMax, short iMin){
    int nyMax=0;
    int nyMin=0;
    int my=m_my;
    nyMax= iMax * my / m_pDoc->m_DisplayLevelValue;
    nyMin= iMin * my / m_pDoc->m_DisplayLevelValue;
	if (nyMax>my){
		nyMax=my;
	}
	if (-nyMax>my){
		nyMax=-my;
	}
    if (nyMin>my){
		nyMin=my;
	}
	if (-nyMin>my){
		nyMin=-my;
	}
    if (nyMax == nyMin){
        m_pDC->SetPixel(px,py, 0);
    }else{
	    m_pDC->MoveTo(px,py+nyMin);
	    m_pDC->LineTo(px,py+nyMax);
    }
	
}
void CWaveArea::DrawError(int px, int py){
    CPen penLine(PS_DASH, 1, RGB(255, 0, 0));
	CPen* pOldPen = m_pDC->SelectObject(&penLine);

	m_pDC->MoveTo(px, py+m_my);
	m_pDC->LineTo(m_Rect.right, py-m_my);
	m_pDC->MoveTo(px, py-m_my);
	m_pDC->LineTo(m_Rect.right, py+m_my);
	m_pDC->MoveTo(px, py-m_my);
	m_pDC->LineTo(px, py+m_my);

	m_pDC->SelectObject(pOldPen);
}
void CWaveArea::DrawSeparator(int px, int py){
    CPen penLine(PS_DASH, 1, RGB(255, 0, 0));
	CPen* pOldPen = m_pDC->SelectObject(&penLine);
	m_pDC->MoveTo(px, py+m_my);
	m_pDC->LineTo(px, py-m_my);
	m_pDC->SelectObject(pOldPen);
}
void CWaveArea::DrawOneDataRaw(IViewTimeLine *pTl){
		int x=0;
		int px=0;
        m_pDoc->m_fdms2.reset();
		int y=(m_Rect.bottom+m_Rect.top)/2;
		try{
			for (x=0; x < m_RectWidth; x++){
				px=x+m_Rect.left;
                short val=m_pFdms2->getValue(pTl->getDisplayPos(x).m_Pos , m_iCh, 0);
				DrawSniped(px, y, val);
			}
		}catch(...){
            DrawError(px+1, y);
		}
}
void CWaveArea::DrawOneData(IViewTimeLine *pTl){
		int x=0;
		int px=0;
        int y=m_Rect.CenterPoint().y;
        int iIdx=0;
        fdms2pos pLen;
        fdms2pos pos;
	    fdms2pos start;
		unsigned int uiDiff = (unsigned int)m_pDoc->m_DisplayXMul;
        try{
            start=m_displayPos;
            pos.setPos(start.m_Pos);
            while (x < m_RectWidth){
                short iMax, iMin;
                px=x+m_Rect.left;
                iMax=0; iMin=0;
                if (!pTl->getPeek(pos.m_Sample, uiDiff, m_iCh, iMax, iMin)) throw CPeekExc();
			    DrawPeek(px, y, iMax, iMin);
                x++;
//                if (pos.m_Sample > m_displayMax.m_Sample) break;
                pos.addSample(uiDiff); // 16
		    }
		}
		catch (const CPeekExc&) {
			DrawError(px + 1, y);
		}
		catch(...){
            DrawError(px+1, y);
		}
}
void CWaveArea::DrawOneScale_dB(){
	int y=m_Rect.CenterPoint().y;
    m_pDC->FillSolidRect(m_Rect.left, m_Rect.top+1, m_RectWidth, m_Rect.Height(), m_ColorBk);
	COLORREF crPenLine=m_ColorScale;
	int my=m_my;
	
	int	psPenLine=PS_DOT;//PS_SOLID;
	CPen penLine(psPenLine, 1, crPenLine);
	CBrush brushLine(RGB(255, 0, 0));
	CPen* pOldPen = m_pDC->SelectObject(&penLine);
	CBrush* pOldBrush = m_pDC->SelectObject(&brushLine);

	m_pDC->MoveTo(m_Rect.left, y);
	m_pDC->LineTo(m_Rect.right,y);
	int nscale=3;
	int mdy= m_pDoc->m_DisplayLevelValue;
	if (0x7FFF<=mdy){
		DrawYMarker(0x7FFF, L"max", true, false, true, false);
		nscale--;
	}
	if (0x3FFF<=mdy){
		DrawYMarker(0x3FFF, L"-6dB", false, true, false, true);
		nscale--;
	}
	if (nscale>0){
		if (0x1FFF<=mdy){
			DrawYMarker(0x1FFF, L"-12dB", true, false, true, false);
			nscale--;
		}
	}
	if (nscale>0){
		if (0x0FFF<=mdy){
			DrawYMarker(0x0FFF, L"-18dB", false, true, false, true);
			nscale--;
		}
	}
	if (nscale>0){
		if (0x07FF<=mdy){
			DrawYMarker(0x07FF, L"-24dB", true, false, true, false);
			nscale--;
		}
	}
	if (nscale>0){
		if (0x03FF<=mdy){
			DrawYMarker(0x3FF, L"-30dB", false, true, false, true);
			nscale--;
		}
	}
	m_pDC->SelectObject(pOldPen );
	m_pDC->SelectObject(pOldBrush );
}