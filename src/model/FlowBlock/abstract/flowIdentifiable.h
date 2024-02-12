//
// Created by tanawin on 11/2/2567.
//

#ifndef KATHRYN_FLOWIDENTIFIABLE_H
#define KATHRYN_FLOWIDENTIFIABLE_H

#include "sim/logicRep/valRep.h"

namespace kathryn{

    extern ull LAST_FLOW_IDENT_ID;

    class FlowBlockBase;
    class Module;

    class FlowIdentifiable{
        ull _globalId = -1; ///// uniq id among all flowblock
        std::string _globalName; //////
        /** assign after parent is set*/
        std::vector<std::string> _inheritName;
        FlowBlockBase* _parentFb    = nullptr;
        Module*        _parentMod = nullptr;
        bool isParentFbAssignYet  = false;
        bool isParentMdAssignYet  = false;
        bool isInheritNameAssignYet = false;

    public:
        explicit FlowIdentifiable(std::string localName);

        void setParent(FlowBlockBase* parentFlowBlock);
        void setParent(Module* parentModule);

        std::vector<std::string>& getInheritName(){
            assert(isParentMdAssignYet);
            return _inheritName;
        }

        void assignInheritName();

        std::string getConCatInheritName(){
            assert(isInheritNameAssignYet);
            std::string ret;
            for (auto str: _inheritName){
                ret += str;
                ret += "_";
            }
            return ret;
        }


    };




}


#endif //KATHRYN_FLOWIDENTIFIABLE_H
