//
// Created by tanawin on 15/9/2024.
//

#ifndef SIMPLEOUTGRESS_H
#define SIMPLEOUTGRESS_H

#include "kathryn.h"
#include "example/cacheAc/outgress.h"


namespace kathryn::cacheServer{

    class SimpleOutgress: public OutgressBase{
    public:

        const int SUFFIX_BIT;
        mReg(curBankItr, SUFFIX_BIT);
        mReg(oKey  , _svParam.kvParam.KEY_SIZE);
        mReg(oValue, _svParam.kvParam.VALUE_SIZE);

        explicit SimpleOutgress(SERVER_PARAM svParam,
                                std::vector<BankOutputInterface*> outputInterfaces):
        OutgressBase(svParam, std::move(outputInterfaces)),
        SUFFIX_BIT(svParam.kvParam.KEY_SIZE - svParam.prefixBit){
            assert(SUFFIX_BIT > 0);

            zif(getResetSignal()){
                curBankItr <<= 0;
            }
        }

        void flow() override{

            cwhile(true){
                for (int idx = 0; idx < _outputInterfaces.size(); idx++){
                    zif( (curBankItr == idx) &&
                         (_outputInterfaces[idx]->readyToSend)
                    ){
                        oKey   <<= _outputInterfaces[idx]->resultKey;
                        oValue <<= _outputInterfaces[idx]->resultValue;
                        _outputInterfaces[idx]->readyToRcv = 1;
                    }
                }
                curBankItr <<= curBankItr + 1;
            }


        }

    };

}

#endif //SIMPLEOUTGRESS_H
