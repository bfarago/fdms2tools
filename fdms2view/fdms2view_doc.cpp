/* Written by Barna Farago <brown@weblapja.com> 2006-2018
*
*/
// testermfc_fdms2libDoc.cpp : implementation of the CFdms2View_Doc class
//

#include "stdafx.h"
#include "fdms2view_app.h"
#include "fdms2view_doc.h"
#include "fdms2view_view.h"
#include "DFdmsProp.h"
#include "DMetr.h"
#include "DFrag.h"
#include "DExport.h"
#include "DAudioDeviceSetup.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DEFAULTBUFFSIZE 2048
#define DEFAULTDisplayXMul 3000
#define DEFAULTDisplayLevelValue (0x3ff)

// CFdms2View_Doc

IMPLEMENT_DYNCREATE(CFdms2View_Doc, CDocument)

BEGIN_MESSAGE_MAP(CFdms2View_Doc, CDocument)
	ON_COMMAND(ID_VIEW_FDMS2PROPERTIES, OnViewFdms2properties)
	ON_COMMAND(ID_VIEW_TEMPOMAP, OnViewTempomap)
	ON_COMMAND(ID_VIEW_PROGRAM1, OnViewProgram1)
	ON_COMMAND(ID_VIEW_PROGRAM2, OnViewProgram2)
	ON_COMMAND(ID_VIEW_PROGRAM3, OnViewProgram3)
	ON_COMMAND(ID_VIEW_PROGRAM4, OnViewProgram4)
	ON_COMMAND(ID_VIEW_PROGRAM5, OnViewProgram5)
	ON_COMMAND(ID_VIEW_FRAGMENTMAP, OnViewFragmentmap)
	ON_COMMAND(ID_VIEW_MIX, OnViewMix)
	ON_COMMAND(ID_VIEW_MODE, OnViewMode)
	ON_COMMAND(ID_VIEW_RECREATEPEEKFILE, &CFdms2View_Doc::OnViewRecreatepeekfile)
    ON_COMMAND(ID_EDIT_PLAY, OnEditPlay)
	ON_COMMAND(ID_EDIT_STOP, OnEditStop)
    ON_COMMAND(ID_EDIT_PLAY_TOGGLE, OnEditPlayToggle)
    ON_COMMAND(ID_EXPORT_MULTITRACK, OnExportMultiTrack)
    ON_COMMAND(ID_ZOOM_1, OnZoom1)
	ON_COMMAND(ID_ZOOM_2, OnZoom2)
	ON_COMMAND(ID_ZOOM_FULL, OnZoomFull)
	ON_COMMAND(ID_ZOOMY_12DB, OnZoomy12db)
	ON_COMMAND(ID_ZOOMY_24DB, OnZoomy24db)
	ON_COMMAND(ID_DEVICE_SOUNDCARD, OnDeviceSoundcard)
    ON_COMMAND(ID_DEVICE_GRAB, OnGrab)
    ON_COMMAND(ID_BUTTON_TP_REC,	OnTPRecord)
	ON_COMMAND(ID_BUTTON_TP_STOP,	OnTPStop)
	ON_COMMAND(ID_BUTTON_TP_PLAY,	OnTPPlay)
	ON_COMMAND(ID_BUTTON_TP_REW,	OnTPRew)
	ON_COMMAND(ID_BUTTON_TP_FF,		OnTPFF)
	ON_COMMAND(ID_BUTTON_TP_BEGIN,	OnTPBegin)
	ON_COMMAND(ID_BUTTON_TP_END,	OnTPEnd)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PROGRAM1, OnUpdateViewProgram1)
	ON_UPDATE_COMMAND_UI(ID_EXPORT_EIGHTWAV, OnUpdateExportEightwav)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_TP_REC, OnUpdateTPRecord)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_TP_STOP, OnUpdateTPStop)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_TP_PLAY, OnUpdateTPPlay)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_TP_REW, OnUpdateTPRew)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_TP_FF, OnUpdateTPFF)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_TP_BEGIN, OnUpdateTPBegin)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_TP_END, OnUpdateTPEnd)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MIX, OnUpdateViewMix)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PLAY, OnUpdateEditPlay)
	ON_UPDATE_COMMAND_UI(ID_EDIT_STOP, OnUpdateEditStop)
	ON_UPDATE_COMMAND_UI(ID_DEVICE_SOUNDCARD, OnUpdateDeviceSoundcard)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MODE, &CFdms2View_Doc::OnUpdateViewMode)
    ON_UPDATE_COMMAND_UI(ID_DEVICE_QUICKFORMAT, &CFdms2View_Doc::OnUpdateDeviceQuickformat)
    ON_COMMAND(ID_DEVICE_QUICKFORMAT, &CFdms2View_Doc::OnDeviceQuickformat)
	ON_COMMAND(ID_EDIT_SELECT_ALL, &CFdms2View_Doc::OnEditSelectall)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, &CFdms2View_Doc::OnUpdateEditSelectall)
