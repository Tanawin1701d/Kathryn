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
        void assignARF_Table (int tableIdx, RegSlot& busySlot, RegSlot& renameSlot);

        void recruitValue() override;

        void printSlotWindow(SlotWriterBase& writer) override{
            SimState::printSlotWindow(writer);
        }


    };

}

#endif //KATHRYN_SIMSTATEKRIDE_H