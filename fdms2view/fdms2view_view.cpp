/* Written by Barna Farago <brown@weblapja.com> 2006-2018
*
*/
// testermfc_fdms2libView.cpp : implementation of the CFdms2View_View class
//
//#define _WIN32_WINNT 0x0500
#include "stdafx.h"
#include "fdms2view_app.h"
#include "fdms2view_doc.h"
#include "fdms2view_view.h"

#include <afxwin.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WAVELEFTPADX  64

// CFdms2View_View

IMPLEMENT_DYNCREATE(CFdms2View_View, CView)

BEGIN_MESSAGE_MAP(CFdms2View_View, CView)
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
    ON_WM_RBUTTONUP()
	ON_WM_TIMER()
    ON_WM_LBUTTONDOWN()
    ON_WM_ERASEBKGND()
    ON_WM_CREATE()
    ON_COMMAND(ID_VIEW_DB, &CFdms2View_View::OnViewDb)
    ON_COMMAND(ID_ZOOMY_MAX, OnZoomyMax)
    ON_WM_KEYDOWN()
END_MESSAGE_MAP()

// CFdms2View_View construction/destruction

CFdms2View_View::CFdms2View_View():m_ViewMode(0),m_ValMode(0), m_yRulerBottom(53)
{
}

CFdms2View_View::~CFdms2View_View()
{

}

BOOL CFdms2View_View::PreCreateWindow(CREATESTRUCT& cs)
{
	InitWaveAreas();
	return CView::PreCreateWindow(cs);
}
void CFdms2View_View::InitWaveAreas(){
	for (int i=0; i<8; i++){
        m_WaveArea[i].setColorBkg(RGB(0xe0, 0xe0+(15* (i&1)), 0xf0));
        m_WaveLabelArea[i].setColorBkg(RGB(0xe0, 0xe0+(15* (i&1)), 0xf0));
        m_WaveArea[i].setChannel(i);
        m_WaveLabelArea[i].setChannel(i);
	}
}
void CFdms2View_View::updateDisplay(bool bInvalidate){
    CFdms2View_Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
    fdms2pos posCursor= pDoc->getPosEditCursor();
    if ((posCursor < m_PosDisplayStart)||(m_PosDisplayStop < posCursor)){
        m_PosDisplayStart=posCursor;
        bInvalidate=true;
    }
    if (bInvalidate){
        Invalidate();
    }
}
void CFdms2View_View::updateData(){
    CFdms2View_Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
    if (pDoc->getPlayNow()) updateDisplay();
    fdms2pos d=getDisplayStart();
    fdms2pos m=getPeekMaxLength();
    m.addPos(d);
    for (int i=0; i<8; i++){
        m_WaveArea[i].setData(d, m);
	}
}
void CFdms2View_View::StartWaveAreas(CDC* pDC){
	CFdms2View_Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
    updateData();
    for (unsigned int i=0; i<8; i++){
        CLineStrip *pStrip=NULL;
        pDoc->getLineStrip(i, pStrip);
        m_WaveArea[i].registerDoc(pDoc);  //TODO: check if its need or not?
        m_WaveLabelArea[i].registerDoc(pDoc);
        m_WaveLabelArea[i].registerLineStrip(pStrip);
        m_WaveArea[i].setData(getDisplayStart(), getPeekMaxLength());
		m_WaveArea[i].setDC(pDC);
        m_WaveLabelArea[i].setDC(pDC);
    }
    m_MapArea.registerDoc(pDoc);
    m_MapArea.setDC(pDC);
    ResizeAreas();
}
void CFdms2View_View::ResizeAreas(){
    int hs=0;
    int hh=0;
    for (unsigned int i=0; i<FOSTEXMAXCHANNELS; i++){
        if (m_WaveLabelArea[i].getHide()) hh++;
        else hs++;
    }
    CFdms2View_Doc* pDoc = GetDocument();
    CRect r;	
	CRect RectCh;
	CRect RectLabel;
    this->GetClientRect(&r);
	int dx=r.Width()-WAVELEFTPADX;
	int dy=r.Height() - (m_yRulerBottom+20);
    if (hs){
        m_my=(dy-20-(10*hh))/hs;
    }
    m_PosDisplayStop=getDisplayPos(dx);
    int y=m_yRulerBottom+10;
	for (unsigned int i=0; i<FOSTEXMAXCHANNELS; i++){
        //TODO: could be the wavelabel and wave in the same class?
        int ay=m_my;
        if (m_WaveLabelArea[i].getHide()){
            ay=10;
        }
		RectCh.top=y+3;
		RectCh.bottom=y+ay;
		RectCh.left=WAVELEFTPADX;
		RectCh.right=dx+WAVELEFTPADX;
        RectLabel=RectCh;
        RectLabel.left=5;//Cursor pixels
        RectLabel.right=WAVELEFTPADX-2;
        m_WaveArea[i].setRect(&RectCh);
        m_WaveLabelArea[i].setRect(&RectLabel);
        y+=ay;
	}
    RectCh.top= RectCh.bottom+5;
    RectCh.bottom= RectCh.top + dy;
    m_MapArea.setRect(&RectCh);
}
void CFdms2View_View::StopWaveAreas(){
}

