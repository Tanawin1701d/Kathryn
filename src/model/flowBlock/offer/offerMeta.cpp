//
// Created by tanawin on 22/3/2025.
//

#include "offerMeta.h"


namespace kathryn{

    ///////// OFFER SIGNAL META-DATA

    void OfferSignalMeta::createOfferSignal() {
        _offerSignal = new expression(1);
    }

    void OfferSignalMeta::setAcceptSignal(Operable* opr) {
        assert(opr != nullptr);
        _accept = opr;
    }

    void OfferSignalMeta::setUserCondSignal(Operable* opr){
        assert(opr != nullptr);
        _userCond = opr;
    }

    ///////// OFFER CHOICE STRUCTURE

    OfferCSt::OfferCSt(const std::string&        offerChoiceName,
                       const OfferSignalMeta&    offerSignals,
                       const std::vector<Node*>& basicNode):
    _offerChoiceName(offerChoiceName),
    _offerSignals(offerSignals),
    _basicNodes(basicNode){
        mfAssert(_offerChoiceName != OFFERCHOICE_UNNAME,
            "can't use " + std::string(OFFERCHOICE_UNNAME) + " as a offer choice name");
    }

    //////// OFFER META

    bool OfferMeta::isThereChoice(const std::string& choiceName){
        return choiceMap.find(choiceName) != choiceMap.end();
    }

    OfferSignalMeta& OfferMeta::getChoice(const std::string& choiceName){
        assert(isThereChoice(choiceName));
        return choiceMap.find(choiceName)->second;
    }

    void OfferMeta::buildChoice(const std::string& choiceName){
        assert(!isThereChoice(choiceName));
        OfferSignalMeta choiceSignalMeta;
        choiceSignalMeta.createOfferSignal();
        choiceMap.insert(std::make_pair(choiceName, choiceSignalMeta));
    }

    void OfferMeta::tryBuildChoice(const std::string& choiceName){
        if (!isThereChoice(choiceName)){
            buildChoice(choiceName);
        }
    }

}