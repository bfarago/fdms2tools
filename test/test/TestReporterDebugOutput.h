#pragma once

#include "UnitTest++.h"
#include "TestReporter.h"

class TestReporterDebugOutput : public UnitTest::TestReporter
{
private:
    virtual void ReportTestStart(UnitTest::TestDetails const& test);
    virtual void ReportFailure(UnitTest::TestDetails const& test, char const* failure);
    virtual void ReportTestFinish(UnitTest::TestDetails const& test, float secondsElapsed);
    virtual void ReportSummary(int totalTestCount, int failedTestCount, int failureCount, float secondsElapsed);
};
