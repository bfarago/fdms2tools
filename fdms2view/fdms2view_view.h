// fdms2view_view.h : interface of the CFdms2View_View class
//


#pragma once
#include "maparea.h"
#include "wavearea.h"
#include "wavelabelarea.h"
#include "iviewtimeline.h"


class CFdms2View_View : public CView, public IViewTimeLine
{
protected: // create from serialization only
	CFdms2View_View();
	DECLARE_DYNCREATE(CFdms2View_View)

// Attributes
public:
	CFdms2View_Doc* GetDocument() const;

// Operations
public:
    virtual fdms2pos& getDisplayStart(){ return m_PosDisplayStart; }
    virtual fdms2pos& getDisplayStop(){ return m_PosDisplayStop; }
    virtual void     getDisplayStartPartOffs(int &iPart, INT64 &iOffs);
    virtual fdms2pos getDisplayPos(int posx);
    virtual bool getPeek(INT64 iOffs, unsigned int uiDiff, short iCh, short &iMax, short &iMin);
    virtual fdms2pos getPeekMaxLength();
    void updateDisplay(bool bInvalidate=false);
    signed short val2db(signed short val);
	void RedrawCursors();
	
// Overrides
public:
    virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	void OnDrawOrdinata(CDC* pDC);
	void OnDrawDiskMap(CDC* pDC);
	void OnDrawVUs(CDC* pDC);
    void OnDrawRawVUs(CDC* pDC);
	void OnDrawOneTimeStamp(CDC* pDC, int x, int y, const CString &str);
	void OnDrawOneData(CDC* pDC, int iCh, int y, int dx, int my);
	void OnDrawOneScaledB(CDC* pDC, int iCh, int y, int dx, int my);
    void OnDrawMouse(CDC* pDC);
	void OnDrawRegion(CDC* pDC);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	
protected:
    fdms2pos m_PosDisplayStart;
	fdms2pos m_PosDisplayStop;
    int m_my;
    int m_ViewMode;
    int m_ValMode;
	CWaveArea m_WaveArea[8];
    CWaveLabelArea m_WaveLabelArea[8];
    CMapArea m_MapArea;
	CPoint m_pointMouse;
	CPoint m_pointMouseOld;
    CPoint m_pointLMouseDown;
    CPoint m_pointRMouseDown;
    CRect m_prevCursor;
    int m_yRulerBottom;
    CDC m_DCTmp;
    CBitmap m_BitmapTmp;
    void updateData();
	void InitWaveAreas();
	void StartWaveAreas(CDC* pDC);
    void ResizeAreas();
	void StopWaveAreas();
// Implementation
public:
	virtual ~CFdms2View_View();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnViewDb();
    afx_msg void OnZoomyMax();
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};

#ifndef _DEBUG  // debug version in testermfc_fdms2libView.cpp
inline CFdms2View_Doc* CFdms2View_View::GetDocument() const
    { 
       ASSERT_VALID(m_pDocument);
       return reinterpret_cast<CFdms2View_Doc*>(m_pDocument);
    }
#endif

