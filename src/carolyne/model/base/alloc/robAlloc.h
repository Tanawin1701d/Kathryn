//
// Created by tanawin on 17/2/2025.
//
///// src/carolyne/model/base/alloc/robAlloc.h
#ifndef KATHRYN_CAROLYNE_MODEL_BASE_ALLOC_ROBALLOC_H
#define KATHRYN_CAROLYNE_MODEL_BASE_ALLOC_ROBALLOC_H

#include "carolyne/arch/base/march/robUnit/robMetaBase.h"
#include "carolyne/model/base/decode/decodeSlotAnal.h"
#include "lib/hw/popper/popTunnel/popTunnelMng.h"

namespace kathryn::carolyne{

    struct RobAllocCtrl: PopTunnelable{
        std::string _prefixName = "uname";
        RobUTM_Base* _robMeta   = nullptr;
        int          _pcWidth   = -1;

        explicit RobAllocCtrl(std::string prefixName,
                              PopTunnelMng* tunnelMng,
                              RobUTM_Base*  robMeta,
                              int           pcWidth):
        PopTunnelable(tunnelMng),
        _prefixName(std::move(prefixName)),
        _robMeta(robMeta),
        _pcWidth(pcWidth){
            crlAss(robMeta != nullptr, "robAlloc Ctrl cannot be nullptr");
        }

        PopHST& createRobAllocTunnel(
        DecodeSlotAnalyzer& dec,
        std::vector<int> desOprIdxs);

        Slot   createRobAllocReqSlot(
        DecodeSlotAnalyzer& dec,
        std::vector<int> desOprIdxs,
        std::vector<Slot> renamedPhyRegSlots, ///// the renamed of slot from opr
        Slot pcSlot,
        const Slot& robCommitMode);

        [[nodiscard]] std::string getValidOprField (int idx);
        [[nodiscard]] std::string getArcRegField   (int idx);
        [[nodiscard]] std::string getPhyRegField   (int idx);

        RowMeta     getOprAllocMeta(int idx, OprTypeBase* oprType);
                                                               //////////    the renamed physical register slot that regmgmt
                                                               //////////    valid is determine that this opr is inused in this uop
        Slot        createOprAllocSlot(DecodeSlotAnalyzer& dec, int idx, const Slot& renamedphyReg);
    };

}

#endif //KATHRYN_CAROLYNE_MODEL_BASE_ALLOC_ROBALLOC_H
