//
// Created by tanawin on 15/9/2024.
//

#ifndef SIMPLEOUTGRESS_H
#define SIMPLEOUTGRESS_H

#include <utility>

#include "kathryn.h"
#include "example/cacheAc/outgress.h"



namespace kathryn::cacheServer{

    class SimpleOutgress: public OutgressBase{
    public:

        const int PREFIX_BIT = -1;

        mReg(curBankItr, PREFIX_BIT);
        mReg(oKey      , _svParam.kvParam.KEY_SIZE);
        std::vector<Reg*> oValues;
        mReg(curResBank, PREFIX_BIT);
        mReg(oValid, 1);
        mWire(areThereFin, _outputInterfaces.size());

        explicit SimpleOutgress(SERVER_PARAM& svParam,
                                std::vector<BankOutputInterface*> outputInterfaces):
        OutgressBase(svParam, std::move(outputInterfaces)),
        PREFIX_BIT(svParam.prefixBit){
            assert(PREFIX_BIT > 0);
            for (auto [key, size] :_svParam.kvParam.valuefield.getAllKeySize()){
                oValues.push_back(&makeOprReg(key, size));
            }
        }

        void flow() override{

            zif(getResetSignal()){
                curBankItr <<= 0;
                oValid     <<= 0;
            }

            for (int idx = 0; idx < _outputInterfaces.size(); idx++){
                zif( (curBankItr == idx) & _outputInterfaces[idx]->isReqToSend()
                ){  ////// tell that we are finish
                    BankOutputInterface& outputItf = *_outputInterfaces[idx];

                    oKey             <<= *_outputInterfaces[idx]->resultKey;
                    curResBank       <<= curBankItr;
                    areThereFin(idx) = 1;
                    outputItf.declareReadyToRcv();
                    int fieldIdx = 0;
                    for (Operable* valueOpr: outputItf.iValues){
                        *oValues[fieldIdx] <<= *valueOpr;
                        fieldIdx++;
                    }
                }
            }
            curBankItr <<= curBankItr + 1;

            zif (areThereFin) oValid <<= 1;
            zelse oValid <<= 0;


        }
    };

}

#endif //SIMPLEOUTGRESS_H
