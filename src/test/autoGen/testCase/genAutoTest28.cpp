//
// Created by tanawin on 6/1/2026.
//
#include "kathryn.h"
#include "genEle.h"
#include "lib/math/math.h"

namespace kathryn{

    class testGenMod28: public Module{
    public:
        mReg (a, 8);
        mReg (b, 8);
        explicit testGenMod28(int x): Module(){
            a.asOutputGlob("a");
            b.asOutputGlob("b");
        }

        void flow() override{
            //////// connect output wire
            seq{
                a <<= 40;
                b <<= sqrtInt(a);
            }
        }

    };

    class GenEle28: public GenEle{
    public:
        explicit GenEle28(int id): GenEle(id){};

        void start(PARAM& param) override{
            mMod(m3, testGenMod28, 1);
        }
    };

    GenEle28 testGen28(28);

}
