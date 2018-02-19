// DGrab.cpp : implementation file
//

#include "stdafx.h"
#include "DGrab.h"
#include <shellapi.h>

#define TIMER_GRAB (29991)
#define TIMER_LIST (29992)

// DGrab dialog

IMPLEMENT_DYNAMIC(DGrab, CDialog)

DGrab::DGrab(CWnd* pParent /*=NULL*/)
	: CDialog(DGrab::IDD, pParent), m_csFileName(_T("grab.img"))
{

}

DGrab::~DGrab()
{
}

void DGrab::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_BUTTON_STOP, m_bnStop);
    DDX_Control(pDX, IDC_BUTTON_GRAB, m_bnGrab);
    DDX_Control(pDX, IDC_BUTTON_LIST, m_bnList);
    DDX_Control(pDX, IDC_COMBO_LIST, m_cbInDevices);
    DDX_Control(pDX, IDC_EDIT_OUT_FILE, m_editOutputFile);
    DDX_Control(pDX, IDC_BUTTON_OUT_BROWSE, m_bnOutBrowse);
    DDX_Control(pDX, IDC_PROGRESS1, m_progress1);
    DDX_Control(pDX, IDOK, m_bnOk);
    DDX_Control(pDX, IDC_STATIC_PROGRESS, m_sProgress);
}
void DGrab::EnterState(DGrab::eState state){
    switch (state){
        case sListed:
            m_bnStop.EnableWindow(FALSE);
            m_bnList.EnableWindow(TRUE);
            m_cbInDevices.EnableWindow(TRUE);
            m_bnGrab.EnableWindow(TRUE);
            m_bnOk.EnableWindow(FALSE);
            m_sProgress.SetWindowText(L"Listed");
            break;
        case sGrabbing:
            m_bnStop.EnableWindow(TRUE);
            m_bnList.EnableWindow(FALSE);
            m_cbInDevices.EnableWindow(FALSE);
            m_bnGrab.EnableWindow(FALSE);
            m_bnOk.EnableWindow(FALSE);
            m_sProgress.SetWindowText(L"Grabbing");
            break;
        case sFinished:
            m_bnStop.EnableWindow(FALSE);
            m_bnList.EnableWindow(TRUE);
            m_cbInDevices.EnableWindow(TRUE);
            m_bnGrab.EnableWindow(TRUE);
            m_bnOk.EnableWindow(TRUE);
            m_sProgress.SetWindowText(L"Finished...");
            break;
        case sFinishedFail:
            m_bnStop.EnableWindow(FALSE);
            m_bnList.EnableWindow(TRUE);
            m_cbInDevices.EnableWindow(TRUE);
            m_bnGrab.EnableWindow(TRUE);
            m_bnOk.EnableWindow(TRUE);
            m_sProgress.SetWindowText(L"Failed...");
            break;
        case sCancel:
            m_bnStop.EnableWindow(FALSE);
            m_sProgress.SetWindowText(L"Canceled");
            break;
        case sInit:
        default:
            m_bnStop.EnableWindow(FALSE);
            m_bnList.EnableWindow(TRUE);
            m_bnOk.EnableWindow(FALSE);
            m_bnGrab.EnableWindow(FALSE);
            m_cbInDevices.EnableWindow(FALSE);
            //m_csFileName.SetString(_T("grab.img"));
            m_editOutputFile.SetWindowText(m_csFileName);
            m_sProgress.SetWindowText(L"Idle");
            //UpdateData(FALSE)
    }
}
BOOL DGrab::OnInitDialog()
{
    CDialog::OnInitDialog();
    EnterState(sInit);
    SetTimer(TIMER_LIST, 150, NULL);
    return TRUE;
}


BEGIN_MESSAGE_MAP(DGrab, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_STOP, &DGrab::OnBnClickedButtonStop)   
    ON_BN_CLICKED(IDC_BUTTON_GRAB, &DGrab::OnBnClickedButtonGrab)
    ON_BN_CLICKED(IDC_BUTTON_LIST, &DGrab::OnBnClickedButtonList)
    ON_BN_CLICKED(IDC_BUTTON_OUT_BROWSE, &DGrab::OnBnClickedButtonOutBrowse)
    ON_WM_TIMER()
