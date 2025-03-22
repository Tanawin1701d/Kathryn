//
// Created by tanawin on 22/3/2025.
//

#ifndef src_model_flowBlock_offer_OFFERMETA_H
#define src_model_flowBlock_offer_OFFERMETA_H

#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"

namespace kathryn{

    const char OFFERCHOICE_UNNAME[] = "OFFER_CHOICE_UNNAME";

    ///////// OFFER SIGNAL META-DATA
    ///////// the neccessary signal used to communicate between
    /// offerbase and designer block
    struct OfferSignalMeta{
        expression* _offerSignal   = nullptr; /// cannot be null when sent to user
        Operable* _userCond        = nullptr;
        Operable*   _accept        = nullptr;
        //////// offer result (typically generated from master flowBlock)
        expression* _offerResult   = nullptr;
        expression* _isAnyPass = nullptr;

        bool _masterOfferRetrieved = false;
        bool _offerGottenByUser    = false;
        bool _userCondSetByUser    = false;
        bool _acceptGottenByUser   = false;

        void createOfferSignal();
        void setAcceptSignal(Operable* opr);
        void setUserCondSignal(Operable* opr);

        ///////set log signal
        void setOfferRetrievedByBase (){ _masterOfferRetrieved = true; }
        void setOfferGottenByUser    (){ _offerGottenByUser    = true; }
        void setUserCondSetByUser    (){ _userCondSetByUser    = true; }
        void setAcceptSetByUser      (){ _acceptGottenByUser   = true; }

        ///////get log signal
        [[nodiscard]] bool getOfferRetrievedByBase () const{ return _masterOfferRetrieved; }
        [[nodiscard]] bool getOfferGottenByUser    () const{ return _offerGottenByUser   ; }
        [[nodiscard]] bool getUserCondSetByUser    () const{ return _userCondSetByUser   ; }
        [[nodiscard]] bool getAcceptSetByUser      () const{ return _acceptGottenByUser  ; }

    };


    ///////// OFFER CHOICE STRUCTURE
    ///////// the neccessary structure to build subblock for offer base

    struct OfferCSt{
        ////// offerSignal will set if previous offerAccept is not set
        const std::string  _offerChoiceName = OFFERCHOICE_UNNAME;
        OfferSignalMeta    _offerSignals;
        std::vector<Node*> _basicNodes;

        explicit OfferCSt(const std::string&         offerChoiceName,
                          const OfferSignalMeta&    offerSignals,
                          const std::vector<Node*>&  basicNode);
    };

    //////// OFFER META
    //////// the neccessary meta data for each offer base

    struct OfferMeta{
        ///  choice name , choice meta
        std::unordered_map<std::string, OfferSignalMeta> choiceMap;

        [[nodiscard]]
        bool             isThereChoice(const std::string& choiceName);
        [[nodiscard]]
        OfferSignalMeta&  getChoice(const std::string& choiceName);

        void              buildChoice(const std::string& choiceName);
        void              tryBuildChoice(const std::string& choiceName);

    };




}

#endif //src/model/flowBlock/offer_OFFERMETA_H
