//
// Created by tanawin on 1/1/26.
//


#ifdef BUILD_RIDECORE

#include "simCtrlComb.h"

#include <utility>
#include <chrono>
#include <fstream>
#include <iomanip>

namespace kathryn::o3{


    CombCtrl::CombCtrl(CYCLE                    limitCycle,
                       const std::string&       prefix,
                       std::vector<std::string> testTypes,
                       SimProxyBuildMode        buildMode,
                       SlotWriterBase&          slotWriter,
                       SimState&                state,
                       TopSim&                  topSim,
                       SimCtrlRide&             slaveRide,
                       bool                     reqRegTest,
                       bool                     recordSlot,
                       ResultWriter*            resultWriter,
                       int                      simOptLevel
    ):
    SimCtrlKride(limitCycle,
                 prefix,
                 std::move(testTypes),
                 buildMode,
                 slotWriter,
                 state,
                 topSim,
                 resultWriter,
                 simOptLevel),
    _slaveRide  (slaveRide),
    _reqRegTest (reqRegTest),
    _recordSlot (recordSlot)
    {}

    bool CombCtrl::doCompare(){
        bool compareValid = _state.compare(_slaveRide._state);
        compareValid &= compareMemOp(_slaveRide);
        return compareValid;

    }

    void CombCtrl::describeCon(){

        std::vector<int>          errorIndexs;
        std::vector<WorkloadTime> workloadTimes;

        for (; _curTestCaseIdx < _testTypes.size(); _curTestCaseIdx++){
            runWorkload(workloadTimes, errorIndexs);
        }
        doWorkloadExit();
        _slaveRide.doWorkloadExit();

        summarizeSimTime      (workloadTimes);
        summarizeCompareResult(errorIndexs);
    }

    ////// one workload (_curTestCaseIdx): init both sides, run the compare loop,
    ////// record rtl-eval time and pass/fail, then per workload reg test
    void CombCtrl::runWorkload(std::vector<WorkloadTime>& workloadTimes,
                               std::vector<int>&          errorIndexs){

        using simClock = std::chrono::steady_clock;

        std::cout << std::endl
                  << std::endl
                  << std::endl;
        std::cout << TC_BLUE << "[O3 RISC-V CMP] test type is "
                  << _testTypes[_curTestCaseIdx]
                  << TC_DEF << std::endl;

        ////// per workload time counter
        auto wallStart = simClock::now();

        ////// init kride and ride
        doWorkloadInit(_curTestCaseIdx, _reqRegTest);
        doWorkloadCycle(false);
        _slaveRide.doWorkloadInit(_curTestCaseIdx, _reqRegTest);

        ////// capture after init (doWorkloadInit resets cycleCnt via doWorkloadExit)
        ull startCycle = cycleCnt;
        ////// rtl-only time base for this workload (init/reset cycles excluded).
        ////// reading here is safe: the sim thread is parked inside the trigger
        ////// handshake, so the model event's accumulator is quiescent.
        double katRtlBase  = getProxySimEvent()->getRtlSimSec();
        double rideRtlBase = _slaveRide.getRtlSimSec();

        bool comparePass = runCompareCycles();

        ////// rtl-eval-only time spent inside this workload's compared cycles
        double kathrynSec = getProxySimEvent()->getRtlSimSec() - katRtlBase;
        double rideSec    = _slaveRide.getRtlSimSec()          - rideRtlBase;
        double wallSec    = std::chrono::duration<double>(simClock::now() - wallStart).count();
        workloadTimes.push_back({_testTypes[_curTestCaseIdx],
                                 cycleCnt - startCycle,
                                 kathrynSec, rideSec, wallSec});

        std::cout << TC_BLUE << "[O3 RISC-V CMP] -----> sim done in " << cycleCnt << " cycles" << TC_DEF << std::endl;
        std::cout << TC_BLUE << "[O3 RISC-V CMP] -----> rtl-eval sim time"
                  << " kathryn "  << kathrynSec << " s |"
                  << " ridecore " << rideSec    << " s |"
                  << " wall "     << wallSec    << " s"
                  << TC_DEF << std::endl;

        if (!comparePass){
            std::cout << TC_RED << "[O3 RISC-V CMP] compare failed see slot writer for the reason mismatch" << TC_DEF << std::endl;
            errorIndexs.push_back(_curTestCaseIdx);
        }else{
            std::cout << TC_GREEN << "[O3 RISC-V CMP] compare pass" << TC_DEF << std::endl;
        }
        /////////////////////////////////

        if (_reqRegTest){
            testRegister();
            _slaveRide.testRegister();
        }
        finalPerfCol();
    }

