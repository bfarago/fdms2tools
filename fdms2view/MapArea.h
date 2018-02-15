#pragma once

#include "area.h"

class CMapArea: public CArea{
public:
    CMapArea(void);
    ~CMapArea(void);
    void Draw(IViewTimeLine *pTl);
};
