//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "genEle.h"

namespace kathryn{

    class testGenMod6: public Module{
    public:
        mVal(hf1, 8, 36);
        mVal(hf2, 8, 12);
        mReg(a, 8);
        mReg(b, 8);
        mReg(c, 8);
        explicit testGenMod6(int x): Module(){}

        void flow() override{

            mWire(plusReg, 8);
            mReg (minsReg, 8);
            mReg (bwAndReg, 8);
            mReg (bwOrReg, 8);

            par{
                a        <<= hf1;
                b        <<= hf1;
                c        <<= hf1;
                plusReg  =   hf1 + hf2;
                minsReg  <<= hf1 - hf2;
                bwAndReg <<= hf1 & hf2;
                bwOrReg  <<= hf1 | hf2;
            }

            a       .asOutputGlob("a");
            b       .asOutputGlob("b");
            c       .asOutputGlob("c");
            plusReg .asOutputGlob("plusReg");
            minsReg .asOutputGlob("minsReg");
            bwAndReg.asOutputGlob("bwAndReg");
            bwOrReg .asOutputGlob("bwOrReg");

        }

    };

    class GenEle6: public GenEle{
    public:
        explicit GenEle6(int id): GenEle(id){};

        void start(PARAM& param) override{
            mMod(m3, testGenMod6, 1);
        }
    };

    GenEle6 testGen6(6);

}