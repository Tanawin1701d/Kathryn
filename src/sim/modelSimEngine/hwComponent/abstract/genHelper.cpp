//
// Created by tanawin on 19/7/2024.
//

#include "genHelper.h"
#include "logicSimEngine.h"


namespace kathryn{

    std::string getSrcOprFromOpr(Operable* opr){
        assert(opr != nullptr);
        return opr->getExactOperable().
                    getLogicSimEngineFromOpr()->genSrcOpr();
    }

    std::string getSlicedSrcOprFromOpr(Operable* opr){
        assert(opr != nullptr);
        LogicSimEngine* simEngine = opr->getLogicSimEngineFromOpr();
        Slice neededSlice = opr->getOperableSlice();
        ////// slice to operabe slice
        return simEngine->genSlicedOprTo(neededSlice);
    }

    std::string getSlicedAndShiftSrcOprFromOpr(Operable* opr, Slice desSlice){
        assert(opr != nullptr);
        LogicSimEngine* simEngine = opr->getLogicSimEngineFromOpr();
        Slice neededSlice = opr->getOperableSlice();
        ////// slice to operabe slice
        return simEngine->genSlicedOprAndShift(desSlice, neededSlice);
    }

    ull createMask(Slice maskSlice){
        assert((maskSlice.checkValidSlice()) && (maskSlice.stop <= bitSizeOfUll));
        assert(maskSlice.start < bitSizeOfUll);
        int size = maskSlice.getSize();
        return (size == bitSizeOfUll) ? -1 : ((((ull)1) << size) - 1);
    }


}