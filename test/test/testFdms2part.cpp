#include "UnitTest++.h"
#include "TestReporterStdout.h"
#include "fdms2.h"
using namespace UnitTest;

SUITE(TestFdms2Part)
{
    class TestFdms2Part{
    public:
        TestFdms2Part(){
        }
    };

    TEST_FIXTURE(TestFdms2Part, Constructors)
    {
        t1_toffset offs;
        fdms2pos len;
        fdms2part pa1;
        pa1.getStartLength(offs, len);
        CHECK_EQUAL(0, offs);
        CHECK_EQUAL(0, len.m_Pos);
        
        fdms2pos logical;
        len.setTime(1,0,0,0,0);
        fdms2part* pad= new fdms2part(0, len, logical);
        CHECK_EQUAL(len.m_Pos, logical.m_Pos);

        t1_toffset offs2;
        fdms2pos len2;
        len2.setPos(0);
        pad->getStartLength(offs2, len2);
        CHECK_EQUAL(0, offs2);
        CHECK_EQUAL(len.m_Pos, len2.m_Pos);

        pad->setData(1,len,logical);
        CHECK_EQUAL(len.m_Pos*2, logical.m_Pos);

        delete pad;

    }
   
}
 

