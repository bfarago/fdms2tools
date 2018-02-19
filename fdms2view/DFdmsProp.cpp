// DFdmsProp.cpp : implementation file
//

#include "stdafx.h"
#include "fdms2view_app.h"
#include "DFdmsProp.h"


// DFdmsProp dialog

IMPLEMENT_DYNAMIC(DFdmsProp, CDialog)
DFdmsProp::DFdmsProp(CWnd* pParent /*=NULL*/)
	: CDialog(DFdmsProp::IDD, pParent)
    , m_csFileName(_T("")), m_pm(NormalPartitionMode), m_iRadioPM(0),m_fdms2disk(NULL)
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
    DDX_Control(pDX, IDC_LIST_DISK, m_listDisks);
}


BEGIN_MESSAGE_MAP(DFdmsProp, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_FN_Browse, OnBnClickedButtonFnBrowse)
    ON_NOTIFY(LVN_ITEMACTIVATE, IDC_LIST_DISK, &DFdmsProp::OnLvnItemActivateListDisk)
    ON_BN_CLICKED(IDC_BUTTON_REFRESH, &DFdmsProp::OnBnClickedButtonRefresh)
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
void DFdmsProp::Refresh(){
    if (m_fdms2disk){
        enum { MAXIDS = 10};
        int nids=0;
        int ids[MAXIDS];
        nids=m_fdms2disk->getNumDisk(ids, MAXIDS, true);
        m_listDisks.DeleteAllItems();
        for (int i=0; i<nids; i++){
            int id=0;
            wchar_t pszInfo[100];
            m_fdms2disk->setDiskId(ids[i]);
            m_fdms2disk->start();
            m_fdms2disk->stop();
            swprintf_s(pszInfo, 100, TEXT("%S"), m_fdms2disk->getFileName());
            LVITEM item;
            item.mask = LVIF_TEXT |LVIF_PARAM| LVIF_IMAGE |LVIF_STATE;
            item.iItem=i; item.pszText= pszInfo;
            item.cchTextMax=wcslen(pszInfo)+1;
            item.iSubItem=0;     item.state = 0; item.stateMask = 0; 
            item.lParam= (LPARAM)ids[i];
            if (id=m_listDisks.InsertItem(&item)==-1){
                break;
            }else{
                id=i;
                CString strText;
                strText.Format(TEXT("%iMb"), m_fdms2disk->getDiskSize()>>20);
                m_listDisks.SetItemText(id, 1, strText);
                switch( m_fdms2disk->getDiskType()){
                    case DTDisk: 
                        if (m_fdms2disk->isInvalidFdms2Disk()){
                            strText.Format(TEXT("unknown: %S"), m_fdms2disk->getInfo());
                        }else{
                            strText= TEXT("Fostex:");
                        }
                        m_listDisks.SetItemText(id, 2, strText);
                        break;
                    case DTImage: 
                        strText= TEXT("Image");
                        m_listDisks.SetItemText(id, 2, strText);
                        break;
                }
                
                
            }
        }
        m_listDisks.Invalidate();
    }
}

BOOL DFdmsProp::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_iRadioPM=(int)m_pm;	 
    if (m_fdms2disk){
        m_listDisks.DeleteAllItems();
        m_listDisks.InsertColumn(0, TEXT("Path"), LVCFMT_LEFT, 140);
        m_listDisks.InsertColumn(1, TEXT("Info"), LVCFMT_LEFT, 80);
        m_listDisks.InsertColumn(2, TEXT("Size"), LVCFMT_LEFT , 180);
        Refresh();
    }
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

void DFdmsProp::OnLvnItemActivateListDisk(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    // TODO: Add your control notification handler code here
    DWORD dw= (DWORD)m_listDisks.GetFirstSelectedItemPosition();
    if (dw){
        m_fdms2disk->setDiskId(dw-1);
        m_fdms2disk->start();
        bool ok= !m_fdms2disk->isInvalidFdms2Disk();
        m_fdms2disk->stop();
        m_csFileName = m_fdms2disk->getFileName();
        UpdateData(FALSE);
    }
    *pResult = 0;
}

void DFdmsProp::OnBnClickedButtonRefresh()
{
    Refresh();
}
