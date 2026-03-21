//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    ////// simple pipeline
    class test_sim_mod57: public Module{
    public:
        SlotMeta meta{{"valid", "src_idx0", "src_idx1"},
            {1,32,32}
        };

        Table table{meta, 2};
        WireSlot ws{meta, "result"};
        m_wire(idx, 1);


        explicit test_sim_mod57(int x){}

        void flow() override{
            ///// dynamic assign and slot assign test
            seq{

                par{
                    table(0)(1) <<= 48;
                    table(0)(2) <<= 2;
                    table(1)(1) <<= 24;
                    table(1)(2) <<= 4;
                }
                auto compare = [](WireSlot& lhs, Operable* lidx, WireSlot& rhs, Operable* ridx) -> Operable&{
                    return lhs(1) < rhs(1);
                };
                par{
                    std::tie(ws, idx) = table.do_reduc_bin_idx(compare);
                }

            }




        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test57.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test57.vcd";


    class sim57 :public SimAutoInterface{
    public:

        test_sim_mod57* _md;

        sim57(test_sim_mod57* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void describe_con() override{

            // // ////// skip first zync State
            con_next_cycle(1);
            con_end_cycle();
            test_and_print("check result row val valid "  , ull(_md->ws(0)), 0);
            test_and_print("check result row val src_idx0 ", ull(_md->ws(1)), 24);
            test_and_print("check result row val src_idx1 ", ull(_md->ws(2)), 4);
            test_and_print("check result idx "            , ull(_md->idx), 1);

        }

    };


    class Sim57TestEle: public AutoTestEle{
    public:
        explicit Sim57TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod57, 1);
            start_model_kathryn();
            sim57 simulator((test_sim_mod57*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim57TestEle ele57(57);
}