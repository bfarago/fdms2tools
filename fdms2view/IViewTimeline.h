/* Written by Barna Farago <brown@weblapja.com> 2006-2018
*/
#pragma once
#include "fdms2.h"

interface IViewTimeLine
{
public:
    virtual fdms2pos& getDisplayStart()=0;
    virtual fdms2pos getDisplayPos(int posx)=0;
    virtual void getDisplayStartPartOffs(int &iPart, INT64 &iOffs)=0;
    virtual bool getPeek(INT64 iOffs, unsigned int uiDiff, short iCh, short &iMax, short &iMin)=0;
    virtual fdms2pos getPeekMaxLength()=0;
};