END_MESSAGE_MAP()

void DGrab::OnTimer(UINT_PTR nIDEvent){
    switch (nIDEvent){
        case TIMER_GRAB:
            OnGrabbingTimer();
            break;
        case TIMER_LIST:
            KillTimer(TIMER_LIST);
            OnBnClickedButtonList();
    }
}

// DGrab message handlers
void DGrab::OnBnClickedButtonOutBrowse()
{
    UpdateData(TRUE);
    m_editOutputFile.GetWindowText(m_csFileName);
	
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
      m_editOutputFile.SetWindowText(m_csFileName);
    }
	delete dlg;
}
void DGrab::OnBnClickedButtonStop(){
    m_ipcGrab.Kill();
}
void DGrab::OnBnClickedButtonGrab()
{
    CString dir;
    CString param;
    CString sInput;
    CString sName;
    CString sSize;
    m_cbInDevices.GetWindowText(sInput);
    int iPos=0;
    sName=sInput.Tokenize(L" ",iPos);
    sSize=sInput.Tokenize(L" ",iPos);
    m_diskSize=_wtol(sSize);
    m_progress1.SetRange32(0,m_diskSize>>10);
    dir=L".\\";
    param.Format(L"if=%s of=%s --progress", sName, m_csFileName);
    m_ipcGrab.setHide(true);
    m_ipcGrab.setDir(dir);
    m_ipcGrab.setParam(param);
    m_ipcGrab.setCmd(TEXT("dd.bat"));
    m_ipcGrab.setPipeOutFileName(L"grab.log");
    m_ipcGrab.StartExecute();
    EnterState(sGrabbing);
    m_ipcGrab.Open();
    SetTimer(TIMER_GRAB, 200, NULL);
}
void DGrab::OnGrabbingTimer(){
    bool bSuccess=false;
    
    if (m_ipcGrab.isOpen()){
        CString s;
        CString s2;
        int dog=100;
        while (m_ipcGrab.ReadString(s)){
            if (s.GetLength()>3){
                s2=s;   
            }
            s.Empty();
            if (!--dog) break;
        }
        m_ipcGrab.SeekEnd();
        if (s2.GetLength()>3){
            m_sProgress.SetWindowText(s2);
            s2.Remove(',');
            long p=_wtol(s2);
            m_progress1.SetPos(p>>10);
            m_progress1.Invalidate();
        }
    }else{
        KillTimer(TIMER_GRAB);
        m_ipcGrab.Close();
        if (m_ipcGrab.getExitCode()) EnterState(sFinishedFail);
        else  EnterState(sFinished);
    }

}
void DGrab::OnBnClickedButtonList(){
    CString param, dir;
    param=L"";
    dir=L".\\";
    m_ipcList.setHide(true);
    m_ipcList.setDir(dir);
    m_ipcList.setParam(param);
    m_ipcList.setCmd(TEXT("ddlist.bat"));
    m_ipcList.setPipeOutFileName(L"dd.log");
    m_ipcList.StartExecute();
    
    //fill
    do{
        Sleep(10);
    }while (m_ipcList.isRunning());

    bool bSuccess=false;
    while (!bSuccess){
        try{
            while (m_cbInDevices.GetCount()) m_cbInDevices.DeleteString(0);
            CString s;
            CString size;
            CString name;
            CString title;
            m_ipcList.Open();
            while (m_ipcList.ReadString(s)){
                if (s.GetLength()<9) continue;
                if (s[0]=='\\'){
                    if ((s[4] == 'D')&(s[11] == 'H')){
                        name=s;
                    }
                }else{
                    if (name.GetLength()<9) continue;
                    if ((s[2]=='s')&&(s[3]=='i')){
                        int iSize=_wtoi64(s.Mid(9,s.Find('b',9)-9))/1024/1024;
                        title.Format(L"%s %iMb",name,iSize);
                        m_cbInDevices.AddString(title);
                        name="";
                    }
                }
                
            }
            m_ipcList.Close();
            bSuccess=true;
        }catch(...){
            Sleep(200);
        };
    };
    m_cbInDevices.SetCurSel(0);
    EnterState(sListed);
}

