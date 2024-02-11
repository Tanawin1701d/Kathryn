//
// Created by tanawin on 11/2/2567.
//

#ifndef KATHRYN_FLOWIDENTIFIABLE_H
#define KATHRYN_FLOWIDENTIFIABLE_H

#include "sim/logicRep/valRep.h"

namespace kathryn{

    extern ull LAST_FLOW_IDENT_ID;

    class FlowBlockBase;

    class FlowIdentifiable{
        ull _globalId = -1; ///// uniq id among all flowblock
        std::string _globalName; //////
        /** assign after parent is set*/
        std::vector<std::string> _inheritName;
        FlowBlockBase* _parent   = nullptr;
        bool isParrentAssignYet  = false;

    public:
        explicit FlowIdentifiable(std::string localName);

        void setParent(FlowBlockBase* parentFlowBlock);

        std::vector<std::string>& getInheritName(){
            assert(isParrentAssignYet);
            return _inheritName;
        }

        std::string getConCatInheritName(){
            std::string ret;
            for (auto str: _inheritName){
                ret += str;
                ret += "_";
            }
        }


    };




}


#endif //KATHRYN_FLOWIDENTIFIABLE_H
