// fdms2view_doc.h : interface of the CFdms2View_Doc class
//
//\dx\samples\VC70\common\$(OutDir)\DXUT.lib

#pragma once
#include <fdms2lib.h>
#include <mmsystem.h>
#include "peekcache.h"
#include "Mixer.h"
#include "DMix1.h"
#include "DGrab.h"
class CFdms2View_View;
#define MAXXMUL (16*1024*1024)


class CFdms2View_Doc : public CDocument, public ICacheUser, public fdms2notifyIF
{
protected: // create from serialization only
	CFdms2View_Doc();
	DECLARE_DYNCREATE(CFdms2View_Doc)
	CString m_csFileName;
	fdms2 m_fdms2;
    fdms2 m_waveFdms2;

    PeekCache m_cache;
	bool m_bPlayable;
	//bool m_bPlayNow;
	bool m_bRecordable;
	int m_DisplayMode;

//ezek a viewba át
    int m_DisplayLevelValue;
    double m_DisplayXMul;
    void addDisplayXZoom(CFdms2View_View* pView, short zdelta);
//eddig

    CPlayer* m_player; //Applicationtol.
    CMixer m_mixer;
    DMix1 *m_MixConsole;
// Attributes
public:
    bool m_bRedraw;
	
	fdms2pos m_PosEditCursor;
	fdms2pos m_PosRegionStart;
	fdms2pos m_PosRegionStop;
//    fdms2pos m_markers[10];
    void DoOrderRegion();
    void ValidatePos(fdms2pos& p);
    virtual void CacheUpdated(){m_bRedraw=true;};
    void setSelectedProgram(int iPrg);
    int getSelectedProgram(){return m_SelectedProgram;}
    bool getPlayNow(){
        if (m_player) return m_player->getPlayNow();
        return false;
    }
    void getLineStrip(unsigned int index, CLineStrip*& strip){
        if (index>=FOSTEXMAXCHANNELS)return;
        m_mixer.getLineStrip(index, strip);
    }
// Operations
public:
	void SetFileName(const char* sFileName);
	void SetFileName(const CString& sFileName);
    void setPosEditCursor(fdms2pos p);
    fdms2pos getPosEditCursor();
    UINT64  getMaxPos();
    
    void setPlayer(CPlayer* player){ m_player=player;}
	void OnPlay();
	void OnPlayStart();
	void OnPlayStop();
    void OnGrab();
// Overrides
public:
    virtual void messageBox(LPCTSTR cat, LPCTSTR msg);
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
    void initPlayer();
    void killPlayer();
// Implementation
public:
	virtual ~CFdms2View_Doc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    int m_SelectedProgram;
    bool isRegionSelected();
    void UpdateDisplay();
    void UpdateCursors();
// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnViewFdms2properties();
	afx_msg void OnViewTempomap();
	afx_msg void OnViewProgram1();
	afx_msg void OnViewProgram2();
	afx_msg void OnViewProgram3();
	afx_msg void OnViewProgram4();
	afx_msg void OnViewProgram5();
	afx_msg void OnViewFragmentmap();
	afx_msg void OnExportMultiTrack();
	afx_msg void OnTPRecord();
	afx_msg void OnTPStop();
	afx_msg void OnTPPlay();
	afx_msg void OnTPRew();
	afx_msg void OnTPFF();
    afx_msg void OnTPBegin();
	afx_msg void OnTPEnd();
	afx_msg void OnZoom1();
	afx_msg void OnZoom2();
	afx_msg void OnZoomFull();
	afx_msg void OnZoomy12db();
	afx_msg void OnZoomy24db();
	afx_msg void OnViewMix();
	afx_msg void OnEditPlay();
	afx_msg void OnEditStop();
	afx_msg void OnDeviceSoundcard();
    afx_msg void OnViewMode();
    afx_msg void OnUpdateViewProgram1(CCmdUI *pCmdUI);
	afx_msg void OnUpdateExportEightwav(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTPRecord(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTPStop(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTPPlay(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTPRew(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTPFF(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTPBegin(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTPEnd(CCmdUI *pCmdUI);
	afx_msg void OnUpdateViewMix(CCmdUI *pCmdUI);
	afx_msg void OnUpdateEditPlay(CCmdUI *pCmdUI);
	afx_msg void OnUpdateEditStop(CCmdUI *pCmdUI);
	afx_msg void OnUpdateDeviceSoundcard(CCmdUI *pCmdUI);
	afx_msg void OnUpdateViewMode(CCmdUI *pCmdUI);
    afx_msg void OnEditPlayToggle();
    afx_msg void OnViewRecreatepeekfile();
    afx_msg void OnUpdateDeviceQuickformat(CCmdUI *pCmdUI);
    afx_msg void OnDeviceQuickformat();
};
