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
