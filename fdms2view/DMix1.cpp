// DMix1.cpp : implementation file
//

#include "stdafx.h"
#include "fdms2view_app.h"
#include "DMix1.h"
#include "math.h"

// DMix1 dialog

IMPLEMENT_DYNAMIC(DMix1, CDialog)
DMix1::DMix1(CWnd* pParent /*=NULL*/)
: CDialog(DMix1::IDD, pParent),m_bRun(false), m_pMixer(NULL)
{
 
}

DMix1::~DMix1()
{
}

void DMix1::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
    for (int i=0; i< 10; i++){
	    DDX_Control(pDX, IDC_PROGRESS1+i, m_pr[i]);
    }
    for (int i=0; i< 9; i++){
	    DDX_Control(pDX, IDC_SLIDER1+i, m_sl[i]);
    }
	m_bRun=true;
}


BEGIN_MESSAGE_MAP(DMix1, CDialog)
    ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER1, &DMix1::OnNMReleasedcaptureSliders)
    ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER2, &DMix1::OnNMReleasedcaptureSliders)
    ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER3, &DMix1::OnNMReleasedcaptureSliders)
    ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER4, &DMix1::OnNMReleasedcaptureSliders)
    ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER5, &DMix1::OnNMReleasedcaptureSliders)
    ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER6, &DMix1::OnNMReleasedcaptureSliders)
    ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER7, &DMix1::OnNMReleasedcaptureSliders)
    ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER8, &DMix1::OnNMReleasedcaptureSliders)
    ON_WM_TIMER()
END_MESSAGE_MAP()


// DMix1 message handlers
void DMix1::DoReset(){
	for (int i=0; i<10; i++){
		m_aVU[i]=0;
		m_pr[i].SetRange(0,(short)log((float)16384));
        if (i<9){
            //m_aGain[i]=16;
		    m_sl[i].SetRange(0,128);
            m_sl[i].SetPos( 16);
            //m_sl[i].SetPos( m_pMixer->getFaderChannel(i) );
        }
	}
	DoSetFaders();
    DoSetVumeters();
}
void DMix1::DoGet(){
    if (!m_bRun) return;
    if (!m_pMixer) return;
    for (int i=0; i<FOSTEXMAXCHANNELS; i++){
        m_pMixer->setFaderChannel(m_sl[i].GetPos(), i);
	}
    m_pMixer->setFaderMaster( m_sl[FOSTEXMAXCHANNELS].GetPos() );
}
void DMix1::DoSetFaders(){
    if (!m_pMixer) return;
    for (int i=0; i<FOSTEXMAXCHANNELS; i++){
		m_pr[i].SetRedraw(FALSE);
        m_sl[i].SetPos(m_pMixer->getFaderChannel(i));
		m_pr[i].SetRedraw(TRUE);
    }
}
void DMix1::DoSetVumeters(){
	if (m_hWnd==NULL) return;
	for (int i=0; i<10; i++){
		m_pr[i].SetRedraw(FALSE);
		m_pr[i].SetPos((int)log((float)m_aVU[i]));
        m_aVU[i]= m_aVU[i]>>1;
		m_pr[i].SetRedraw(TRUE);
	}
	Invalidate(FALSE);
}
void DMix1::registerMixer(CMixer* mixer){
    m_pMixer=mixer;
    Invalidate(FALSE);
}
void DMix1::OnNMReleasedcaptureSliders(NMHDR *pNMHDR, LRESULT *pResult)
{
    if (!m_pMixer) return ;
    int i= pNMHDR->idFrom - IDC_SLIDER1;
    m_pMixer->setFaderChannel(m_sl[i].GetPos(), i);
    *pResult = 0;
}


void DMix1::OnTimer(UINT_PTR nIDEvent)
{
    DoSetVumeters();

    CDialog::OnTimer(nIDEvent);
}

BOOL DMix1::OnInitDialog()
{
    CDialog::OnInitDialog();

    SetTimer(1003, 150, NULL);

    return TRUE;
}
