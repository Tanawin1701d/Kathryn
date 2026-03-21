//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    ////// simple pipeline
    class test_sim_mod56: public Module{
    public:
        SlotMeta meta{{"valid", "src_idx0", "src_idx1"},
            {1,32,32}
        };
        RegSlot rs{meta};
        WireSlot ws{{"src_idx0", "src_idx1"},
            {32,32}};

        explicit test_sim_mod56(int x){}

        void flow() override{

            seq{
                rs("src_idx0") <<= 24;
                rs("src_idx1") <<= 48;
                par{
                    ws = rs;
                    zif(rs("src_idx0") == 24){
                        ws("src_idx0") = 26;
                    }
                }
            }




        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test56.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test56.vcd";


    class sim56 :public SimAutoInterface{
    public:

        test_sim_mod56* _md;

        sim56(test_sim_mod56* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void describe_con() override{

            // // ////// skip first zync State
            std::cout << TC_BLUE << "test priority test" << TC_DEF << std::endl;
            con_next_cycle(2);
            con_end_cycle();
            test_and_print("check wire assign 26 ", ull(_md->ws("src_idx0")), 26);
            test_and_print("check wire assign 48 ", ull(_md->ws("src_idx1")), 48);
        }

    };


    class Sim56TestEle: public AutoTestEle{
    public:
        explicit Sim56TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod56, 1);
            start_model_kathryn();
            sim56 simulator((test_sim_mod56*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim56TestEle ele56(56);
}