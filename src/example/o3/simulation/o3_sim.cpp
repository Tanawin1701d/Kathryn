//
// Created by tanawin on 14/10/25.
//

#include "o3_sim.h"


namespace kathryn::o3{

    O3Sim::O3Sim(CYCLE limitCycle,
                 const std::string& prefix,
                 std::vector<std::string> testTypes,
                 Core& core,
                 SimProxyBuildMode buildMode):

    SimInterface(limitCycle,
                 "/tmp/vcdDummy",
                 "/tmp/prodummy",
                 "O3RiscV",
                 buildMode),
    _core(core),
    slotWriter({"MPFT","ARF","RRF","FETCH","DECODE",
                "DISPATCH","RSV","ISSUE","EXECUTE","WB","COMMIT"},
                25,
                std::move(prefix + testTypes[0] + "/oslot.sl")),
    _prefixFolder(prefix),
    _slotRecorder(&slotWriter, &core),
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
            readAssertVal(_prefixFolder + _testTypes[_curTestCaseIdx] + "/asm.out");
            //////// iterate for 100 cycle
            for (int i = 0; i <= 100; i++){
                conEndCycle();
                _slotRecorder.recordSlot();
                conNextCycle(1);
            }
            /////////////////////////////////
            testRegister();
            finalPerfCol();
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
        MemBlock*  iMems[4] = {
            &_core.pFetch.iMem0,
            &_core.pFetch.iMem1,
            &_core.pFetch.iMem2,
            &_core.pFetch.iMem3
        };
        while(asmFile.read(reinterpret_cast<char*>(&instr), sizeof instr)){
            assert((instr & 0b11) == 0b11); ////// check instruction
            uint32_t parititionedAddr = writeAddr >> 2;
            uint32_t offset           = writeAddr & 0b11;
            iMems[offset]->at(parititionedAddr).setVar(instr);
            writeAddr++;
        }
        asmFile.close();

        ///////////// fill it with zero
        for (int i = 0; i < 4; i++){
            /// i = 00/01/10/11
            uint32_t partitionedAddr = writeAddr >> 2;
            if (i < (writeAddr & 0b11)){
                //////// the address partition address has been written already
                partitionedAddr++;
            }
            size_t setZeroAmt = - iMems[i]->getDepthSize() - partitionedAddr;
            iMems[i]->at(partitionedAddr).setVarArr(0, setZeroAmt);
        }

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