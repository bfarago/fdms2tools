/* Written by Barna Farago <brown@weblapja.com> 2006-2018
 */
#pragma once

class CFileListCubaseXML
{
    CString m_csFileName;
public:
    CFileListCubaseXML(void);
    ~CFileListCubaseXML(void);
    void setFileName(CString &filename);
    void open();
    void close();
    void addFile(CString &file);

};
