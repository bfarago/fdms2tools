#pragma once
#include "area.h"
#include "fdms2view_app.h"
#include "fdms2view_doc.h"
#include "iviewtimeline.h"

class CWaveArea:public CArea{
public:
	CWaveArea(void);
	~CWaveArea(void);
	void DrawOneData(IViewTimeLine *pTl);
    void DrawOneDataRaw(IViewTimeLine *pTl);
	void DrawOneScale_dB();
    virtual void Draw(IViewTimeLine *pTl);
    void OnLButtonUp(UINT nFlags, CPoint pointUp, CPoint pointDown){}
    void OnRButtonUp(UINT nFlags, CPoint pointUp, CPoint pointDown){}
    void setData(fdms2pos &displayPos, fdms2pos &displayMax){
        m_displayPos= displayPos;
        m_displayMax= displayMax;
    }
private:
	COLORREF m_ColorScale;
	void DrawError(int px, int py);
    void DrawSniped(int px, int py, short val);
    void DrawPeak(int px, int py, short iMax, short iMin);
    void DrawSeparator(int px, int py);
	void DrawYMarker(unsigned short valueY, const CString & str,
							bool bTopLeft, bool bBottomLeft,
							bool bTopRight, bool bBottomRight);
    fdms2pos m_displayPos;
    fdms2pos m_displayMax;
};
