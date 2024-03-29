//
// Created by tanawin on 18/1/2567.
//

#ifndef KATHRYN_MODELSIMENGINELEGACY_H
#define KATHRYN_MODELSIMENGINELEGACY_H

#include <utility>
#include <vector>
#include "sim/logicRep/valRep.h"
#include "sim/simResWriter/simResWriter.h"
#include "model/hwComponent/abstract/Slice.h"

namespace kathryn {


//    class FlowSimEngineLegacy{
//    private:
//
//        int amtUsed = 0;
//        bool isStateRunningIn = false; /// check that are there
//                                       /// state is running in this block
//        bool _isSimMetaSet    = false;
//    public:
//        struct FLOW_Meta_afterMf{
//
//            ///std::string      _recName = "UNDEFINED_SIGNAL_RECORD_NAME";
//            FlowColEle*      _writer  = nullptr;
//        };
//
//        FLOW_Meta_afterMf _meta;
//
//        explicit FlowSimEngineLegacy() = default;
//
//        void incUsedTime();
//        int&  getAmtUsed(){return amtUsed;}
//
//        void setSimMeta(FLOW_Meta_afterMf meta){
//            _isSimMetaSet = true;
//            _meta = std::move(meta);
//        }
//
//        void setRunningStatus  (){isStateRunningIn = true;}
//        void unsetRunning()      {isStateRunningIn = false;}
//        bool isRunning   ()const {return isStateRunningIn; }
//    };
}
#endif //KATHRYN_MODELSIMENGINELEGACY_H
