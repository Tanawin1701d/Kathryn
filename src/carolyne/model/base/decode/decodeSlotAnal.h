//
// Created by tanawin on 13/2/2025.
//

#ifndef src_carolyne_model_base_decode_DECODESLOTANAL_H
#define src_carolyne_model_base_decode_DECODESLOTANAL_H

#include "carolyne/arch/base/isa/repo/repo.h"
#include "carolyne/model/base/coreArgs.h"


namespace kathryn::carolyne{

    constexpr int MAX_DECODE_OPR_NUMBER = 100;
    constexpr int UNDEF_OPR_IN_POOL     = -1;

    ////////// the decoder slot will be like this ////////////////
    /// |uop|valid_0|opr_raw_src_0|valid_1|opr_raw_des_1|...
    //////////////////////////////////////////////////////////////




    struct DecodeSlotAnalyzer{
        /////// in currently version we assume the equal of identification width
        /// for convinient

        int _maxIdentWidth = -1;
        IsaBaseRepo& _isaBaseRepo;
        std::vector<OprTypeBase*> pooledSrcOprs;
        std::vector<OprTypeBase*> pooledDesOprs;
        RegSlot* _regSlot = nullptr;
        mVal(validSet  , 1, 1);
        mVal(validUnset, 1, 0);


        ///////// command function
        virtual ~DecodeSlotAnalyzer(){delete _regSlot;}
        explicit DecodeSlotAnalyzer(IsaBaseRepo& isaBaseRepo);
        void     addOprsInConcern  (UopTypeBase* uopType, bool isSrc);
        void     addUopInConcern   (UopTypeBase* uopType);
        int      findIdxInPool     (UopTypeBase* uopType,
                                    int oprIdxInUop,
                                    bool isSrc); ///// it occurr repeatly at times

        ///////// get opr type base
        std::vector<OprTypeBase*>& getOprTypeRefs (bool isSrc);
        OprTypeBase*               getOprType     (int idx, bool isSrc);
        Slot                       getOprRawData  (int idx, bool isSrc);
        Slot                       getOprValidData(int idx, bool isSrc);
        bool                       isThereOpr     (int idx, bool isSrc);
        ///////// command function
        void         analIsa            ();
        void         buildDecoderSlot   ();
        void         addOprsTORowMeta(RowMeta& desRowMeta, bool isSrc);
        RegSlot&     getDecoderSlot     () const;
        virtual void blk_decode(Slot& rawSlot);
        virtual void decodeOprs (Wire& stateWire, Slot& rawSlot, UopTypeBase* uopType, UopMatcherBase* uopMatcher, bool isSrc);
        virtual void decodeUop  (Wire& stateWire, Slot& rawSlot, UopTypeBase* uopType, UopMatcherBase* uopMatcher);


        ////////// decoder slot
        std::string getValidFieldName(int idx, bool isSrc);
        std::string getDecOprFieldName(int idx, bool isSrc);
    };


}


#endif //src_carolyne_model_base_decode_DECODESLOTANAL_H
