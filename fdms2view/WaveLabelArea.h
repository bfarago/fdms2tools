#pragma once
#include "area.h"
#include "fdms2view_app.h"
#include "fdms2view_doc.h"
#include "iviewtimeline.h"
#include "linestrip.h"

class CWaveLabelArea:public CArea{
public:
	CWaveLabelArea(void);
	~CWaveLabelArea(void);
	void DrawOne();
    virtual void Draw(IViewTimeLine *pTl);
    void OnLButtonUp(UINT nFlags, CPoint pointUp, CPoint pointDown);
    void OnRButtonUp(UINT nFlags, CPoint pointUp, CPoint pointDown);
    void registerLineStrip(CLineStrip *strip){
        m_lineStrip=strip;
    }
private:
    CLineStrip* m_lineStrip; 
	COLORREF m_ColorScale;
    CFont m_fontSmall;
	void DrawOneFader(CRect &r,CString label, int val);
};