// CFdms2View_View drawing

void CFdms2View_View::OnDraw(CDC* pDC){
    
    HCURSOR hCurs1 = LoadCursor(NULL, IDC_WAIT);
    HCURSOR oldCursor= SetCursor(hCurs1);
    CFdms2View_Doc* pDoc = GetDocument();
    CRect r;	
	this->GetClientRect(&r);
    
    ASSERT_VALID(pDoc);
    m_DCTmp.CreateCompatibleDC(pDC);
    CPalette* pPalette= pDC->GetCurrentPalette();
    m_BitmapTmp.CreateCompatibleBitmap(pDC, r.Width(), r.Height());
    m_DCTmp.SelectObject(&m_BitmapTmp);
    
    m_prevCursor.left=0;
    CBrush bWhite( RGB(0xff, 0xff, 0xff));
    CBrush bRed( RGB(0xff, 0x00, 0x00));
    m_DCTmp.FillRect(&r, &bWhite);
	StartWaveAreas(&m_DCTmp);
    //m_yRulerBottom=1;
    switch (pDoc->m_DisplayMode){
        case 1:
            OnDrawOrdinata(&m_DCTmp);
	        OnDrawRawVUs(&m_DCTmp);
	        OnDrawRegion(&m_DCTmp);
            break;
        case 0:
	        OnDrawOrdinata(&m_DCTmp);
	        OnDrawVUs(&m_DCTmp);
	        OnDrawRegion(&m_DCTmp);
            break;
        case 2:
            OnDrawDiskMap(&m_DCTmp);
            break;
    }
    m_MapArea.Draw(this);
	StopWaveAreas();
    pDC->BitBlt(r.left,r.top,r.Width(),r.Height(), &m_DCTmp, 0,0,SRCCOPY);
    GdiFlush();
    m_DCTmp.DeleteDC();
    m_BitmapTmp.DeleteObject();
    SetCursor(oldCursor);
    DeleteObject(oldCursor);
}
#include <math.h>
void CFdms2View_View::OnDrawOrdinata(CDC* pDC)
{
	CFdms2View_Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CRect rect;
	this->GetClientRect(&rect);
	rect.left+=WAVELEFTPADX;
	int dx=rect.Width();
	int dy=rect.Height();
	fdms2pos pLen;
	COLORREF crBack=RGB(0xd0, 0xd0, 0xd0);
	COLORREF crGray = RGB(0xb0, 0xb0, 0xb0);
	int ny=0;
	int my=(dy-10)/18-5;
	int y=5;
	int x=10;
	pDC->TextOut(0, y,		_T("hh:mm:ss"));
	pDC->TextOut(0, y + 16, _T("ss.frame"));
	pDC->TextOut(0, y + 32, _T("img size"));
	pDC->FillSolidRect(rect.left, y, dx,16*3,crBack);
	pDC->SetBkColor(crGray);
	CPen rulerPen(PS_DOT, 1, crGray);
	CPen* oldPen= pDC->SelectObject(&rulerPen);

	for (int i = 0; i < 16; i++) {
		int xx = rect.left + ((i*dx) >> 4);
		pDC->MoveTo(xx, y - 1);
		pDC->LineTo(xx, y + 34);
	}
	pDC->SelectObject(oldPen);
	pDC->SetBkColor(crBack);
    //bytepos 
	for (int i=0;i<8; i++){
		int xx=i*dx/8;
        char * str=NULL;
        getDisplayPos(xx).dumpTimeStrHMS(str);
        CString sstr;
		sstr.Format(L"%S", str);
        OnDrawOneTimeStamp(pDC, rect.left+xx , y, sstr);
        free(str);
	}
	y+=16;
    for (int i=0;i<7; i++){
		int xx=((i*dx)>>3) + (dx>>4);	// i*dx/8+ (dx/16);
        char * str=NULL;
        getDisplayPos(xx).dumpTimeStrSF(str);
        CString sstr;
		sstr.Format(L"%S", str);
        OnDrawOneTimeStamp(pDC, rect.left+xx , y, sstr);
        free(str);
	}
	int olds=-1;
	fdms2pos ps;
    //time
	/*for (int i=dx; i>0; i--){
		ps.setSample(i* pDoc->m_DisplayXMul /16);
        ps.addPos(m_PosDisplayStart.m_Pos);
		if (ps.m_Frame<1)
		if (ps.m_Sec!=olds){
			char* str=NULL;
			ps.dumpTimeStrHMS(str);
			CString sstr;
			sstr.Format(L"%S", str);
			OnDrawOneTimeStamp(pDC, rect.left+ i, y, sstr);
			free(str);
			olds=ps.m_Sec;
			i-=100;
		}
	}*/
	y+=16;
	olds=-1;
    //frame
	for (int i=dx; i>0; i--){
		ps.setSample((long long)(i* pDoc->m_DisplayXMul)>>4); //  /16
        ps.addPos(m_PosDisplayStart.m_Pos);
		if (ps.m_Frame<1)
		if (ps.m_Sec!=olds){
			char* str=NULL;
			ps.dumpByteStr(str);
			CString sstr;
			sstr.Format(L"%S", str);
			OnDrawOneTimeStamp(pDC, rect.left+ i, y, sstr);
			free(str);
			olds=ps.m_Sec;
			i-=100;
		}
	}
    //midi
    //pDoc->m_fdms2.getMtcOffset();
/*    
	for (int i=dx; i>0; i--){
		ps.setSample(i* pDoc->m_DisplayXMul /16);
        ps.addPos(m_PosDisplayStart.m_Pos);
		if (ps.m_Frame<1)
		if (ps.m_Sec!=olds){
			char* str=NULL;
			ps.dumpByteStr(str);

			CString sstr;
			sstr.Format(L"%S", str);
			OnDrawOneTimeStamp(pDC, rect.left+ i, y, sstr);
			free(str);
			olds=ps.m_Sec;
			i-=100;
		}
	}
*/
    m_yRulerBottom=y+16;
}
void CFdms2View_View::OnDrawOneTimeStamp(CDC* pDC, int x, int y, const CString &str){
	pDC->TextOut(x, y, str);	
	pDC->MoveTo(x,y-1);
	pDC->LineTo(x,y+17);
}

