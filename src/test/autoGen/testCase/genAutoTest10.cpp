//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "genEle.h"

namespace kathryn{

    class testGenMod10: public Module{
    public:
        mVal(hf1, 8, 36);
        mVal(hf2, 8, 12);
        mReg(a, 8);
        mReg(b, 8);
        mReg(c, 8);
        mReg(d, 8);
        explicit testGenMod10(int x): Module(){
            a.asOutputGlob("a");
            b.asOutputGlob("b");
            c.asOutputGlob("c");
            d.asOutputGlob("d");
        }

        void flow() override{

            seq{
                par{
                    a = 0;
                    b = 0;
                    c = 0;
                    d = 0;
                }
                par{
                    swhile(a < hf1){
                        a <<= a + 1;
                    }
                    b <<= 48;
                }
                c <<= 49;

            }
        }

    };

    class GenEle10: public GenEle{
    public:
        explicit GenEle10(int id): GenEle(id){};

        void start(PARAM& param) override{
            mMod(m3, testGenMod10, 1);
        }
    };

    GenEle10 testGen10(10);

}