//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "genEle.h"

namespace kathryn{

    class testGenMod13: public Module{
    public:
        mVal(hf1, 8, 36);
        mVal(hf2, 8, 12);
        mReg(a, 8);
        mReg(b, 8);
        mReg(c, 8);
        mReg(d, 8);
        explicit testGenMod13(int x): Module(){
            a.asOutputGlob("a");
            b.asOutputGlob("b");
            c.asOutputGlob("c");
            d.asOutputGlob("d");
        }

        void flow() override{

            seq{
                par{
                    a = 2;
                    b = 4;
                    c = 0;
                    d = 0;
                }
                par{
                    d(2,6) <<= b(2, 6) + a(0, 4);
                }
            }
        }

    };

    class GenEle13: public GenEle{
    public:
        explicit GenEle13(int id): GenEle(id){};

        void start(PARAM& param) override{
            mMod(m3, testGenMod13, 1);
        }
    };

    GenEle13 testGen13(13);

}