//
// Created by tanawin on 3/1/2567.
//

#ifndef KATHRYN_WAITREG_H
#define KATHRYN_WAITREG_H

#include <iostream>
#include <cmath>
#include "ctrlFlowRegBase.h"
#include "model/hwComponent/expression/expression.h"
#include "model/hwComponent/value/value.h"
#include "model/hwComponent/abstract/makeComponent.h"

namespace kathryn{

    /**
      *
      * conditional wait state register
      *
      * */

    class CondWaitStateReg : public CtrlFlowRegBase{

        makeVal(_upState  ,1, 1);
        makeVal(_downState,1, 0);
        Operable* _condOpr = nullptr;

    protected:

        void com_init() override;

    public:
        /** constructor*/
        explicit CondWaitStateReg(Operable* condOpr);
        /** add prior state that trigger this state*/
        UpdateEvent* addDependState(Operable* dependState, Operable* activateCond, CLOCK_MODE cm) override;
        /** generate reset event*/
        void makeUnSetStateEvent(CLOCK_MODE cm) override;
        /** make user reset event*/
        void  makeUserRstEvent(Operable* rst, CLOCK_MODE cm) override;
        /** generate out expression*/
        Operable* generateEndExpr() override;
        /** oevrride operator to prevent false input*/
        Reg& operator <<= (Operable& b) override {
            std::cout << "we not support = operator in register";
            return *this;
        }
    };

    /**
      *
      * cycle count wait state register
      *
      * */


    /**
     *                    2 1 0
     * [ | | | | | | | | | | | ]
     *                        ^---------- state bit
     * */

    class CycleWaitStateReg : public CtrlFlowRegBase{
    private:
        /**wait cycle meta data*/

        int               _waitCycle = -1;
        int               _cntBitSz     = -1;
        static const int  stateSize = 1;
        const int         _totalBitSize = -1;
        const int         startVal = 0b11;
        /**when counter is reached exit expression will be set*/
        Operable* IdleCnt     = nullptr;
        Operable* _startCnt    = nullptr;
        Operable* _endCnt      = nullptr;
    protected:

        void com_init() override;

    public:
        /** constructor*/
        explicit CycleWaitStateReg(int waitCycle);
        explicit CycleWaitStateReg(Operable* endCnt);

        /** add prior state that trigger this state*/
        UpdateEvent* addDependState(Operable* dependState, Operable* activateCond, CLOCK_MODE cm) override;
        /** add Inc to count*/
        void makeIncStateEvent(Operable* holdSignal, CLOCK_MODE cm);
        /** reset event*/
        void makeUnSetStateEvent(CLOCK_MODE cm) override;
        /** make user reset event*/
        void  makeUserRstEvent(Operable* rst, CLOCK_MODE cm) override;
        /** generate out expression*/
        Operable* generateEndExpr() override;
        /** oevrride operator to prevent false input*/
        Reg& operator <<= (Operable& b) override {
            std::cout << "we not support = operator in register";
            return *this;
        }
    };

    static int calBitUsed(int maxNumber){
        assert(maxNumber > 0);
        int amtNumberUsed = maxNumber + 1;
        /** time 2 and minus 1 to make ceiling of the number*/
        return (int) log2(amtNumberUsed * 2 - 1);
    }



}

#endif //KATHRYN_WAITREG_H
