//
// Created by tanawin on 19/7/2024.
//

#include "genHelper.h"
#include "logicSimEngine.h"


namespace kathryn{

    ValR getSrcOprFromOpr(Operable* opr1){
        assert(opr1 != nullptr);
        return opr1->getExactOperable().
                getLogicSimEngineFromOpr()->genSrcOpr();
    }


    ValR getSlicedSrcOprFromOpr(Operable* opr1){
        assert(opr1 != nullptr);
        LogicSimEngine* simEngine = opr1->getLogicSimEngineFromOpr();
        ////// slice to operabe slice
        return getSlicedSrcOprFromOpr(opr1, simEngine->getValR_Type());
    }

    ValR getSlicedSrcOprFromOpr(Operable* opr1, SIM_VALREP_TYPE_ALL desField){
        assert(opr1 != nullptr);
        LogicSimEngine* simEngine = opr1->getLogicSimEngineFromOpr();
        Slice neededSlice = opr1->getOperableSlice();
        ////// slice to operabe slice
        return simEngine->genSlicedOprTo(neededSlice, desField);
    }

    ValR getSlicedAndShiftSrcOprFromOpr(Operable* opr1, Slice desSlice, SIM_VALREP_TYPE_ALL desField){
        assert(opr1 != nullptr);
        LogicSimEngine* simEngine = opr1->getLogicSimEngineFromOpr();
        Slice neededSlice = opr1->getOperableSlice();
        ////// slice to operabe slice
        return simEngine->genSlicedOprAndShift(desSlice, neededSlice, desField);
    }

    ull createMask(Slice maskSlice){
        assert((maskSlice.checkValidSlice()) && (maskSlice.stop <= bitSizeOfUll));
        assert(maskSlice.start < bitSizeOfUll);
        int size = maskSlice.getSize();
        return (size == bitSizeOfUll) ? -1 : ((((ull)1) << size) - 1);
    }


}