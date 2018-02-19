#include "fdms2.h"
class EmuFdms2 : public fdms2streamerIF{
public:
    EmuFdms2(){ emuReset();}
    virtual void reset(){iReset++;}
    virtual int start(){iStart++; return 0;}
    virtual int stop(){iStop++; return 0;}
    virtual fdms2streamerIF* duplicate(){
        iDuplicate++;
        EmuFdms2* p=new EmuFdms2();
        return p;
    }
    virtual int getValueArrays(t1_toffset pos, int samplenum, short** ptrArray){
        iGetValueArrays++;
        return samplenum;
    }
    virtual int convertLogical2Abs(int iPrg, fdms2pos posLogic, t1_toffset& iOffs, t1_toffset& iStart, t1_toffset& iMaxLen, int& iLastPart){
        iConvertLogical2Abs++;
        iOffs=0;
        iStart=0;
        iMaxLen=16384;
        iLastPart=0;
        return 0;
    }
    virtual int getPart(int iPrg, int iIdx, t1_toffset& riStart, fdms2pos& rpLen){
        iGetPart++;
        riStart=0;
        rpLen.setPos(16384);
        if (iIdx>0) return 1; //only one partition.
        return 0;
    }
    virtual void setWriteable(bool bWrite){
    }
    void emuReset(){
        iReset=iStart=iStop=iDuplicate=iGetValueArrays=iConvertLogical2Abs=iGetPart=0;
    }
    
private:
    int iReset, iStart, iStop, iDuplicate, iGetValueArrays, iConvertLogical2Abs,iGetPart;
};

 

