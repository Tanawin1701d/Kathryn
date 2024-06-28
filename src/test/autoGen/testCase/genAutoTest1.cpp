//
// Created by tanawin on 25/6/2024.
//

#include "kathryn.h"
#include "genEle.h"

namespace kathryn{

    class testGenMod1: public Module{
    public:
        mReg(a, 8);
        mReg(b, 8);
        mWire(c, 8);

        explicit testGenMod1(int x): Module(){}

        void flow() override{

            a.asOutputGlob();
            c.asInputGlob();

            seq{
                cif(c == 0){
                    a <<= b;
                }
            }

        }

    };

    class GenEle1: public GenEle{

    public:
        explicit GenEle1(int id):GenEle(id){}

        void start(const PARAM& param) override{
            mMod(myMd, testGenMod1, 1);
        }

    };

    GenEle1 testCase(1);

}