void CFdms2View_View::OnDrawMouse(CDC* pDC){
    if (!pDC) return;
//	COLORREF crBack=RGB(0xd0, 0xd0, 0xd0);
	COLORREF crBack=RGB(0xFF, 0xFF, 0xFF);
	COLORREF crFore=RGB(0x00, 0x00, 0x00);
	for (int i=4 ; i; i--){
		pDC->SetPixel(m_pointMouseOld.x,i,crBack);
		pDC->SetPixel(m_pointMouse.x,i,crFore);
		//pDC->SetPixel(i,m_pointMouseOld.y,crBack);
		//pDC->SetPixel(i,m_pointMouse.y,crFore);
	}
	m_pointMouseOld= m_pointMouse;
}
void CFdms2View_View::RedrawCursors(){
	CDC* pDC=GetDC();
	StartWaveAreas(pDC);
	OnDrawRegion(pDC);
    m_MapArea.setDC(pDC);
    m_MapArea.Draw(this);
	StopWaveAreas();
    ReleaseDC(pDC);
//    pDC->DeleteDC();
}
void CFdms2View_View::OnDrawRegion(CDC* pDC){
	COLORREF crBack=RGB(0xFF, 0xFF, 0xFF);
	COLORREF crFore=RGB(0x00, 0x00, 0x00);
	CFdms2View_Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
    CRect rect;
	this->GetClientRect(&rect);
	rect.left+=WAVELEFTPADX;
	int y=m_yRulerBottom;//16*4+4;
	int dx=rect.Width();
    CPen* pOldPen =NULL;
    COLORREF crPenLine=RGB(0, 0x70,0);
	if (!pDoc->m_DisplayXMul)pDoc->m_DisplayXMul = 1;
	int x1=rect.left + (int)((pDoc->m_PosRegionStart.m_Sample - m_PosDisplayStart.m_Sample)/pDoc->m_DisplayXMul);
	int x2=rect.left + (int)((pDoc->m_PosRegionStop.m_Sample - m_PosDisplayStart.m_Sample)/pDoc->m_DisplayXMul);
	int xp=rect.left + (int)((pDoc->m_PosEditCursor.m_Sample - m_PosDisplayStart.m_Sample)/pDoc->m_DisplayXMul);
	if (x2<x1){
		//crPenLine=RGB(0xff, 0,0);
		//CBrush brushLine(RGB(255, 0, 0));
	    int tmp=x1;
        x1=x2; x1=tmp;
	}
    CPen penLine(PS_SOLID, 1, crPenLine);
    pOldPen = pDC->SelectObject(&penLine);
	//CBrush* pOldBrush =pDC->SelectObject(&brushLine);
	pDC->FillSolidRect(rect.left, y, dx,16,crBack);

    if (x1> WAVELEFTPADX){
	    pDC->MoveTo(x1, y+4);
	    pDC->LineTo(x1, y);
    }else{
        pDC->MoveTo(WAVELEFTPADX, y);
    }
    int x22=x2;
    int xma=rect.right;
    if (x22<WAVELEFTPADX) x22=WAVELEFTPADX;
	if (x2<xma) pDC->LineTo(x22, y);else pDC->LineTo(xma,y);
    if (x2> WAVELEFTPADX){
	    if (x2<xma) pDC->LineTo(x2, y+4);
    }
    if ((xp>= WAVELEFTPADX)&&(xp<=xma)){
	    pDC->MoveTo(xp, m_yRulerBottom+5);
	    pDC->LineTo(xp, y);
        
        CRect rC(xp,m_yRulerBottom+18,xp+2,m_yRulerBottom+8+m_my*16);
        if (m_prevCursor.left) pDC->InvertRect(m_prevCursor);
        m_prevCursor=rC;
        if (rC.left) pDC->InvertRect(rC);
    }
    pDC->SelectObject(pOldPen);
    pOldPen->DeleteObject();
    //m_pDC->SelectObject(pOldBrush );
    //pOldBrush->DeleteObject();
}
		
