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
        /** assign after parent is set*/
        FlowBlockBase* _parentFb     = nullptr;
        Module*        _parentMod    = nullptr;
        bool           _isJoinMaster = false; /////// this hints par block or other block that have multiple sublocks
                                                ///// to only listen the exit event of this block

        std::string    _zepTrackName; //// it is declared by user to track dep
        bool           _isZepTrackName = false;

    public:
        explicit FlowIdentifiable(const std::string& localName);

        /* seeter*/
        void setParent(FlowBlockBase* parentFlowBlock);
        void setParent(Module* parentModule);
        void setZepTrackName(const std::string& zepTrackName);
        void setJoinMaster(){ _isJoinMaster = true; }

        FlowBlockBase* getFlowBlockParrent();
        Module*        getModuleParent();
        bool           isZepTrackNameSet();
        std::string    getZepTrackName();
        bool           isJoinMaster() { return _isJoinMaster; }

        void buildInheritName() override;



    };




}


#endif //KATHRYN_FLOWIDENTIFIABLE_H