END_MESSAGE_MAP()

// CFdms2View_Doc construction/destruction

CFdms2View_Doc::CFdms2View_Doc():m_DisplayMode(0),m_bRedraw(false)
{
	m_SelectedProgram=0;
	m_bPlayable=false;
	m_bRecordable=false;
	m_MixConsole=NULL;
    m_player=CPlayer::getInstance();
}

CFdms2View_Doc::~CFdms2View_Doc()
{
    killPlayer();
    if (m_MixConsole) m_MixConsole->registerMixer(NULL);
	if (m_player) m_player->setFdms2(NULL);
	m_bPlayable=false;
	m_bRecordable=false;
    m_fdms2.stop();
}

BOOL CFdms2View_Doc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
    m_player=CPlayer::getInstance();
	m_bPlayable=false;
	m_bRecordable=false;
	m_DisplayXMul=DEFAULTDisplayXMul;
	m_DisplayLevelValue=DEFAULTDisplayLevelValue;
	return TRUE;
}




// CFdms2View_Doc serialization

void CFdms2View_Doc::Serialize(CArchive& ar)
{
	CString csVersion=L"1.0.0.7";
	if (ar.IsStoring())
	{
        
		ar << csVersion;
		ar << m_csFileName;
        ar << (int)m_fdms2.getPartitionMode();
        ar << m_SelectedProgram;
        ar << m_PosEditCursor.m_Sample;
        ar << m_PosRegionStart.m_Sample;
        ar << m_PosRegionStop.m_Sample;
        m_mixer.Serialize(ar);
        SetModifiedFlag(FALSE);
	}
	else
	{
		CString csFileName;
        int iPM=0;
        INT64 i64CursorPos=0;
        INT64 i64RegionStart=0;
        INT64 i64RegionStop=0;
        try{
		    ar >> csVersion;
		    ar >> csFileName;
            ar >> iPM;
            ar >> m_SelectedProgram;
            ar >> i64CursorPos;
            ar >> i64RegionStart;
            ar >> i64RegionStop;
            m_mixer.Serialize(ar);
        }catch(...){
        }
        m_fdms2.setPartitionMode( (enPartitionMode) iPM);
		m_DisplayXMul=DEFAULTDisplayXMul;
		m_DisplayLevelValue=DEFAULTDisplayLevelValue;
		SetFileName(csFileName);
        if (m_MixConsole){
            m_MixConsole->registerMixer(&m_mixer);
            m_MixConsole->DoSetFaders();
            m_MixConsole->DoSetVumeters();
        }
        m_PosEditCursor.setSample(i64CursorPos);
        m_PosRegionStart.setSample(i64RegionStart);
        m_PosRegionStop.setSample(i64RegionStop);
        m_player=CPlayer::getInstance();
        m_player->setFdms2(&m_fdms2);
        m_player->setMixer(&m_mixer);
        m_player->setVUMeters(m_MixConsole);
        m_player->setCursor(m_PosEditCursor);
        m_player->setSelectedProgram(m_SelectedProgram);
        SetModifiedFlag(FALSE);
	}
}

void CFdms2View_Doc::setSelectedProgram(int iPrg){ 
        m_SelectedProgram=iPrg;
        m_player->setSelectedProgram(m_SelectedProgram);
        //setPosEditCursor(0);
        SetModifiedFlag(TRUE);
}
void CFdms2View_Doc::ValidatePos(fdms2pos& p){
    t1_toffset m= m_fdms2.getProgramSampleCount(m_SelectedProgram);
    if (p.m_Sample > m) p.setSample(m);
}
void CFdms2View_Doc::setPosEditCursor(fdms2pos p){
    m_PosEditCursor=p;
    ValidatePos(m_PosEditCursor);
    m_player->setSelectedProgram(m_SelectedProgram);
    m_player->setCursor(m_PosEditCursor);
}
fdms2pos CFdms2View_Doc::getPosEditCursor(){
    if (getPlayNow()){
        m_PosEditCursor=m_player->getCursor();
    }
    return m_PosEditCursor;
}
// CFdms2View_Doc diagnostics

