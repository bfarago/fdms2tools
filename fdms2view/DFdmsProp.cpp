// DFdmsProp.cpp : implementation file
//

#include "stdafx.h"
#include "fdms2view_app.h"
#include "DFdmsProp.h"


// DFdmsProp dialog

IMPLEMENT_DYNAMIC(DFdmsProp, CDialog)
DFdmsProp::DFdmsProp(CWnd* pParent /*=NULL*/)
	: CDialog(DFdmsProp::IDD, pParent)
    , m_csFileName(_T("")), m_pm(NormalPartitionMode), m_iRadioPM(0)
{
}

DFdmsProp::~DFdmsProp()
{
}

void DFdmsProp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_FileName, m_csFileName);
    DDX_Radio(pDX, IDC_RADIO1, m_iRadioPM);
}


BEGIN_MESSAGE_MAP(DFdmsProp, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_FN_Browse, OnBnClickedButtonFnBrowse)
END_MESSAGE_MAP()


// DFdmsProp message handlers

void DFdmsProp::OnBnClickedButtonFnBrowse()
{
    UpdateData(TRUE);
	
#ifndef UNICODE
	static char BASED_CODE szFilter[] =
#else
	static TCHAR szFilter[] =
#endif
    	L"FDMS2 Image(*.img)|*.img|All Files (*.*)|*.*||";
	CFileDialog* dlg=new CFileDialog(true, L"f2lproj",m_csFileName,0,szFilter,this);
	if( dlg->DoModal() == IDOK ){
        
      m_csFileName = dlg->GetPathName();
      UpdateData(FALSE);
    }
	delete dlg;
}

BOOL DFdmsProp::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_iRadioPM=(int)m_pm;	  
    UpdateData(FALSE);
    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void DFdmsProp::OnOK()
{
    UpdateData(TRUE);
    m_pm=(enPartitionMode)m_iRadioPM;
    CDialog::OnOK();
}
