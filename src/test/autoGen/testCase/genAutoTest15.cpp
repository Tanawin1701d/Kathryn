//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "genEle.h"

namespace kathryn{

    class testGenMod15: public Module{
    public:
        mVal(hf1, 8, 36);
        mVal(hf2, 8, 12);
        mReg(a, 8);
        mReg(b, 8);
        mReg(c, 8);
        mReg(d, 8);
        mMem(storage, 48, 8);
        explicit testGenMod15(int x): Module(){
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
                    scWait(a == 16);
                    cwhile(a < 15){
                        a <<= a + 1;
                    }
                }
                c <<= 1;


            }
        }

    };

    class GenEle15: public GenEle{
    public:
        explicit GenEle15(int id): GenEle(id){};

        void start(PARAM& param) override{
            mMod(m3, testGenMod15, 1);
        }
    };

    GenEle15 testGen15(15);

}