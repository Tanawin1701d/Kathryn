//
// Created by tanawin on 18/7/2024.
//

#include "valueSim.h"
#include "model/hwComponent/value/value.h"


namespace kathryn{


    /**
         * value simulation
         * */

    ValSimEngine::ValSimEngine(Val* master,
                             VCD_SIG_TYPE sigType,
                             ull rawValue):
    LogicSimEngine(master, master,
                   VST_INTEGER, false, rawValue),
    _master(master){ assert(master != nullptr);}


    std::string ValSimEngine::genSrcOpr(){
        return std::to_string(_initVal);
    }
    std::string ValSimEngine::genSlicedOprTo(Slice srcSlice){
        assert(srcSlice.checkValidSlice());
        assert(srcSlice.start < bitSizeOfUll);
        ull mask = createMask(srcSlice);
        return std::to_string((_initVal >> srcSlice.start) & mask);
    }
    std::string ValSimEngine::genSlicedOprAndShift(Slice desSlice, Slice srcSlice ){
        assert(srcSlice.checkValidSlice());
        assert(srcSlice.start < bitSizeOfUll);
        ull mask = createMask({srcSlice.start,
                                 srcSlice.start +
                                 std::min(desSlice.getSize(),srcSlice.getSize())});
        return std::to_string(((_initVal >> srcSlice.start) & mask) << desSlice.start);
    }

    std::string
    ValSimEngine::createGlobalVariable(){
        return "";
    }


}