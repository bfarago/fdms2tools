#pragma once

#include "area.h"

class CMapArea: public CArea{
public:
    CMapArea(void);
    ~CMapArea(void);
    void Draw(IViewTimeLine *pTl);
    void OnLButtonUp(UINT nFlags, CPoint pointUp, CPoint pointDown);
    void OnRButtonUp(UINT nFlags, CPoint pointUp, CPoint pointDown);
};
