//
// Created by tanawin on 10/1/26.
//

#ifndef SRC_MODEL_FLOWBLOCK_STATE_ZTATECLASSASM_H
#define SRC_MODEL_FLOWBLOCK_STATE_ZTATECLASSASM_H

#include "model/flow_block/abstract/flow_block__base.h"

namespace kathryn{


    struct ZStateClassAsm{

        Operable*                     _identifier = nullptr;
        std::vector<int>              _caseIdent;
        std::vector<ClassAssignMeta*> _caseAssignMetas; /// < case_id, ue
        ///////// the conclude result
        UpdateEventSwitch* state_ue_event = nullptr;


        /////// disable the assign_metas because ztate don't allow it
        AssignMeta*get_sample_assign_meta_ptr();
        bool        is_empty()       {return _caseAssignMetas.empty();}
        int         get_size() const {return _caseAssignMetas.size();}
        bool        is_joinable(AssignMeta* sample){ assert(false);}
        bool        is_joinable(ClassAssignMeta* sample);
        ////////////////////////////////////////////////////////////

        int get_max_support_size() const;
        /** constructor*/
        explicit ZStateClassAsm(Operable* identifier);
        /** create the block belong to the caseid*/
        void add_case_block(int case_ident, ClassAssignMeta* class_assign_meta);
        /** create assignment node and state_ue that conclude this event*/
        AsmNode* create_asm_node(const std::vector<int>& glob_case_idents);


    };

}

#endif //SRC_MODEL_FLOWBLOCK_STATE_ZTATECLASSASM_H