fdms2pos CFdms2View_View::getDisplayPos(int posx){
    UINT64 pos=m_PosDisplayStart.m_Sample+ (UINT64)( (UINT64(posx)) * GetDocument()->m_DisplayXMul );
    return fdms2pos(pos<<4); //*16
}
void CFdms2View_View::getDisplayStartPartOffs(int &iPart, INT64 &iOffs){
    CFdms2View_Doc* pDoc=GetDocument();
    pDoc->m_fdms2.reset();
    INT64 start;
    INT64 len;
    pDoc->m_fdms2.convertLogical2Abs(pDoc->getSelectedProgram(), m_PosDisplayStart, iOffs, start, len, iPart);
}
void CFdms2View_View::OnDrawVUs(CDC* pDC)
{
	for (int ch=0; ch<8; ch++){
        m_WaveLabelArea[ch].DrawOne();
		m_WaveArea[ch].DrawOneScale_dB();
		m_WaveArea[ch].DrawOneData(this);
	}
}
void CFdms2View_View::OnDrawRawVUs(CDC* pDC)
{
	for (int ch=0; ch<8; ch++){
        m_WaveLabelArea[ch].DrawOne();
		m_WaveArea[ch].DrawOneScale_dB();
		m_WaveArea[ch].DrawOneDataRaw(this);
	}
}
void CFdms2View_View::OnDrawDiskMap(CDC* pDC)
{
	CFdms2View_Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	int iPrg = pDoc->getSelectedProgram();
	CRect rect;
	this->GetClientRect(rect);
	int dx = rect.right - rect.left - 16;
	fdms2pos pLen;
	t1_toffset start;
	if (!dx) dx = 1;
	UINT64 maxpos =  pDoc->getMaxPos();
	for (int i = 0; i < 5; i++) {
		int iIdx = 0;
		while (!pDoc->m_fdms2.getPart(i, iIdx, start, pLen)) {
			UINT64 dwTmp = start + pLen.m_Pos;
			if (maxpos < dwTmp) maxpos = dwTmp;
			iIdx++;
		}
	}

	UINT64 ddx = maxpos / dx;
	//if (ddx > INT_MAX) ddx = INT_MAX;
	if (ddx < 1) {
		ddx = 1;
	}
	int y = m_yRulerBottom + 4;
	y = 20;
	int iIdx = 0;
	CString s;
	s.Format(L"Relative Program map");
	pDC->TextOut(0, 0, s);

	for (int i = 0; i < 5; i++) {
		iIdx = 0;
		int x = 0;
		s.Format(L"Program%i:", i);
		pDC->TextOut(0, y, s);
		y += 13;
		while (!pDoc->m_fdms2.getPart(i, iIdx, start, pLen)) {
			//int x=start/ddx;
			int x2 = x + (int)((pLen.m_Pos) / ddx);
			pDC->MoveTo(x2, 0 + y);
			pDC->LineTo(x2, 5 + y);
			pDC->MoveTo(x, 1 + y + (iIdx % 4));
			pDC->LineTo(x2, 1 + y + (iIdx % 4));
			pDC->MoveTo(x, 0 + y);
			pDC->LineTo(x, 5 + y);
			iIdx++; x = x2 + 1;
		}
		y -= 13;
		pDC->MoveTo(0, y);
		pDC->LineTo(x, y);

		pDC->MoveTo(0, y + 20);
		pDC->LineTo(x, y + 20);
		pDC->MoveTo(x, y);
		pDC->LineTo(x, y + 20);
		y += 22;
	}
	s.Format(L"Absolute Program map");
	pDC->TextOut(0, y, s);
	y += 16;
	for (iPrg = 0; iPrg < 5; iPrg++) {
		iIdx = 0;
		y += 10;
#define BYTEPERSEC 0xac440ul
		s.Format(L"Program%i:(%llis)", iPrg, pDoc->m_fdms2.getProgramSampleCount(iPrg) / 44100 );
		pDC->TextOut(0, y, s);
		y += 13;
		int ytop = y;
		y += 20;
		while (!pDoc->m_fdms2.getPart(iPrg, iIdx, start, pLen)) {
			int x = (int)(start / ddx);
			int x2 = (int)((start + pLen.m_Pos) / ddx);
			s.Format(L"%i(blk:%llx+ %llis)", iIdx, start >> 9, pLen.m_Pos / BYTEPERSEC);
			pDC->TextOut(x, y, s);
			y += 12;
			pDC->MoveTo(x2, 0 + y);
			pDC->LineTo(x2, 5 + y);
			pDC->MoveTo(x, 1 + y + (iIdx % 4));
			pDC->LineTo(x2, 1 + y + (iIdx % 4));
			pDC->MoveTo(x, 0 + y);
			pDC->LineTo(x, 5 + y);
			pDC->MoveTo(x, ytop);
			pDC->LineTo(x, ytop + 20);
			pDC->MoveTo(x2, ytop);
			pDC->LineTo(x2, ytop + 20);
			iIdx++; y += 2;
		};
	}
}

