//
// Created by tanawin on 19/7/2024.
//

#include "genHelper.h"
#include "logicSimEngine.h"


namespace kathryn{

    ValR getSrcOprFromOpr(Operable* opr){
        assert(opr != nullptr);
        return opr->getExactOperable().
                getLogicSimEngineFromOpr()->genSrcOpr();
    }


    ValR getSlicedSrcOprFromOpr(Operable* opr){
        assert(opr != nullptr);
        LogicSimEngine* simEngine = opr->getLogicSimEngineFromOpr();
        ////// slice to operabe slice
        return getSlicedSrcOprFromOpr(opr, simEngine->getValR_Type());
    }

    ValR getSlicedSrcOprFromOpr(Operable* opr, SIM_VALREP_TYPE_ALL desField){
        assert(opr != nullptr);
        LogicSimEngine* simEngine = opr->getLogicSimEngineFromOpr();
        Slice neededSlice = opr->getOperableSlice();
        ////// slice to operabe slice
        return simEngine->genSlicedOprTo(neededSlice, desField);
    }

    ValR getSlicedAndShiftSrcOprFromOpr(Operable* opr, Slice desSlice, SIM_VALREP_TYPE_ALL desField){
        assert(opr != nullptr);
        LogicSimEngine* simEngine = opr->getLogicSimEngineFromOpr();
        Slice neededSlice = opr->getOperableSlice();
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