//
// Created by tanawin on 26/4/2567.
//

#include "RISCV_sim.h"
#include "util/termColor/termColor.h"

#include <utility>

namespace kathryn{

    namespace riscv{


        RiscvSim::RiscvSim(CYCLE       limitCycle,
                           const std::string& prefix,
                           std::vector<std::string> testTypes,
                           Riscv& core,
                           SimProxyBuildMode buildMode): ///// init first test case here
                SimInterface(limitCycle,
                             "/tmp/vcdDummy",
                             "/tmp/profdummy",
                             "simpleRiscV",
                             buildMode
                             ),
                _core(core),
                slotWriter({"fetch", "decode", "execute", "wb", "mem"},
                           25,
                           std::move(prefix + testTypes[0] + "/oslot.sl")),
                _prefixFolder(prefix),
                _slotSecorder(&slotWriter, &core),
                _testTypes(testTypes){}

        void RiscvSim::describeCon() {


            for (;_curTestCaseIdx < _testTypes.size(); _curTestCaseIdx++){
                std::cout << TC_BLUE << "[RISCV] test type is " << _testTypes[_curTestCaseIdx] << TC_DEF << std::endl;
                //////////////  read assembly and assertVal

                _vcdWriter-> renew(_prefixFolder + _testTypes[_curTestCaseIdx]+ "/owave.vcd");
                _flowWriter->renew(_prefixFolder + _testTypes[_curTestCaseIdx]+ "/oprofile.prof");
                slotWriter.  renew(_prefixFolder + _testTypes[_curTestCaseIdx]+ "/oslot.sl");

                /////////// set reset wire to 1
                *rstWire = 1;
                ////////// value before cycle is running
                conNextCycle(1);
                *rstWire = 0;
                _core.coreData.pc = 0;
                resetRegister();
                readAssembly(_prefixFolder +  _testTypes[_curTestCaseIdx] + "/asm.out");
                readAssertVal(_prefixFolder + _testTypes[_curTestCaseIdx] + "/ast.out");
                //////////////////////////////////////////////////////////////////////
                for (int i = 0; i <= 100; i++){
                    conEndCycle();
                    _slotSecorder.recordSlot();
                    conNextCycle(1);
                }
                //////////////////////////////////////////////////////////////////////
                testRegister();
                finalPerfCol();
                ////////////////////////////////////////////////////////////////////////
            }


        }

        void RiscvSim::readAssembly(const std::string& filePath){

            ///////// initialize file
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
                _core.memBlk._myMem.at(writeAddr).setVar(instr);
                writeAddr++;
            }
            asmFile.close();

            ///////////// fill it with zero


            // for (;writeAddr < _core.memBlk._myMem.getDepthSize(); writeAddr++){
            //     _core.memBlk._myMem.at(writeAddr).setVar(0);
            // }

            size_t setZeroAmt = _core.memBlk._myMem.getDepthSize() - writeAddr;
            _core.memBlk._myMem.at(writeAddr).setVarArr(0, setZeroAmt);





            std::cout << TC_GREEN << "initialize mem finish" << TC_DEF << std::endl;



        }

        void RiscvSim::resetRegister(){
            for (int i = 0; i < AMT_REG; i++){
                _core.regFile.at(i).setVar(0);
            }
        }

        void RiscvSim::readAssertVal(const std::string& filePath){

            std::vector<std::string> rawVals;
            FileReaderBase reader(filePath);
            rawVals = reader.readLines();

            assert(rawVals.size() == AMT_REG);

            for (int regIdx = 0; regIdx < AMT_REG; regIdx++){
                ///std::cout << regIdx << " reg val  "<< stoul(rawVals[regIdx]) << std::endl;
                _regTestVal[regIdx] = stoul(rawVals[regIdx]);
                ///std::cout << regIdx << " reg val  "<< _regTestVal[regIdx] << std::endl;
            }
        }

        void RiscvSim::testRegister(){
            bool pass = true;
            for (int i = 0;  i < AMT_REG; i++){
                if (_regTestVal[i] != (ull)_core.regFile.at(i)){
                    pass = false;
                    testAndPrint("fail reg" + std::to_string(i),
                                 (ull)_core.regFile.at(i), _regTestVal[i]);
                }
            }
            if (pass){
                std::cout << TC_GREEN << "register val test pass" << TC_DEF << std::endl;
            }else{
                std::cout << TC_RED << "register val test fail" << TC_DEF << std::endl;
            }
        }


    }

}