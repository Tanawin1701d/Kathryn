//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "genEle.h"

namespace kathryn{


    class testGenMod3Sub: public Module{

    public:
        mReg(cnt, 8);
        explicit testGenMod3Sub(int x): Module(){}

        void flow() override{

            cwhile(true){
                cnt <<= cnt + 1;
            }
        }
    };

    class testGenMod3: public Module{
    public:
        mMod(x1, testGenMod3Sub, 0);
        mMod(x2, testGenMod3Sub, 5);
        mReg(xr1, 8);
        mReg(xr2, 8);
        explicit testGenMod3(int x): Module(){}

        void flow() override{

            seq{
                xr1 <<= x1.cnt;
                xr2 <<= x2.cnt;
            }
        }

    };

    class GenEle3: public GenEle{
    public:
        explicit GenEle3(int id): GenEle(id){};

        void start(PARAM& param) override{
            mMod(m3, testGenMod3, 1);
        }
    };

    GenEle3 testGen3(3);

}