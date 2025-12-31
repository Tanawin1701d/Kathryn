//
// Created by tanawin on 14/10/25.
//

#include "o3_sim.h"


namespace kathryn::o3{

    O3Sim::O3Sim(CYCLE limitCycle,
                 const std::string& prefix,
                 std::vector<std::string> testTypes,
                 TopSim& top,
                 SimProxyBuildMode buildMode):

    SimInterface(limitCycle,
                 "/tmp/vcdDummy",
                 "/tmp/prodummy",
                 "O3RiscV",
                 buildMode,
                 false,
                 false,
                 1),
    _top(top),
    _core(_top.myCore),
    slotWriter({"MPFT"    , "ARF","RRF"  , "FETCH"  ,"DECODE",
                "DISPATCH", "RSV","ISSUE", "EXECUTE","COMMIT",
                "STBUF"
                   },
                {20       , 40   , 25    , 25       , 30,
                 30       , 35   , 25    , 35       , 25,
                 25},
                std::move(prefix + testTypes[0] + "/oslot.sl")),
    _prefixFolder(prefix),
    _slotRecorder(&slotWriter, &_core),
    _testTypes(testTypes){}

    void O3Sim::describeCon(){

        for (; _curTestCaseIdx < _testTypes.size(); _curTestCaseIdx++){
            std::cout << TC_BLUE << "[O3 RISC-V] test type is " << _testTypes[_curTestCaseIdx] << TC_DEF << std::endl;
            //////////////  read assembly and assertVal

            _vcdWriter-> renew(_prefixFolder + _testTypes[_curTestCaseIdx]+ "/owave.vcd");
            _flowWriter->renew(_prefixFolder + _testTypes[_curTestCaseIdx]+ "/oprofile.prof");
            slotWriter.  renew(_prefixFolder + _testTypes[_curTestCaseIdx]+ "/oslot.sl");
            //////// set reset wire to 1
            *rstWire = 1;
            //////// cycle before cycle cycle is running
            conNextCycle(1);
            *rstWire = 0;
            resetRegister();
            readAssembly (_prefixFolder + _testTypes[_curTestCaseIdx] + "/asm.out");
            readAssertVal(_prefixFolder + _testTypes[_curTestCaseIdx] + "/ast.out");
            resetDmem();
            //////// iterate for 100 cycle
            for (int i = 0; i <= 150; i++){
                ///////// give the data to
                readMem2Fetch();
                readWriteDataMemDoCmd(); ///// do the dmem command command
                ///////// record the system
                conEndCycle();
                readWriteDataMemGetCmd();
                _slotRecorder.recordSlot();
                conNextCycle(1);
            }
            /////////////////////////////////
            testRegister();
            finalPerfCol();
        }

    }

    void O3Sim::readMem2Fetch(){

        ///// get new instruction data
        ull curPc     = ull(_core.pm.ft.curPc);
            curPc     = curPc >> 2; ///// make 4bytes align
        ull aligner   = (ull(1) << 2) - 1; ///// to align 4 instructions per read 111111...11100
            aligner   = (~aligner);
        ull alignedPc = curPc & aligner;

        ///// get new instruction data
        _top.ijImem0.s(_imem[alignedPc + 0]);
        _top.ijImem1.s(_imem[alignedPc + 1]);
        _top.ijImem2.s(_imem[alignedPc + 2]);
        _top.ijImem3.s(_imem[alignedPc + 3]);

    }

    void O3Sim::resetDmem(){
        std::memset(_dmem, 0, sizeof(_dmem));
        lastDmemEnable  = false;
        lastDmemRead    = true;
        lastDmemAddr    = 0;
        lastDmemData    = 0;
    }

    void O3Sim::readWriteDataMemGetCmd(){

        ///// make command enable
        lastDmemEnable = true;
        ///// read data from CPU
        ull dmem_we     = ull(_core.pm.ldSt.dmem_we);
        ull dmem_rwaddr = ull(_core.pm.ldSt.dmem_rwaddr);
        ull dmem_wdata  = ull(_core.pm.ldSt.dmem_wdata);
        assert((dmem_rwaddr & 0b11) == 0b00);

        lastDmemRead = (dmem_we == 0);
        lastDmemAddr = static_cast<uint32_t>(dmem_rwaddr);
        lastDmemData = static_cast<uint32_t>(dmem_wdata);

    }

    void O3Sim::readWriteDataMemDoCmd(){

        if (!lastDmemEnable){return;}

        ///// At now, lastDmemAddr is quiet sure that there is not polute bit
        uint32_t aligned_addr = lastDmemAddr >> 2;

        if (lastDmemRead){
            _top.ijDmem0.s(_dmem[aligned_addr]);
        }else{
            _dmem[aligned_addr] = lastDmemData;
            std::cout << "write Detect at @ " << cvtNum2HexStr(lastDmemAddr) << " with data " << lastDmemData << std::endl;
        }

    }

    void O3Sim::readAssembly(const std::string& filePath){
        ////////// initialize file
        std::ifstream asmFile(filePath, std::ios::binary);
        if (!asmFile.is_open()){assert(false);}
        asmFile.seekg(0, std::ios::end);
        std::streampos fileSize = asmFile.tellg();
        assert((fileSize % 4) == 0);
        asmFile.seekg(0, std::ios::beg);

        /** read instruction from file and write it to memory block*/
        uint32_t writeAddr = 0;
        uint32_t instr;
        while(asmFile.read(reinterpret_cast<char*>(&instr), sizeof instr)){
            assert((instr & 0b11) == 0b11); ////// check instruction
            _imem[writeAddr] = instr;
            writeAddr++;
        }
        asmFile.close();

        ///// fill all with zero
        std::fill(_imem + writeAddr, _imem + IMEM_ROW, 0);

        std::cout << TC_GREEN << "initialize mem finish" << TC_DEF << std::endl;
    }

    void O3Sim::readAssertVal(const std::string& filePath){

        std::vector<std::string> rawVals;
        FileReaderBase reader(filePath);
        rawVals = reader.readLines();

        assert(rawVals.size() == REG_NUM);

        for (int regIdx = 0; regIdx < REG_NUM; regIdx++){
            _regTestVal[regIdx] = stoul(rawVals[regIdx]);
        }

    }

    void O3Sim::resetRegister(){
        for (int i = 0; i < REG_NUM; i++){
            _core.regArch.arf.archRegs(i) = 0;
        }
    }

    void O3Sim::testRegister(){
        bool pass = true;
        for (int i = 0;  i < REG_NUM; i++){
            if (_regTestVal[i] != (ull)_core.regArch.arf.archRegs(i)){
                pass = false;
                testAndPrint("fail reg" + std::to_string(i),
                             (ull)_core.regArch.arf.archRegs(i), _regTestVal[i]);
            }
        }
        if (pass){
            std::cout << TC_GREEN << "register val test pass" << TC_DEF << std::endl;
        }else{
            std::cout << TC_RED << "register val test fail" << TC_DEF << std::endl;
        }
    }

}