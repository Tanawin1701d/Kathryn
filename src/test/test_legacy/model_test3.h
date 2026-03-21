//
// Created by tanawin on 13/1/2567.
//

#ifndef KATHRYN_MODELTEST3_H
#define KATHRYN_MODELTEST3_H

#include "kathryn.h"
#include "test.h"

namespace kathryn{

    class test_mod7: public Module{

        make_reg(a, 32);
        make_reg(b, 16);
        make_reg(c, 15);
        make_reg(d, 16);

    public:
        explicit test_mod7(int x): Module(){}

        void flow() override{

        cwhile(b >= d) {
            a <<= d;
        }

        }


    };

    class test_mod8: public Module{
        make_reg(a, 32);
        make_reg(b, 16);
        make_reg(c, 15);

    public:
        explicit test_mod8(int x): Module(){}

        void flow() override{

            cwhile(a >= b){
                seq{
                    a <<= b;
                    b <<= c;
                    cbreak;
                }
                seq{
                    a <<= c;
                }
                par{
                    c <<= a;
                    b <<= a;
                };
            }

        }
    };

    class test3: public Test{

    public:
        void test() override{
            make_mod(tm, test_mod8, 0);

            auto md_log_val = new MdLogVal();
            tm.add_md_log(md_log_val);
            log_md("tm", md_log_val);

        }

    };

}


#endif //KATHRYN_MODELTEST3_H
