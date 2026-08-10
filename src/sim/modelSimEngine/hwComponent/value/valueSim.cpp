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
    LogicSimEngine(master, master, master,
                   VST_INTEGER, false, rawValue),
    _master(master){ assert(master != nullptr);}


    ValR ValSimEngine::getValRep(){
        int size = _asb->getAssignSlice().getSize();
        SIM_VALREP_TYPE_ALL svt = getValR_Type();

        //////// >64-bit constant: inline every LSB-first word (no truncation to ull)
        if (svt.type == SVT_U64M){
            const std::vector<ull>& words = _master->getConstOprWide();
            std::string init = SVT_toUnitType(svt) + "({";
            for (size_t i = 0; i < words.size(); i++){
                if (i){ init += ","; }
                init += std::to_string(words[i]) + CXX_ULL_SUFFIX;
            }
            init += "})";
            return ValR(svt, size, init);
        }

        ValR x(svt, size,
            std::to_string(_initVal)+CXX_ULL_SUFFIX);
        /////// due to ull string init we must cast first
        return x.castBase(x._valType, x._size);
    }

    void
    ValSimEngine::createGlobalVariable(CbBaseCxx& cb){}


}
