#include "StdAfx.h"
#include "Windows.h"
#include "TestReporterDebugOutput.h"


void TestReporterDebugOutput::ReportFailure(UnitTest::TestDetails const& details, char const* failure)
{
    char const* const errorFormat = "%s(%d): error: Failure in %s: %s\n";
    const int buf_size = 1024;
    char buf[buf_size];
    int len = _snprintf(buf, buf_size-1, errorFormat, details.filename, details.lineNumber, details.testName, failure);
//    ASSERT(len>=0);
    buf[buf_size-1] = 0;
    WCHAR wbuf[buf_size];
    for(int i = 0; i <= len; i++){
        wbuf[i] = buf[i];
    }
    wbuf[len] = buf[len];

    OutputDebugString(&wbuf[0]);
    //OutputDebugStringW(&wbuf[0]);
}

void TestReporterDebugOutput::ReportTestStart(UnitTest::TestDetails const& /*test*/)
{
}

void TestReporterDebugOutput::ReportTestFinish(UnitTest::TestDetails const& /*test*/, float)
{
}

void TestReporterDebugOutput::ReportSummary(int const totalTestCount, int const failedTestCount,
                                       int const failureCount, float secondsElapsed)
{
    const int buf_size = 1024;
    char buf[buf_size];
    WCHAR wbuf[buf_size];
    int len;
    if (failureCount > 0)
        len = _snprintf(buf, buf_size-1, "FAILURE: %d out of %d tests failed (%d failures).\n", failedTestCount, totalTestCount, failureCount);
    else
        len = _snprintf(buf, buf_size-1, "Success: %d tests passed.\n", totalTestCount);

    for(int i = 0; i <= len; i++){
        wbuf[i] = buf[i];
    }
    wbuf[len] = buf[len];
    OutputDebugString(&wbuf[0]);

    len = _snprintf(buf, buf_size-1, "Test time: %.2f seconds.\n", secondsElapsed);
    for(int i = 0; i <= len; i++){
        wbuf[i] = buf[i];
    }
    wbuf[len] = buf[len];
    OutputDebugString(&wbuf[0]);
}

