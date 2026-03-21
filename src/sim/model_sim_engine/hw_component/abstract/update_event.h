//
// Created by tanawin on 27/11/25.
//

#ifndef SRC_SIM_MODELSIMENGINE_HWCOMPONENT_ABSTRACT_UPDATEEVENT_H
#define SRC_SIM_MODELSIMENGINE_HWCOMPONENT_ABSTRACT_UPDATEEVENT_H
#include "util/file_writer/code_writer/cpp_writer.h"

namespace kathryn{
    class LogicSimEngine;

    /**
     * update event base
     */
    struct UpdateEventBaseSimEngine{

        std::vector<UpdateEventBaseSimEngine*> sub_engine;

        virtual ~UpdateEventBaseSimEngine();

        virtual void create_sim_op(CbBaseCxx& cb,
                                 LogicSimEngine& logic_sim_engine,
                                 const std::string& aux_ass_str) = 0;
        
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
        void create_sim_op(CbBaseCxx& cb,
                         LogicSimEngine& logic_sim_engine,
                         const std::string& aux_ass_str) override;
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

        void create_sim_op(CbBaseCxx& cb,
                         LogicSimEngine& logic_sim_engine,
                         const std::string& aux_ass_str = "") override;
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
        void create_sim_op(CbBaseCxx& cb,
                         LogicSimEngine& logic_sim_engine,
                         const std::string& aux_ass_str = "") override;


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
        void create_sim_op(CbBaseCxx& cb,
                         LogicSimEngine& logic_sim_engine,
                         const std::string& aux_ass_str = "") override;

    };

    

    

    

}

#endif //SRC_SIM_MODELSIMENGINE_HWCOMPONENT_ABSTRACT_UPDATEEVENT_H
