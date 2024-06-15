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
                           Riscv& core): ///// init first test case here
                SimInterface(limitCycle,
                             "/tmp/vcdDummy",
                             "/tmp/profdummy",
                             "simpleRiscV"
                             ),
                _core(core),
                slotWriter({"fetch", "decode", "execute", "wb", "mem"},
                           25,
                           std::move(prefix + testTypes[0] + "/oslot.sl")),
                _prefixFolder(prefix),
                _testTypes(testTypes){}

        void RiscvSim::describeCon() {


            for (;_curTestCaseIdx < _testTypes.size(); _curTestCaseIdx++){
                //////////////  read assembly and assertVal
                std::cout << TC_GREEN << "testing riscv instruction >>>> " +
                    _testTypes[_curTestCaseIdx] << TC_DEF << std::endl;
                _vcdWriter-> renew(_prefixFolder + _testTypes[_curTestCaseIdx]+ "/owave.vcd");
                _flowWriter->renew(_prefixFolder + _testTypes[_curTestCaseIdx]+ "/oprofile.prof");
                slotWriter.  renew(_prefixFolder + _testTypes[_curTestCaseIdx]+ "/oslot.sl");

                /////////// set reset wire to 1
                *rstWire = 1;
                ////////// value before cycle is running
                conNextCycle(1);
                *rstWire = 0;
                _core.pc = 0;
                resetRegister();
                readAssembly(_prefixFolder +  _testTypes[_curTestCaseIdx] + "/asm.out");
                readAssertVal(_prefixFolder + _testTypes[_curTestCaseIdx] + "/ast.out");
                //////////////////////////////////////////////////////////////////////
                for (int i = 0; i <= 100; i++){
                    // if (i == 55){
                    //     std::cout << i << std::endl;
                    // }
                    conEndCycle();
                    recordSlot();
                    conNextCycle(1);
                }
                //////////////////////////////////////////////////////////////////////
                testRegister();
                ////////////////////////////////////////////////////////////////////////
            }

        }

        void RiscvSim::recordSlot() {

            /** please bare in mind that this recorder work correctly when
             *  it is the end of the cycle
             * */
             auto& pipStages = _core.pipProbe->getPipStage();

            /*** record fetch */
            FlowBlockPipeBase* fetch = pipStages[RISC_FETCH];
            writeFetchSlot(fetch);
            /*** decode*/
            FlowBlockPipeBase* decode = pipStages[RISC_DECODE];
            writeDecodeSlot(decode);
            /*** execute*/
            FlowBlockPipeBase* execute = pipStages[RISC_EXECUTE];
            writeExecuteSlot(execute);
            /*** write back*/
            FlowBlockPipeBase* writeBack = pipStages[RISC_WB];
            writeWbSlot(writeBack);

            writeMem();

            slotWriter.iterateCycle();
        }

        bool RiscvSim::writeSlotIfStall(PIPE_STAGE2 stageIdx,
                                                     FlowBlockPipeBase* pipfb) {

            ///////// if it is running in con thread type it will be run after model sim but before exit event of all type
            assert(pipfb != nullptr);
            /** check recv block*/
            FlowBlockPipeCom* recvPipCom = pipfb->getRecvFbPipCom();
            bool recvRunning = recvPipCom->getSimEngine()->isBlockRunning();
            if (recvRunning)
                slotWriter.addSlotVal(stageIdx, "WAIT_RECV");

            /** check send block*/
            FlowBlockPipeCom* sendPipCom = pipfb->getSendFbPipCom();
            bool sendRunning =  sendPipCom->getSimEngine()->isBlockRunning();
            if (sendRunning)
                slotWriter.addSlotVal(stageIdx, "WAIT_SEND");

            return recvRunning | sendRunning;
        }

        void RiscvSim::writeFetchSlot(FlowBlockPipeBase* pipblock) {
            assert(pipblock != nullptr);
            if (writeSlotIfStall(RISC_FETCH, pipblock)){return;}

            slotWriter.addSlotVal(RISC_FETCH, std::to_string(ull(_core.fetch.parCheck)));

            if (_core.fetch.fetchBlock->getSimEngine()->isBlockRunning()) {

                if (ull(_core.fetch.readEn)) {
                    if (ull(_core.fetch.readFin)) {
                        slotWriter.addSlotVal(RISC_FETCH, "READ ADDR");
                        slotWriter.addSlotVal(RISC_FETCH,
                                              cvtNum2HexStr(ull(_core.fetch._reqPc)));
                    } else {
                        slotWriter.addSlotVal(RISC_FETCH, "FETCHING WAIT4MEM");
                    }
                } else {
                    slotWriter.addSlotVal(RISC_FETCH, "FETCHING  WAIT4REQ");
                }

            }else{
                slotWriter.addSlotVal(RISC_FETCH, "Unknown State");
            }

            slotWriter.addSlotVal(RISC_FETCH, "isFin " + std::to_string(ull(_core.fetch.readFin)));
            slotWriter.addSlotVal(RISC_FETCH, "isEn " + std::to_string(ull(_core.fetch.readEn)));


        }

        void RiscvSim::writeDecodeSlot(FlowBlockPipeBase* pipblock) {
            assert(pipblock != nullptr);

            slotWriter.addSlotVal(RISC_FETCH, "parStart" + std::to_string(ull(_core.decode.parCheck)));

            if (writeSlotIfStall(RISC_DECODE, pipblock)){return;}

            //////////// now decoding

            ull rawIntr  = ull(_core.fetchData.fetch_instr);
            ull pc       = ull(_core.fetchData.fetch_pc);
            ull next_pc  = ull(_core.fetchData.fetch_nextpc);


            ull opMaskBit = (1 << 7) - 1;
            ull op = rawIntr & opMaskBit;

            std::map<ull, std::string> decMap = {
                {0b00'000'11, "LOAD"},
                {0b01'000'11, "STORE"},
                {0b00'011'11, "MISCMEM"},
                {0b11'000'11, "BRANCH"},
                {0b11'001'11, "JALR"},
                {0b11'011'11, "JAL"},
                {0b00'100'11, "OP_IMM"},
                {0b01'100'11, "OP"},
                {0b00'101'11, "AUIPC"},
                {0b01'101'11, "LUI"},
                {0b11'100'11, "SYSTEM"},
            };

            if (_core.decode.decodeBlk->getSimEngine()->isBlockRunning()) {

                std::string decStr = (decMap.find(op) != decMap.end()) ? decMap[op] : "UNKNOWN";
                slotWriter.addSlotVal(RISC_DECODE, decStr);
                slotWriter.addSlotVal(RISC_DECODE, cvtNum2HexStr(pc));
                slotWriter.addSlotVal(RISC_DECODE, cvtNum2HexStr(next_pc));
            }else{
                slotWriter.addSlotVal(RISC_DECODE, "UNKNOWN STATE");
            }

        }

        void RiscvSim::writeExecuteSlot(FlowBlockPipeBase* pipblock) {
            assert(pipblock != nullptr);

            if (writeSlotIfStall(RISC_EXECUTE, pipblock)){return;}

            UOp& decodedUop = _core.execute._decodedUop;

            /** decode
             *
             * uop
             *
             * */
            if (ull(decodedUop.opLs.isUopUse)){
                int size = 8 * (1 << ull(decodedUop.opLs.size));
                slotWriter.addSlotVal(RISC_EXECUTE,
                                      (ull(decodedUop.opLs.isMemLoad) ? "LOAD": "STORE") +
                                      std::to_string(size));
            }else if (ull(decodedUop.opAlu.isUopUse)){
                slotWriter.addSlotVal(RISC_EXECUTE,
                                      ull(decodedUop.opAlu.isAdd)        ? "Add":
                                      ull(decodedUop.opAlu.isSub)              ? "Sub":
                                      ull(decodedUop.opAlu.isXor)              ? "Xor":
                                      ull(decodedUop.opAlu.isOr)               ? "Or":
                                      ull(decodedUop.opAlu.isAnd)              ? "And":
                                      ull(decodedUop.opAlu.isCmpLessThanSign)  ? "CmpLessThanSign":
                                      ull(decodedUop.opAlu.isCmpLessThanUSign) ? "CmpLessThanUSign":
                                      ull(decodedUop.opAlu.isShiftLeftLogical) ? "ShiftLeftLogical":
                                      ull(decodedUop.opAlu.isShiftRightLogical)? "ShiftRightLogical":
                                      ull(decodedUop.opAlu.isShiftRightArith)  ? "ShiftRightArith":
                                      "UNKNOW_alu"
                );

            }else if (ull(decodedUop.opCtrlFlow.isUopUse)){
                slotWriter.addSlotVal(RISC_EXECUTE,
                                      ull(decodedUop.opCtrlFlow.isJal)  ? "JAL":
                                      ull(decodedUop.opCtrlFlow.isJalR) ? "JALR":
                                      ull(decodedUop.opCtrlFlow.isEq)   ? "JALEQ":
                                      ull(decodedUop.opCtrlFlow.isNEq)  ? "BRA_NEQ":
                                      ull(decodedUop.opCtrlFlow.isLt)   ? "BRA_LT":
                                      ull(decodedUop.opCtrlFlow.isGe)   ? "BRA_GE":
                                      "UNKNOW_BRANCH"
                );
            }else if (ull(decodedUop.opLdPc.isUopUse)){
                slotWriter.addSlotVal(RISC_EXECUTE,ull(decodedUop.opLdPc.needPc) ? "LDPC": "LDPC_PC");
            }else{
                slotWriter.addSlotVal(RISC_EXECUTE, "UNKNOWN_UOP");
            }

            if (_core.execute.regAccessBlock
                ->getSimEngine()->isBlockRunning()){
                slotWriter.addSlotVal(RISC_EXECUTE, "REG_ACCESS");
            }else if (_core.execute.aluBlock
                    ->getSimEngine()->isBlockRunning()){
                slotWriter.addSlotVal(RISC_EXECUTE, "SIMPLE_ALU");
            }else if (_core.execute.aluBlock
                    ->getSimEngine()->isBlockRunning()){
                slotWriter.addSlotVal(RISC_EXECUTE, "COMPLEX_ALU");
            }else{
                slotWriter.addSlotVal(RISC_EXECUTE, "unknownState");
            }

            /** register write */

            writeReg("r1", RISC_EXECUTE, decodedUop.regData[RS_1  ]);
            writeReg("r2", RISC_EXECUTE, decodedUop.regData[RS_2  ]);
            writeReg("r3", RISC_EXECUTE, decodedUop.regData[RS_3  ]);
            writeReg("rd", RISC_EXECUTE, decodedUop.regData[RS_des]);
            slotWriter.addSlotVal(RISC_EXECUTE, "readFin " + std::to_string(ull(_core.execute.readFn)));
            slotWriter.addSlotVal(RISC_EXECUTE, "readEn " + std::to_string(ull(_core.execute.readEn)));
            slotWriter.addSlotVal(RISC_EXECUTE, "finLS " + std::to_string(ull(_core.execute.testExit)));
            slotWriter.addSlotVal(RISC_EXECUTE, "readAddr " + std::to_string(ull(_core.execute.readAddr)));
            slotWriter.addSlotVal(RISC_EXECUTE, "m16 " + std::to_string(ull(_core.execute.m16)));
            slotWriter.addSlotVal(RISC_EXECUTE, "read1020 " + std::to_string(ull(_core.memBlk._myMem.at(1020 >> 2))));
            slotWriter.addSlotVal(RISC_EXECUTE, "read1024 " + std::to_string(ull(_core.memBlk._myMem.at(1024 >> 2))));
            slotWriter.addSlotVal(RISC_EXECUTE, "read1028 " + std::to_string(ull(_core.memBlk._myMem.at(1028 >> 2))));
            slotWriter.addSlotVal(RISC_EXECUTE, "resetSignal " + std::to_string(ull(_core.misPredic)));


        }

        void RiscvSim::writeWbSlot(FlowBlockPipeBase* pipblock) {
            assert(pipblock != nullptr);
            if (writeSlotIfStall(RISC_WB, pipblock)){return;}

            RegEle& wbReg = _core.wbData;

            writeReg("rwb", RISC_WB, wbReg);
            slotWriter.addSlotVal
            (RISC_WB, std::to_string(ull(wbReg.val)));
        }



        void RiscvSim::writeReg(const std::string& prefix,
                                             PIPE_STAGE2         pipeStage,
                                             RegEle&            regEle){

            slotWriter.addSlotVal(pipeStage, prefix + " id " +
                                                    std::to_string(ull(regEle.idx)) + " v" +
                                                    std::to_string(ull(regEle.valid)) + " val " +
                                                    std::to_string(ull(regEle.val))
                                                    );
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

            for (;writeAddr < _core.memBlk._myMem.getDepthSize(); writeAddr++){
                _core.memBlk._myMem.at(writeAddr).setVar(0);
            }

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