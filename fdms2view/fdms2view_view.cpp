// testermfc_fdms2libView.cpp : implementation of the CFdms2View_View class
//
#define _WIN32_WINNT 0x0500
#include "stdafx.h"
#include "fdms2view_app.h"
#include "fdms2view_doc.h"
#include "fdms2view_view.h"

#include <afxwin.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WAVELEFTPADX  20

// CFdms2View_View

IMPLEMENT_DYNCREATE(CFdms2View_View, CView)

BEGIN_MESSAGE_MAP(CFdms2View_View, CView)
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
    ON_WM_LBUTTONDOWN()
    ON_WM_ERASEBKGND()
    ON_WM_CREATE()
    ON_COMMAND(ID_VIEW_DB, &CFdms2View_View::OnViewDb)
    ON_COMMAND(ID_ZOOMY_MAX, OnZoomyMax)
END_MESSAGE_MAP()

// CFdms2View_View construction/destruction

CFdms2View_View::CFdms2View_View():m_ViewMode(0),m_ValMode(0), m_yRulerBottom(0)
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
        m_WaveArea[i].setChannel(i);
	}
}
void CFdms2View_View::updateDisplay(){
    CFdms2View_Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
    fdms2pos posCursor= pDoc->getPosEditCursor();
    if ((posCursor < m_PosDisplayStart)||(m_PosDisplayStop < posCursor)){
        m_PosDisplayStart=posCursor;
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
    CRect r;	
	CRect RectCh;
    
    this->GetClientRect(&r);
	int dx=r.Width()-WAVELEFTPADX;
	int dy=r.Height() - (m_yRulerBottom+20);
    m_my=(dy-20)/18;
    m_PosDisplayStop=getDisplayPos(dx);
    updateData();
    
	for (int i=0; i<8; i++){
        m_WaveArea[i].registerDoc(pDoc);  //TODO: check if its need or not?
        m_WaveArea[i].setData(getDisplayStart(), getPeekMaxLength());
		int y=m_yRulerBottom+10+i*m_my*2;
		RectCh.top=y+3;
		RectCh.bottom=y+m_my*2;
		RectCh.left=WAVELEFTPADX;
		RectCh.right=dx+WAVELEFTPADX;
		m_WaveArea[i].setDC(pDC);
        m_WaveArea[i].setRect(&RectCh);
	}
    m_MapArea.registerDoc(pDoc);
    m_MapArea.setDC(pDC);
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
    m_yRulerBottom=1;
    switch (pDoc->m_DisplayMode){
        case 1:
	        OnDrawRawVUs(&m_DCTmp);
	        OnDrawOrdinata(&m_DCTmp);
	        OnDrawRegion(&m_DCTmp);
            break;
        case 0:
	        OnDrawVUs(&m_DCTmp);
	        OnDrawOrdinata(&m_DCTmp);
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
	int ny=0;
	int my=(dy-10)/18-5;
	int y=5;
	int x=10;
	pDC->FillSolidRect(rect.left, y, dx,16*3,crBack);
	pDC->SetBkColor(crBack);
    //bytepos 
	for (int i=0;i<8; i++){
		int xx=i*dx/8;
        char * str=NULL;
        getDisplayPos(xx).dumpTimeStrHMSF(str);
        CString sstr;
		sstr.Format(L"%S", str);
        OnDrawOneTimeStamp(pDC, rect.left+xx , y, sstr);
        free(str);
	}
	y+=16;
	int olds=-1;
	fdms2pos ps;
    //time
	for (int i=dx; i>0; i--){
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
	}
	y+=16;
	olds=-1;
    //frame
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
	pDC->MoveTo(x,y);
	pDC->LineTo(x,y+14);
}

void CFdms2View_View::OnDrawMouse(CDC* pDC){
    if (!pDC) return;
//	COLORREF crBack=RGB(0xd0, 0xd0, 0xd0);
	COLORREF crBack=RGB(0xFF, 0xFF, 0xFF);
	COLORREF crFore=RGB(0x00, 0x00, 0x00);
	for (int i=4 ; i; i--){
		pDC->SetPixel(m_pointMouseOld.x,i,crBack);
		pDC->SetPixel(m_pointMouse.x,i,crFore);
		pDC->SetPixel(i,m_pointMouseOld.y,crBack);
		pDC->SetPixel(i,m_pointMouse.y,crFore);
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
    CPen penLine2(PS_SOLID, 2, RGB(0, 0x70,0));
		
	int x1=rect.left + (pDoc->m_PosRegionStart.m_Sample-m_PosDisplayStart.m_Sample)/pDoc->m_DisplayXMul;
	int x2=rect.left + (pDoc->m_PosRegionStop.m_Sample-m_PosDisplayStart.m_Sample)/pDoc->m_DisplayXMul;
	int xp=rect.left + (pDoc->m_PosEditCursor.m_Sample-m_PosDisplayStart.m_Sample)/pDoc->m_DisplayXMul;
	if (x2<x1){
		CPen penLine(PS_SOLID, 2, RGB(0xff, 0,0));
		CBrush brushLine(RGB(255, 0, 0));
		CPen* pOldPen = pDC->SelectObject(&penLine);
		CBrush* pOldBrush =pDC->SelectObject(&brushLine);
        int tmp=x1;
        x1=x2; x1=tmp;
	}
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
}
		
fdms2pos CFdms2View_View::getDisplayPos(int posx){
    UINT64 pos=m_PosDisplayStart.m_Sample+ (UINT64(posx)) * GetDocument()->m_DisplayXMul;
    return fdms2pos(pos*16);
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
		m_WaveArea[ch].DrawOneScale_dB();
		m_WaveArea[ch].DrawOneData(this);
	}
}
void CFdms2View_View::OnDrawRawVUs(CDC* pDC)
{
	for (int ch=0; ch<8; ch++){
		m_WaveArea[ch].DrawOneScale_dB();
		m_WaveArea[ch].DrawOneDataRaw(this);
	}
}
void CFdms2View_View::OnDrawDiskMap(CDC* pDC)
{
	CFdms2View_Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
    int iPrg= pDoc->getSelectedProgram();
    CRect rect;
	this->GetClientRect(rect);
	int dx=rect.right-rect.left-16;
	fdms2pos pLen;
	t1_toffset start;
	int ddx=pDoc->getMaxPos() / dx;
	if (ddx<1) ddx=1;
	int y=m_yRulerBottom+4;
	int iIdx=0;
	while(!pDoc->m_fdms2.getPart(iPrg, iIdx, start, pLen)){
		int x=start/ddx;
		int x2=(start+pLen.m_Pos)/ddx;
		CString s;
		s.Format(L"%i", iIdx);
		pDC->TextOut(x, y, s);
		iIdx++; y+=2; 
	}
	y=0;iIdx=0;
	while(!pDoc->m_fdms2.getPart(iPrg, iIdx, start, pLen)){
		int x=start/ddx;
		int x2=(start+pLen.m_Pos)/ddx;
		pDC->MoveTo(x2, 0+y);
		pDC->LineTo(x2,5+y);
		pDC->MoveTo(x, 1+y+(iIdx%4));
		pDC->LineTo(x2,1+y+(iIdx%4));
		pDC->MoveTo(x, 0+y);
		pDC->LineTo(x,5+y);
		iIdx++; y+=2;
	};

    for (int i=0; i<5; i++){
        iIdx=0;
        int x=0;
	    while(!pDoc->m_fdms2.getPart(i, iIdx, start, pLen)){
		    //int x=start/ddx;
		    int x2=x+(pLen.m_Pos)/ddx;
		    pDC->MoveTo(x2, 0+y);
		    pDC->LineTo(x2,5+y);
		    pDC->MoveTo(x, 1+y+(iIdx%4));
		    pDC->LineTo(x2,1+y+(iIdx%4));
		    pDC->MoveTo(x, 0+y);
		    pDC->LineTo(x,5+y);
		    iIdx++; x=x2+1;
	    }
        y+=15;
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
         m_PosDisplayStart.addPos( zDelta* pDoc->m_DisplayXMul);
         if (m_PosDisplayStart.m_Pos<0) m_PosDisplayStart.setPos(0);
    }
	Invalidate();
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
    INT64 s=((point.x-WAVELEFTPADX)* pDoc->m_DisplayXMul)+m_PosDisplayStart.m_Sample; //TODO: van rá függvény
    
	if (nFlags & MK_CONTROL){
		pDoc->m_PosRegionStart.setSample(s);
	}
	if (nFlags & MK_SHIFT){
		pDoc->m_PosRegionStop.setSample(s);
	}
    if ((nFlags & (MK_CONTROL | MK_SHIFT))==0){
        if (abs(point.x-m_pointMouseDown.x)>2){
            m_PosDisplayStart.addSample( -(point.x-m_pointMouseDown.x)* pDoc->m_DisplayXMul);
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
    m_pointMouseDown=point;
    __super::OnLButtonDown(nFlags, point);
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
    return log((double)abs(val))*32*1024/log((double)0x7FFF)*s;
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
