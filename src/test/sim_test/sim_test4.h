//
// Created by tanawin on 22/1/2567.
//

#ifndef KATHRYN_SM_SIMTEST4_H
#define KATHRYN_SM_SIMTEST4_H

#include "kathryn.h"
#include "test/test_legacy/test.h"



namespace kathryn{

    class test_sim_mod: public Module{
    public:
        make_reg(a0, 8);
        make_reg(b0, 8);
        /** lane1*/
        make_reg(a1, 8);
        make_reg(b1, 8);
        make_reg(c1, 8);
        make_reg(d1, 8);
        /** lane2*/
        make_reg(a2, 8);
        make_reg(b2, 8);
        make_reg(c2, 8);

        make_val(iv,   8,48);
        make_val(iv2,  8,64);
        make_val(zero, 8, 0);


        explicit test_sim_mod(int x): Module(){}

        void flow() override{
            seq {
                a0 <<= iv;
                par {
                    seq {
                        a1 <<= iv;
                        b1 <<= a1;
                        c1 <<= b1;
                        d1 <<= c1;
                    }
                    seq {
                        a2 <<= iv2;
                        b2 <<= a2;
                        c2 <<= b2;
                    }
                }
                b0 <<= iv;
            }

        }

    };

    static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_test4.vcd";

    class sim1 :public SimInterface{
    public:
        test_sim_mod* _md = nullptr;
        sim1(test_sim_mod* md):SimInterface(100, vcd_path),
                             _md(md){
            assert(_md != nullptr);
        }

        void describe() override{

            inc_cycle(2);

            for (int i = 0; i < 100; i++) {
                sim {
                    //_md->iv.sv() = NumConverter::cvt_str_to_val_rep(8, 7);
                };
                inc_cycle(1);
            }


        }
    };

    class test1: public Test{
    public:

        void test() override {
            make_mod(tm, test_sim_mod, 0);

            /**logger */
            auto md_log_val = new MdLogVal();
            tm.add_md_log(md_log_val);
            log_md("tm", md_log_val);
            //////////////////////////////////
            sim1 s((test_sim_mod*)(&tm));
            s.sim_start();

        }
    };



}

#endif