//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"
#include "lib/math/math.h"

namespace kathryn{

    ////// simple pipeline
    class test_sim_mod67: public Module{
    public:
        m_reg(a, 32);
        m_reg(b, 32);
        m_reg(r1, 32);
        m_reg(r2, 32);
        m_reg(res, 32);
        SyncMeta fetch{"fetch"};
        SyncMeta decode{"decode"};


        explicit test_sim_mod67(int x){}

        void flow() override{
            a.make_reset_event();
            b.make_reset_event();

            pip(fetch){ auto_sync
                zync(decode){
                    a <<= 128;
                    b <<= 256;
                }
            }

            pip(decode){
                seq{
                    par{
                        sqrt_int(a, r1);
                        sqrt_int(b, r2);
                    }
                    res <<= r1 * r2;
                }
            }

            seq{
                sy_wait(3)
                par{decode.kill_slave(false);}
            }

        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test67.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test67.vcd";


    class sim67 :public SimAutoInterface{
    public:

        test_sim_mod67* _md;

        sim67(test_sim_mod67* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void describe_con() override{

            // ////// skip first zync State
            con_next_cycle(10);
            test_and_print("check r1  equal to " + std::to_string( 64), ull(_md->r1 ), 64 );
            test_and_print("check r2  equal to " + std::to_string(128), ull(_md->r2 ), 128);
            test_and_print("check res equal to " + std::to_string(  0), ull(_md->res), 0  );
            // for (int i = 1; i < 5; i++){
            //     con_end_cycle();
            //     test_and_print("check a equal to " + std::to_string(i), ull(_md->a), i);
            //     test_and_print("check b equal to " + std::to_string(i-1), ull(_md->b), i-1);
            //     con_next_cycle(1);
            // }
            // ///////////////////////////
            // con_end_cycle();
            // test_and_print(" holding part check a equal to " + std::to_string(4), ull(_md->a), 4);
            // test_and_print(" holding part check b equal to " + std::to_string(4), ull(_md->b), 4);
            // con_next_cycle(1);
            // for (int i = 5; i < 10; i++){
            //     con_end_cycle();
            //     test_and_print("check a equal to " + std::to_string(i), ull(_md->a), i);
            //     test_and_print("check b equal to " + std::to_string(i-1), ull(_md->b), i-1);
            //     con_next_cycle(1);
            // }

        }

    };


    class Sim67TestEle: public AutoTestEle{
    public:
        explicit Sim67TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod67, 1);
            start_model_kathryn();
            sim67 simulator((test_sim_mod67*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim67TestEle ele67(67);
}
