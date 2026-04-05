//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "genEle.h"

namespace kathryn{


    class subMod25: public Module{
    public: //// io wire
        mIn (i1, 8);mIn (i2, 8);
        mOut(e1, 8);mOut(e2, 8);
        ///////
        mReg(r1, 8);
        mReg(r2, 8);

        explicit subMod25(int x): Module(){}

        void flow() override{
            e1 = r1;
            e2 = r2;

            seq{
                r1 <<= i1;
                r2 <<= i2;
            }
        }
    };

    class testGenMod25: public Module{
    public:
        mVal  (hf1, 8, 36);
        mVal  (hf2, 8, 12);
        mReg  (a, 8);
        mReg  (b, 8);
        mWire (c, 8);
        mWire (d, 8);
        mMod  (sm, subMod25, 1);

        explicit testGenMod25(int x): Module(){
            a.asOutputGlob("a");
            b.asOutputGlob("b");
            c.asOutputGlob("c");
            d.asOutputGlob("d");

        }

        void flow() override{
            ////////// we use d as a interrupt
            sm.i1 = a;
            sm.i2 = b;
            c     = sm.e1;
            d     = sm.e2;
            par{
                a = 48;
                b = 49;

            }
        }
    };



    class GenEle25: public GenEle{
    public:
        explicit GenEle25(int id): GenEle(id){};

        void start(PARAM& param) override{
            mMod(m3, testGenMod25, 1);
        }
    };

    GenEle25 testGen25(25);

}