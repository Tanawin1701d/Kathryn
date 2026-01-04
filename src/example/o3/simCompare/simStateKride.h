//
// Created by tanawin on 24/12/25.
//

#ifndef KATHRYN_SIMSTATEKRIDE_H
#define KATHRYN_SIMSTATEKRIDE_H

#include "kathryn.h"
#include "simState.h"
#include "example/o3/core/core.h"

namespace kathryn::o3{


    struct SimStateKride : SimState{

        Core& _core;

        bool isLastCycleMisPred = false;
        bool isLastCycleSucc    = false;

        bool isLastCycleDisp1 = false;
        bool isLastCycleDisp2 = false;
        ull  lastDispatchPtr  = 0;

        SimStateKride(Core& core):
            SimState(),
            _core(core){}

        pipStat generatePipState(PipSimProbe* pipProbe,
                                 ZyncSimProb* zyncSimProbe);

        ///// rsv helper
        void assignEleIfThere(ull& ele, RegSlot& val , const std::string& eleName);
        void assignRSV_Entry (RSV_BASE_ENTRY& entry  , RegSlot& regSlot);
        void assignRSV_Branch(RSV_BRANCH_ENTRY& entry, RegSlot& regSlot);
        void assignRSV_Mul   (RSV_MUL_ENTRY& entry   , RegSlot& regSlot);

        void assignEXEC_Entry (RSV_BASE_ENTRY& entry  , RegSlot& regSlot);
        void assignEXEC_Branch(RSV_BRANCH_ENTRY& entry, RegSlot& regSlot);
        void assignEXEC_Mul   (RSV_MUL_ENTRY& entry   , RegSlot& regSlot);
        void assignEXEC_LDST (RSV_BASE_ENTRY& entry  , RegSlot& regSlot) ;

        void assignARF_Table (int tableIdx, RegSlot& busySlot, RegSlot& renameSlot);

        void recruitValue() override;
        void recruitNextCycle() override;

        void printSlotWindow(SlotWriterBase& writer) override;

        void writeSlotIfTableChange(SlotWriterBase& writer,
                                    REC_PIP_STAGE stageIdx,
                                    std::vector<SlotSimInfo64> changeRows,
                                    int rowLimToPrintEntireRow) const;


    };

}

#endif //KATHRYN_SIMSTATEKRIDE_H