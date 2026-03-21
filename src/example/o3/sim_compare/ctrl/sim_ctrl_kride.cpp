//
// Created by tanawin on 25/12/25.
//

#include "sim_ctrl_kride.h"

#include "utility"


namespace kathryn::o3{


    SimCtrlKride::SimCtrlKride(CYCLE                    limit_cycle,
                               const std::string&       prefix,
                               std::vector<std::string> test_types,
                               SimProxyBuildMode        build_mode,
                               SlotWriterBase&          slot_writer,
                               SimState&                state,
                               TopSim&                  top_sim,
                               ResultWriter*            result_writer):

    O3SimCtrlBase(  limit_cycle,
                    prefix,
                    std::move(test_types),
                    build_mode,
                    slot_writer,
                    state,
                    result_writer),
    SimInterface(limit_cycle,
                 "/tmp/vcd_dummy",
                 "/tmp/prodummy",
                 "O3RiscV",
                 build_mode,
                 false,
                 false,
                 1),
    _topSim(top_sim),
    _core(top_sim.my_core){}

    void SimCtrlKride::do_workload_init(int cur_test_case_idx, bool req_reg_test){

        _vcdWriter-> renew(_prefixFolder + _testTypes[cur_test_case_idx]+ "/owave.vcd");
        _flowWriter->renew(_prefixFolder + _testTypes[cur_test_case_idx]+ "/oprofile.prof");
        _slotWriter. renew(_prefixFolder + _testTypes[cur_test_case_idx]+ "/oslot_kride.sl");
        do_workload_exit();
        if (_resultWriter != nullptr){
            _resultWriter->renew(_prefixFolder + _testTypes[cur_test_case_idx]+ "/kathryn_kride_result");
        }
        //////// set reset wire to 1
        *rst_wire = 1;
        //////// cycle before cycle cycle is running
        con_next_cycle(1);
        *rst_wire = 0;
        reset_register();
        read_assembly (_prefixFolder + _testTypes[cur_test_case_idx] + "/asm.out");
        if (req_reg_test){
            read_assert_val(_prefixFolder + _testTypes[cur_test_case_idx] + "/ast.out");
        }
        //reset_dmem();   we dont use it any more na krub

    }

    void SimCtrlKride::do_workload_cycle(bool record_this_cycle){
        ///////// give the data to
        read_mem2_fetch();
        read_write_data_mem_do_cmd(); ///// do the dmem command command

        con_end_cycle();
        read_write_data_mem_get_cmd();
        ///////// record the system
        _state.recruit_value();
        if (record_this_cycle){
            _state.print_slot_window(_slotWriter);
            write_mem_op();

            _slotWriter.add_slot_val(RPS_RSV, "--BR-------");
            _slotWriter.add_slot_val(RPS_RSV, "alloc_ptr " + std::to_string(ull(_core.rsvs.br.alloc_ptr)));
            _slotWriter.add_slot_val(RPS_RSV, "b1"  +   std::to_string(ull( _core.rsvs.br.dbg_b1_valid))  +   "  value " + std::to_string(ull( _core.rsvs.br.dbg_b1_idx))) ;
            _slotWriter.add_slot_val(RPS_RSV, "e1"  +   std::to_string(ull( _core.rsvs.br.dbg_e1_valid))  +   "  value " + std::to_string(ull( _core.rsvs.br.dbg_e1_idx))) ;
            _slotWriter.add_slot_val(RPS_RSV, "e0"  +   std::to_string(ull( _core.rsvs.br.dbg_e0_valid))  +   "  value " + std::to_string(ull( _core.rsvs.br.dbg_e0_idx))) ;
            _slotWriter.add_slot_val(RPS_RSV, "nb1"   + std::to_string(ull(  _core.rsvs.br.dbg_nb1_valid))  +   "  value " + std::to_string(ull(  _core.rsvs.br.dbg_nb1_idx))) ;
            _slotWriter.add_slot_val(RPS_RSV, "ne1"   + std::to_string(ull(  _core.rsvs.br.dbg_ne1_valid))  +   "  value " + std::to_string(ull(  _core.rsvs.br.dbg_ne1_idx))) ;
            _slotWriter.add_slot_val(RPS_RSV, "nb0"   + std::to_string(ull(  _core.rsvs.br.dbg_nb0_valid))  +   "  value " + std::to_string(ull(  _core.rsvs.br.dbg_nb0_idx))) ;

            _slotWriter.add_slot_val(RPS_RSV, "--LS-------");
            _slotWriter.add_slot_val(RPS_RSV, "alloc_ptr " + std::to_string(ull(_core.rsvs.ls.alloc_ptr)));
            _slotWriter.add_slot_val(RPS_RSV, "b1"  +   std::to_string(ull( _core.rsvs.ls.dbg_b1_valid))  +   "  value " + std::to_string(ull( _core.rsvs.ls.dbg_b1_idx))) ;
            _slotWriter.add_slot_val(RPS_RSV, "e1"  +   std::to_string(ull( _core.rsvs.ls.dbg_e1_valid))  +   "  value " + std::to_string(ull( _core.rsvs.ls.dbg_e1_idx))) ;
            _slotWriter.add_slot_val(RPS_RSV, "e0"  +   std::to_string(ull( _core.rsvs.ls.dbg_e0_valid))  +   "  value " + std::to_string(ull( _core.rsvs.ls.dbg_e0_idx))) ;
            _slotWriter.add_slot_val(RPS_RSV, "nb1"   + std::to_string(ull(  _core.rsvs.ls.dbg_nb1_valid))  +   "  value " + std::to_string(ull(  _core.rsvs.ls.dbg_nb1_idx))) ;
            _slotWriter.add_slot_val(RPS_RSV, "ne1"   + std::to_string(ull(  _core.rsvs.ls.dbg_ne1_valid))  +   "  value " + std::to_string(ull(  _core.rsvs.ls.dbg_ne1_idx))) ;
            _slotWriter.add_slot_val(RPS_RSV, "nb0"   + std::to_string(ull(  _core.rsvs.ls.dbg_nb0_valid))  +   "  value " + std::to_string(ull(  _core.rsvs.ls.dbg_nb0_idx))) ;

            _slotWriter.add_slot_val(RPS_DISPATCH, "--imm1------- "  + cvt_num2_hex_str(ull(_core.p_disp.dbg_imm1)));
            _slotWriter.add_slot_val(RPS_DISPATCH, "--imm2------- "  + cvt_num2_hex_str(ull(_core.p_disp.dbg_imm2)));
            _slotWriter.add_slot_val(RPS_DISPATCH, "--inst1------- " + cvt_num2_hex_str(ull(_core.pm.dc.dcd1(inst))));
            _slotWriter.add_slot_val(RPS_DISPATCH, "--inst2------- " + cvt_num2_hex_str(ull(_core.pm.dc.dcd2(inst))));

            _slotWriter.add_slot_val(RPS_MPFT, "-- cycle --");
            _slotWriter.add_slot_val(RPS_MPFT, "cc: " + std::to_string(cycle_cnt));

        }
        _state.recruit_next_cycle();
        post_cycle_action(); ///// assign value to the print
        _slotWriter.conclude_each_cycle();
        //////////////////////////////////
        con_next_cycle(1);
    }

