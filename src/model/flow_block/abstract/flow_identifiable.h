//
// Created by tanawin on 11/2/2567.
//

#ifndef KATHRYN_FLOWIDENTIFIABLE_H
#define KATHRYN_FLOWIDENTIFIABLE_H

#include "sim/logic_rep/val_rep.h"
#include "model/abstract/ident_base/ident_base.h"

namespace kathryn{

    class FlowBlockBase;
    class Module;

    class FlowIdentifiable: public IdentBase{
        /** assign after parent is set*/
        FlowBlockBase* _parentFb     = nullptr;
        Module*        _parentMod    = nullptr;
        bool           _isJoinMaster = false; /////// this hints par block or other block that have multiple sublocks
                                                ///// to only listen the exit event of this block

        std::string    _zepTrackName; //// it is declared by user to track dep
        bool           _isZepTrackName = false;

    public:
        explicit FlowIdentifiable(const std::string& local_name);

        /* seeter*/
        void set_parent(FlowBlockBase* parent_flow_block);
        void set_parent(Module* parent_module);
        void set_zep_track_name(const std::string& zep_track_name);
        void set_join_master(){ _isJoinMaster = true; }

        FlowBlockBase*get_flow_block_parrent_ptr();
        Module*get_module_parent_ptr();
        bool           is_zep_track_name_set();
        std::string    get_zep_track_name();
        bool           is_join_master() { return _isJoinMaster; }

        void build_inherit_name() override;



    };




}


#endif //KATHRYN_FLOWIDENTIFIABLE_H