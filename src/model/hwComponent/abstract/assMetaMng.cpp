//
// Created by tanawin on 27/11/25.
//

#include "assMetaMng.h"
#include "model/flowBlock/abstract/nodes/asmNode.h"
#include <vector>

namespace kathryn{

    void tryAddOrCreateAsmMeta(
        AsmNode* asmNode,
        std::vector<ClassAssignMeta*>& assignMetas
    ){

        assert(asmNode != nullptr);
        for (AssignMeta* asmMeta: asmNode->getAssignMetas()){
            bool found = false;
            for (ClassAssignMeta* classAsm: assignMetas){
                if (classAsm->isJoinable(asmMeta)){
                    classAsm->addAssignMeta(asmMeta);
                    found = true;
                    break;
                }
            }
            if (!found){
                assignMetas.push_back(new ClassAssignMeta(asmMeta));
            }
        }
        asmNode->transferOutAssignMetaOwnership();

    }



    // std::vector<ClassAssignMeta*> classifyAss(std::vector<AssignMeta*>& baseMetas){
    //
    //     std::vector<ClassAssignMeta*> result;
    //
    //     /**
    //      * try to find group, if it is not, create the new group
    //      */
    //     for(AssignMeta* meta : baseMetas){
    //         bool foundGrp = false;
    //         /////// find group for it
    //         for (ClassAssignMeta* classMeta : result){
    //             if (classMeta->isJoinable(meta)){
    //                 classMeta->addAssignMeta(meta);
    //                 foundGrp = true;
    //                 break;
    //             }
    //         }
    //         if (!foundGrp){
    //             result.push_back(new ClassAssignMeta(meta));
    //         }
    //     }
    //     return result;
    //
    // }


}