    ////// cycle by cycle master/slave stepping + state compare. on a mismatch it
    ////// belays BELAYED_AFTER_MIS_CMP extra cycles so the slot writer catches the
    ////// window after the mismatch. return true if every compared cycle matched
    bool CombCtrl::runCompareCycles(){

        bool retard = false;
        int  retartedCount = 0;
        std::cout << TC_BLUE <<
                "[O3 RISC-V CMP] -----> start compare"
              << TC_DEF << std::endl;
        while (true){
            if (retard && (retartedCount < BELAYED_AFTER_MIS_CMP)){
                break;
            }
            doWorkloadCycle(_recordSlot);
            _slaveRide.doWorkloadCycle(_recordSlot);

            if (!retard){
                retard = !doCompare(); ///// if belayed  = commpare not corect!
            }
            if (retard){
                if (retartedCount >= BELAYED_AFTER_MIS_CMP){break;}
                retartedCount++;
            }


            if (isExecFin() && _slaveRide.isExecFin()){
                std::cout << TC_GREEN << "slave is equal " << TC_DEF << std::endl;
                break;
            }else if (isExecFin()){
                std::cout << TC_RED << "master is finish but slave not" << TC_DEF << std::endl;
                break;
            }else if (_slaveRide.isExecFin()){
                std::cout << TC_RED << "slave is  finish not like" << TC_DEF << std::endl;
                break;
            }
            if (cycleCnt % 10000 == 0){
                std::cout << TC_BLUE << "[O3 RISC-V CMP] -----> computing cycle " << cycleCnt << TC_DEF << std::endl;
            }

            ////// increase cycle counter
            incCycleCnt();
            _slaveRide.incCycleCnt();
        }
        return !retard;
    }

    ////// summarize per workload simulation time (stdout + csv)
    void CombCtrl::summarizeSimTime(const std::vector<WorkloadTime>& workloadTimes){

        double sumCyc = 0, sumKat = 0, sumRide = 0, sumWall = 0;

        std::ofstream csvFile(_prefixFolder + "simTime.csv");
        csvFile << "workload,cycles,kathryn_rtl_sec,ridecore_rtl_sec,wall_sec,"
                   "kathryn_rtl_cyclePerSec,ridecore_rtl_cyclePerSec\n";

        std::cout << TC_BLUE << "[O3 RISC-V CMP] per workload RTL-EVAL simulation time"
                  << "  [kathryn = inside model sim event cycle calls |"
                     " ridecore = inside Vpipeline::eval();"
                     " testbench drive/capture/compare + thread sync excluded,"
                     " wall keeps everything]"
                  << TC_DEF << std::endl;
        std::cout << std::left     << std::setw(12) << "workload"
                  << std::right    << std::setw(12) << "cycles"
                  << std::setw(16) << "kathrynRtl(s)"
                  << std::setw(16) << "rideRtl(s)"
                  << std::setw(12) << "wall(s)"
                  << std::setw(16) << "kathryn(c/s)"
                  << std::setw(16) << "ridecore(c/s)"
                  << std::endl;

        for (auto& wt: workloadTimes){
            double katCps  = (wt.kathrynSec > 0) ? (double)wt.cycles / wt.kathrynSec : 0;
            double rideCps = (wt.rideSec    > 0) ? (double)wt.cycles / wt.rideSec    : 0;

            csvFile << wt.name       << ","
                    << wt.cycles     << ","
                    << wt.kathrynSec << ","
                    << wt.rideSec    << ","
                    << wt.wallSec    << ","
                    << katCps        << ","
                    << rideCps       << "\n";

            std::cout << std::left     << std::setw(12) << wt.name
                      << std::right    << std::setw(12) << wt.cycles
                      << std::fixed    << std::setprecision(4)
                      << std::setw(16) << wt.kathrynSec
                      << std::setw(16) << wt.rideSec
                      << std::setw(12) << wt.wallSec
                      << std::setprecision(0)
                      << std::setw(16) << katCps
                      << std::setw(16) << rideCps
                      << std::endl;

            sumCyc  += (double)wt.cycles;
            sumKat  += wt.kathrynSec;
            sumRide += wt.rideSec;
            sumWall += wt.wallSec;
        }

        csvFile << "TOTAL," << (ull)sumCyc << "," << sumKat << "," << sumRide << "," << sumWall << ","
                << ((sumKat  > 0) ? sumCyc / sumKat  : 0) << ","
                << ((sumRide > 0) ? sumCyc / sumRide : 0) << "\n";

        std::cout << std::left     << std::setw(12) << "TOTAL"
                  << std::right    << std::setw(12) << (ull)sumCyc
                  << std::fixed    << std::setprecision(4)
                  << std::setw(16) << sumKat
                  << std::setw(16) << sumRide
                  << std::setw(12) << sumWall
                  << std::setprecision(0)
                  << std::setw(16) << ((sumKat  > 0) ? sumCyc / sumKat  : 0)
                  << std::setw(16) << ((sumRide > 0) ? sumCyc / sumRide : 0)
                  << std::endl;
        std::cout << std::defaultfloat;
        std::cout << TC_BLUE << "[O3 RISC-V CMP] sim time csv saved to "
                  << _prefixFolder + "simTime.csv" << TC_DEF << std::endl;
    }

    ////// final pass/fail rollup over every workload
    void CombCtrl::summarizeCompareResult(const std::vector<int>& errorIndexs){

        if (errorIndexs.empty()){
            std::cout << TC_RED << "[O3 RISC-V CMP] all tests passes" << TC_DEF << std::endl;
        }else{
            for (int errorIdx: errorIndexs){
                std::cout << TC_RED << "[O3 RISC-V CMP] error in test case " << errorIdx << TC_DEF << std::endl;
            }
        }
    }
}

#endif //BUILD_RIDECORE
