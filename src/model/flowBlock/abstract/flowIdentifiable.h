//
// Created by tanawin on 11/2/2567.
//

#ifndef KATHRYN_FLOWIDENTIFIABLE_H
#define KATHRYN_FLOWIDENTIFIABLE_H

#include "sim/logicRep/valRep.h"
#include "model/abstract/identBase/identBase.h"

namespace kathryn{

    class FlowBlockBase;
    class Module;

    class FlowIdentifiable: public IdentBase{
        /// assign after parent is set
        FlowBlockBase* _parent_fb     = nullptr;
        Module*        _parent_mod    = nullptr;
        bool           _is_join_master = false; /// this hints par block or other block that have multiple sublocks
                                                /// to only listen the exit event of this block

        std::string    _user_zep_track_name; /// it is declared by user to track dep
        bool           _is_zep_track_name_set = false;

    public:
        explicit FlowIdentifiable(const std::string& local_name);

        /* seeter*/
        void set_parent(FlowBlockBase* parent_flow_block);
        void set_parent(Module* parent_module);
        void set_zep_track_name(const std::string& zep_track_name);
        void set_join_master(){ _is_join_master = true; }

        FlowBlockBase* get_flow_block_parent_ptr();
        Module*        get_module_parent_ptr();
        bool           is_zep_track_name_set();
        std::string    get_zep_track_name();
        bool           is_join_master() { return _is_join_master; }

        void build_inherit_name() override;



    };




}


#endif //KATHRYN_FLOWIDENTIFIABLE_H