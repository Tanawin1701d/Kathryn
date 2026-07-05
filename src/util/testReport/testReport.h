//
// Created by tanawin.
//
// Lightweight, in-process test-result collector for the auto sim suite.
// `SimInterface::testAndPrint(...)` pushes every assertion here; the auto-sim
// manager (`startAutoSimTest`) opens a case per test element and prints one
// cocotb-style summary table at the end of the run.
//

#ifndef KATHRYN_TESTREPORT_H
#define KATHRYN_TESTREPORT_H

#include <string>
#include <vector>

namespace kathryn{

    enum class TestStatus{ PASS, FAIL, NO_RESULT, ERROR };

    /** one assertion outcome inside a case */
    struct TestRecord{
        std::string testName;
        TestStatus  status;
        std::string detail;   // "expect X got Y" on failure
    };

    /** one test element (identified by its sim id) and all its assertions */
    struct CaseResult{
        std::string             caseId;
        std::vector<TestRecord> records;
        bool                    errored = false;   // threw before finishing
        std::string             errorDetail;
    };

    class TestReport{
    public:
        static TestReport& instance();

        /** open a new case; subsequent record()/markError() attach to it */
        void beginCase(const std::string& caseId);
        /** append an assertion outcome to the current case */
        void record(const std::string& testName, bool pass, const std::string& detail = "");
        /** flag the current case as crashed (exception before it finished) */
        void markError(const std::string& detail);
        /** render the summary table (uses termColor) */
        void printSummary();
        /** drop every recorded case (call before a fresh run) */
        void clear();

    private:
        TestReport() = default;
        std::vector<CaseResult> _cases;
    };

}

#endif //KATHRYN_TESTREPORT_H
