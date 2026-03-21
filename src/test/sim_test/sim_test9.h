//
// Created by tanawin on 3/2/2567.
//

#ifndef KATHRYN_SIMTEST9_H
#define KATHRYN_SIMTEST9_H


#include "kathryn.h"
#include "test/test_legacy/test.h"


namespace kathryn{

    class test_sim_mod: public Module{
    public:
        bool test_auto_skip = false;

        make_val(bnk, 32, 48);
        make_val(akb, 32, 48);
        make_val(end_const, 112, 0b1111111111111111);
        make_reg(cnt, 32);
        make_reg(frd, 32);
        make_reg(end, 3);

        make_val(one, 32,  1);


        explicit test_sim_mod(bool test_auto_skip): Module(){}

        void flow() override{

            seq {
                par {
                    swhile(cnt < bnk) {
                        cnt <<= cnt + one;
                    }
                    frd <<= akb;

                }
                end <<= end_const;
            }

        }

    };

    static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_test9.vcd";

    class sim1 :public SimInterface{
    public:
        test_sim_mod* _md = nullptr;
        sim1(test_sim_mod* md):SimInterface(300, vcd_path),
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
            make_mod(tm, test_sim_mod, false);

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

#endif //KATHRYN_SIMTEST9_H

