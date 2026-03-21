//
// Created by tanawin on 15/6/2024.
//

#include "slot_recorder.h"

namespace kathryn::riscv{

    void SlotRecorder::record_slot() {

            /** please bare in mind that this recorder work correctly when
             *  it is the end of the cycle
             * */
            assert(_riscv != nullptr);
             //auto& pip_stages = _riscv->pip_probe->get_pip_stage();

            /*** record fetch */
            PipSimProbe* fetch = &_riscv->fetch.pip_sim_probe;
            write_fetch_slot(fetch);
            /*** decode*/
            PipSimProbe* decode = &_riscv->decode.pip_sim_probe;
            write_decode_slot(decode);
            /*** execute*/
            PipSimProbe* execute = &_riscv->execute.pip_sim_probe;
            write_execute_slot(execute);
            /*** write back*/
            PipSimProbe* write_back = &_riscv->write_back.pip_sim_probe;
            write_wb_slot(write_back);

            write_mem();

            _slotWriter->conclude_each_cycle();
        }

    bool SlotRecorder::write_slot_if_pip_stall(PIPE_STAGE2 stage_idx,
                                        PipSimProbe* pip_sim_probe) const{
        if (pip_sim_probe->is_waiting()){
            _slotWriter->add_slot_val(stage_idx, "WAIT_RECV");
            return true;
        }
        return false;
    }

    bool SlotRecorder::write_slot_if_zync_stall(PIPE_STAGE2 stage_idx,
                                            ZyncSimProb* zync_sim_probe){
        if (zync_sim_probe->is_waiting()){
            _slotWriter->add_slot_val(stage_idx, "WAIT_SEND");
            return true;
        }
        return false;

    }

    void SlotRecorder::write_fetch_slot(PipSimProbe* pip_sim_probe) {
        assert(pip_sim_probe != nullptr);
        if (write_slot_if_pip_stall(RISC_FETCH, pip_sim_probe)){return;}
        if (write_slot_if_zync_stall(RISC_FETCH, &_riscv->fetch.zync_sim_prob)){return;}

        _slotWriter->add_slot_val(RISC_FETCH, std::to_string(ull(_riscv->fetch.par_check)));

        ///if (_riscv->fetch.fetch_block->get_sim_engine_ptr()->is_block_running()) {
        if (_riscv->fetch.zync_sim_prob.is_executing()){

            if (ull(_riscv->fetch.read_en)) {
                if (ull(_riscv->fetch.read_fin)) {
                    _slotWriter->add_slot_val(RISC_FETCH, "READ ADDR");
                    _slotWriter->add_slot_val(RISC_FETCH,
                                          cvt_num2_hex_str(ull(_riscv->core_data.pc)));
                } else {
                    _slotWriter->add_slot_val(RISC_FETCH, "FETCHING WAIT4MEM");
                }
            } else {
                _slotWriter->add_slot_val(RISC_FETCH, "FETCHING  WAIT4REQ");
            }

        }else{
            _slotWriter->add_slot_val(RISC_FETCH, "Unknown State");
        }

        _slotWriter->add_slot_val(RISC_FETCH, "is_fin " + std::to_string(ull(_riscv->fetch.read_fin)));
        _slotWriter->add_slot_val(RISC_FETCH, "is_en " + std::to_string(ull(_riscv->fetch.read_en)));


    }

