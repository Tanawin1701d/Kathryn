//
// Created by tanawin on 15/6/2024.
//

#include "slotRecorder.h"

namespace kathryn::riscv{

    void SlotRecorder::recordSlot() {

            /** please bare in mind that this recorder work correctly when
             *  it is the end of the cycle
             * */
            assert(_riscv != nullptr);
             auto& pipStages = _riscv->pipProbe->getPipStage();

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

            _slotWriter->iterateCycle();
        }

    bool SlotRecorder::writeSlotIfStall(PIPE_STAGE2 stageIdx,
                                                 FlowBlockPipeBase* pipfb) {

        ///////// if it is running in con thread type it will be run after model sim but before exit event of all type
        assert(pipfb != nullptr);
        /** check recv block*/
        FlowBlockPipeCom* recvPipCom = pipfb->getRecvFbPipCom();
        bool recvRunning = recvPipCom->getSimEngine()->isBlockRunning();
        if (recvRunning)
            _slotWriter->addSlotVal(stageIdx, "WAIT_RECV");

        /** check send block*/
        FlowBlockPipeCom* sendPipCom = pipfb->getSendFbPipCom();
        bool sendRunning =  sendPipCom->getSimEngine()->isBlockRunning();
        if (sendRunning)
            _slotWriter->addSlotVal(stageIdx, "WAIT_SEND");

        return recvRunning | sendRunning;
    }

        void SlotRecorder::writeFetchSlot(FlowBlockPipeBase* pipblock) {
            assert(pipblock != nullptr);
            if (writeSlotIfStall(RISC_FETCH, pipblock)){return;}

            _slotWriter->addSlotVal(RISC_FETCH, std::to_string(ull(_riscv->fetch.parCheck)));

            if (_riscv->fetch.fetchBlock->getSimEngine()->isBlockRunning()) {

                if (ull(_riscv->fetch.readEn)) {
                    if (ull(_riscv->fetch.readFin)) {
                        _slotWriter->addSlotVal(RISC_FETCH, "READ ADDR");
                        _slotWriter->addSlotVal(RISC_FETCH,
                                              cvtNum2HexStr(ull(_riscv->fetch._reqPc)));
                    } else {
                        _slotWriter->addSlotVal(RISC_FETCH, "FETCHING WAIT4MEM");
                    }
                } else {
                    _slotWriter->addSlotVal(RISC_FETCH, "FETCHING  WAIT4REQ");
                }

            }else{
                _slotWriter->addSlotVal(RISC_FETCH, "Unknown State");
            }

            _slotWriter->addSlotVal(RISC_FETCH, "isFin " + std::to_string(ull(_riscv->fetch.readFin)));
            _slotWriter->addSlotVal(RISC_FETCH, "isEn " + std::to_string(ull(_riscv->fetch.readEn)));


        }

        void SlotRecorder::writeDecodeSlot(FlowBlockPipeBase* pipblock) {
            assert(pipblock != nullptr);

            _slotWriter->addSlotVal(RISC_FETCH, "parStart" + std::to_string(ull(_riscv->decode.parCheck)));

            if (writeSlotIfStall(RISC_DECODE, pipblock)){return;}

            //////////// now decoding

            ull rawIntr  = ull(_riscv->fetchData.fetch_instr);
            ull pc       = ull(_riscv->fetchData.fetch_pc);
            ull next_pc  = ull(_riscv->fetchData.fetch_nextpc);


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

            if (_riscv->decode.decodeBlk->getSimEngine()->isBlockRunning()) {

                std::string decStr = (decMap.find(op) != decMap.end()) ? decMap[op] : "UNKNOWN";
                _slotWriter->addSlotVal(RISC_DECODE, decStr);
                _slotWriter->addSlotVal(RISC_DECODE, cvtNum2HexStr(pc));
                _slotWriter->addSlotVal(RISC_DECODE, cvtNum2HexStr(next_pc));
            }else{
                _slotWriter->addSlotVal(RISC_DECODE, "UNKNOWN STATE");
            }

        }