#ifdef _DEBUG
void CFdms2View_Doc::AssertValid() const
{
	CDocument::AssertValid();
}

void CFdms2View_Doc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

void CFdms2View_Doc::SetFileName(const char* lpszFileName){
	//abstract
}
void CFdms2View_Doc::messageBox(LPCTSTR cat, LPCTSTR msg){
    wchar_t wCat[1024];
    wchar_t wMsg[1024];
    MultiByteToWideChar(CP_ACP, 0, (LPCSTR)cat, -1, wCat, 1024);
    MultiByteToWideChar(CP_ACP, 0, (LPCSTR)msg, -1, wMsg, 1024);
    CString s;
    s.Format(TEXT("%s: %s"), wCat, wMsg);
    ::MessageBox(NULL, s, TEXT("Error"), MB_OK | MB_ICONINFORMATION);
}
void CFdms2View_Doc::SetFileName(const CString & sFileName){
	m_bPlayable=false;
    m_fdms2.setNotify(this);
    m_fdms2.setWriteable(true);
	if (m_csFileName.Compare(sFileName)==0){
		//return;	//
	}
	m_csFileName=sFileName;
    char *sTmp=NULL;
#ifdef UNICODE
	CStringToCharBuff(m_csFileName, sTmp, 0);
    m_fdms2.setFileName(sTmp);
    free(sTmp);
#else
	m_fdms2.setFileName(m_csFileName.GetBuffer());
#endif
	m_fdms2.m_startpos=0;
	m_fdms2.m_length=1000;
	m_fdms2.m_step=10;
	int iRet=m_fdms2.start();
    if (!iRet){
	    m_bPlayable= m_fdms2.getDiskAudioSize()>10000;
		if (m_player) m_player->setFdms2(&m_fdms2);
	    m_bRecordable=false;
        m_cache.setFdms2(&m_fdms2, m_SelectedProgram, this);
	    UpdateAllViews(NULL);
    }
}

// CFdms2View_Doc commands

void CFdms2View_Doc::OnViewFdms2properties()
{

	DFdmsProp *dlg = new DFdmsProp(0);
    dlg->m_pm=m_fdms2.getPartitionMode();
	dlg->m_csFileName= m_csFileName;
    dlg->setFdms2Disk(m_fdms2.getDisk());

	INT_PTR nRet = -1;
	nRet =dlg->DoModal();
	switch ( nRet )
	{
	case -1: 
		AfxMessageBox(L"Dialog box could not be created!");
		break;
	case IDABORT:
		// Do something
		break;
	case IDOK:
        m_fdms2.setFileName(NULL);
        m_fdms2.setPartitionMode(dlg->m_pm);
		SetFileName( dlg->m_csFileName );
        SetModifiedFlag(TRUE);
		break;
	case IDCANCEL:
		// Do something
		break;
	default:
		// Do something
		break;
	};
	delete dlg;
}

void CFdms2View_Doc::OnViewTempomap()
{
	DMetr* dlg = new DMetr(0);
	dlg->m_fdms2 = &m_fdms2;
	dlg->m_SelectedProgram = m_SelectedProgram;
	dlg->DoModal();
	delete dlg;
}

void CFdms2View_Doc::OnViewFragmentmap()
{
	DFrag* dlg = new DFrag(0);
	dlg->m_Fdms2 = &m_fdms2;
	dlg->m_SelectedProgram = m_SelectedProgram;
	dlg->DoModal();
	delete dlg;
}
void CFdms2View_Doc::OnViewProgram1()
{
    setSelectedProgram(0);
    m_cache.setFdms2(&m_fdms2, m_SelectedProgram, this);
	UpdateAllViews(NULL, 0);
}

void CFdms2View_Doc::OnViewProgram2()
{
	setSelectedProgram(1);
    m_cache.setFdms2(&m_fdms2, m_SelectedProgram, this);
	UpdateAllViews(NULL, 0);
}

void CFdms2View_Doc::OnViewProgram3()
{
	setSelectedProgram(2);
    m_cache.setFdms2(&m_fdms2, m_SelectedProgram, this);
	UpdateAllViews(NULL, 0);
}

void CFdms2View_Doc::OnViewProgram4()
{
	setSelectedProgram(3);
    m_cache.setFdms2(&m_fdms2, m_SelectedProgram, this);
	UpdateAllViews(NULL, 0);
}

