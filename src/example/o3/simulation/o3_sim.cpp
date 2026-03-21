//
// Created by tanawin on 14/10/25.
//

#include "o3_sim.h"


namespace kathryn::o3{

    O3Sim::O3Sim(CYCLE limit_cycle,
                 const std::string& prefix,
                 std::vector<std::string> test_types,
                 TopSim& top,
                 SimProxyBuildMode build_mode):

    SimInterface(limit_cycle,
                 "/tmp/vcd_dummy",
                 "/tmp/prodummy",
                 "O3RiscV",
                 build_mode,
                 false,
                 false,
                 1),
    _top(top),
    _core(_top.my_core),
    slot_writer({"MPFT"    , "ARF","RRF"  , "FETCH"  ,"DECODE",
                "DISPATCH", "RSV","ISSUE", "EXECUTE","COMMIT",
                "STBUF"
                   },
                {20       , 40   , 25    , 25       , 30,
                 30       , 35   , 25    , 35       , 25,
                 25},
                std::move(prefix + test_types[0] + "/oslot.sl")),
    _prefixFolder(prefix),
    _slotRecorder(&slot_writer, &_core),
    _testTypes(test_types){}

    void O3Sim::describe_con(){

        for (; _curTestCaseIdx < _testTypes.size(); _curTestCaseIdx++){
            std::cout << TC_BLUE << "[O3 RISC-V] test type is " << _testTypes[_curTestCaseIdx] << TC_DEF << std::endl;
            //////////////  read assembly and assert_val

            _vcdWriter-> renew(_prefixFolder + _testTypes[_curTestCaseIdx]+ "/owave.vcd");
            _flowWriter->renew(_prefixFolder + _testTypes[_curTestCaseIdx]+ "/oprofile.prof");
            slot_writer.  renew(_prefixFolder + _testTypes[_curTestCaseIdx]+ "/oslot.sl");
            //////// set reset wire to 1
            *rst_wire = 1;
            //////// cycle before cycle cycle is running
            con_next_cycle(1);
            *rst_wire = 0;
            reset_register();
            read_assembly (_prefixFolder + _testTypes[_curTestCaseIdx] + "/asm.out");
            read_assert_val(_prefixFolder + _testTypes[_curTestCaseIdx] + "/ast.out");
            reset_dmem();
            //////// iterate for 100 cycle
            for (int i = 0; i <= 150; i++){
                ///////// give the data to
                read_mem2_fetch();
                read_write_data_mem_do_cmd(); ///// do the dmem command command
                ///////// record the system
                con_end_cycle();
                read_write_data_mem_get_cmd();
                _slotRecorder.record_slot();
                con_next_cycle(1);
            }
            /////////////////////////////////
            test_register();
            final_perf_col();
        }

    }

    void O3Sim::read_mem2_fetch(){

        ///// get new instruction data
        ull cur_pc     = ull(_core.pm.ft.cur_pc);
            cur_pc     = cur_pc >> 2; ///// make 4bytes align
        ull aligner   = (ull(1) << 2) - 1; ///// to align 4 instructions per read 111111...11100
            aligner   = (~aligner);
        ull aligned_pc = cur_pc & aligner;

        ///// get new instruction data
        _top.ij_imem0.s(_imem[aligned_pc + 0]);
        _top.ij_imem1.s(_imem[aligned_pc + 1]);
        _top.ij_imem2.s(_imem[aligned_pc + 2]);
        _top.ij_imem3.s(_imem[aligned_pc + 3]);

    }

    void O3Sim::reset_dmem(){
        std::memset(_dmem, 0, sizeof(_dmem));
        last_dmem_enable  = false;
        last_dmem_read    = true;
        last_dmem_addr    = 0;
        last_dmem_data    = 0;
    }

    void O3Sim::read_write_data_mem_get_cmd(){

        ///// make command enable
        last_dmem_enable = true;
        ///// read data from CPU
        ull dmem_we     = ull(_core.pm.ld_st.dmem_we);
        ull dmem_rwaddr = ull(_core.pm.ld_st.dmem_rwaddr);
        ull dmem_wdata  = ull(_core.pm.ld_st.dmem_wdata);
        assert((dmem_rwaddr & 0b11) == 0b00);

        last_dmem_read = (dmem_we == 0);
        last_dmem_addr = static_cast<uint32_t>(dmem_rwaddr);
        last_dmem_data = static_cast<uint32_t>(dmem_wdata);

    }

    void O3Sim::read_write_data_mem_do_cmd(){

        if (!last_dmem_enable){return;}

        ///// At now, last_dmem_addr is quiet sure that there is not polute bit
        uint32_t aligned_addr = last_dmem_addr >> 2;

        if (last_dmem_read){
            _top.ij_dmem0.s(_dmem[aligned_addr]);
        }else{
            _dmem[aligned_addr] = last_dmem_data;
            std::cout << "write Detect at @ " << cvt_num2_hex_str(last_dmem_addr) << " with data " << last_dmem_data << std::endl;
        }

    }

    void O3Sim::read_assembly(const std::string& file_path){
        ////////// initialize file
        std::ifstream asm_file(file_path, std::ios::binary);
        if (!asm_file.is_open()){assert(false);}
        asm_file.seekg(0, std::ios::end);
        std::streampos file_size = asm_file.tellg();
        assert((file_size % 4) == 0);
        asm_file.seekg(0, std::ios::beg);

        /** read instruction from file and write it to memory block*/
        uint32_t write_addr = 0;
        uint32_t instr;
        while(asm_file.read(reinterpret_cast<char*>(&instr), sizeof instr)){
            assert((instr & 0b11) == 0b11); ////// check instruction
            _imem[write_addr] = instr;
            write_addr++;
        }
        asm_file.close();

        ///// fill all with zero
        std::fill(_imem + write_addr, _imem + IMEM_ROW, 0);

        std::cout << TC_GREEN << "initialize mem finish" << TC_DEF << std::endl;
    }

    void O3Sim::read_assert_val(const std::string& file_path){

        std::vector<std::string> raw_vals;
        FileReaderBase reader(file_path);
        raw_vals = reader.read_lines();

        assert(raw_vals.size() == REG_NUM);

        for (int reg_idx = 0; reg_idx < REG_NUM; reg_idx++){
            _regTestVal[reg_idx] = stoul(raw_vals[reg_idx]);
        }

    }

    void O3Sim::reset_register(){
        for (int i = 0; i < REG_NUM; i++){
            _core.reg_arch.arf.arch_regs(i) = 0;
        }
    }

    void O3Sim::test_register(){
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

}