        void SlotRecorder::writeExecuteSlot(FlowBlockPipeBase* pipblock) {
            assert(pipblock != nullptr);

            if (writeSlotIfStall(RISC_EXECUTE, pipblock)){return;}

            UOp& decodedUop = _riscv->execute._decodedUop;

            /** decode
             *
             * uop
             *
             * */
            if (ull(decodedUop.opLs.isUopUse)){
                int size = 8 * (1 << ull(decodedUop.opLs.size));
                _slotWriter->addSlotVal(RISC_EXECUTE,
                                      (ull(decodedUop.opLs.isMemLoad) ? "LOAD": "STORE") +
                                      std::to_string(size));
            }else if (ull(decodedUop.opAlu.isUopUse)){
                _slotWriter->addSlotVal(RISC_EXECUTE,
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
                _slotWriter->addSlotVal(RISC_EXECUTE,
                                      ull(decodedUop.opCtrlFlow.isJal)  ? "JAL":
                                      ull(decodedUop.opCtrlFlow.isJalR) ? "JALR":
                                      ull(decodedUop.opCtrlFlow.isEq)   ? "JALEQ":
                                      ull(decodedUop.opCtrlFlow.isNEq)  ? "BRA_NEQ":
                                      ull(decodedUop.opCtrlFlow.isLt)   ? "BRA_LT":
                                      ull(decodedUop.opCtrlFlow.isGe)   ? "BRA_GE":
                                      "UNKNOW_BRANCH"
                );
            }else if (ull(decodedUop.opLdPc.isUopUse)){
                _slotWriter->addSlotVal(RISC_EXECUTE,ull(decodedUop.opLdPc.needPc) ? "LDPC": "LDPC_PC");
            }else{
                _slotWriter->addSlotVal(RISC_EXECUTE, "UNKNOWN_UOP");
            }

            if (_riscv->execute.regAccessBlock
                ->getSimEngine()->isBlockRunning()){
                _slotWriter->addSlotVal(RISC_EXECUTE, "REG_ACCESS");
            }else if (_riscv->execute.aluBlock
                    ->getSimEngine()->isBlockRunning()){
                _slotWriter->addSlotVal(RISC_EXECUTE, "SIMPLE_ALU");
            }else if (_riscv->execute.aluBlock
                    ->getSimEngine()->isBlockRunning()){
                _slotWriter->addSlotVal(RISC_EXECUTE, "COMPLEX_ALU");
            }else{
                _slotWriter->addSlotVal(RISC_EXECUTE, "unknownState");
            }

            /** register write */

            writeReg("r1", RISC_EXECUTE, decodedUop.regData[RS_1  ]);
            writeReg("r2", RISC_EXECUTE, decodedUop.regData[RS_2  ]);
            writeReg("r3", RISC_EXECUTE, decodedUop.regData[RS_3  ]);
            writeReg("rd", RISC_EXECUTE, decodedUop.regData[RS_des]);
            _slotWriter->addSlotVal(RISC_EXECUTE, "readFin " + std::to_string(ull(_riscv->execute.readFn)));
            _slotWriter->addSlotVal(RISC_EXECUTE, "readEn " + std::to_string(ull(_riscv->execute.readEn)));
            _slotWriter->addSlotVal(RISC_EXECUTE, "finLS " + std::to_string(ull(_riscv->execute.testExit)));
            _slotWriter->addSlotVal(RISC_EXECUTE, "readAddr " + std::to_string(ull(_riscv->execute.readAddr)));
            _slotWriter->addSlotVal(RISC_EXECUTE, "m16 " + std::to_string(ull(_riscv->execute.m16)));
            _slotWriter->addSlotVal(RISC_EXECUTE, "read1020 " + std::to_string(ull(_riscv->memBlk._myMem.at(1020 >> 2))));
            _slotWriter->addSlotVal(RISC_EXECUTE, "read1024 " + std::to_string(ull(_riscv->memBlk._myMem.at(1024 >> 2))));
            _slotWriter->addSlotVal(RISC_EXECUTE, "read1028 " + std::to_string(ull(_riscv->memBlk._myMem.at(1028 >> 2))));
            _slotWriter->addSlotVal(RISC_EXECUTE, "resetSignal " + std::to_string(ull(_riscv->misPredic)));


        }

        void SlotRecorder::writeWbSlot(FlowBlockPipeBase* pipblock) {
            assert(pipblock != nullptr);
            if (writeSlotIfStall(RISC_WB, pipblock)){return;}

            RegEle& wbReg = _riscv->wbData;

            writeReg("rwb", RISC_WB, wbReg);
            _slotWriter->addSlotVal
            (RISC_WB, std::to_string(ull(wbReg.val)));
        }



        void SlotRecorder::writeReg(const std::string& prefix,
                                             PIPE_STAGE2         pipeStage,
                                             RegEle&            regEle){

            _slotWriter->addSlotVal(pipeStage, prefix + " id " +
                                                    std::to_string(ull(regEle.idx)) + " v" +
                                                    std::to_string(ull(regEle.valid)) + " val " +
                                                    std::to_string(ull(regEle.val))
                                                    );
        }

}