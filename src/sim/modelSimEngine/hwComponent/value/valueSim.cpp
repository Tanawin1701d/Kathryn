//
// Created by tanawin on 18/7/2024.
//

#include "valueSim.h"
#include "model/hwComponent/value/value.h"
#include "sim/modelSimEngine/hwComponent/abstract/genHelper.h"


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
        std::string retVal = SVT_toType(getValR_Type());
        retVal += "(" + std::to_string(_initVal) + CXX_ULL_SUFFIX + ")";
        return retVal;
    }
    std::string ValSimEngine::genSlicedOprTo(Slice srcSlice, SIM_VALREP_TYPE svt){
        assert(srcSlice.checkValidSlice());
        assert(srcSlice.start < bitSizeOfUll);
        ull mask = createMask(srcSlice);
        ull value = (_initVal >> srcSlice.start) & mask;
        std::string retVal = SVT_toType(svt);
        retVal += "(" + std::to_string(value) + CXX_ULL_SUFFIX + ")";
        return retVal;
    }
    std::string ValSimEngine::genSlicedOprAndShift(Slice desSlice, Slice srcSlice,
                                                   SIM_VALREP_TYPE svt){
        assert(srcSlice.checkValidSlice());
        assert(srcSlice.start < bitSizeOfUll);
        ull mask = createMask({srcSlice.start,
                                 srcSlice.start +
                                 std::min(desSlice.getSize(),srcSlice.getSize())});
        ull value = ((_initVal >> srcSlice.start) & mask) << desSlice.start;
        std::string retVal = SVT_toType(svt);
        retVal += "(" + std::to_string(value) + CXX_ULL_SUFFIX + ")";
        return retVal;
    }

    void
    ValSimEngine::createGlobalVariable(CbBaseCxx& cb){}


}
