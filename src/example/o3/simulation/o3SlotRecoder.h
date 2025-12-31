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
#include "example/o3/simShare/recPipStage.h"


namespace kathryn::o3{

    class O3SlotRecorder{
    public:
        SlotWriter*  _slotWriter = nullptr;
        Core*        _core       = nullptr;
        PipStage*    _ps         = nullptr;

        bool isLastCycleMisPred = false;
        bool isLastCycleSucPred = false;

        ull  lastDispatchPtr  = 0;
        bool isLastCycleDisp1 = false;
        bool isLastCycleDisp2 = false;


        O3SlotRecorder(SlotWriter*  slotWriter,
                       Core*        core):
        _slotWriter(slotWriter),
        _core(core),
        _ps(&_core->pm){
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
        std::vector<std::string> getArfSlotVal(RegSlot& busyEntry,
                                               RegSlot& renameEntry);
        void writeArfSlot ();
        void writeRrfSlot () const;

        void writeFetchSlot   ();
        void writeDecodeSlot  ();
        void writeDispatchSlot();

        std::pair<bool, std::vector<std::string>>
             writeRsvSlot      (RegSlot& entry);
        void writeRsvBasicSlot (Table& table);
        void writeRsvAluSlot   (int idx, ORsv& orsv);
        void writeRsvMulSlot   ();
        void writeRsvBranchSlot();
        void writeRsvLoadSlot  ();

        std::pair<bool, std::vector<std::string>>
             writeStBufSlot  (RegSlot& entry);
        void writeStBufTable ();


        void writeIssueAluSlot   (int idx, ORsv& orsv, ZyncSimProb& zIssueProbe);
        void writeIssueMulSlot   ();
        void writeIssueBranchSlot();
        void writeIssueLdStSlot  ();

        void writeExecuteBasic     (RegSlot& src);
        void writeExecuteLDSTBasic (RegSlot& src);
        void writeExecuteAluSlot   (int idx, ORsv& orsv, PipSimProbe& pExecProbe);
        void writeExecuteMulSlot   ();
        std::pair<bool, bool>
             writeExecuteBranchSlot();
        void writeExecuteLdStSlot  ();

        ////// {misPred, sucPred}


        std::vector<std::string>
             writeRobSlot(ull robIdx);
        void writeCommitSlot();

        std::string translateOpcode(ull rawInstr);
        std::string translateAluOp(ull aluOpIdx);


    };
}


#endif //KATHRYN_O3SLOTRECODER_H