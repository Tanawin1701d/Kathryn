//
// Created by tanawin on 28/6/2024.
//

#include "kathryn.h"
#include "genEle.h"

namespace kathryn{
    class testGenMod2: public Module{
    public:
        mReg(blink, 1);
        mReg(cnt, 8);

        explicit testGenMod2(int x): Module(){}

        void flow() override{

            blink.asOutputGlob();

            seq{
                cnt <<= 0;
                cnt <<= 0;
                cwhile(cnt < 7){
                    blink <<= 1;
                    syWait(100);
                    par{
                        blink <<= 0;
                        cnt <<= cnt + 1;
                    }
                    syWait(100);
                }

            }



        }
    };

    class GenEle2: public GenEle{
    public:
        explicit GenEle2(int id): GenEle(id){}

        void start(PARAM& param) override{
            mMod(myMd, testGenMod2, 1);
        }
    };

    GenEle2 tc2(2);

}