    void SimCtrlKride::describe_con(){

        for (; _curTestCaseIdx < _testTypes.size(); _curTestCaseIdx++){
            std::cout << TC_BLUE << "[O3 RISC-V] test type is " << _testTypes[_curTestCaseIdx] << TC_DEF << std::endl;
            //////////////  read assembly and assert_val
            do_workload_init(_curTestCaseIdx, true);
            //////// iterate for 100 cycle
            for (int i = 0; i <= 150; i++){
                do_workload_cycle(true);
            }
            /////////////////////////////////
            test_register();
            final_perf_col();
        }
    }

    void  SimCtrlKride::read_mem2_fetch(){

        ///// get new instruction data
        ull cur_pc     = ull(_core.pm.ft.cur_pc);
        cur_pc         = cur_pc >> 2; ///// make 4bytes align
        ull aligner   = (ull(1) << 2) - 1; ///// to align 4 instructions per read 111111...11100
        aligner       = (~aligner);
        ull aligned_pc = cur_pc & aligner;

        ///// get new instruction data
        _topSim.ij_imem0.s(_imem[aligned_pc + 0]);
        _topSim.ij_imem1.s(_imem[aligned_pc + 1]);
        _topSim.ij_imem2.s(_imem[aligned_pc + 2]);
        _topSim.ij_imem3.s(_imem[aligned_pc + 3]);

    }

    void  SimCtrlKride::read_write_data_mem_get_cmd(){
        ///// make command enable
        last_dmem_enable = true;
        ///// read data from CPU
        ull dmem_we     = ull(_core.pm.ld_st.dmem_we);
        ull dmem_rwaddr = ull(_core.pm.ld_st.dmem_rwaddr);
        ull dmem_wdata  = ull(_core.pm.ld_st.dmem_wdata);
        assert((dmem_rwaddr & 0b11) == 0b00);

        last_dmem_read = (dmem_we == 0);
        last_dmem_addr = static_cast<uint32_t>(dmem_rwaddr);
        last_dmem_w_data = static_cast<uint32_t>(dmem_wdata);
    }


    void  SimCtrlKride::read_write_data_mem_do_cmd (){
        if (!last_dmem_enable){return;}

        ///// At now, last_dmem_addr is quiet sure that there is not polute bit
        uint32_t aligned_addr = last_dmem_addr >> 2;

        if (last_dmem_read){

            if (aligned_addr >= DMEM_ROW){
                std::cout << "skip read due to exceed memory address" << std::endl;
            }else{
                _topSim.ij_dmem0.s(_dmem[aligned_addr]);
            }
        }else{

            if (aligned_addr >= DMEM_ROW){
                std::cout << "skip write due to exceed memory address" << std::endl;
            }else{
                _dmem[aligned_addr] = last_dmem_w_data;
                if ((_resultWriter != nullptr) && (aligned_addr == 0x0)){
                    _resultWriter->fill_result(last_dmem_w_data);
                }
                if (last_dmem_addr == 0x0 || last_dmem_addr == 0x4 || last_dmem_addr == 0x8){
                    std::cout << "write Detect at KRide @ " << cvt_num2_hex_str(last_dmem_addr) << " with data " << last_dmem_w_data << std::endl;
                }
                //
            }


        }
    }
    void  SimCtrlKride::reset_register(){
        for (int i = 0; i < REG_NUM; i++){
            _core.reg_arch.arf.arch_regs(i) = 0;
        }
    }
    void  SimCtrlKride::test_register(){
        bool pass = true;
        for (int i = 0;  i < REG_NUM; i++){
            if (_regTestVal[i] != (ull)_core.reg_arch.arf.arch_regs(i)){
                pass = false;
                test_and_print("fail reg" + std::to_string(i),
                             (ull)_core.reg_arch.arf.arch_regs(i), _regTestVal[i]);
            }
        }
        if (pass){
            std::cout << TC_GREEN << "register val test pass" << TC_DEF << std::endl;
        }else{
            std::cout << TC_RED << "register val test fail" << TC_DEF << std::endl;
        }
    }

    void SimCtrlKride::post_cycle_action(){



    }

}


