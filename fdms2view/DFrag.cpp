// DFrag.cpp : implementation file
//

#include "stdafx.h"
#include "fdms2view_app.h"
#include "DFrag.h"


// DFrag dialog

IMPLEMENT_DYNAMIC(DFrag, CDialog)
DFrag::DFrag(CWnd* pParent /*=NULL*/)
	: CDialog(DFrag::IDD, pParent)
{
}

DFrag::~DFrag()
{
}

void DFrag::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_FRAG, m_lbFrag);
}


BEGIN_MESSAGE_MAP(DFrag, CDialog)
END_MESSAGE_MAP()


// DFrag message handlers

BOOL DFrag::OnInitDialog()
{
	CDialog::OnInitDialog();
	fdms2pos pLen;
	//t1_toffset start;
	DWORD dwMax=0;
	int iIdx=0;
	if (m_Fdms2){
		for (iIdx=0; iIdx<m_Fdms2->m_PartTableLength[m_SelectedProgram]; iIdx++){
			char* pszInfo=NULL;
			m_Fdms2->m_PartTable[m_SelectedProgram][iIdx]->dumpStr(pszInfo);
			CString s;
			s.Format(L"%S", pszInfo);
			if (pszInfo){
				m_lbFrag.AddString(s);
				free(pszInfo);
			}
		}
		/*
		while(!m_Fdms2->getPart(m_SelectedProgram, iIdx, start, pLen)){
			DWORD dwTmp= start+pLen.m_Pos;
			if (dwMax<dwTmp) dwMax=dwTmp;
			CString s;
			char* pszLen=NULL;
			char* pszLenTime=NULL;
			pLen.dumpTimeStr(pszLenTime);
			pLen.dumpByteStr(pszLen);
			s.Format("%08x %s %sbyte", start,pszLenTime, pszLen);
			free(pszLen);
			free(pszLenTime);
			m_lbFrag.AddString(s);
			iIdx++;
		}
		*/
	}
	return TRUE;  // return TRUE unless you set the focus to a control
}
