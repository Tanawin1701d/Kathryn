//
// Created by tanawin on 2/2/2567.
//

#ifndef KATHRYN_SWHILE_H
#define KATHRYN_SWHILE_H

#include "whileBase.h"

#define swhile(expr) for(auto kathrynBlock = new FlowBlockSWhile(expr); kathrynBlock->doPrePostFunction(); kathrynBlock->step())


namespace kathryn {

    class FlowBlockSWhile : public FlowBlockWhileBase {

    private:
        StateNode*  conNode       = nullptr;
        PseudoNode* backToConNode = nullptr;
        PseudoNode* exposedConNode= nullptr; ///// the trigger from outer node to prevent conflict between con node and backto connode

        PseudoNode* normalExit    = nullptr;


    public:
        explicit FlowBlockSWhile(Operable& opr);
        ~FlowBlockSWhile();

        void buildHwComponent() override;
        /** get describe*/
        std::string getMdDescribe() override;
        void addMdLog(MdLogVal* mdLogVal) override;
        /** override simulator*/
        void simStartCurCycle() override;
        void simExitCurCycle() override;
    };
}

#endif //KATHRYN_SWHILE_H
