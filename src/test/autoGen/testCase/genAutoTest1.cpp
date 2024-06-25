//
// Created by tanawin on 25/6/2024.
//

#include "kathryn.h"
#include "genAutoTest1.h"

namespace kathryn{

    class testGenMod1: public Module{
    public:
        mReg(a, 8);
        mReg(b, 8);

        explicit testGenMod1(int x): Module(){}

        void flow() override{

            seq{
                a <<= b;
            }

        }

    };

    void startGen(PARAM& params){
        mMod(news, testGenMod1, 1);
        startModelKathryn();

        GenController* genCtrl = getGenController();
        assert(genCtrl != nullptr);
        genCtrl->initEnv(params);
        genCtrl->routeIo();
        genCtrl->generateEveryModule();
        genCtrl->reset();

    }


}