// CFdms2View_View diagnostics

#ifdef _DEBUG
void CFdms2View_View::AssertValid() const
{
	CView::AssertValid();
}

void CFdms2View_View::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CFdms2View_Doc* CFdms2View_View::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFdms2View_Doc)));
	return (CFdms2View_Doc*)m_pDocument;
}
#endif //_DEBUG

// CFdms2View_View message handlers
BOOL CFdms2View_View::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	CFdms2View_Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	
	if (nFlags & MK_CONTROL){
        pDoc->addDisplayXZoom(this, zDelta);
	}
    if (nFlags & MK_SHIFT){
		pDoc->m_DisplayLevelValue+=zDelta;
		if (pDoc->m_DisplayLevelValue<1) pDoc->m_DisplayLevelValue=1;
		if (pDoc->m_DisplayLevelValue>0x7fff) pDoc->m_DisplayLevelValue=0x7fff;
	}
    if ((nFlags & (MK_SHIFT | MK_CONTROL))==0){
         m_PosDisplayStart.addPos( (__int64)(zDelta* pDoc->m_DisplayXMul));
    }
    pDoc->ValidatePos(m_PosDisplayStart);
    pDoc->ValidatePos(m_PosDisplayStop);
	Invalidate();
    //updateDisplay(true);
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CFdms2View_View::OnMouseMove(UINT nFlags, CPoint point)
{
	m_pointMouse=point;
	CDC* pDC=GetDC();
	OnDrawMouse(pDC);
	CView::OnMouseMove(nFlags, point);
}

