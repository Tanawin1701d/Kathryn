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
             //auto& pipStages = _riscv->pipProbe->getPipStage();

            /*** record fetch */
            PipSimProbe* fetch = &_riscv->fetch.pipSimProbe;
            writeFetchSlot(fetch);
            /*** decode*/
            PipSimProbe* decode = &_riscv->decode.pipSimProbe;
            writeDecodeSlot(decode);
            /*** execute*/
            PipSimProbe* execute = &_riscv->execute.pipSimProbe;
            writeExecuteSlot(execute);
            /*** write back*/
            PipSimProbe* writeBack = &_riscv->writeBack.pipSimProbe;
            writeWbSlot(writeBack);

            writeMem();

            _slotWriter->iterateCycle();
        }

    bool SlotRecorder::writeSlotIfPipStall(PIPE_STAGE2 stageIdx,
                                        PipSimProbe* pipSimProbe) const{
        if (pipSimProbe->isWaiting()){
            _slotWriter->addSlotVal(stageIdx, "WAIT_RECV");
            return true;
        }
        return false;
    }

    bool SlotRecorder::writeSlotIfZyncStall(PIPE_STAGE2 stageIdx,
                                            ZyncSimProb* zyncSimProbe){
        if (zyncSimProbe->isWaiting()){
            _slotWriter->addSlotVal(stageIdx, "WAIT_SEND");
            return true;
        }
        return false;

    }

    void SlotRecorder::writeFetchSlot(PipSimProbe* pipSimProbe) {
        assert(pipSimProbe != nullptr);
        if (writeSlotIfPipStall(RISC_FETCH, pipSimProbe)){return;}
        if (writeSlotIfZyncStall(RISC_FETCH, &_riscv->fetch.zyncSimProb)){return;}

        _slotWriter->addSlotVal(RISC_FETCH, std::to_string(ull(_riscv->fetch.parCheck)));

        ///if (_riscv->fetch.fetchBlock->getSimEngine()->isBlockRunning()) {
        if (_riscv->fetch.zyncSimProb.isExecuting()){

            if (ull(_riscv->fetch.readEn)) {
                if (ull(_riscv->fetch.readFin)) {
                    _slotWriter->addSlotVal(RISC_FETCH, "READ ADDR");
                    _slotWriter->addSlotVal(RISC_FETCH,
                                          cvtNum2HexStr(ull(_riscv->coreData.pc)));
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

        void SlotRecorder::writeDecodeSlot(PipSimProbe* pipSimProbe) {
            assert(pipSimProbe != nullptr);

            //_slotWriter->addSlotVal(RISC_FETCH, "parStart" + std::to_string(ull(_riscv->decode.parCheck)));

            if (writeSlotIfPipStall (RISC_DECODE, pipSimProbe)){return;}
            if (writeSlotIfZyncStall(RISC_DECODE, &_riscv->decode.zyncSimProb)){return;}

            //////////// now decoding

            ull rawIntr  = ull(_riscv->coreData.ft.fetch_instr);
            ull pc       = ull(_riscv->coreData.ft.fetch_pc);
            ull next_pc  = ull(_riscv->coreData.ft.fetch_nextpc);


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

            //if (_riscv->decode.decodeBlk->getSimEngine()->isBlockRunning()) {
            if (_riscv->decode.zyncSimProb.isExecuting()){
                std::string decStr = (decMap.find(op) != decMap.end()) ? decMap[op] : "UNKNOWN";
                _slotWriter->addSlotVal(RISC_DECODE, decStr);
                _slotWriter->addSlotVal(RISC_DECODE, cvtNum2HexStr(pc));
                _slotWriter->addSlotVal(RISC_DECODE, cvtNum2HexStr(next_pc));
            }else{
                _slotWriter->addSlotVal(RISC_DECODE, "UNKNOWN STATE");
            }

        }

        void SlotRecorder::writeExecuteSlot(PipSimProbe* pipSimProbe) {
            assert(pipSimProbe != nullptr);

            if (writeSlotIfPipStall(RISC_EXECUTE, pipSimProbe)){return;}

            DECODE_DATA& decodedUop = _riscv->coreData.dc;
            InstrRepo& instrRepo    = decodedUop.repo;
            /** decode
             *
             * uop
             *
             * */

            InstrRepoDebugMsg decodedDebugMsg = instrRepo.getGetDbgMsg();

            _slotWriter->addSlotVal(RISC_EXECUTE,
                                    "mop " + decodedDebugMsg.mopName);
            _slotWriter->addSlotVal(RISC_EXECUTE,
                                    "uop" + decodedDebugMsg.uopName);
            if (decodedDebugMsg.uopName.empty()){
                _slotWriter->addSlotVal(RISC_EXECUTE,
                                        "error" + decodedDebugMsg.errorCause);
            }


            if (_riscv->execute.acRegSimProb.isExecuting()){
                _slotWriter->addSlotVal(RISC_EXECUTE, "REG_ACCESS");
            }else if (_riscv->execute.aluSimProb.isExecuting()){
                _slotWriter->addSlotVal(RISC_EXECUTE, "SIMPLE_ALU");
            }else if (_riscv->execute.complexAluSimProb.isExecuting()){
                _slotWriter->addSlotVal(RISC_EXECUTE, "COMPLEX_ALU");
            }else{
                _slotWriter->addSlotVal(RISC_EXECUTE, "unknownState");
            }

            /** register write */

            writeReg("r1", RISC_EXECUTE, instrRepo.getSrcReg(0));
            writeReg("r2", RISC_EXECUTE, instrRepo.getSrcReg(1));
            writeReg("r3", RISC_EXECUTE, instrRepo.getSrcReg(2));
            writeReg("rd", RISC_EXECUTE, instrRepo.getDesReg(0));
            _slotWriter->addSlotVal(RISC_EXECUTE, "readFin " + std::to_string(ull(_riscv->execute.readFn)));
            _slotWriter->addSlotVal(RISC_EXECUTE, "readEn " + std::to_string(ull(_riscv->execute.readEn)));
            //_slotWriter->addSlotVal(RISC_EXECUTE, "finLS " + std::to_string(ull(_riscv->execute.testExit)));
            _slotWriter->addSlotVal(RISC_EXECUTE, "readAddr " + std::to_string(ull(_riscv->execute.readAddr)));
            _slotWriter->addSlotVal(RISC_EXECUTE, "read1020 " + std::to_string(ull(_riscv->memBlk._myMem.at(1020 >> 2))));
            _slotWriter->addSlotVal(RISC_EXECUTE, "read1024 " + std::to_string(ull(_riscv->memBlk._myMem.at(1024 >> 2))));
            _slotWriter->addSlotVal(RISC_EXECUTE, "read1028 " + std::to_string(ull(_riscv->memBlk._myMem.at(1028 >> 2))));
            _slotWriter->addSlotVal(RISC_EXECUTE, "resetSignal " + std::to_string(ull(_riscv->execute.misPredic)));


        }

        void SlotRecorder::writeWbSlot(PipSimProbe* pipSimProbe) {
            assert(pipSimProbe != nullptr);
            if (writeSlotIfPipStall(RISC_WB, pipSimProbe)){return;}

            OPR_HW& wbReg = _riscv->coreData.ex.wbData;

            writeReg("rwb", RISC_WB, wbReg);
            _slotWriter->addSlotVal
            (RISC_WB, std::to_string(ull(wbReg.data)));
        }



        void SlotRecorder::writeReg(const std::string& prefix,
                                             PIPE_STAGE2  pipeStage,
                                             OPR_HW&      regEle){

            _slotWriter->addSlotVal(pipeStage, prefix + " id " +
                                    std::to_string(ull(regEle.idx)) + " v" +
                                    std::to_string(ull(regEle.valid)) + " val " +
                                    std::to_string(ull(regEle.data))
                                    );
        }

}