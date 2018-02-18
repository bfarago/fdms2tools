#include "windows.h"
#include "UnitTest++.h"
#include "TestReporterStdout.h"
#include "fdms2.h"
#include "..\..\fdms2view\fdms2reader.h"
#include "emuFdms2.h"
#include "Fdms2ImGen.h"

using namespace UnitTest;

SUITE(TestFdms2Reader)
{
    class TestFdms2Reader: public fdms2streamingIF{
    public:
        EmuFdms2 m_fdms2;
        TestFdms2Reader(){
            iRead=0;
            iReadSample=0;
            start.setPos(0);
            stop.setPos(0);
            stop.addSample(32);
        }
        virtual bool read(fdms2stream *pStream){
            iRead++;
            iReadSample+= pStream->iSamples;
            return true;
        }
    protected:
        int iRead, iReadSample;
        fdms2pos start,stop;
    };

    TEST_FIXTURE(TestFdms2Reader, Constructors)
    {
        fdms2reader p1;
//        CHECK_EQUAL(0, p1.m_Pos);
        fdms2reader* pd= new fdms2reader();
//        CHECK_EQUAL(1, pd->m_Pos);
        delete pd;
    }
    TEST_FIXTURE(TestFdms2Reader, Registers)
    {
        fdms2reader p1;
        p1.registerFdms2(&m_fdms2);
        p1.registerStreaming(this);
        p1.setBlockSizeSample(16);
        p1.setRegion(0,start, stop);
        p1.start();
        while (p1.isRunning()){
            Sleep(100);
        }
        CHECK_EQUAL((stop.m_Sample-start.m_Sample), iReadSample );
        CHECK_EQUAL((2), iRead);
    }

	TEST_FIXTURE(TestFdms2Reader, ImageRead)
	{
		Fdms2ImGen img;
		img.create("test1.img");
		fdms2 f1;
		f1.setFileName(img.getFilename());
		long long s = f1.getDiskSize();
		CHECK_EQUAL(s , img.getSize());

		printf_s("size:%lli\n", s);
		f1.m_step = 2; // dump partitions
		f1.dump();

		img.remove();
	}

}
 

