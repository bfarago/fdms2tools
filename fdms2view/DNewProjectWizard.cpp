// DNewProjectWizard.cpp : implementation file
//

#include "stdafx.h"
#include "DNewProjectWizard.h"


// DNewProjectWizard dialog

IMPLEMENT_DYNAMIC(DNewProjectWizard, CDialog)

DNewProjectWizard::DNewProjectWizard(CWnd* pParent /*=NULL*/)
	: CDialog(DNewProjectWizard::IDD, pParent)
    , m_iSelection(0)
{

}

DNewProjectWizard::~DNewProjectWizard()
{
}

void DNewProjectWizard::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
    DDX_Radio(pDX, IDC_RADIO1, m_iSelection);
}


BEGIN_MESSAGE_MAP(DNewProjectWizard, CDialog)
END_MESSAGE_MAP()


// DNewProjectWizard message handlers
