//
// Created by tanawin on 18/3/2025.
//

#ifndef src_model_flowBlock_offer_OFFERPOOLER_H
#define src_model_flowBlock_offer_OFFERPOOLER_H

#include <string>
#include <unordered_map>

#include "offer.h"
#include "offerMeta.h"
#include "abstract/mainControlable.h"

#define getOffer(offerName, offerChoice) getOfferPooler()->getOfferSig(offerName, offerChoice)
#define ackOffer(offerName, offerChoice, cond) getOfferPooler()->setAcceptSig(offerName, offerChoice, cond);

namespace kathryn{


    class OfferPooler: public MainControlable{
    public:
        ////////  <offerBaseName, offerMeta>
        std::unordered_map<std::string, OfferMeta> offerMaps;

        bool isThereOffer   (const std::string& offerName);
        void tryBuildOffer  (const std::string& offerName);
        void tryBuildChoice (const std::string& offerName,
                             const std::string& choiceName);

        Operable& getOfferSig(const std::string& offerName,
                              const std::string& choiceName);

        void      setAcceptSig(const std::string& offerName,
                               const std::string& offerChoice,
                               Operable& acceptSignal);

        OfferMeta& getOfferMeta(const std::string& offerName);

        void start() override;
        void reset() override;
        void clean() override;


    };

    OfferPooler* getOfferPooler();

}

#endif //src_model_flowBlock_offer_OFFERPOOLER_H
