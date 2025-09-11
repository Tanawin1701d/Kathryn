//
// Created by tanawin on 13/5/2025.
//

#include <model/flowBlock/offer/offerPooler.h>

#include "pipeTran.h"
#include "pipePooler.h"

namespace kathryn{

    ////////// TARGET PIPE TRAN META

    void PipeTranMeta::createActivateSignal(){
        _activateSignal = new expression(1);
    }

    void PipeTranMeta::assignActivateSignal(Operable* notRstSignal) const{

        PipePooler* pipePooler = getPipePooler();
        assert(pipePooler   != nullptr);
        assert(_activateSignal != nullptr);
        Operable* pipeReady = pipePooler->getPipeReadySignal(_targetPipeName);
        pipeReady  = addLogic(pipeReady, _userAddCond, BITWISE_AND);
        pipeReady  = addLogic(pipeReady, notRstSignal, BITWISE_AND);

        (*_activateSignal) = (*pipeReady);

    }

    Operable* PipeTranMeta::addLogic(Operable* src0, Operable* src1, LOGIC_OP lop) const{
        assert(src0 != nullptr);

        if (src1 == nullptr){
            return src0;
        }
        switch (lop){
        case BITWISE_AND : {return &((*src0)&(*src1));}
        case BITWISE_OR  : {return &((*src0)|(*src1));}
        default          : {assert(false);}
        }
    }


    ///////// TARGET PIPE SYNC META

    void PipeSyncMeta::createPredefSignal(){
        _intendOfferSignal = new expression(1);
    }

    void PipeSyncMeta::assignOfferSignal(){
        OfferPooler* offerPooler = getOfferPooler();
        assert(offerPooler != nullptr);
        assert(_intendOfferSignal != nullptr);
        Operable* offerSignal = &offerPooler->getOfferSig(_masterOffer, _choiceName);

        if (_userReqOfferIf == nullptr){
            (*_intendOfferSignal) = *offerSignal;
        }else{ //// if the condition met ( userReqTOConsiderOffer -> offerReq ) == (~userReqTOConsiderOffer v offerReq)
            Operable* notReqUser = &(!(*_userReqOfferIf));
            (*_intendOfferSignal) = *addLogic(offerSignal, notReqUser, BITWISE_OR);
        }
    }

    void PipeSyncMeta::assignAcceptSignal(Operable* acceptSig){
        assert(acceptSig != nullptr);
        assert(acceptSig->getOperableSlice().getSize() == 1, "size of accept signal mismatch");
        _acceptSignal = acceptSig;
    }

    void PipeSyncMeta::notifyOfferPoolerAcceptSig(){
        assert(_acceptSignal != nullptr);
        OfferPooler* offerPooler = getOfferPooler();
        offerPooler->setAcceptSig(_masterOffer, _choiceName, *_acceptSignal);
    }

    Operable* PipeSyncMeta::addLogic(Operable* src0, Operable* src1, LOGIC_OP lop) const{
        assert(src0 != nullptr);

        if (src1 == nullptr){
            return src0;
        }
        switch (lop){
        case BITWISE_AND : {return &((*src0)&(*src1));}
        case BITWISE_OR  : {return &((*src0)|(*src1));}
        default          : {assert(false);}
        }
    }






}