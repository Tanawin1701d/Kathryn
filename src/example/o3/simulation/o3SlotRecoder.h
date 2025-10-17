//
// Created by tanawin on 14/10/25.
//

#ifndef KATHRYN_O3SLOTRECODER_H
#define KATHRYN_O3SLOTRECODER_H

#include "kathryn.h"
#include <string>
#include <vector>

#include "example/o3/core/core.h"
#include "lib/instr/instrBase.h"
#include "sim/modelSimEngine/hwCollection/dataStructure/slot/slotProber.h"


namespace kathryn::o3{

    enum REC_PIP_STAGE{
        RPS_MPFT     = 0,
        RPS_ARF      = 1,
        RPS_RRF      = 2,
        RPS_FETCH    = 3,
        RPS_DECODE   = 4,
        RPS_DISPATCH = 5,
        RPS_RSV      = 6,
        RPS_ISSUE    = 7,
        RPS_EXECUTE  = 8,
        RPS_WB       = 9,
        RPS_COMMIT   = 10,
        RPS_NUM      = 11
    };
    class O3SlotRecorder{
    public:
        SlotWriter*  _slotWriter = nullptr;
        Core*        _core       = nullptr;
        PipStage*    _ps         = nullptr;

        bool isLastCycleMisPred = false;
        bool isLastCycleSucPred = false;


        O3SlotRecorder(SlotWriter*  slotWriter,
                       Core*        core):
        _slotWriter(slotWriter),
        _core(core){
            assert(_slotWriter != nullptr);
        }

        void recordSlot();

        /////// pipeline idle case writer
        bool writeSlotIfPipIdle(REC_PIP_STAGE stageIdx,
                                      PipSimProbe* pipProbe);
        bool writeSlotIfZyncStall(REC_PIP_STAGE stageIdx,
                                      ZyncSimProb* zyncSimProbe);
        //////// table writer
        void writeSlotIfTableChange(REC_PIP_STAGE stageIdx,
                                    std::vector<SlotSimInfo64> changeRows,
                                    int rowLimToPrintEntireRow) const;

        ////// pipeline writer
        void writeMpftSlot();
        void writeArfSlot ();
        void writeRrfSlot ();

        void writeFetchSlot   ();
        void writeDecodeSlot  ();
        void writeDispatchSlot();



        void writeRsvAluSlot     ();
        void writeRsvBranchSlot  ();

        void writeIssueAluSlot   ();
        void writeIssueBranchSlot();

        void writeExecuteBasic     (RegSlot& src);
        void writeExecuteAluSlot   ();
        ////// {misPred, sucPred}
        std::pair<bool, bool> writeExecuteBranchSlot();

        void writeCommitSlot();

        std::string translateOpcode(ull rawInstr);


    };
}


#endif //KATHRYN_O3SLOTRECODER_H