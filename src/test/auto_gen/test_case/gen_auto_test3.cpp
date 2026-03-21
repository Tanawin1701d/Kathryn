//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "gen_ele.h"

namespace kathryn{


    class test_gen_mod3_sub: public Module{

    public:
        m_reg(cnt, 8);
        explicit test_gen_mod3_sub(int x): Module(){}

        void flow() override{

            cwhile(true){
                cnt <<= cnt + 1;
            }
        }
    };

    class test_gen_mod3: public Module{
    public:
        m_mod(x1, test_gen_mod3_sub, 0);
        m_mod(x2, test_gen_mod3_sub, 5);
        m_reg(xr1, 8);
        m_reg(xr2, 8);
        explicit test_gen_mod3(int x): Module(){}

        void flow() override{

            seq{
                xr1 <<= x1.cnt;
                xr2 <<= x2.cnt;
            }
        }

    };

    class GenEle3: public GenEle{
    public:
        explicit GenEle3(int id): GenEle(id){};

        void start(PARAM& param) override{
            m_mod(m3, test_gen_mod3, 1);
        }
    };

    GenEle3 test_gen3(3);

}