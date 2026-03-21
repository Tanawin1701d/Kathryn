////
//// Created by tanawin on 26/4/2567.
////
//
//#ifndef KATHRYN_RISCV_SIM_SORT_H
//#define KATHRYN_RISCV_SIM_SORT_H
//
//#include "fstream"
//#include "chrono"
//#include "kathryn.h"
//#include "example/riscv/core/core.h"
//#include "front_end/cmd/param_reader.h"
//
//
//namespace kathryn{
//
//    namespace riscv{
//
//        enum PIPE_STAGE_SORT{
//            RISC_FETCH_SORT   = 0,
//            RISC_DECODE_SORT  = 1,
//            RISC_EXECUTE_SORT = 2,
//            RISC_WB_SORT      = 3,
//            RISC_MEM_SORT     = 4
//        };
//
//        class RiscvSimInterfaceSort: public SimInterface{
//        private:
//
//            Riscv&      _core;
//            const int  AMT_STAGE = 4;
//            SlotWriter slot_writer;
//            std::string _prefixFolder;
//            std::string _testType;
//            uint32_t    _regTestVal[AMT_REG];
//
//            uint32_t _startNumIdx0 = 1 << 20;
//            uint32_t _startNumIdx1 = 1 << (20 + 1);
//            uint32_t _numSize      = 4;
//
//
//        public:
//
//            explicit RiscvSimInterfaceSort(CYCLE limit_cycle,
//                                       std::string prefix,
//                                       std::string test_type,
//                                       Riscv& core
//                                       );
//
//            void describe() override;
//
//            void describe_con() override;
//
//            void record_slot();
//
//            bool write_slot_if_stall(PIPE_STAGE_SORT stage_idx,
//                                  FlowBlockPipeBase* pipfb);
//
//
//            void write_fetch_slot  (FlowBlockPipeBase* pipblock);
//            void write_decode_slot (FlowBlockPipeBase* pipblock);
//            void write_execute_slot(FlowBlockPipeBase* pipblock);
//            void write_wb_slot     (FlowBlockPipeBase* pipblock);
//            void write_reg        (const std::string& prefix,
//                                  PIPE_STAGE_SORT pipe_stage,
//                                  RegEle&    reg_ele);
//            void write_mem        ();
//
//            void read_assembly(const std::string& file_path);
//            void read_assert_val(const std::string& file_path);
//
//        };
//
//        class RISCV_MNG_SORT{
//        public:
//            void start(PARAM& params){
//
//
//                    /** test each type*/
//                    std::cout << TC_GREEN << "testing riscv instruction SORTING>>>> " << std::endl;
//
//                    m_mod(risc_core, Riscv, false);
//                    start_model_kathryn();
//                    std::cout << TC_BLUE << "amount element in simulation" << GLOBAL_MODEL_ID <<std::endl;
//                    RiscvSimInterfaceSort simulator(20000,
//                                                params["prefix"],
//                                                "sorter",
//                                                (Riscv &) risc_core
//                    );
//                    auto start = std::chrono::steady_clock::now();
//                    ////// start simulate
//                    simulator.sim_start();
//                    ////// reset system
//                    reset_kathryn();
//                    std::cout << TC_GREEN << "--------------------------------" << std::endl;
//
//                auto end = std::chrono::steady_clock::now();
//                std::chrono::duration<double> elapsed_seconds = end - start;
//                std::cout << "Elapsed time: " << elapsed_seconds.count() << "s\n";
//
//                }
//
//        };
//
//
//    }
//
//}
//
//#endif //KATHRYN_RISCV_SIM_H
//