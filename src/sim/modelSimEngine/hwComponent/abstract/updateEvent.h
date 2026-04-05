//
// Created by tanawin on 27/11/25.
//

#ifndef SRC_SIM_MODELSIMENGINE_HWCOMPONENT_ABSTRACT_UPDATEEVENT_H
#define SRC_SIM_MODELSIMENGINE_HWCOMPONENT_ABSTRACT_UPDATEEVENT_H
#include "util/fileWriter/codeWriter/cppWriter.h"

namespace kathryn{
    class LogicSimEngine;

    /**
     * update event base
     */
    struct UpdateEventBaseSimEngine{

        std::vector<UpdateEventBaseSimEngine*> subEngine;

        virtual ~UpdateEventBaseSimEngine();

        virtual void createSimOp(CbBaseCxx& cb,
                                 LogicSimEngine& logicSimEngine,
                                 const std::string& auxAssStr) = 0;
        
    };

    /**
     * update event basic
     */
    struct UpdateEventBasic;
    struct UpdateEventBasicSimEngine: UpdateEventBaseSimEngine{
        UpdateEventBasic* master;

        explicit UpdateEventBasicSimEngine(UpdateEventBasic* m): master(m){
            assert(m != nullptr);
        }
        void createSimOp(CbBaseCxx& cb,
                         LogicSimEngine& logicSimEngine,
                         const std::string& auxAssStr) override;
    };

    /**
     * update event group
     */

    struct UpdateEventGrp;
    struct UpdateEventGrpSimEngine: UpdateEventBaseSimEngine{

        UpdateEventGrp* master = nullptr;

        explicit UpdateEventGrpSimEngine(UpdateEventGrp* m): master(m){
            assert(m != nullptr);
        }

        void createSimOp(CbBaseCxx& cb,
                         LogicSimEngine& logicSimEngine,
                         const std::string& auxAssStr = "") override;
    };

    /**
     * update event cond
     */

    struct UpdateEventCond;
    struct UpdateEventCondSimEngine: UpdateEventBaseSimEngine{

        UpdateEventCond* master = nullptr;

        UpdateEventCondSimEngine(UpdateEventCond* m): master(m){
            assert(m != nullptr);
        }
        void createSimOp(CbBaseCxx& cb,
                         LogicSimEngine& logicSimEngine,
                         const std::string& auxAssStr = "") override;


    };

    /**
     * update event state
     */

    struct UpdateEventSwitch;
    struct UpdateEventSwitchSimEngine: UpdateEventBaseSimEngine{

        UpdateEventSwitch* master = nullptr;
        UpdateEventSwitchSimEngine(UpdateEventSwitch* m): master(m){
            assert(m != nullptr);
        }
        void createSimOp(CbBaseCxx& cb,
                         LogicSimEngine& logicSimEngine,
                         const std::string& auxAssStr = "") override;

    };

    

    

    

}

#endif //SRC_SIM_MODELSIMENGINE_HWCOMPONENT_ABSTRACT_UPDATEEVENT_H
