//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "gen_ele.h"

namespace kathryn{

    class test_gen_mod4_sub2: public Module{
    public:
        m_reg(cnt2, 8);
        explicit test_gen_mod4_sub2(int x): Module(){}
        void flow() override{
            cnt2.as_output_glob();
            cwhile(true){
                cnt2 <<= cnt2+1;
            }
        }
    };


    class test_gen_mod4_sub: public Module{
    public:
        m_reg(cnt, 8);
        m_mod(r1, test_gen_mod4_sub2, 0);
        explicit test_gen_mod4_sub(int x): Module(){}
        void flow() override{
            cwhile(true){
                cnt <<= r1.cnt2 + 1;
            }
        }
    };

    class test_gen_mod4: public Module{
    public:
        m_mod(x1, test_gen_mod4_sub, 0);
        m_mod(x2, test_gen_mod4_sub, 5);
        m_reg(xr1, 8);
        m_reg(xr2, 8);
        explicit test_gen_mod4(int x): Module(){}

        void flow() override{

            seq{
                xr1 <<= x1.cnt;
                xr2 <<= x2.cnt;
            }
        }

    };

    class GenEle4: public GenEle{
    public:
        explicit GenEle4(int id): GenEle(id){};

        void start(PARAM& param) override{
            m_mod(m3, test_gen_mod4, 1);
        }
    };

    GenEle4 test_gen4(-1);

}