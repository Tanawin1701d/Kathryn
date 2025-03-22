//
// Created by tanawin on 18/3/2025.
//

#include "offerPooler.h"


namespace kathryn{


    bool OfferPooler::isThereOffer(const std::string &offerName) {
        return offerMaps.find(offerName) != offerMaps.end();
    }

    void OfferPooler::tryBuildOffer(const std::string &offerName) {
        if (!isThereOffer(offerName)){
            offerMaps.insert({offerName, OfferMeta()});
        }
    }

    void OfferPooler::tryBuildChoice(const std::string& offerName,
                                     const std::string &choiceName) {
        assert(isThereOffer(offerName));
        OfferMeta& curOffer = offerMaps[offerName];
        curOffer.tryBuildChoice(choiceName);
    }

    Operable& OfferPooler::getOfferSig(const std::string& offerName,
                                       const std::string& choiceName) {
        ///// incase offer meta is not built yet;
        tryBuildOffer(offerName);
        tryBuildChoice(offerName, choiceName);
        ///// get data
        OfferMeta&        offerMeta = offerMaps[offerName];
        OfferSignalMeta& offerCMeta = offerMeta.getChoice(choiceName);
        offerCMeta.setOfferGottenByUser(); ///// mark log
        assert(offerCMeta._offerSignal != nullptr);
        return *offerCMeta._offerSignal;
    }

    void OfferPooler::setAcceptSig(const std::string& offerName,
                                   const std::string& offerChoice,
                                   Operable& acceptSignal) {
        ///// incase offer meta is not built yet;
        tryBuildOffer(offerName);
        tryBuildChoice(offerName, offerChoice);
        ///// set data
        OfferMeta& offerMeta = offerMaps[offerName];
        OfferSignalMeta& offerCMeta = offerMeta.getChoice(offerChoice);
        offerCMeta.setAcceptSignal(&acceptSignal);
        offerCMeta.setAcceptSetByUser(); ////// marklog
    }

    OfferMeta& OfferPooler::getOfferMeta(const std::string& offerName){
        tryBuildOffer(offerName);
        //////
        return offerMaps.find(offerName)->second;
    }


    void OfferPooler::start() { /* do nothing */}

    void OfferPooler::reset() {
        offerMaps.clear();
        ////// offerMeta and MetaSignal can be discard automatically
    }

    void OfferPooler::clean() { OfferPooler::reset();}


    OfferPooler* offerPoolerBase = nullptr;

    OfferPooler* getOfferPooler(){

        if (offerPoolerBase == nullptr){
            offerPoolerBase = new OfferPooler();
        }
        return offerPoolerBase;

    }
}
