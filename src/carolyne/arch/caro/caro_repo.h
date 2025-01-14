//
// Created by tanawin on 12/1/2025.
//

#ifndef src_carolyne_arch_caro_CARO_REPO_H
#define src_carolyne_arch_caro_CARO_REPO_H
#include "carolyne/arch/base/isa/repo/repo.h"
#include "carolyne/arch/base/isa/uop/opr_li.h"
#include "carolyne/arch/base/isa/uop/opr_lrf.h"
#include "carolyne/arch/base/isa/uop/opr_srf.h"
#include "carolyne/arch/base/march/repo/repo.h"
#include "isa/mop/caro_mop.h"
#include "isa/param/param.h"
#include "isa/regFile/caro_archRegFile.h"
#include "isa/uop/caro_uop.h"
#include "march/fetchUnit/caro_fetchMeta.h"
#include "march/param/param.h"
#include "march/pRegFile/caro_preg.h"
#include "march/alloc/caro_allocMeta.h"
#include "march/execUnit/caro_execMeta.h"
#include "march/robUnit/caro_robMeta.h"
#include "march/rsvUnit/caro_rsvMeta.h"



namespace kathryn::carolyne::caro{

    class CaroRepo: public IsaBaseRepo,
                    public MarchBaseRepo{
    public:

        explicit CaroRepo(){
            auto* caroArchRegfile = new ArchRegFile();
            auto* caroPhyRegFile  = new PRegFile();
            setArchRegFiles(caroArchRegfile);
            setPhyFileBase(caroPhyRegFile);
            APRegTypeMatch aprMatchAll = {ARCH_REGFILE_NAME, PHY_REGFILE_NAME};
            ////reqRobField is redundant with store operand sometime
            APRegRobFieldMatch aprobMatch_NO_ROB_REQ              {ARCH_REGFILE_NAME, PHY_REGFILE_NAME, -1, false, false};
            APRegRobFieldMatch aprobMatch_ROB_STORE_MEM_OR_PREG   {ARCH_REGFILE_NAME, PHY_REGFILE_NAME, -1, true , false};
            APRegRobFieldMatch aprobMatch_ROB_STORE_ADDR          {ARCH_REGFILE_NAME, PHY_REGFILE_NAME, -1, true , true };
            /**
             * ISA
             */
            /*
             * build opr Type
             * */

            auto* r1  = new OprTypeLoadRegFile (aprobMatch_NO_ROB_REQ           , _archRegfiles, _phyFileBase);
            auto* r2  = new OprTypeLoadRegFile (aprobMatch_NO_ROB_REQ           , _archRegfiles, _phyFileBase);
            auto* rd  = new OprTypeStoreRegFile(aprobMatch_ROB_STORE_MEM_OR_PREG, _archRegfiles, _phyFileBase);
            auto* ri  = new OprTypeLoadImm     (aprobMatch_NO_ROB_REQ           , _archRegfiles, _phyFileBase);

            auto* rta = new OprTypeStoreRegFile(aprobMatch_ROB_STORE_ADDR       , _archRegfiles, _phyFileBase);

            addOprTypes({r1,r2,rd,ri, rta});
            /*
             * build uop
             * */
            auto* a_uop = new A_UOP(r1, r2, rd);
            auto* l_uop = new L_UOP(r1, rta, rd);
            auto* s_uop = new S_UOP(r1, r2, rta, rd);
            auto* i_uop = new I_UOP(ri, rd);
            addUopTypes({a_uop,l_uop,s_uop,i_uop});
            /**
             * build mop
             * */
            auto* a_mop = new A_MOP(a_uop);
            auto* l_mop = new L_MOP(l_uop);
            auto* s_mop = new S_MOP(s_uop);
            auto* i_mop = new I_MOP(i_uop);
            auto* b_mop = new B_MOP(a_uop);
            addMopTypes({a_mop,l_mop,s_mop,i_mop,b_mop});

            /**
             * MARCH
             */
           /**
            * fetch Unit
            */
            auto* fetchUnitType = new FetchUTM();
            addFetchType(fetchUnitType);
            /**
             * alloc unit
             */
            auto* allocUnitType = new AllocUTM();
            allocUnitType->addSpUopTypes({a_uop,l_uop,s_uop,i_uop});
            addAllocType(allocUnitType);
            /**
             * exec unit
             *
             */
            auto* arithExecType  = new ArithExecUTM();
            auto* lsExecUnitType = new LSExecUTM();
            auto* bExecUnitType  = new BExecUTM();
            addExecTypes({arithExecType,lsExecUnitType,bExecUnitType});


            /**
             * rsvUnit
             */
            auto* rsvUnitType   = new RsvUTM();
            rsvUnitType->addExecUnit(arithExecType, AMT_EXEC_PER_RSV_PER_TYPE);
            rsvUnitType->addExecUnit(lsExecUnitType, AMT_EXEC_PER_RSV_PER_TYPE);
            rsvUnitType->addExecUnit(bExecUnitType, AMT_EXEC_PER_RSV_PER_TYPE);
            addRsvType(rsvUnitType);

            /**
             * robUnit
             */
            auto* robUnitType   = new RobUTM(caroArchRegfile, caroPhyRegFile);
            robUnitType->addTransferType(aprobMatch_ROB_STORE_ADDR);
            robUnitType->addTransferType(aprobMatch_ROB_STORE_MEM_OR_PREG);

            addRobType(robUnitType);
        }


    };

}

#endif //src_carolyne_arch_caro_CARO_REPO_H
