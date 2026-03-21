//
// Created by tanawin on 12/12/2566.
//

#ifndef KATHRYN_MODELTEST2_H
#define KATHRYN_MODELTEST2_H

#include "kathryn.h"
#include "test.h"



namespace kathryn{

    class test_mod5: public Module{
        make_reg(a, 32);
        make_reg(b, 64);
        make_reg(c, 16);
        make_reg(d, 21);
    public:
        explicit test_mod5(int x): Module(){

        }

        void flow() override {
            par{


                
            }
        }
    };

    class test_mod6: public Module{
        make_reg(a, 32);
        make_reg(b, 64);
        make_reg(c, 16);
        make_reg(d, 21);
    public:
        explicit test_mod6(int x): Module(){}

        void flow() override {
            seq{
                a <<= b;
                cif(a == b){
                    par{
                        b <<= c;
                        c <<= a;
                    }
                }celif(b < c){
                    seq{
                        a <<= c;
                        b <<= c;
                        d <<= a;
                    }
                }celse{
                    a <<= b;
                };
                d <<= b;
            }
        }
    };

    class test2: public Test{

    public:
        void test() override{
            make_mod(tm, test_mod6, 0);

            auto md_log_val = new MdLogVal();
            tm.add_md_log(md_log_val);
            log_md("tm", md_log_val);

        }

    };

}

#endif //KATHRYN_MODELTEST2_H
