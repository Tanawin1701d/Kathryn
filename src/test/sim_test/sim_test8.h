//
// Created by tanawin on 3/2/2567.
//

#ifndef KATHRYN_SIMTEST8_H
#define KATHRYN_SIMTEST8_H


#include "kathryn.h"
#include "test/test_legacy/test.h"


namespace kathryn{

    class test_sim_mod: public Module{
    public:
        bool test_auto_skip = false;
        make_val(bnk, 32, 48);
        make_val(zero, 32, 0);
        make_val(hf, 20, 64);
        make_val(nine, 20, 9);
        make_val(ele, 20, 11);
        make_reg(a , 32);
        make_reg(b , 32);
        make_reg(c , 32);

        make_reg(s , 32);
        make_reg(s2, 32);
        make_reg(s3, 32);
        make_reg(s4, 32);

        make_reg(cnt, 2);
        make_val(one, 2, 1);
        make_val(max_cnt, 2, 0);


        explicit test_sim_mod(bool test_auto_skip): Module(){}

        void flow() override{

            seq{
                a <<= bnk;
                cnt <<= zero;
                b <<= zero;
                par {
                    cif(a > hf) {
                        s <<= nine;
                        s2 <<= nine;
                        s3 <<= nine;
                    }celif(a > nine) {
                        seq {
                            s <<= ele;
                            s2 <<= ele;
                            cwhile(cnt == max_cnt){
                                /** to test sync Reg*/
                                cnt <<= cnt + one;
                            }
                            s3 <<= ele;
                        }
                    }
                    s4 <<= nine;
                }
                c <<= bnk;
            }

        }

    };

    static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_test8.vcd";

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

#endif //KATHRYN_SIMTEST8_H

