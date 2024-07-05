//
// Created by tanawin on 20/6/2024.
//

#include "memGen.h"
#include "model/hwComponent/memBlock/MemBlock.h"

namespace kathryn{

    MemGen::MemGen(ModuleGen* mdGenMaster,
                        MemBlock* memBlockMaster):
    LogicGenBase(
        mdGenMaster,
        nullptr,
        (Identifiable*) memBlockMaster
    ),
    _master(memBlockMaster){}


    void MemGen::genCerf(MODULE_GEN_GRP mgg, int grpIdx, int idx){
        assert(grpIdx >= 0);
        assert(idx    >= 0);
        assert(_asb   == nullptr);
        assert(_ident != nullptr);
        _cerf.comptype   = _ident->getType();
        _cerf.md_gen_grp = mgg;
        _cerf.varMeta    = _ident->getVarMeta();
        _cerf.grpIdx     = grpIdx;
        _cerf.idx        = idx;
        _cerf.curSl      = {0, _master->getWidthSize()};
    }

    std::string MemGen::decVariable(){
        int memWidth = _master->getWidthSize();
        int memDepth = _master->getDepthSize();
        return "reg [" + std::to_string(memWidth-1) +
               ": 0] " + LogicGenBase::getOpr() + " [" +
               std::to_string(memDepth-1) + ": 0];";
    }


    bool MemGen::compare(LogicGenBase* lgb){
        assert(lgb->getLogicCef().comptype == TYPE_MEM_BLOCK);

        auto* rhs = dynamic_cast<MemGen*>(lgb);
        ////// check depth only; do not check width because cerf handle it
        return checkCerfEqLocally(*rhs) &&
            (_master->getDepthSize() == rhs->_master->getDepthSize());
    }

    std::string MemGen::getOpr(Slice sl){
        assert( (sl.start == 0) &&
                (sl.getSize() == _master->getDepthSize()));

        return LogicGenBase::getOpr();
    }



}