void CFdms2View_View::OnLButtonUp(UINT nFlags, CPoint point)
{
	CFdms2View_Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
    if (point.x<WAVELEFTPADX){
        for (int i=0; i< FOSTEXMAXCHANNELS; i++){
            CRect* r= m_WaveLabelArea[i].getRect();
            if (r->PtInRect(point)){
                m_WaveLabelArea[i].OnLButtonUp(nFlags, point, m_pointLMouseDown);
                Invalidate();
            }
        }
        CView::OnLButtonUp(nFlags, point);
        return;
    }
    INT64 s= (__int64)((point.x-WAVELEFTPADX)* pDoc->m_DisplayXMul) + m_PosDisplayStart.m_Sample; //TODO: van rá függvény
    if (m_MapArea.getRect()->PtInRect(point)){
        m_MapArea.OnLButtonUp(nFlags, point, m_pointLMouseDown);// TODO return value about handling.
        return;
    }   
	if (nFlags & MK_CONTROL){
		pDoc->m_PosRegionStart.setSample(s);
	}
	if (nFlags & MK_SHIFT){
		pDoc->m_PosRegionStop.setSample(s);
	}
    if ((nFlags & (MK_CONTROL | MK_SHIFT))==0){

        if (abs(point.x-m_pointLMouseDown.x)>2){
            m_PosDisplayStart.addSample((__int64)(-(point.x - m_pointLMouseDown.x) * pDoc->m_DisplayXMul));
                //(point.x-m_WaveArea[0].m_dx/2) * pDoc->m_DisplayXMul);
            if (m_PosDisplayStart.m_Pos<0) m_PosDisplayStart.setPos(0);
            Invalidate();
        }else{
            pDoc->m_PosEditCursor.setSample(s);
            pDoc->setPosEditCursor(pDoc->m_PosEditCursor); //TODO: refactoring needed.
        }

	}
	CDC* pDC=GetDC();
	OnDrawRegion(pDC);
	CView::OnLButtonUp(nFlags, point);
}
void CFdms2View_View::OnLButtonDown(UINT nFlags, CPoint point)
{
    m_pointLMouseDown=point;
    __super::OnLButtonDown(nFlags, point);
}
void CFdms2View_View::OnRButtonUp(UINT nFlags, CPoint point)
{
    if (point.x<WAVELEFTPADX){
        for (int i=0; i< FOSTEXMAXCHANNELS; i++){
            CRect* r= m_WaveLabelArea[i].getRect();
            if (r->PtInRect(point)){
                m_WaveLabelArea[i].OnRButtonUp(nFlags, point, m_pointRMouseDown);
                Invalidate();
            }
        }
    }
    __super::OnRButtonUp(nFlags, point);
}
void CFdms2View_View::OnRButtonDown(UINT nFlags, CPoint point)
{
    m_pointRMouseDown=point;
    __super::OnRButtonDown(nFlags, point);
}
void CFdms2View_View::OnTimer(UINT nIDEvent)
{
	CFdms2View_Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
    if (nIDEvent==1002) 
        if (pDoc->m_cache.isRunning() || pDoc->m_bRedraw){
            updateData();
            Invalidate();
            pDoc->m_bRedraw=false;
        }
    
	if (nIDEvent==1001) pDoc->OnPlay();
	CView::OnTimer(nIDEvent);
}


