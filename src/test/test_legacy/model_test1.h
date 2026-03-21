//
// Created by tanawin on 11/12/2566.
//

#ifndef KATHRYN_MODELTEST1_H
#define KATHRYN_MODELTEST1_H

#include "kathryn.h"
#include "test.h"

namespace kathryn{

    class test_mod: public Module{
        make_reg(a, 32);
        make_reg(b, 64);
        make_reg(c, 16);
    public:
        explicit test_mod(int x): Module(){

        }

        void flow() override {
            seq{
                a <<= b;
                b <<= c;
                c <<= a;
            }
        }

    };

    class test_mod2: public Module{
        make_reg(a, 32);
        make_reg(b, 64);
        make_reg(c, 16);
    public:
        explicit test_mod2(int x): Module(){

        }

        void flow() override {
            par{
                a <<= b;
                b <<= c;
                c <<= a;
            }
        }

    };

    class test_mod3: public Module{
        make_reg(a, 32);
        make_reg(b, 64);
        make_reg(c, 16);
    public:
        explicit test_mod3(int x): Module(){}

        void flow() override {
            seq {
                par {
                    a <<= b;
                    b <<= c;
                    c <<= a;
                }
                par{
                    b <<= c;
                };
            }
        }
    };

    class test_mod4: public Module{
        make_reg(a, 32);
        make_reg(b, 64);
        make_reg(c, 16);
        make_reg(d, 3);
    public:
        explicit test_mod4(int x): Module(){

        }

        void flow() override {
            par{
                seq{
                    make_wire(xx, 2);
                    a <<= b + xx;
                }
                seq{
                    b <<= c;
                    d <<= a;
                };
            }
        }

    };

    class test_mod5: public Module{
        make_reg(wt, 5);
        make_reg(b, 64);
        make_reg(c, 16);
        make_reg(d,  3);
    public:
        explicit test_mod5(int x): Module(){}

        void flow() override{
            par{
                cy_wait(wt);
                seq{
                    b <<= c;
                    d <<= c;
                };
                seq{
                    d <<= c + d;
                    d <<= b + d;
                };


            }
        }
    };

    class test1: public Test{

    public:
        void test() override{
            make_mod(tm, test_mod5, 0);

            //tm.get_md_describe();
            auto md_log_val = new MdLogVal();
            tm.add_md_log(md_log_val);
            log_md("tm", md_log_val);

        }

    };

}

#endif //KATHRYN_MODELTEST1_H
