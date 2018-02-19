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
        const char* pszTmp=NULL;
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
        fdms2diskPtrIF* pt= p1.getNewPtr();
        CHECK(pt);
        t_length lread=0;
        ec=pt->startPtr(ptr, 0, 100, lread, RWReadOnly);
        CHECK_EQUAL(ErrMapping, ec);
        pt->stopPtr(ptr);
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
        pt= p1.getNewPtr();
        ec=pt->startPtr(ptr, 0, 100, lread, RWReadOnly);
        CHECK_EQUAL( ErrNone, ec );
        CHECK_EQUAL( '#', ((char*)ptr)[0] );
        pt->stopPtr(ptr);
        CHECK( ptr==NULL);
        p1.stop();
        
        int diskid[10];
        int nDisk= p1.getNumDisk(diskid, 10);
        for (int i=0; i<nDisk; i++){
            int id= diskid[i];
            p1.setDiskId(id);
            ec=p1.start();
            t_length l=p1.getDiskSize();
            CHECK_EQUAL( ErrNone, ec);
            //CHECK( l>0 );
            printf("%s\n", p1.getInfo() );

            p1.stop();
        };

        if (nDisk>0){
            //TODO: Physical drive accessing doesn't work yet!
            p1.setDiskId(diskid[1]);        //drive number...
            p1.setVerbose(true);    //msgbox actually displays "invalid"
            ec= p1.start();
            CHECK_EQUAL( ErrNone, ec );
            pt=p1.getNewPtr();
            ec=pt->startPtr(ptr, 0, 100, lread, RWReadOnly);
            CHECK_EQUAL( ErrNone, ec );
            if (ptr){
                //CHECK_EQUAL( '#', ((char*)ptr)[0] ); //This is not what we want, skip.
            }else{
                CHECK(false);   //bad if ptr is null.
            }
            p1.stop();
        }

    }


}
 

