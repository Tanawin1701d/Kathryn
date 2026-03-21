//
// Created by tanawin on 3/2/2567.
//

#ifndef KATHRYN_SIMTEST10_H
#define KATHRYN_SIMTEST10_H


#include "kathryn.h"
#include "test/test_legacy/test.h"


namespace kathryn{

    class test_sim_mod: public Module{
    public:
        bool test_auto_skip = false;

        make_reg(a, 8);
        make_reg(b, 8);
        make_reg(result, 8);
        make_val(result_cna,  8, 255);
        make_val(result_cnb,  8, 128);
        make_val(result_cnb2, 8, 129);
        make_val(result_cnc, 8, 20);

        make_reg(inner_a, 8);
        make_reg(inner_b, 8);

        make_val(inner_val_a, 8, 2);
        make_val(inner_val_b, 8, 3);



        explicit test_sim_mod(bool test_auto_skip): Module(){}

        void flow() override{

            seq {
                make_val(bnk, 8, 48);
                make_val(akb, 8, 49);
                inner_a <<= inner_val_a;
                inner_b <<= inner_val_b;
                a <<= bnk;
                b <<= akb;

                sif(a > b){
                    result <<= result_cna;
                }selif(a < b){
                    cif(inner_a > inner_b){
                        result <<= result_cnb;
                    }celse{
                        result <<= result_cnb2;
                    }
                }selse{
                    result <<= result_cnc;
                }

            }

        }

    };

    static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_test10.vcd";

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

