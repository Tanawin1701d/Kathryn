//
// Created by tanawin on 3/2/2567.
//

#ifndef KATHRYN_SIMTEST11_H
#define KATHRYN_SIMTEST11_H


#include "kathryn.h"
#include "test/test_legacy/test.h"


namespace kathryn{

    class test_sim_mod: public Module{
    public:
        bool test_auto_skip = false;

        make_reg(a, 8);
        make_reg(b, 8);
        make_val(cond, 3, 2);
        make_val(one, 8, 1);
        make_val(two, 8, 2);
        make_val(three, 8, 3);

        explicit test_sim_mod(bool test_auto_skip): Module(){}

        void flow() override{

            cwhile(cond){
                zif(a > b){
                    a <<= a + one;
                }zelif(a < b){
                    a <<= a + two;
                    zif(a > b){
                        b <<= b - one;
                    }zelse{
                        b <<= b - two;
                    }
                }
            }

        }

    };

    static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_test11.vcd";

    class sim1 :public SimInterface{
    public:
        test_sim_mod* _md = nullptr;
        sim1(test_sim_mod* md):SimInterface(300, vcd_path),
                             _md(md){
            assert(_md != nullptr);
        }

        void describe() override{

            inc_cycle(2);

            for (int i = 0; i < 1; i++) {
                sim {
                    _md->b.sv() = NumConverter::cvt_str_to_val_rep(8, 48);
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

#endif //KATHRYN_SIMTEST11_H

