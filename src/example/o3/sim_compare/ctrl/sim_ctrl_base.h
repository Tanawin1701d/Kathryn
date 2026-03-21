//
// Created by tanawin on 25/12/25.
//

#ifndef EXAMPLE_O3_SIMCOMPARE_SIMCTRLBASE_H
#define EXAMPLE_O3_SIMCOMPARE_SIMCTRLBASE_H

#include "fstream"
#include "kathryn.h"
#include "front_end/cmd/param_reader.h"
#include "../sim_state.h"
#include "example/o3/sim_compare/result_writer.h"

namespace kathryn::o3{

    class O3SimCtrlBase{
    public:
        const int AMT_STAGE = 5;
        int _curTestCaseIdx = 0;
        std::string _prefixFolder;
        std::vector<std::string> _testTypes;
        uint32_t _imem      [IMEM_ROW]{}; ///// 512  * 4 rows
        uint32_t _dmem      [DMEM_ROW]{}; ///// 2048 * 1 rows
        uint32_t _regTestVal[REG_NUM] {};

        bool     last_dmem_enable = false; //// enabler
        bool     last_dmem_read   = true;  //// used if enabler is true
        uint32_t last_dmem_addr   = 0;
        uint32_t last_dmem_w_data   = 0;

        SlotWriterBase& _slotWriter;
        SimState& _state;
        ResultWriter* _resultWriter = nullptr; ///// null result pointer is acceptable

        ull cycle_cnt = 0;

        explicit O3SimCtrlBase(CYCLE                    limit_cycle,
                               const std::string&       prefix,
                               std::vector<std::string> test_types,
                               SimProxyBuildMode        build_mode,
                               SlotWriterBase&          slot_writer,
                               SimState&                state,
                               ResultWriter*            result_writer = nullptr);

        virtual ~O3SimCtrlBase() = default;


        virtual void do_workload_init (int cur_test_case_idx, bool req_reg_test)   = 0;
        virtual void do_workload_cycle(bool record_this_cycle) = 0;
        virtual void do_workload_exit();


        ////// memory management for each cycle
        bool is_exec_fin();

        void          reset_dmem();

        bool          compare_mem_op(O3SimCtrlBase& rhs);

        uint32_t      read_assembly_base(const std::string& file_path, //// return remaining
                                       uint32_t* mem_ptr,
                                       uint32_t  num_row);

        ////// memory initialization
        virtual void  read_assembly (const std::string& file_path);
        virtual void  read_assert_val(const std::string& file_path);

        void write_mem_op();

        void inc_cycle_cnt(){ cycle_cnt++;}

        ///// on model action
        virtual void  read_mem2_fetch         (){assert(false);} //// it has to place at the begin of cycle
        virtual void  read_write_data_mem_get_cmd(){assert(false);} //// it has to place at the end of cycle
        virtual void  read_write_data_mem_do_cmd (){assert(false);}  //// it has to place at the bigin of cycle
        virtual void  reset_register         (){assert(false);}
        virtual void  test_register          (){assert(false);}
        virtual void  post_cycle_action       (){assert(false);}
    };

}

#endif //EXAMPLE_O3_SIMCOMPARE_SIMCTRLBASE_H