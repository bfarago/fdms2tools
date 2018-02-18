#include "windows.h"
#include "UnitTest++.h"
#include "TestReporterStdout.h"
#include "fdms2.h"
#include "..\..\fdms2lib\fdms2disk.h"
using namespace UnitTest;

SUITE(TestFdms2Disk)
{
    class TestFdms2Disk{
    public:
        TestFdms2Disk(){
        }
    protected:
    };

    TEST_FIXTURE(TestFdms2Disk, Constructors)
    {
        fdms2disk p1;
        fdms2disk* pd= new fdms2disk();
        delete pd;
    }
    TEST_FIXTURE(TestFdms2Disk, Init)
    {
        fdms2disk p1;
        CHECK_EQUAL(DTUninitialized, p1.getDiskType());

        p1.setDiskId(0);
        char* pszTmp=NULL;
        pszTmp=p1.getFileName();
        size_t iLen=0;
        if (pszTmp) iLen=strlen(pszTmp);
        CHECK_EQUAL('0', pszTmp[iLen-1]);
        CHECK_EQUAL(DTDisk, p1.getDiskType());

        p1.setFileName(NULL);
        CHECK_EQUAL(DTUninitialized, p1.getDiskType());
        p1.setFileName("alma");
        CHECK_EQUAL(DTImage, p1.getDiskType());
        void* ptr=NULL;
        EErrorCode ec;
        ec= p1.start();
        CHECK_EQUAL(ErrFileNotFound, ec);
        ec=p1.startPtr(ptr, 0, 100);
        CHECK_EQUAL(ErrMapping, ec);
        p1.stopPtr(ptr);
        p1.stop();
		const char* fname = "testfdms2disk.cpp";
        p1.setFileName(fname);
		FILE* fp = _fsopen(fname, "r", _SH_DENYWR);
		fseek(fp, 0, SEEK_END);
		long scheck = ftell(fp);
		fclose(fp);

        ec=p1.start();
        t_length len= p1.getDiskSize();
        CHECK_EQUAL( ErrNone, ec );
        CHECK_EQUAL(scheck, len);
        printf("%s\n", p1.getInfo() );
        ec=p1.startPtr(ptr, 0, 100);
        CHECK_EQUAL( ErrNone, ec );
        CHECK_EQUAL( '#', ((char*)ptr)[0] );
        p1.stopPtr(ptr);
        CHECK( ptr==NULL);
        p1.stop();
        
        int nDisk= p1.getNumDisk();
        for (int i=0; i<nDisk; i++){
            
            p1.setDiskId(i);
            ec=p1.start();
            t_length l=p1.getDiskSize();
            CHECK_EQUAL( ErrNone, ec);
            CHECK( l>0 );
            printf("%i %s %c\n", i, p1.getInfo(), l>0?' ':'!' );

            p1.stop();
        };

        /*
        //TODO: Physical drive accessing doesn't work yet!

        p1.setDiskId(1);        //drive number...
        p1.setVerbose(true);    //msgbox actually displays "invalid"
        ec= p1.start();
        CHECK_EQUAL( ErrNone, ec );
        ec=p1.startPtr(ptr, 0, 100);
        CHECK_EQUAL( ErrNone, ec );
        if (ptr){
            //CHECK_EQUAL( '#', ((char*)ptr)[0] ); //This is not what we want, skip.
        }else{
            CHECK(false);   //bad if ptr is null.
        }
        p1.stop();
        */

    }


}
 

