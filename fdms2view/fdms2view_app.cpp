/* Written by Barna Farago <brown@weblapja.com> 2006-2018
*
*/
// testermfc_fdms2lib.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "fdms2view_app.h"
#include "ChildFrm.h"
#include "fdms2view_doc.h"
#include "fdms2view_view.h"
#include "DNewProjectWizard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFdms2View_App

BEGIN_MESSAGE_MAP(CFdms2View_App, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

// The one and only CFdms2View_App object

CFdms2View_App theApp;


// CFdms2View_App construction
//CMemoryState msOld;
CFdms2View_App::CFdms2View_App()
:m_pDocTemplate(NULL),m_pMainFrame(NULL)
{
	EnableHtmlHelp();
	//msOld.Checkpoint();
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

CFdms2View_App::~CFdms2View_App(){

#ifdef _DEBUG
	//msOld.DumpAllObjectsSince();
#endif
}

// CFdms2View_App initialization

BOOL CFdms2View_App::InitInstance()
{
	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local Brown Fdms2Viewer"));
	LoadStdProfileSettings(8);  // Load standard INI file options (including MRU)
	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	
	m_pDocTemplate = new CMultiDocTemplate(IDR_fdms2viewTYPE,
		RUNTIME_CLASS(CFdms2View_Doc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CFdms2View_View));
	AddDocTemplate(m_pDocTemplate);
	// create main MDI Frame window
	m_pMainFrame = new CMainFrame;
	if (!m_pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = m_pMainFrame; //TODO: eliminate m_pMainFrame ?

	// call DragAcceptFiles only if there's a suffix
	//  In an MDI app, this should occur immediately after setting m_pMainWnd
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();
	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);
	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// The main window has been initialized, so show and update it
	m_pMainFrame->ShowWindow(m_nCmdShow);
	m_pMainFrame->UpdateWindow();
	return TRUE;
}



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CFdms2View_App::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}
void CFdms2View_App::OnFileNew(){
    DNewProjectWizard dlg;
    INT_PTR r=dlg.DoModal();
    if (r==IDOK){
        int iSel=dlg.getSelection();
        
        switch(iSel){
            case 0:
                CWinApp::OnFileNew();
                SendMessage(m_pMainWnd->m_hWnd, WM_COMMAND, ID_DEVICE_GRAB,0);
                break;
            case 1:
                CWinApp::OnFileNew();
                SendMessage(m_pMainWnd->m_hWnd, WM_COMMAND, ID_VIEW_FDMS2PROPERTIES,0);
                break;
            case 2:
                CWinApp::OnFileNew();
                break;
            case 3:
                CWinApp::OnFileOpen();
                break;
            case 4:
                SendMessage(m_pMainWnd->m_hWnd, WM_COMMAND, ID_FILE_MRU_FILE1,0);
                break;
                
        };
    
    }
    
}

// CFdms2View_App message handlers
bool DisplayError(){
LPVOID lpMsgBuf;
FormatMessage( 
    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
    FORMAT_MESSAGE_FROM_SYSTEM | 
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    GetLastError(),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPTSTR) &lpMsgBuf,
    0,
    NULL 
);
// Process any inserts in lpMsgBuf.
// ...
// Display the string.
MessageBox( NULL, (LPCTSTR)lpMsgBuf, L"Error", MB_OK | MB_ICONINFORMATION );
// Free the buffer.
LocalFree( lpMsgBuf );
  return false;
}


bool CStringToCharBuff(const CString& cs, char* &buf, int len){
	bool bWasSpacer=false;
	bool bOk=true;
	if (len<1){
		len=cs.GetLength()+1;
		if (buf != NULL) free(buf);
		buf=(char*)malloc(len);
		FillMemory(buf,len,0);
	}
	int iRes=WideCharToMultiByte(CP_ACP,
		0,
//		WC_SEPCHARS | WC_DEFAULTCHAR,
		cs, cs.GetLength(),
		buf, len, "_", NULL);// &bWasSpacer );
	bOk= (iRes!=0);
	if (!bOk){
		DisplayError();
	}
	return bOk;
}