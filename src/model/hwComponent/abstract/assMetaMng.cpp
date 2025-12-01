//
// Created by tanawin on 27/11/25.
//

#include "assMetaMng.h"

#include <vector>

namespace kathryn{

    std::vector<ClassAssignMeta*> classifyAss(std::vector<AssignMeta*>& baseMetas){

        std::vector<ClassAssignMeta*> result;

        /**
         * try to find group, if it is not, create the new group
         */
        for(AssignMeta* meta : baseMetas){
            bool foundGrp = false;
            /////// find group for it
            for (ClassAssignMeta* classMeta : result){
                if (classMeta->isJoinable(meta)){
                    classMeta->addAssignMeta(meta);
                    foundGrp = true;
                    break;
                }
            }
            if (!foundGrp){
                result.push_back(new ClassAssignMeta(meta));
            }
        }
        return result;

    }


}