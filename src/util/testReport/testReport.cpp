//
// Created by tanawin.
//

#include "testReport.h"
#include "util/termColor/termColor.h"

#include <iostream>
#include <algorithm>

namespace kathryn{

    TestReport& TestReport::instance(){
        static TestReport singleton;
        return singleton;
    }

    void TestReport::beginCase(const std::string& caseId){
        _cases.push_back(CaseResult{caseId, {}, false, ""});
    }

    void TestReport::record(const std::string& testName, bool pass, const std::string& detail){
        if (_cases.empty()){ beginCase("unnamed"); }   // defensive: record before beginCase
        _cases.back().records.push_back(
            TestRecord{testName, pass ? TestStatus::PASS : TestStatus::FAIL, detail});
    }

    void TestReport::markError(const std::string& detail){
        if (_cases.empty()){ beginCase("unnamed"); }
        _cases.back().errored     = true;
        _cases.back().errorDetail = detail;
    }

    void TestReport::clear(){
        _cases.clear();
    }

    /////////////////////////////////////////////////////////////////////////
    /** table rendering helpers */
    /////////////////////////////////////////////////////////////////////////

    namespace {

        struct Row{
            TestStatus  status;
            std::string caseCol;   // "<caseId>.<testName>"
            std::string noteCol;   // expect/got on fail, reason on no-result/error
        };

        const char* statusLabel(TestStatus s){
            switch (s){
                case TestStatus::PASS:      return "PASS";
                case TestStatus::FAIL:      return "FAIL";
                case TestStatus::NO_RESULT: return "NO-RESULT";
                case TestStatus::ERROR:     return "ERROR";
            }
            return "?";
        }

        TC statusColor(TestStatus s){
            switch (s){
                case TestStatus::PASS:      return TC_GREEN;
                case TestStatus::FAIL:      return TC_RED;
                case TestStatus::NO_RESULT: return TC_YELLOW;
                case TestStatus::ERROR:     return TC_RED;
            }
            return TC_DEF;
        }

        std::string pad(const std::string& s, size_t w){
            return s.size() >= w ? s : s + std::string(w - s.size(), ' ');
        }

        std::string trunc(const std::string& s, size_t w){
            if (s.size() <= w){ return s; }
            return s.substr(0, w > 1 ? w - 1 : 0) + "~";
        }

    } // namespace

    void TestReport::printSummary(){

        //////////////// flatten cases -> rows ////////////////
        std::vector<Row> rows;
        for (auto& c : _cases){
            if (c.errored){
                rows.push_back(Row{TestStatus::ERROR, c.caseId,
                                   "crashed: " + c.errorDetail});
            } else if (c.records.empty()){
                rows.push_back(Row{TestStatus::NO_RESULT, c.caseId,
                                   "no assertions executed"});
            } else {
                for (auto& r : c.records){
                    rows.push_back(Row{r.status,
                                       c.caseId + "." + r.testName,
                                       r.detail});
                }
            }
        }

        //////////////// column widths ////////////////
        const size_t NOTE_CAP = 48;
        size_t stW = std::string("STATUS").size();
        size_t caW = std::string("TEST CASE").size();
        size_t noW = std::string("NOTE").size();
        for (auto& r : rows){
            stW = std::max(stW, std::string(statusLabel(r.status)).size());
            caW = std::max(caW, r.caseCol.size());
            noW = std::max(noW, std::min(NOTE_CAP, r.noteCol.size()));
        }

        auto sep = [&](){
            std::cout << "+-" << std::string(stW, '-')
                      << "-+-" << std::string(caW, '-')
                      << "-+-" << std::string(noW, '-') << "-+\n";
        };

        //////////////// header ////////////////
        std::cout << "\n" << TC_BLUE << " KATHRYN AUTO SIM  -  TEST SUMMARY" << TC_DEF << "\n";
        sep();
        std::cout << "| " << pad("STATUS", stW)
                  << " | " << pad("TEST CASE", caW)
                  << " | " << pad("NOTE", noW) << " |\n";
        sep();

        //////////////// body ////////////////
        for (auto& r : rows){
            std::cout << "| " << statusColor(r.status)
                      << pad(statusLabel(r.status), stW) << TC_DEF
                      << " | " << pad(r.caseCol, caW)
                      << " | " << pad(trunc(r.noteCol, noW), noW) << " |\n";
        }
        sep();

        //////////////// counts ////////////////
        int nPass = 0, nFail = 0, nNo = 0, nErr = 0;
        for (auto& r : rows){
            switch (r.status){
                case TestStatus::PASS:      ++nPass; break;
                case TestStatus::FAIL:      ++nFail; break;
                case TestStatus::NO_RESULT: ++nNo;   break;
                case TestStatus::ERROR:     ++nErr;  break;
            }
        }
        std::cout << " "
                  << TC_GREEN  << nPass << " passed"      << TC_DEF << ", "
                  << TC_RED    << nFail << " failed"      << TC_DEF << ", "
                  << TC_YELLOW << nNo   << " no-result"   << TC_DEF << ", "
                  << TC_RED    << nErr  << " errored"     << TC_DEF
                  << "   (" << rows.size() << " total)\n";
        if (nFail || nErr){
            std::cout << TC_RED   << " RESULT: FAIL" << TC_DEF << "\n";
        } else {
            std::cout << TC_GREEN << " RESULT: PASS" << TC_DEF << "\n";
        }
    }

}
