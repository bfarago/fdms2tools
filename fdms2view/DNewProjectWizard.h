#pragma once

#include "resource.h"

// DNewProjectWizard dialog

class DNewProjectWizard : public CDialog
{
	DECLARE_DYNAMIC(DNewProjectWizard)

public:
	DNewProjectWizard(CWnd* pParent = NULL);   // standard constructor
	virtual ~DNewProjectWizard();
    int getSelection(){return m_iSelection;}
// Dialog Data
	enum { IDD = IDD_DIALOG_NEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    
	DECLARE_MESSAGE_MAP()
public:
    int m_iSelection;
};
