// DMetr.cpp : implementation file
//

#include "stdafx.h"
#include "fdms2view_app.h"
#include "DMetr.h"


// DMetr dialog

IMPLEMENT_DYNAMIC(DMetr, CDialog)
DMetr::DMetr(CWnd* pParent /*=NULL*/)
	: CDialog(DMetr::IDD, pParent)
	, m_iMBar(1)
	, m_iMQ(1)
	, m_iM1(4)
	, m_iM2(4)
	, m_iTBar(1)
	, m_iTQ(1)
	, m_iTempo(120)
    , m_iMTCO_H(0)
    , m_iMTCO_M(0)
    , m_iMTCO_S(0)
    , m_iMTCO_F(0)
    , m_iMTCO_SF(0)
{

}

DMetr::~DMetr()
{
}

void DMetr::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_METR, m_lbMetr);
	DDX_Control(pDX, IDC_LIST_TEMPO, m_lbTempo);
	DDX_Control(pDX, IDC_BUTTON_MADD, m_bnMAdd);
	DDX_Control(pDX, IDC_BUTTON_TADD, m_bnTAdd);

	DDX_Text(pDX, IDC_EDIT_Q2, m_iTQ);
	DDV_MinMaxInt(pDX, m_iTQ, 1, 4);
	DDX_Text(pDX, IDC_EDIT_TEMPO, m_iTempo);
	DDV_MinMaxInt(pDX, m_iTempo, 0, 300);
	DDX_Text(pDX, IDC_EDIT1, m_iM1);
	DDV_MinMaxInt(pDX, m_iM1, 1, 15);
	DDX_Text(pDX, IDC_EDIT2, m_iM2);
	DDV_MinMaxInt(pDX, m_iM2, 1, 16);
	DDX_Text(pDX, IDC_EDIT_BAR, m_iTBar);
	DDV_MinMaxInt(pDX, m_iTBar, 1, 9999);
	DDX_Text(pDX, IDC_EDIT_MBAR, m_iMBar);
	DDV_MinMaxInt(pDX, m_iMBar, 1, 9999);
	DDX_Text(pDX, IDC_EDIT_Q, m_iMQ);
	DDV_MinMaxInt(pDX, m_iMQ, 1, 15);
    
    DDX_Text(pDX, IDC_EDIT_MTCO_H, m_iMTCO_H);
	DDV_MinMaxInt(pDX, m_iMTCO_H, 0, 24);
    DDX_Text(pDX, IDC_EDIT_MTCO_M, m_iMTCO_M);
	DDV_MinMaxInt(pDX, m_iMTCO_M, 0, 59);
    DDX_Text(pDX, IDC_EDIT_MTCO_S, m_iMTCO_S);
	DDV_MinMaxInt(pDX, m_iMTCO_S, 0, 59);
    DDX_Text(pDX, IDC_EDIT_MTCO_F, m_iMTCO_F);
	DDV_MinMaxInt(pDX, m_iMTCO_F, 0, 30);
    DDX_Text(pDX, IDC_EDIT_MTCO_SF, m_iMTCO_SF);
	DDV_MinMaxInt(pDX, m_iMTCO_SF, 0, 99);
    
}


BEGIN_MESSAGE_MAP(DMetr, CDialog)
END_MESSAGE_MAP()


// DMetr message handlers

BOOL DMetr::OnInitDialog()
{
	CDialog::OnInitDialog();
	CString s;
	int iRet=0, iIdx=0;
	do {
		int iBar, iNum, iDen;
		iRet=m_fdms2->getMetrum(m_SelectedProgram, iIdx, iBar, iNum, iDen);
		if (!iRet){
			s.Format(L"%04dbar %i/%i", iBar, iNum, iDen);
			m_lbMetr.AddString(s);	
			iIdx++;
		}
	}while(!iRet);
	iIdx=0;
	do {
		int iBar, iBeat, iTempo;
		iRet=m_fdms2->getTempo(m_SelectedProgram, iIdx, iBar, iBeat, iTempo);
		if (!iRet){
			s.Format(L"%04dbar %ibeat tempo: %i", iBar, iBeat, iTempo);
			m_lbTempo.AddString(s);	
			iIdx++;
		}
	}while(!iRet);
    m_fdms2->getMtcOffset(m_SelectedProgram, m_iMTCO_H, m_iMTCO_M, m_iMTCO_S, m_iMTCO_F, m_iMTCO_SF);
    UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
}
