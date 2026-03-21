//
// Created by tanawin on 13/9/25.
//

#ifndef KATHRYN_MODEL_FLOWBLOCK_PIPELINE_PIPEMETA_H
#define KATHRYN_MODEL_FLOWBLOCK_PIPELINE_PIPEMETA_H


#include "model/flow_block/abstract/flow_block__base.h"
#include "model/flow_block/abstract/loop_st_macro.h"
#include "model/flow_block/abstract/sp_reg/state_reg.h"
#include "model/flow_block/abstract/nodes/node.h"
#include "model/flow_block/abstract/nodes/state_node.h"

#define m_zync(name) SyncMeta name{#name}


namespace kathryn{

    ///// sync is used to sync transfer data between
    ///   master (zync block) and slave (pip block)
    struct SyncMeta{

        const std::string _name;

        ///// main synchronizer
        m_wire(_syncMasterReady, 1);
        m_wire(_syncSlaveReady , 1) ;
        ///// slave ready after execution
        m_wire(_syncSlaveFin, 1);

        ///// typically the master should be zync block
        ///// typically the slave should be pipeline block
        m_wire(_syncMatched, 1);

        m_wire(kill_slave_signal  , 1);
        m_wire(kill_master_signal , 1);
        m_wire(hold_slave_signal   , 1);
        m_wire(hold_master_signal , 1);
        m_wire(start_slave_signal , 1);


        explicit SyncMeta(const std::string& name = "unnamed_sync_meta"):
        _name(name){
            set_sync_wire_base(_syncMatched, _syncMasterReady & _syncSlaveReady);
        }

        ~SyncMeta()= default;

        std::string get_name() const{ return _name;}

        ////// to ask that is slave fin in this cycle
        Operable& is_slave_fin(){return _syncSlaveFin;}

        void set_sync_wire_base(Wire& des_wire, Operable& opr1){
            UpdateEventBase* new_event = create_ue_helper(&opr1,
                                                       Slice({0,1}),
                                                       DEFAULT_UE_PRI_INTERNAL_MIN,
                                                       CM_CLK_FREE,
                                                       false);

            des_wire.add_update_meta(new_event);
        }

        /*
         * INTERNAL SIGNAL USED BY internal node belonging PIPE ZYNC node
         */

        ////// set start signal
        void set_master_ready(Operable& opr1){
            mf_assert(opr1.get_operable_slice().get_size() == 1, "set_master_ready size must be 1");
            set_sync_wire_base(_syncMasterReady, opr1);
        }

        void set_slave_ready(Operable& opr1){
            mf_assert(opr1.get_operable_slice().get_size() == 1, "set_slave_ready size must be 1");
            set_sync_wire_base(_syncSlaveReady, opr1);
        }

        void set_slave_finish(Operable& opr1){
            mf_assert(opr1.get_operable_slice().get_size() == 1, "set_slave_ready size must be 1");
            set_sync_wire_base(_syncSlaveFin, opr1);
        }

        /////// for use use signal

        void hold_slave(){ hold_slave_signal = 1; }

        void hold_master(){ hold_master_signal = 1; }

        void restart_slave(Operable* cond = nullptr){
            if (cond == nullptr){
                start_slave_signal = 1;
            }else{
                assert(cond->get_operable_slice().get_size() == 1);
                start_slave_signal = *cond;
            }
        }

        void kill_slave(bool auto_restart = false, Operable* cond = nullptr){

            if(cond == nullptr){
                kill_slave_signal = 1;
            }else{
                assert(cond->get_operable_slice().get_size() == 1);
                kill_slave_signal = *cond;
            }
            if (auto_restart){
                restart_slave(cond);
            }

        }

        void kill_master(){
            kill_master_signal = 1;
        }

    };

}

#endif //KATHRYN_MODEL_FLOWBLOCK_PIPELINE_PIPEMETA_H