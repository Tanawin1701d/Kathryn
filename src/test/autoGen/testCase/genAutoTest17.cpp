//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "genEle.h"

namespace kathryn{

    class testGenMod17: public Module{
    public:
        mVal(hf1, 8, 36);
        mVal(hf2, 8, 12);
        mReg(a, 8);
        mReg(b, 8);
        mReg(c, 8);
        mReg(d, 8);
        mMem(storage, 48, 8);
        explicit testGenMod17(int x): Module(){
            a.asOutputGlob("a");
            b.asOutputGlob("b");
            c.asOutputGlob("c");
            d.asOutputGlob("d");
        }

        void flow() override{

            seq{
                par{
                    a = 8;
                    b = 0;
                    c = 0;
                    d = 0;
                }
                syWait(5);
                b <<= 1;
                syWait(a);
                c <<= 2;

            }
        }

    };

    class GenEle17: public GenEle{
    public:
        explicit GenEle17(int id): GenEle(id){};

        void start(PARAM& param) override{
            mMod(m3, testGenMod17, 1);
        }
    };

    GenEle17 testGen17(17);

}