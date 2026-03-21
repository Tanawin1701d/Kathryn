//
// Created by tanawin on 27/11/25.
//

#include "ass_meta_mng.h"
#include "model/flow_block/abstract/nodes/asm_node.h"
#include "vector"

namespace kathryn{

    void try_add_or_create_asm_meta(
        AsmNode* asm_node,
        std::vector<ClassAssignMeta*>& assign_metas
    ){

        assert(asm_node != nullptr);
        for (AssignMeta* asm_meta: asm_node->get_assign_metas()){
            bool found = false;
            for (ClassAssignMeta* class_asm: assign_metas){
                if (class_asm->is_joinable(asm_meta)){
                    class_asm->add_assign_meta(asm_meta);
                    found = true;
                    break;
                }
            }
            if (!found){
                assign_metas.push_back(new ClassAssignMeta(asm_meta));
            }
        }
        asm_node->transfer_out_assign_meta_ownership();

    }



    // std::vector<ClassAssignMeta*> classify_ass(std::vector<AssignMeta*>& base_metas){
    //
    //     std::vector<ClassAssignMeta*> result;
    //
    //     /**
    //      * try to find group, if it is not, create the new group
    //      */
    //     for(AssignMeta* meta : base_metas){
    //         bool found_grp = false;
    //         /////// find group for it
    //         for (ClassAssignMeta* class_meta : result){
    //             if (class_meta->is_joinable(meta)){
    //                 class_meta->add_assign_meta(meta);
    //                 found_grp = true;
    //                 break;
    //             }
    //         }
    //         if (!found_grp){
    //             result.push_back(new ClassAssignMeta(meta));
    //         }
    //     }
    //     return result;
    //
    // }


}
