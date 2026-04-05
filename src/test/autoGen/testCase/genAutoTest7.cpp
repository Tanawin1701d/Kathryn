//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "genEle.h"

namespace kathryn{

    class testGenMod7: public Module{
    public:
        mVal(hf1, 8, 36);
        mVal(hf2, 8, 12);
        mReg(a, 8);
        mReg(b, 8);
        mReg(c, 8);
        explicit testGenMod7(int x): Module(){
            a.asOutputGlob("a");
            b.asOutputGlob("b");
            c.asOutputGlob("c");
        }

        void flow() override{

            seq{
                par{
                    a <<= hf1;
                    b <<= hf2;
                }
                par{
                    a <<= a + hf2;
                    c <<= a + b;
                }
            }

        }

    };

    class GenEle7: public GenEle{
    public:
        explicit GenEle7(int id): GenEle(id){};

        void start(PARAM& param) override{
            mMod(m3, testGenMod7, 1);
        }
    };

    GenEle7 testGen7(7);

}