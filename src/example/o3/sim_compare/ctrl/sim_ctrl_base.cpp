//
// Created by tanawin on 25/12/25.
//

#include "sim_ctrl_base.h"


namespace kathryn::o3{

    O3SimCtrlBase::O3SimCtrlBase(CYCLE                    limit_cycle,
                                 const std::string&       prefix,
                                 std::vector<std::string> test_types,
                                 SimProxyBuildMode        build_mode,
                                 SlotWriterBase&          slot_writer,
                                 SimState&                state,
                                 ResultWriter*            result_writer):


    _prefixFolder(prefix),
    _testTypes   (test_types),
    _slotWriter  (slot_writer),
    _state       (state),
    _resultWriter(result_writer){}

    void O3SimCtrlBase::do_workload_exit(){
        if (_resultWriter != nullptr){
            _resultWriter->fill_cycle_cnt(cycle_cnt);
            cycle_cnt = 0;
        }

    }


    bool O3SimCtrlBase::is_exec_fin(){

        if ((!last_dmem_read) && (last_dmem_addr == 8) && (last_dmem_w_data == 1)){
            return true;
        }
        return false;

    }



    void O3SimCtrlBase::reset_dmem(){
        std::memset(_dmem, 0, sizeof(_dmem));
        last_dmem_enable  = false;
        last_dmem_read    = true;
        last_dmem_addr    = 0;
        last_dmem_w_data    = 0;
    }

    bool O3SimCtrlBase::compare_mem_op(O3SimCtrlBase& rhs){

        if (!last_dmem_enable){return true;}
        bool result = true;

        if (last_dmem_read != rhs.last_dmem_read){
            std::cout << TC_RED
                      << "Dmem read mis_match Kride got: "
                      << last_dmem_read
                      << "    Ride got: "
                      << rhs.last_dmem_read
                      << std::endl;
            result = false;
        }
        if (last_dmem_addr != rhs.last_dmem_addr){
            std::cout << TC_RED
                      << "Dmem addr mis_match Kride got: "
                      << cvt_num2_hex_str(last_dmem_addr)
                      << "    Ride got: "
                      << cvt_num2_hex_str(rhs.last_dmem_addr)
                      << std::endl;
            result = false;
        }

        if (!last_dmem_read){ ///// it is write
            if (last_dmem_w_data != rhs.last_dmem_w_data){
                std::cout << TC_RED
                          << "Dmem write Data mis_match Kride got: "
                          << cvt_num2_hex_str(last_dmem_w_data)
                          << "    Ride got: "
                          << cvt_num2_hex_str(rhs.last_dmem_w_data)
                          << std::endl;
            }
        }
        return result;

    }

    uint32_t O3SimCtrlBase::read_assembly_base(const std::string& file_path,
                                             uint32_t* mem_ptr,
                                             uint32_t num_row){

        ////////// initialize file
        std::ifstream asm_file(file_path, std::ios::binary);
        if (!asm_file.is_open()){assert(false);}
        asm_file.seekg(0, std::ios::end);
        std::streampos file_size = asm_file.tellg();
        assert((file_size % 4) == 0);
        assert(file_size <= (num_row << 2));
        asm_file.seekg(0, std::ios::beg);

        /** read instruction from file and write it to memory block*/
        uint32_t write_addr = 0;
        uint32_t instr;
        while(asm_file.read(reinterpret_cast<char*>(&instr), sizeof instr)){
            mem_ptr[write_addr] = instr;
            write_addr++;
        }
        asm_file.close();

        ///// fill all with zero
        std::fill(mem_ptr + write_addr, mem_ptr + num_row, 0);

        return num_row - write_addr; //// the result is in byte * 4
    }



    void O3SimCtrlBase::read_assembly(const std::string& file_path){
        std::cout << TC_BLUE << "initialize IMEM" << TC_DEF << std::endl;
        uint32_t iremain_row = read_assembly_base(file_path, _imem, IMEM_ROW);
        std::cout << TC_BLUE << "remain_row: " << iremain_row << TC_DEF << std::endl;
        std::cout << TC_GREEN << "initialize IMEM finish" << TC_DEF << std::endl;

        std::cout << TC_BLUE << "initialize DMEM" << TC_DEF << std::endl;
        uint32_t dremain_row = read_assembly_base(file_path, _dmem, DMEM_ROW);
        std::cout << TC_BLUE << "remain_row: " << dremain_row << TC_DEF << std::endl;
        std::cout << TC_GREEN << "initialize IMEM finish" << TC_DEF << std::endl;
    }

    void O3SimCtrlBase::read_assert_val(const std::string& file_path){

        std::vector<std::string> raw_vals;
        FileReaderBase reader(file_path);
        raw_vals = reader.read_lines();

        assert(raw_vals.size() == REG_NUM);

        for (int reg_idx = 0; reg_idx < REG_NUM; reg_idx++){
            _regTestVal[reg_idx] = stoul(raw_vals[reg_idx]);
        }

    }


    void O3SimCtrlBase::write_mem_op(){

        if (!last_dmem_read){
            _slotWriter.add_slot_val(RPS_STBUF, "-------");
            _slotWriter.add_slot_val(RPS_STBUF, "write @ " + cvt_num2_hex_str(last_dmem_addr));
            _slotWriter.add_slot_val(RPS_STBUF, "    with Data" + cvt_num2_hex_str(last_dmem_w_data));

        }


    }


}