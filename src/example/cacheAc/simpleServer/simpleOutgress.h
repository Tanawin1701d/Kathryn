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

        explicit SimpleOutgress(SERVER_PARAM svParam,
                                std::vector<BankOutputInterface*> outputInterfaces):
        OutgressBase(svParam, std::move(outputInterfaces)),
        PREFIX_BIT(svParam.prefixBit){
            assert(PREFIX_BIT > 0);
            for (int idx = 0; idx < _svParam.kvParam.valuefield.amtField(); idx++){
                oValues.push_back(
                    &makeOprReg(_svParam.kvParam.valuefield._valueFieldNames[0],
                                _svParam.kvParam.valuefield._valueFieldSizes[0]));
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
                    _outputInterfaces[idx]->declareReadyToRcv();
                    oKey  <<= *_outputInterfaces[idx]->resultKey;
                    int fieldIdx = 0;
                    for (Operable* valueOpr: _outputInterfaces[fieldIdx]->iValues){
                        *oValues[fieldIdx] <<= *valueOpr;
                        fieldIdx++;
                    }
                    curResBank <<= curBankItr;
                    areThereFin(idx) = 1;
                }
            }
            curBankItr <<= curBankItr + 1;

            zif (areThereFin) oValid <<= 1;
            zelse oValid <<= 0;


        }
    };

}

#endif //SIMPLEOUTGRESS_H
