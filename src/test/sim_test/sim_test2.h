//
// Created by tanawin on 22/1/2567.
//

#ifndef KATHRYN_SM_SIMTEST1_H
#define KATHRYN_SM_SIMTEST1_H

#include "kathryn.h"
#include "test/test_legacy/test.h"



namespace kathryn{

    class test_sim_mod: public Module{
    public:
        make_reg(a, 8);
        make_reg(b, 8);
        make_reg(c, 8);
        make_val(iv, 8, 0b10101010);

        explicit test_sim_mod(int x): Module(){}

        void flow() override{

            make_val(bnk, 8, 48);
            make_val(one, 8, 1);
            make_val(two, 8, 2);
            make_val(maxer, 8, 255);
            make_reg(ota, 8);
            make_reg(max_reg, 8);
            make_reg(bw_or_reg, 8);


            par{
                a <<= iv;
                b <<= iv;
                c <<= bnk;
                ota <<= bnk - one;
                max_reg <<= maxer + two;
                bw_or_reg <<= bnk | one;
            }

        }

    };

    static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_test2.vcd";

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
                    _md->iv.sv() = NumConverter::create_val_rep(8, 7);
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

#endif //KATHRYN_SIMTEST1_H
