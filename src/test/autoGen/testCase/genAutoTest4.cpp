//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "genEle.h"

namespace kathryn{

    class testGenMod4Sub2: public Module{
    public:
        mReg(cnt2, 8);
        explicit testGenMod4Sub2(int x): Module(){}
        void flow() override{
            cnt2.asOutputGlob();
            cwhile(true){
                cnt2 <<= cnt2+1;
            }
        }
    };


    class testGenMod4Sub: public Module{
    public:
        mReg(cnt, 8);
        mMod(r1, testGenMod4Sub2, 0);
        explicit testGenMod4Sub(int x): Module(){}
        void flow() override{
            cwhile(true){
                cnt <<= r1.cnt2 + 1;
            }
        }
    };

    class testGenMod4: public Module{
    public:
        mMod(x1, testGenMod4Sub, 0);
        mMod(x2, testGenMod4Sub, 5);
        mReg(xr1, 8);
        mReg(xr2, 8);
        explicit testGenMod4(int x): Module(){}

        void flow() override{

            seq{
                xr1 <<= x1.cnt;
                xr2 <<= x2.cnt;
            }
        }

    };

    class GenEle4: public GenEle{
    public:
        explicit GenEle4(int id): GenEle(id){};

        void start(PARAM& param) override{
            mMod(m3, testGenMod4, 1);
        }
    };

    GenEle4 testGen4(-1);

}