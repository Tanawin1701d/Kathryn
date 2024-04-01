//
// Created by tanawin on 2/2/2567.
//

#ifndef KATHRYN_CWHILE_H
#define KATHRYN_CWHILE_H

#include "whileBase.h"

#define cwhile(expr) for(auto kathrynBlock = new FlowBlockcWhile(expr); kathrynBlock->doPrePostFunction(); kathrynBlock->step())


namespace kathryn{

    class FlowBlockcWhile : public FlowBlockWhileBase{
    private:
        /**loop condition node*/
        PseudoNode*  upConditionNode     = nullptr;
        PseudoNode*  loopConditionNode   = nullptr;
        PseudoNode*  conditionNode       = nullptr;

        /** exit management node*/
        PseudoNode*  byPassExitNode      = nullptr;
        PseudoNode*  subBlockExitNode    = nullptr;

    public:
        FlowBlockcWhile(Operable& opr);
        FlowBlockcWhile(bool fallTrue);
        ~FlowBlockcWhile();
        void buildHwComponent() override;

        /** get describe*/
        std::string getMdDescribe() override; /** it is marked as legacy*/
        void addMdLog(MdLogVal *mdLogVal) override;


        /**override simulator*/
        void simStartCurCycle() override;
        void simExitCurCycle() override;




    };

}

#endif //KATHRYN_CWHILE_H