void CFdms2View_Doc::OnViewProgram5()
{
	setSelectedProgram(4);
    m_cache.setFdms2(&m_fdms2, m_SelectedProgram, this);
	UpdateAllViews(NULL, 0);
}

void CFdms2View_Doc::OnUpdateViewProgram1(CCmdUI *pCmdUI)
{
	if (!pCmdUI) return;
	if (!pCmdUI->m_pMenu) return;
	pCmdUI->m_pMenu->CheckMenuItem(ID_VIEW_PROGRAM1, MF_UNCHECKED);
	pCmdUI->m_pMenu->CheckMenuItem(ID_VIEW_PROGRAM2, MF_UNCHECKED);
	pCmdUI->m_pMenu->CheckMenuItem(ID_VIEW_PROGRAM3, MF_UNCHECKED);
	pCmdUI->m_pMenu->CheckMenuItem(ID_VIEW_PROGRAM4, MF_UNCHECKED);
	pCmdUI->m_pMenu->CheckMenuItem(ID_VIEW_PROGRAM5, MF_UNCHECKED);
	switch (m_SelectedProgram){
		case 0:
			pCmdUI->m_pMenu->CheckMenuItem(ID_VIEW_PROGRAM1, MF_CHECKED); break;
		case 1:
			pCmdUI->m_pMenu->CheckMenuItem(ID_VIEW_PROGRAM2, MF_CHECKED); break;
		case 2:
			pCmdUI->m_pMenu->CheckMenuItem(ID_VIEW_PROGRAM3, MF_CHECKED); break;
		case 3:
			pCmdUI->m_pMenu->CheckMenuItem(ID_VIEW_PROGRAM4, MF_CHECKED); break;
		case 4:
			pCmdUI->m_pMenu->CheckMenuItem(ID_VIEW_PROGRAM5, MF_CHECKED); break;

	}
}


void CFdms2View_Doc::OnExportMultiTrack()
{
	DExport *dlg = new DExport(0);
	dlg->registerFdms2(&m_fdms2);
    dlg->registerDoc(this);

	INT_PTR nRet = -1;
	nRet =dlg->DoModal();
	switch ( nRet )
	{
	case -1: 
		AfxMessageBox(L"Dialog box could not be created!");
		break;
	case IDABORT:
		// Do something
		break;
	case IDOK:
		// Do something
		break;
	case IDCANCEL:
		// Do something
		break;
	default:
		// Do something
		break;
	};
	delete dlg;
}
bool CFdms2View_Doc::isRegionSelected(){
    return m_PosRegionStart.m_Sample != m_PosRegionStop.m_Sample;
}
void CFdms2View_Doc::OnUpdateExportEightwav(CCmdUI *pCmdUI){
	BOOL bEn=	m_bPlayable && (isRegionSelected());
	pCmdUI->Enable(bEn);
}
void CFdms2View_Doc::OnGrab(){
	DGrab *dlg=new DGrab(NULL);
    if( dlg->DoModal() == IDOK ){
        CString s;
        dlg->getFileName(s);
        SetFileName(s);
      //m_csFileName = dlg->GetPathName();
    }
	delete dlg;
}

//---------------------------------------------------------------
//Transport
//---------------------------------------------------------------

void CFdms2View_Doc::OnTPRecord(){
}
void CFdms2View_Doc::OnTPStop(){
	OnPlayStop();
}
void CFdms2View_Doc::OnTPPlay(){
	OnPlayStart();
}
void CFdms2View_Doc::OnEditPlayToggle()
{
    if (m_bPlayable){
        if (getPlayNow()){
            OnPlayStop();
        }else{
            OnPlayStart();
        }
    }
}