BOOL CFdms2View_View::OnEraseBkgnd(CDC* pDC)
{
    // TODO: Add your message handler code here and/or call default

    //return __super::OnEraseBkgnd(pDC);
    return TRUE;
}
signed short CFdms2View_View::val2db(signed short val){
    int s=1;
    if (val<0) s=-1; 
    //return log((double)abs(val))/log((double)2)*64*1024/96*s;
	double l= log((double)abs(val))*32*1024/log((double)0x7FFF);
	/*Bug fixed: in case of small value, wrong display with dB scale. */
	if (l < 0) l = 0; // exponent is negativ, not viewable.
	l = l * s; //sign
	if (l > SHORT_MAX) l = SHORT_MAX;
	if (l < SHORT_MIN) l = SHORT_MIN;
	return (signed short)l;
}
bool CFdms2View_View::getPeek(INT64 iOffs, unsigned int uiDiff, short iCh, short &iMax, short &iMin){
    bool bRet=false;
    CFdms2View_Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
    bRet=pDoc->m_cache.getPeek(iOffs, iCh, iMax, iMin);
    if (m_ValMode){
     iMax=val2db(iMax);
     iMin=val2db(iMin);
    }
    return bRet;
}
fdms2pos CFdms2View_View::getPeekMaxLength(){
    CFdms2View_Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
    return pDoc->m_cache.getPeekMaxLength();
}
int CFdms2View_View::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (__super::OnCreate(lpCreateStruct) == -1)
        return -1;

    // TODO:  Add your specialized creation code here
    SetTimer(1001, 60, NULL);
    SetTimer(1002, 2000, NULL);

    return 0;
}

void CFdms2View_View::OnViewDb()
{
    m_ValMode++;
    if (m_ValMode>1) m_ValMode=0;
    if (m_ValMode==1){
        OnZoomyMax();        
    }
	Invalidate();    
}
void CFdms2View_View::OnZoomyMax()
{
    CFdms2View_Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc); //TODO: a display level lehetne-e a view-ban? mert arra vonatkozik.
	pDoc->m_DisplayLevelValue = 0x7FFF;
	//UpdateAllViews(NULL, 0);
}

void CFdms2View_View::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    CFdms2View_Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc); //TODO: a display level lehetne-e a view-ban? mert arra vonatkozik.
    pDoc->DoOrderRegion();
	bool alt = (nFlags&KF_ALTDOWN);
    switch (nChar){
        case VK_HOME: m_PosDisplayStart.setPos( pDoc->m_PosRegionStart.m_Pos); break;
        case VK_END: m_PosDisplayStart.setPos( pDoc->m_PosRegionStop.m_Pos); break;
		case VK_LEFT: m_PosDisplayStart.addSample(-44100*(alt?10:1)); break;
        case VK_RIGHT: m_PosDisplayStart.addSample(44100 * (alt ? 10 : 1)); break;
        case VK_UP: pDoc->addDisplayXZoom(this, 40 * (alt ? 0.5 : 1)); break;
        case VK_DOWN: pDoc->addDisplayXZoom(this, 70 * (alt ? 5 : 1)); break;
			
    }
    Invalidate();
    __super::OnKeyDown(nChar, nRepCnt, nFlags);
}
