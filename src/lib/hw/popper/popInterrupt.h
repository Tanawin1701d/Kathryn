//
// Created by tanawin on 26/1/2025.
//

#ifndef src_lib_hw_popper_POPINTERRUPT_H
#define src_lib_hw_popper_POPINTERRUPT_H
#include "model/controller/controller.h"

namespace kathryn{

    struct PopIntrMng{

        std::map<std::string,Wire*> intrSigMaps;

        ////// todo next we will reroute the signal' owner module to
        ////// fix point module

        Wire& registerIntrSignal(const std::string& sigName){

            mfAssert(intrSigMaps.find(sigName) == intrSigMaps.end(),
                     "there is duplicated signal");
            Wire& newSig = mOprWire(sigName, 1);
            intrSigMaps.insert(std::make_pair(sigName, &newSig));
            return newSig;
        }

        void raiseIntrSignal(const std::string& sigName){
            mfAssert(intrSigMaps.find(sigName) != intrSigMaps.end(),
                "there is no declared interrupt signal");
            Wire& targetSignal = *intrSigMaps.find(sigName)->second;
            targetSignal = 1;
        }

        Wire& getIntrSignal(const std::string& sigName){
            mfAssert(intrSigMaps.find(sigName) != intrSigMaps.end(),
                "there is no declared interrupt signal");
            Wire& targetSignal = *intrSigMaps.find(sigName)->second;
            return targetSignal;
        }

    };

    struct PopIntrable{
        PopIntrMng* _intrMng = nullptr;

        explicit PopIntrable(PopIntrMng* intrMng = nullptr):
        _intrMng(intrMng){}

        Wire& registerIntrSignal(const std::string& sigName) const{
            return _intrMng->registerIntrSignal(sigName);
        }
        void raiseIntrSignal(const std::string& sigName) const{
            _intrMng->raiseIntrSignal(sigName);
        }
        Wire& getIntrSignal(const std::string& sigName) const{
            return _intrMng->getIntrSignal(sigName);
        }
    };



}

#endif //src_lib_hw_popper_POPINTERRUPT_H
