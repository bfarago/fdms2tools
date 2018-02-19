// DExport.cpp : implementation file
//

#include "stdafx.h"
#include "fdms2view_app.h"
#include "DExport.h"
#include "Logoff.h"

#define MAXGFORMATS 4

struct tformat {
	char label[50];
	char ext[5];
	int	  code;
};
//Must be in ABC order.
tformat g_formats[MAXGFORMATS] = {
    {"Apple/SGI AIFF format (big endian)", "aiff",	SF_FORMAT_AIFF},
	{"Microsoft WAV format (little endian default)", "wav",		SF_FORMAT_WAV},
	{"RAW PCM data", "raw",		SF_FORMAT_RAW},
	{"Sun/NeXT AU format (big endian)",	"au",		SF_FORMAT_AU}
}
;
// DExport dialog

IMPLEMENT_DYNAMIC(DExport, CDialog)
DExport::DExport(CWnd* pParent /*=NULL*/)
	: CDialog(DExport::IDD, pParent)
	, m_sPrefix(_T(""))
    , m_bShutdown(FALSE),
    m_timeFirst(0), m_timeLast(0)
{
}

DExport::~DExport()
{
}
void DExport::registerFdms2(fdms2* pFdms2){
    m_Fdms2.copy(pFdms2);
    m_Fdms2.start();
}
void DExport::registerDoc(CFdms2View_Doc* pDoc){
    m_pDoc=pDoc;
}
void DExport::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    for (int i=0; i<FOSTEXMAXCHANNELS; i++)
        DDX_Control(pDX, IDC_CHECK1+i, m_bnCh[i]);
    DDX_Control(pDX, IDC_CHECKALL, m_bnChAll);
    DDX_Control(pDX, IDC_PROGRESS1, m_progress1);
    DDX_Text(pDX, IDC_EDIT_PREFIX, m_sPrefix);
    DDX_Text(pDX, IDC_OUT_LPOS, m_sOutLPos);
    DDX_Text(pDX, IDC_OUT_LPOS_REMAIN, m_sOutLPosRemain);
    DDX_Text(pDX, IDC_OUT_TIME_REMAIN, m_sOutTimeRemain);
    DDX_Control(pDX, IDC_COMBO1, m_cbFormat);
    DDX_CBIndex(pDX, IDC_COMBO1, m_iFormat);
    DDX_Check(pDX, IDC_CHECK_SHUTDOWN, m_bShutdown);
}


BEGIN_MESSAGE_MAP(DExport, CDialog)
	ON_BN_CLICKED(IDC_CHECKALL, OnBnClickedCheckall)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_WM_CREATE()
	ON_WM_ACTIVATE()
END_MESSAGE_MAP()


// DExport message handlers

void DExport::OnBnClickedCheckall()
{
	int iVal= m_bnChAll.GetCheck();
    for (int i=0; i<FOSTEXMAXCHANNELS; i++)
	    m_bnCh[i].SetCheck(iVal);
}
bool DExport::UpdateProgressBar(fdms2pos start, fdms2pos pos, fdms2pos len){
    MSG msg; 
    bool stop=false;
    fdms2pos progress;
    progress.setPos(pos.m_Pos);
    progress.addPos(-start.m_Pos);
	int pr = 0;
	if (len.m_Sample) pr=(int)((UINT64)(1000 * progress.m_Sample) / len.m_Sample);
    m_progress1.SetPos( pr );
    PeekMessage(&msg, m_hWnd,  0, 0, PM_REMOVE);
    switch(msg.message) 
    { 
		    case WM_LBUTTONDOWN: 
		    case WM_RBUTTONDOWN: 
		    case WM_KEYDOWN: 
			    stop = true; 
    } 
    fdms2pos p(len.m_Pos);
    p.addPos(-progress.m_Pos);
    char* psz=NULL;
    progress.dumpTimeStrHMSF(psz);
    m_sOutLPos=psz;
    p.dumpTimeStrHMSF(psz);
    m_sOutLPosRemain=psz;
    m_timeLast=GetTickCount();
    DWORD dift= m_timeLast-m_timeFirst;
    if (dift>0){
        DWORD v= progress.m_Pos / dift;
        if (v>0){
            DWORD rt= 16*p.m_Pos /v;
            p.setPos(rt);
        }
    }
    //p.setSample(dift*1000/44100);
    p.dumpTimeStrHMSF(psz);
    m_sOutTimeRemain=psz;
    free(psz);
    UpdateData(FALSE);
    m_progress1.Invalidate();
    
    return stop;
}
bool DExport::read(fdms2stream* pStream){
    if (!m_bRun) return false;
    for (int i=0; i<FOSTEXMAXCHANNELS; i++){
        if (m_bCh[i]){
            sf_write_short   (m_files[i], pStream->pBuff[i], pStream->iSamples);
        }
	}
    return true;
}

