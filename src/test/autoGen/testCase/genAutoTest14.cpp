//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "genEle.h"

namespace kathryn{

    class testGenMod14: public Module{
    public:
        mVal(hf1, 8, 36);
        mVal(hf2, 8, 12);
        mReg(a, 8);
        mReg(b, 8);
        mReg(c, 8);
        mReg(d, 8);
        mMem(storage, 48, 8);
        explicit testGenMod14(int x): Module(){
            a.asOutputGlob("a");
            b.asOutputGlob("b");
            c.asOutputGlob("c");
            d.asOutputGlob("d");
        }

        void flow() override{

            seq{
                par{
                    a = 16;
                    b = 16;
                    c = 16;
                    d = 16;
                }

                par{
                    g(a(0, 4), b(0, 4))
                     <<= g(c(4, 8), d(4, 8));
                }
            }
        }

    };

    class GenEle14: public GenEle{
    public:
        explicit GenEle14(int id): GenEle(id){};

        void start(PARAM& param) override{
            mMod(m3, testGenMod14, 1);
        }
    };

    GenEle14 testGen14(14);

}