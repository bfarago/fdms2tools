// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "UnitTest++.h"
#include "TestReporterStdout.h" 
#include "TestReporterDebugOutput.h" 
int _tmain(int argc, _TCHAR* argv[])
{
    return UnitTest::RunAllTests(
            TestReporterDebugOutput(),
            //UnitTest::TestReporterStdout(),
            UnitTest::Test::GetTestList(),
            0 );
	//return UnitTest::RunAllTests();
}