void CFdms2View_Doc::OnTPRew(){
    m_PosEditCursor.addSample(-3*44100);
    setPosEditCursor(m_PosEditCursor); //TODO: refactoring needed.
    UpdateDisplay();
}
void CFdms2View_Doc::OnTPFF(){
    m_PosEditCursor.addSample(3*44100);
    setPosEditCursor(m_PosEditCursor); //TODO: refactoring needed.
    UpdateDisplay();
}
void CFdms2View_Doc::OnTPBegin(){
    m_PosEditCursor.setSample(0);
    setPosEditCursor(m_PosEditCursor); //TODO: refactoring needed.
    UpdateDisplay();
}
void CFdms2View_Doc::OnTPEnd(){
    m_PosEditCursor.setSample(m_fdms2.getProgramSampleCount(m_SelectedProgram)-1);
    setPosEditCursor(m_PosEditCursor); //TODO: refactoring needed.
    UpdateDisplay();
}
void CFdms2View_Doc::OnUpdateTPRecord(CCmdUI *pCmdUI){
	BOOL bEn=	m_bRecordable;
	pCmdUI->Enable(bEn);
}
void CFdms2View_Doc::OnUpdateTPStop(CCmdUI *pCmdUI){
    m_player->OnUpdateCmdIfPlayNow(pCmdUI);
}
void CFdms2View_Doc::OnUpdateTPPlay(CCmdUI *pCmdUI){
	m_player->OnUpdateCmdIfPlayableButNotPlayNow(pCmdUI);
}
void CFdms2View_Doc::OnUpdateTPRew(CCmdUI *pCmdUI){
    m_player->OnUpdateCmdIfPlayable(pCmdUI);
}
void CFdms2View_Doc::OnUpdateTPFF(CCmdUI *pCmdUI){
    m_player->OnUpdateCmdIfPlayable(pCmdUI);
}
void CFdms2View_Doc::OnUpdateTPBegin(CCmdUI *pCmdUI){
	m_player->OnUpdateCmdIfPlayable(pCmdUI);
}
void CFdms2View_Doc::OnUpdateTPEnd(CCmdUI *pCmdUI){
	m_player->OnUpdateCmdIfPlayable(pCmdUI);
}
void CFdms2View_Doc::OnEditPlay(){
	OnPlayStart();
}
void CFdms2View_Doc::OnEditStop(){
	OnPlayStop();
}
void CFdms2View_Doc::OnUpdateEditPlay(CCmdUI *pCmdUI){
    m_player->OnUpdateCmdIfPlayableButNotPlayNow(pCmdUI);
}
void CFdms2View_Doc::OnPlayStart(){
    SetModifiedFlag(TRUE);
    initPlayer();
}
void CFdms2View_Doc::OnPlayStop(){
    killPlayer();
}

void CFdms2View_Doc::OnUpdateEditStop(CCmdUI *pCmdUI){
    m_player->OnUpdateCmdIfPlayNow(pCmdUI);
}
void CFdms2View_Doc::UpdateCursors(){
    POSITION pos=GetFirstViewPosition();
    while (pos != NULL){
      CView* pView = GetNextView(pos);
	  CFdms2View_View* p=(CFdms2View_View*)pView;
	  p->RedrawCursors();
   }
}
void CFdms2View_Doc::UpdateDisplay(){
    POSITION pos=GetFirstViewPosition();
    while (pos != NULL){
      CView* pView = GetNextView(pos);
	  CFdms2View_View* p=(CFdms2View_View*)pView;
	  p->updateDisplay();
      p->RedrawCursors();
   }
}
void CFdms2View_Doc::OnPlay(){
	if (!getPlayNow()) return;
    getPosEditCursor();
	if (m_PosEditCursor.m_Sample> m_fdms2.getProgramSampleCount(m_SelectedProgram)){
        OnPlayStop();
    }
    UpdateCursors();   
}
void CFdms2View_Doc::initPlayer(){
    m_player->setFdms2(&m_fdms2);
    m_player->setMixer(&m_mixer);
    m_player->setSelectedProgram(m_SelectedProgram);
    m_player->initPlayer();
}
void CFdms2View_Doc::killPlayer(){
    m_player->killPlayer();
}
//TODO: mire kell meg? // refactoring needed
UINT64  CFdms2View_Doc::getMaxPos(){
    INT64 len;
	t1_toffset start;
	UINT64 max=0;
	int iIdx=0;
	while(!m_fdms2.getPart(m_SelectedProgram, iIdx, start, len)){
		UINT64 dwTmp= start+len;
		if (max<dwTmp) max=dwTmp;
		iIdx++;
	}
	return max;
}
//---------------------------------------------------------------
//Zoom
//---------------------------------------------------------------

void CFdms2View_Doc::OnZoom1()
{
	m_DisplayXMul=1;
	UpdateAllViews(NULL, 0);
}