void DExport::OnBnClickedOk()
{
    m_timeFirst=m_timeLast=GetTickCount();
	bool stop=false;	
	int size=128;
    int counter=0;
	
    for (int i=0; i<FOSTEXMAXCHANNELS; i++){
        m_bCh[i] = m_bnCh[i].GetCheck()== BST_CHECKED;
	}
	m_progress1.SetRange(0,1000);
    UpdateData(true);
	OpenFiles();
    m_bRun=true;
    m_fdms2reader.registerFdms2(&m_Fdms2);
    m_fdms2reader.registerStreaming(this);
    m_fdms2reader.setBlockSizeSample(size);
    m_pDoc->DoOrderRegion();
    m_fdms2reader.setRegion(m_pDoc->getSelectedProgram(), m_pDoc->m_PosRegionStart, m_pDoc->m_PosRegionStop);
    m_fdms2reader.start();
    while( m_fdms2reader.isRunning()){
        Sleep(400);
        stop|=UpdateProgressBar(m_pDoc->m_PosRegionStart, m_fdms2reader.getActualPos(), m_fdms2reader.getRegionMax());
        m_bRun=!stop;
    }
    m_bRun=false;
    m_fdms2reader.stop();
	CloseFiles();
	OnOK();
    if (m_bShutdown){
        CLogoff lo;
        lo.Shutdown();
    }
}
void DExport::OpenFiles(){
	CString path;
    
    if ((m_iFormat<0)||(m_iFormat>MAXGFORMATS)) m_iFormat=1; //wav
	char* buf;
	for (int i=0; i<FOSTEXMAXCHANNELS; i++){
        if (m_bCh[i]){
			m_sfinfos[i].channels=1;
			m_sfinfos[i].frames=0;
			m_sfinfos[i].samplerate=44100;
			m_sfinfos[i].sections=0;
			m_sfinfos[i].seekable=1;
            m_sfinfos[i].format=SF_FORMAT_PCM_16|g_formats[m_iFormat].code;
            path.Format(L"%s%i.%S", m_sPrefix,i,g_formats[m_iFormat].ext);
			buf=NULL;
			CStringToCharBuff(path, buf,0);
			m_files[i]= sf_open(buf, SFM_WRITE, &m_sfinfos[i]);
			if (buf != NULL) free(buf);
		}
	}
}
void DExport::CloseFiles(){
	for (int i=0; i<FOSTEXMAXCHANNELS; i++){
        if (m_bCh[i]){
			sf_close(m_files[i]);
		}
	}
}
void DExport::InitDialog(){
	CString s;
	if (m_cbFormat.GetCount() < MAXGFORMATS) 
	    for (int i=0; i<MAXGFORMATS; i++){
		    s.Format(L"%S", g_formats[i].label);
		    m_cbFormat.AddString(s);
	    };
    m_cbFormat.SetCurSel(1);
    m_iFormat=1;
    m_pDoc->DoOrderRegion();
    UpdateProgressBar(m_pDoc->m_PosRegionStart, m_pDoc->m_PosRegionStart, m_pDoc->m_PosRegionStop);
}
int DExport::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	return 0;
}

void DExport::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CDialog::OnActivate(nState, pWndOther, bMinimized);
	InitDialog();
}
