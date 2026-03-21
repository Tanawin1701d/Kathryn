//
// Created by tanawin on 26/4/2567.
//

#include "riscv_sim.h"
#include "util/term_color/term_color.h"

#include "utility"

namespace kathryn{

    namespace riscv{


        RiscvSim::RiscvSim(CYCLE       limit_cycle,
                           const std::string& prefix,
                           std::vector<std::string> test_types,
                           Riscv& core,
                           SimProxyBuildMode build_mode,
                           bool require_lrc,
                           bool req_inline,
                           int  op_level): ///// init first test case here
                SimInterface(limit_cycle,
                             "/tmp/vcd_dummy",
                             "/tmp/profdummy",
                             "simple_risc_v",
                             build_mode,
                             require_lrc,
                             req_inline,
                             op_level
                             ),
                _core(core),
                slot_writer({"fetch", "decode", "execute", "wb", "mem"},
                           25,
                           std::move(prefix + test_types[0] + "/oslot.sl")),
                _prefixFolder(prefix),
                _slotSecorder(&slot_writer, &core),
                _testTypes(test_types){}

        void RiscvSim::describe_con() {


            for (;_curTestCaseIdx < _testTypes.size(); _curTestCaseIdx++){
                std::cout << TC_BLUE << "[RISCV] test type is " << _testTypes[_curTestCaseIdx] << TC_DEF << std::endl;
                //////////////  read assembly and assert_val

                _vcdWriter-> renew(_prefixFolder + _testTypes[_curTestCaseIdx]+ "/owave.vcd");
                _flowWriter->renew(_prefixFolder + _testTypes[_curTestCaseIdx]+ "/oprofile.prof");
                slot_writer.  renew(_prefixFolder + _testTypes[_curTestCaseIdx]+ "/oslot.sl");

                /////////// set reset wire to 1
                *rst_wire = 1;
                ////////// value before cycle is running
                con_next_cycle(1);
                *rst_wire = 0;
                _core.core_data.pc = 0;
                reset_register();
                read_assembly(_prefixFolder +  _testTypes[_curTestCaseIdx] + "/asm.out");
                read_assert_val(_prefixFolder + _testTypes[_curTestCaseIdx] + "/ast.out");
                //////////////////////////////////////////////////////////////////////
                for (int i = 0; i <= 100; i++){
                    con_end_cycle();
                    _slotSecorder.record_slot();
                    con_next_cycle(1);
                }
                //////////////////////////////////////////////////////////////////////
                test_register();
                final_perf_col();
                ////////////////////////////////////////////////////////////////////////
            }


        }

        void RiscvSim::read_assembly(const std::string& file_path){

            ///////// initialize file
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
                _core.mem_blk._myMem.at(write_addr).set_var(instr);
                write_addr++;
            }
            asm_file.close();

            ///////////// fill it with zero


            // for (;write_addr < _core.mem_blk._myMem.get_depth_size(); write_addr++){
            //     _core.mem_blk._myMem.at(write_addr).set_var(0);
            // }

            size_t set_zero_amt = _core.mem_blk._myMem.get_depth_size() - write_addr;
            _core.mem_blk._myMem.at(write_addr).set_var_arr(0, set_zero_amt);





            std::cout << TC_GREEN << "initialize mem finish" << TC_DEF << std::endl;



        }

        void RiscvSim::read_assert_val(const std::string& file_path){

            std::vector<std::string> raw_vals;
            FileReaderBase reader(file_path);
            raw_vals = reader.read_lines();

            assert(raw_vals.size() == AMT_REG);

            for (int reg_idx = 0; reg_idx < AMT_REG; reg_idx++){
                ///std::cout << reg_idx << " reg val  "<< stoul(raw_vals[reg_idx]) << std::endl;
                _regTestVal[reg_idx] = stoul(raw_vals[reg_idx]);
                ///std::cout << reg_idx << " reg val  "<< _regTestVal[reg_idx] << std::endl;
            }
        }

        void RiscvSim::reset_register(){
            for (int i = 0; i < AMT_REG; i++){
                _core.reg_file.at(i).set_var(0);
            }
        }

        void RiscvSim::test_register(){
            bool pass = true;
            for (int i = 0;  i < AMT_REG; i++){
                if (_regTestVal[i] != (ull)_core.reg_file.at(i)){
                    pass = false;
                    test_and_print("fail reg" + std::to_string(i),
                                 (ull)_core.reg_file.at(i), _regTestVal[i]);
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