        void SlotRecorder::write_decode_slot(PipSimProbe* pip_sim_probe) {
            assert(pip_sim_probe != nullptr);

            //_slotWriter->add_slot_val(RISC_FETCH, "par_start" + std::to_string(ull(_riscv->decode.par_check)));

            if (write_slot_if_pip_stall (RISC_DECODE, pip_sim_probe)){return;}
            if (write_slot_if_zync_stall(RISC_DECODE, &_riscv->decode.zync_sim_prob)){return;}

            //////////// now decoding

            ull raw_intr  = ull(_riscv->core_data.ft.fetch_instr);
            ull pc       = ull(_riscv->core_data.ft.fetch_pc);
            ull next_pc  = ull(_riscv->core_data.ft.fetch_nextpc);


            ull op_mask_bit = (1 << 7) - 1;
            ull op = raw_intr & op_mask_bit;

            std::map<ull, std::string> dec_map = {
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

            //if (_riscv->decode.decode_blk->get_sim_engine_ptr()->is_block_running()) {
            if (_riscv->decode.zync_sim_prob.is_executing()){
                std::string dec_str = (dec_map.find(op) != dec_map.end()) ? dec_map[op] : "UNKNOWN";
                _slotWriter->add_slot_val(RISC_DECODE, dec_str);
                _slotWriter->add_slot_val(RISC_DECODE, cvt_num2_hex_str(pc));
                _slotWriter->add_slot_val(RISC_DECODE, cvt_num2_hex_str(next_pc));
            }else{
                _slotWriter->add_slot_val(RISC_DECODE, "UNKNOWN STATE");
            }

        }

        void SlotRecorder::write_execute_slot(PipSimProbe* pip_sim_probe) {
            assert(pip_sim_probe != nullptr);

            if (write_slot_if_pip_stall(RISC_EXECUTE, pip_sim_probe)){return;}

            DECODE_DATA& decoded_uop = _riscv->core_data.dc;
            InstrRepo& instr_repo    = decoded_uop.repo;
            /** decode
             *
             * uop
             *
             * */

            InstrRepoDebugMsg decoded_debug_msg = instr_repo.get_get_dbg_msg();

            _slotWriter->add_slot_val(RISC_EXECUTE,
                                    "mop " + decoded_debug_msg.mop_name);
            _slotWriter->add_slot_val(RISC_EXECUTE,
                                    "uop" + decoded_debug_msg.uop_name);
            if (decoded_debug_msg.uop_name.empty()){
                _slotWriter->add_slot_val(RISC_EXECUTE,
                                        "error" + decoded_debug_msg.error_cause);
            }


            if (_riscv->execute.ac_reg_sim_prob.is_executing()){
                _slotWriter->add_slot_val(RISC_EXECUTE, "REG_ACCESS");
            }else if (_riscv->execute.alu_sim_prob.is_executing()){
                _slotWriter->add_slot_val(RISC_EXECUTE, "SIMPLE_ALU");
            }else if (_riscv->execute.complex_alu_sim_prob.is_executing()){
                _slotWriter->add_slot_val(RISC_EXECUTE, "COMPLEX_ALU");
            }else{
                _slotWriter->add_slot_val(RISC_EXECUTE, "unknown_state");
            }

            /** register write */

            write_reg("r1", RISC_EXECUTE, instr_repo.get_src_reg(0));
            write_reg("r2", RISC_EXECUTE, instr_repo.get_src_reg(1));
            write_reg("r3", RISC_EXECUTE, instr_repo.get_src_reg(2));
            write_reg("rd", RISC_EXECUTE, instr_repo.get_des_reg(0));
            _slotWriter->add_slot_val(RISC_EXECUTE, "read_fin " + std::to_string(ull(_riscv->execute.read_fn)));
            _slotWriter->add_slot_val(RISC_EXECUTE, "read_en " + std::to_string(ull(_riscv->execute.read_en)));
            //_slotWriter->add_slot_val(RISC_EXECUTE, "fin_ls " + std::to_string(ull(_riscv->execute.test_exit)));
            _slotWriter->add_slot_val(RISC_EXECUTE, "read_addr " + std::to_string(ull(_riscv->execute.read_addr)));
            _slotWriter->add_slot_val(RISC_EXECUTE, "read1020 " + std::to_string(ull(_riscv->mem_blk._myMem.at(1020 >> 2))));
            _slotWriter->add_slot_val(RISC_EXECUTE, "read1024 " + std::to_string(ull(_riscv->mem_blk._myMem.at(1024 >> 2))));
            _slotWriter->add_slot_val(RISC_EXECUTE, "read1028 " + std::to_string(ull(_riscv->mem_blk._myMem.at(1028 >> 2))));
            _slotWriter->add_slot_val(RISC_EXECUTE, "reset_signal " + std::to_string(ull(_riscv->execute.mis_predic)));


        }

        void SlotRecorder::write_wb_slot(PipSimProbe* pip_sim_probe) {
            assert(pip_sim_probe != nullptr);
            if (write_slot_if_pip_stall(RISC_WB, pip_sim_probe)){return;}

            OPR_HW& wb_reg = _riscv->core_data.ex.wb_data;

            write_reg("rwb", RISC_WB, wb_reg);
            _slotWriter->add_slot_val
            (RISC_WB, std::to_string(ull(wb_reg.data)));
        }



        void SlotRecorder::write_reg(const std::string& prefix,
                                             PIPE_STAGE2  pipe_stage,
                                             OPR_HW&      reg_ele){

            _slotWriter->add_slot_val(pipe_stage, prefix + " id " +
                                    std::to_string(ull(reg_ele.idx)) + " v" +
                                    std::to_string(ull(reg_ele.valid)) + " val " +
                                    std::to_string(ull(reg_ele.data))
                                    );
        }

}