void CFdms2View_Doc::OnZoom2()
{
	m_DisplayXMul=44100;
	UpdateAllViews(NULL, 0);
}
void CFdms2View_Doc::addDisplayXZoom(CFdms2View_View* pView, short zDelta){
    double prevX= m_DisplayXMul;
    fdms2pos difDisp=pView->getDisplayStop()- pView->getDisplayStart();
    if (zDelta>=0){
			m_DisplayXMul=m_DisplayXMul*zDelta/50;
            if (m_DisplayXMul>MAXXMUL) m_DisplayXMul=MAXXMUL;
    }else{
		double r = (-zDelta) / 50;
		if (0.0 == r) r = 0.1;
		m_DisplayXMul= m_DisplayXMul /(r);
		if (m_DisplayXMul<1) m_DisplayXMul=1;
    }
    if (m_DisplayXMul != prevX){
        if (pView){
            pView->getDisplayStart().addSample((__int64)(-(m_DisplayXMul-prevX)*200));
        }
    }
}
void CFdms2View_Doc::OnZoomFull()
{
	m_DisplayXMul=(double)m_fdms2.getDiskAudioSize()/12800;
	UpdateAllViews(NULL, 0);
}

void CFdms2View_Doc::OnZoomy12db()
{
	m_DisplayLevelValue = 0x1FFF;
	UpdateAllViews(NULL, 0);
}

void CFdms2View_Doc::OnZoomy24db()
{
	m_DisplayLevelValue = 0x07FF;
	UpdateAllViews(NULL, 0);
}
//---------------------------------------------------------------
//Mix
//---------------------------------------------------------------

void CFdms2View_Doc::OnViewMix()
{
	if (m_MixConsole==NULL){
		m_MixConsole=new DMix1();
	}
	if(m_MixConsole != NULL){
		if (m_MixConsole->m_hWnd == NULL){
			BOOL ret = m_MixConsole->Create(IDD_DIALOG_MIX1);
			if(!ret)   //Create failed.
			AfxMessageBox(L"Error creating Mix1 Dialog");
		}
		m_MixConsole->ShowWindow(SW_SHOW);
        m_MixConsole->registerMixer(&m_mixer);
        m_player->setVUMeters(m_MixConsole);
        m_MixConsole->DoReset();
   }
	
}

void CFdms2View_Doc::OnUpdateViewMix(CCmdUI *pCmdUI)
{
	BOOL bEn=	m_bPlayable;
	pCmdUI->Enable(bEn);
}
//---------------------------------------------------------------
//Device
//---------------------------------------------------------------

void CFdms2View_Doc::OnUpdateDeviceSoundcard(CCmdUI *pCmdUI)
{
    m_player->OnUpdateCmdIfPlayableButNotPlayNow(pCmdUI);
}

void CFdms2View_Doc::OnDeviceSoundcard()
{
	DAudioDeviceSetup* dlg= new DAudioDeviceSetup(0);
//	dlg->m_fdms2 = &m_fdms2;
	dlg->m_pDoc=this;
	dlg->DoModal();
	delete dlg;
}

void CFdms2View_Doc::OnViewMode()
{
    m_DisplayMode++;
    if (m_DisplayMode>1)m_DisplayMode=0;
    UpdateAllViews(NULL);
}

void CFdms2View_Doc::OnUpdateViewMode(CCmdUI *pCmdUI)
{
    BOOL bEn=	m_bPlayable;
    pCmdUI->Enable(bEn);
    //pCmdUI->DoUpdate(this,false);
}
void CFdms2View_Doc::DoOrderRegion(){
    if (m_PosRegionStart.m_Sample < m_PosRegionStop.m_Sample) return; //ok
    fdms2pos tmp= m_PosRegionStart;
    m_PosRegionStart= m_PosRegionStop;
    m_PosRegionStop=tmp;
}

void CFdms2View_Doc::OnViewRecreatepeekfile()
{
    m_cache.regenerate(this);
    m_bRedraw=true;
    UpdateAllViews(NULL);
    
}

void CFdms2View_Doc::OnUpdateDeviceQuickformat(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_fdms2.getDiskType()==DTDisk);
}

void CFdms2View_Doc::OnDeviceQuickformat()
{
   m_fdms2.quickFormat();
}


void CFdms2View_Doc::OnEditSelectall()
{
	// ID_EDIT_SELECTALL
	m_PosRegionStart.setSample(0);
	m_PosRegionStop.setSample(m_fdms2.getProgramSampleCount(m_SelectedProgram));
	UpdateCursors();
}


void CFdms2View_Doc::OnUpdateEditSelectall(CCmdUI *pCmdUI)
{
	BOOL bEn = m_bPlayable;
	pCmdUI->Enable(bEn);
}
