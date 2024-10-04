//
// Created by tanawin on 3/10/2024.
//

#ifndef src_model_interface_singleHandShake_SHS_H
#define src_model_interface_singleHandShake_SHS_H



#include "model/hwComponent/box/box.h"
#include "model/hwComponent/abstract/makeComponent.h"
#include "model/hwComponent/register/register.h"
#include "model/hwComponent/wire/wire.h"
#include "model/hwComponent/expression/expression.h"

#include "model/interface/base/interface.h"

namespace kathryn{


    class SingleHandShakeBase: public ModelInterface{
    public:
        bool _autoAccept = true; ////// accept immediately when not busy
        ///// inside datastruct
        mReg(isBusy, 1);
        ///// recv react
        mWire(readyToGetNew, 1);
        mWire(reqResult    , 1);
        //////transceiver
        mWire(reqToSend    , 1);

        SingleHandShakeBase(bool autoAcc = true): _autoAccept(autoAcc){}

        void buildLogicBase() override;
        void send(){ reqToSend = 1;}
        void sendOn(Operable& opr){
            assert(opr.getOperableSlice().getSize() == 1);
            reqToSend = opr;
        }
        void sendAndWaitUntillSuccess();



        void declareReadyToRcv(){ readyToGetNew = 1; }

        //////////// get method
        Operable& isCurCycleBusy(){ return isBusy;}
        Operable& isNextCycleBusy(){return (isBusy & (~readyToGetNew)) | reqResult; }
        Operable& isReqSuccess  (){ return reqResult;}
        Operable& isReqToSend   (){ return reqToSend;}


    };







}

#endif //src_model_interface_singleHandShake_SHS_H
