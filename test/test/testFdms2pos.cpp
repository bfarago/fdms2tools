#include "UnitTest++.h"
#include "TestReporterStdout.h"
#include "fdms2.h"
using namespace UnitTest;

SUITE(TestFdms2Pos)
{
    class TestFdms2Pos{
    public:
        TestFdms2Pos(){
        }
    };

    TEST_FIXTURE(TestFdms2Pos, Constructors)
    {
        fdms2pos p1,p2;
        CHECK_EQUAL(0, p1.m_Pos);

        fdms2pos* pd= new fdms2pos(1);
        CHECK_EQUAL(1, pd->m_Pos);
        delete pd;

        p1.setSample(1);
        p2.setTime(2,3,4,5,6);
        p1=p2;
        CHECK_EQUAL(p2.m_Pos, p1.m_Pos);
        CHECK_EQUAL(p2.m_Sample, p1.m_Sample);
        CHECK_EQUAL(p2.m_Sec, p1.m_Sec);
        CHECK_EQUAL(p2.m_Min, p1.m_Min);
        CHECK_EQUAL(p2.m_Hour, p1.m_Hour);
        CHECK_EQUAL(p2.m_Frame, p1.m_Frame);
    }
    TEST_FIXTURE(TestFdms2Pos, Setter)
    {
        fdms2pos p1;
        CHECK_EQUAL(0, p1.m_Pos);
        for (int v=0; v<10; v++){
            p1.setPos(sizeof(short)*FOSTEXMAXCHANNELS*v);
            CHECK_EQUAL(v, p1.m_Sample);
            p1.setPos(sizeof(short)*FOSTEXMAXCHANNELS*FOSTEXSAMPLERATE*v);
            CHECK_EQUAL(v, p1.m_Sec);
            p1.setPos(sizeof(short)*FOSTEXMAXCHANNELS*FOSTEXSAMPLERATE*60*v);
            CHECK_EQUAL(v, p1.m_Min);
            p1.setPos(sizeof(short)*FOSTEXMAXCHANNELS*FOSTEXSAMPLERATE*60*60*(t1_toffset)v); //Cast >32bit
            CHECK_EQUAL(v, p1.m_Hour);

            p1.setSample(v);
            CHECK_EQUAL(v, p1.m_Sample);
            p1.setSample(FOSTEXSAMPLERATE*v);
            CHECK_EQUAL(v, p1.m_Sec);
            p1.setSample(FOSTEXSAMPLERATE*60*v);
            CHECK_EQUAL(v, p1.m_Min);
            p1.setSample(FOSTEXSAMPLERATE*60*60*v);
            CHECK_EQUAL(v, p1.m_Hour);

            p1.setTime(v,0,0,0,0);
            CHECK_EQUAL(v, p1.m_Hour);
            CHECK_EQUAL(v*3600*FOSTEXSAMPLERATE, p1.m_Sample);
            p1.setTime(0,v,0,0,0);
            CHECK_EQUAL(v, p1.m_Min);
            CHECK_EQUAL(v*60*FOSTEXSAMPLERATE, p1.m_Sample);
        }
        
    }
    TEST_FIXTURE(TestFdms2Pos, AddPos)
    {
        fdms2pos p1;
        fdms2pos p2;
        p1.setPos(1);
        p2.setPos(2);
        p1.addPos(p2);
        CHECK_EQUAL(3, p1.m_Pos);
        p2.addPos(2);
        CHECK_EQUAL(4, p2.m_Pos);
        p1+=p2;
        CHECK_EQUAL(4, p2.m_Pos);
        CHECK_EQUAL(7, p1.m_Pos);

        p1.setSample(FOSTEXSAMPLERATE);
        p2.setSample(FOSTEXSAMPLERATE*59);
        p1+=p2;
        CHECK_EQUAL(0, p1.m_Sec);
        CHECK_EQUAL(1, p1.m_Min);
        CHECK_EQUAL(0, p1.m_Hour);
        CHECK_EQUAL(0, p1.m_Frame);
        p1.addSample(FOSTEXSAMPLERATE);
        CHECK_EQUAL(1, p1.m_Sec);
        CHECK_EQUAL(1, p1.m_Min);
        CHECK_EQUAL(0, p1.m_Hour);
        CHECK_EQUAL(0, p1.m_Frame);
    }
    TEST_FIXTURE(TestFdms2Pos, BinaryOperator)
    {
        fdms2pos p1,p2;
        p1.setSample(1);
        p2.setSample(2);
        CHECK( p1<p2 );
    }
}
 

