//
// Created by tanawin on 26/4/2567.
//

#include "RISCV_sim.h"
#include "util/termColor/termColor.h"

#include <utility>

namespace kathryn{

    namespace riscv{


        RiscvSimInterface::RiscvSimInterface(CYCLE       limitCycle,
                                             std::string vcdFilePath,
                                             std::string profileFilePath,
                                             std::string slotFilePath,
                                             Riscv& core):
                SimInterface(limitCycle,std::move(vcdFilePath),std::move(profileFilePath)),
                _core(core),
                slotWriter({"fetch", "decode", "execute", "wb"},
                           25,
                           std::move(slotFilePath)){
        }

        void RiscvSimInterface::describe() {

            for (int writeAddr = 0;
                 writeAddr < 400;////(1 << MEM_ADDR_IDX_ACTUAL_AL32);
                 writeAddr++){
                _core.memBlk._myMem.s(writeAddr, 0b0010011);
            }
            std::cout << TC_GREEN << "initialize mem finish" << TC_DEF << std::endl;

        }

        void RiscvSimInterface::describeCon() {

            for (int i = 0; i <= 100; i++){
                conEndCycle();
                recordSlot();
                conNextCycle(1);
            }

        }

        void RiscvSimInterface::recordSlot() {

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

            slotWriter.iterateCycle();
        }

        bool RiscvSimInterface::writeSlotIfStall(PIPE_STAGE stageIdx,
                                                 FlowBlockPipeBase* pipfb) {

            ///////// if it is running in con thread type it will be run after model sim but before exit event of all type
            assert(pipfb != nullptr);
            /** check recv block*/
            FlowBlockPipeCom* recvPipCom = pipfb->getRecvFbPipCom();
            bool recvRunning = recvPipCom->getFlowSimEngine()->isBlockOrNodeRunning();
            if (recvRunning)
                slotWriter.addSlotVal(stageIdx, "WAIT_RECV");

            /** check send block*/
            FlowBlockPipeCom* sendPipCom = pipfb->getSendFbPipCom();
            bool sendRunning =  sendPipCom->getFlowSimEngine()->isBlockOrNodeRunning();
            if (sendRunning)
                slotWriter.addSlotVal(stageIdx, "WAIT_SEND");

            return recvRunning | sendRunning;
        }

        void RiscvSimInterface::writeFetchSlot(FlowBlockPipeBase* pipblock) {
            assert(pipblock != nullptr);
            if (writeSlotIfStall(RISC_FETCH, pipblock)){return;}

            if (ull(_core.fetch.readEn)){
                if (ull(_core.fetch.readFin)){
                    slotWriter.addSlotVal(RISC_FETCH, "READ ADDR");
                    slotWriter.addSlotVal(RISC_FETCH,
                                          cvtNum2HexStr(ull(_core.fetch._reqPc)));
                }else{
                    slotWriter.addSlotVal(RISC_FETCH, "WAIT4MEM");
                }
            }else{
                slotWriter.addSlotVal(RISC_FETCH, "WAIT4REQ");
            }


        }

        void RiscvSimInterface::writeDecodeSlot(FlowBlockPipeBase* pipblock) {
            assert(pipblock != nullptr);

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

            std::string decStr = (decMap.find(op) != decMap.end()) ? decMap[op]: "UNKNOWN";
            slotWriter.addSlotVal(RISC_DECODE, decStr);
            slotWriter.addSlotVal(RISC_DECODE, cvtNum2HexStr(pc));
            slotWriter.addSlotVal(RISC_DECODE, cvtNum2HexStr(next_pc));

        }

        void RiscvSimInterface::writeExecuteSlot(FlowBlockPipeBase* pipblock) {
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

            /** register write */

            writeReg("r1", RISC_EXECUTE, decodedUop.regData[RS_1  ]);
            writeReg("r2", RISC_EXECUTE, decodedUop.regData[RS_2  ]);
            writeReg("rd", RISC_EXECUTE, decodedUop.regData[RS_des]);


        }

        void RiscvSimInterface::writeWbSlot(FlowBlockPipeBase* pipblock) {
            assert(pipblock != nullptr);
            if (writeSlotIfStall(RISC_WB, pipblock)){return;}

            RegEle& wbReg = _core.wbData;

            writeReg("rwb", RISC_WB, wbReg);
        }

        void RiscvSimInterface::writeReg(const std::string& prefix,
                                         PIPE_STAGE         pipeStage,
                                         RegEle&            regEle){

            slotWriter.addSlotVal(pipeStage, prefix + " id " +
                                                    std::to_string(ull(regEle.idx)) + " v" +
                                                    std::to_string(ull(regEle.valid))
                                                    );
        }


    }

}