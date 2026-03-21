//
// Created by tanawin on 28/6/2024.
//

#include "kathryn.h"
#include "gen_ele.h"

namespace kathryn{
    class test_gen_mod2: public Module{
    public:
        m_reg(blink, 1);
        m_reg(cnt, 8);

        explicit test_gen_mod2(int x): Module(){}

        void flow() override{

            blink.as_output_glob();

            seq{
                cnt <<= 0;
                cnt <<= 0;
                cwhile(cnt < 7){
                    blink <<= 1;
                    sy_wait(100);
                    par{
                        blink <<= 0;
                        cnt <<= cnt + 1;
                    }
                    sy_wait(100);
                }

            }



        }
    };

    class GenEle2: public GenEle{
    public:
        explicit GenEle2(int id): GenEle(id){}

        void start(PARAM& param) override{
            m_mod(my_md, test_gen_mod2, 1);
        }
    };

    GenEle2 tc2(2);

}