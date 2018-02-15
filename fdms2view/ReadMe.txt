================================================================================
    MICROSOFT FOUNDATION CLASS LIBRARY : testermfc_fdms2lib Project Overview
===============================================================================

The application wizard has created this testermfc_fdms2lib application for 
you.  This application not only demonstrates the basics of using the Microsoft 
Foundation Classes but is also a starting point for writing your application.

This file contains a summary of what you will find in each of the files that
make up your testermfc_fdms2lib application.

testermfc_fdms2lib.vcproj
    This is the main project file for VC++ projects generated using an application wizard. 
    It contains information about the version of Visual C++ that generated the file, and 
    information about the platforms, configurations, and project features selected with the
    application wizard.

fdms2view_app.h
    This is the main header file for the application.  It includes other
    project specific headers (including Resource.h) and declares the
    CFdms2View_App application class.

testermfc_fdms2lib.cpp
    This is the main application source file that contains the application
    class CFdms2View_App.

testermfc_fdms2lib.rc
    This is a listing of all of the Microsoft Windows resources that the
    program uses.  It includes the icons, bitmaps, and cursors that are stored
    in the RES subdirectory.  This file can be directly edited in Microsoft
    Visual C++. Your project resources are in 1033.

res\testermfc_fdms2lib.ico
    This is an icon file, which is used as the application's icon.  This
    icon is included by the main resource file testermfc_fdms2lib.rc.

res\testermfc_fdms2lib.rc2
    This file contains resources that are not edited by Microsoft 
    Visual C++. You should place all resources not editable by
    the resource editor in this file.
testermfc_fdms2lib.reg
    This is an example .reg file that shows you the kind of registration
    settings the framework will set for you.  You can use this as a .reg
    file to go along with your application or just delete it and rely
    on the default RegisterShellFileTypes registration.
/////////////////////////////////////////////////////////////////////////////

For the main frame window:
    The project includes a standard MFC interface.
MainFrm.h, MainFrm.cpp
    These files contain the frame class CMainFrame, which is derived from
    CMDIFrameWnd and controls all MDI frame features.
res\Toolbar.bmp
    This bitmap file is used to create tiled images for the toolbar.
    The initial toolbar and status bar are constructed in the CMainFrame
    class. Edit this toolbar bitmap using the resource editor, and
    update the IDR_MAINFRAME TOOLBAR array in testermfc_fdms2lib.rc to add
    toolbar buttons.
/////////////////////////////////////////////////////////////////////////////

For the child frame window:

ChildFrm.h, ChildFrm.cpp
    These files define and implement the CChildFrame class, which
    supports the child windows in an MDI application.

/////////////////////////////////////////////////////////////////////////////

The application wizard creates one document type and one view:

fdms2view_doc.h, testermfc_fdms2libDoc.cpp - the document
    These files contain your CFdms2View_Doc class.  Edit these files to
    add your special document data and to implement file saving and loading
    (via CFdms2View_Doc::Serialize).
    The Document will have the following strings:
        File extension:      f2lproj
        File type ID:        testermfcfdms2lib.Document
        Main frame caption:  testermfc_fdms2lib
        Doc type name:       testermfc_fdms2
        Filter name:         testermfc_fdms2lib Files (*.f2lproj)
        File new short name: testermfc_fdms2
        File type long name: testermfc_fdms2lib.Document
fdms2view_view.h, testermfc_fdms2libView.cpp - the view of the document
    These files contain your CFdms2View_View class.
    CFdms2View_View objects are used to view CFdms2View_Doc objects.
res\testermfc_fdms2libDoc.ico
    This is an icon file, which is used as the icon for MDI child windows
    for the CFdms2View_Doc class.  This icon is included by the main
    resource file testermfc_fdms2lib.rc.
/////////////////////////////////////////////////////////////////////////////

Help Support:

hlp\testermfc_fdms2lib.hhp
    This file is a help project file. It contains the data needed to
    compile the help files into a .chm file.

hlp\testermfc_fdms2lib.hhc
    This file lists the contents of the help project.

hlp\testermfc_fdms2lib.hhk
    This file contains an index of the help topics.

hlp\afxcore.htm
    This file contains the standard help topics for standard MFC
    commands and screen objects. Add your own help topics to this file.

makehtmlhelp.bat
    This file is used by the build system to compile the help files.

hlp\Images\*.gif
    These are bitmap files required by the standard help file topics for
    Microsoft Foundation Class Library standard commands.

/////////////////////////////////////////////////////////////////////////////

Other Features:

ActiveX Controls
    The application includes support to use ActiveX controls.

Windows Sockets
    The application has support for establishing communications over TCP/IP networks.
/////////////////////////////////////////////////////////////////////////////

Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named testermfc_fdms2lib.pch and a precompiled types file named StdAfx.obj.

Resource.h
    This is the standard header file, which defines new resource IDs.
    Microsoft Visual C++ reads and updates this file.

/////////////////////////////////////////////////////////////////////////////

Other notes:

The application wizard uses "TODO:" to indicate parts of the source code you
should add to or customize.

If your application uses MFC in a shared DLL, and your application is in a 
language other than the operating system's current language, you will need 
to copy the corresponding localized resources MFC70XXX.DLL from the Microsoft
Visual C++ CD-ROM under the Win\System directory to your computer's system or 
system32 directory, and rename it to be MFCLOC.DLL.  ("XXX" stands for the 
language abbreviation.  For example, MFC70DEU.DLL contains resources 
translated to German.)  If you don't do this, some of the UI elements of 
your application will remain in the language of the operating system.

/////////////////////////////////////////////////////////////////////////////
