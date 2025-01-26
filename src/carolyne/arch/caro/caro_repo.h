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
#include "isa/uop/caro_uop.h"
#include "march/fetchUnit/caro_fetchMeta.h"
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
            auto* caroArchRegFile = new ArchRegFileUTM();
            auto* caroPhyRegFile  = new PRegFile();
            caroArchRegFile->linkPhyRegFileUTM(caroPhyRegFile);
            caroPhyRegFile->setLinkArchRegFile(caroArchRegFile);
            /**link Arch Phy regfile*/
            addArchRegFiles(caroArchRegFile);
            addPhyFileBase(caroPhyRegFile);
            ////reqRobField is redundant with store operand sometime
            /// ROB_PRF_INFERENCE call rpi   ////// if it is required, physical register and arf will be involve with this system
            /// UPA   stand for please update arch register file
            ALLOC_INFO srcArch_AllocInfo {caroArchRegFile, caroPhyRegFile, ROB_OPT::ROB_NO_REQ, REG_OPT::REG_REQ_ARCH_READ};
            ALLOC_INFO srcImm_AllocInfo  {nullptr, caroPhyRegFile, ROB_OPT::ROB_NO_REQ, REG_OPT::REG_NO_REQ};
            ALLOC_INFO desU2A_AllocInfo  {caroArchRegFile, caroPhyRegFile, ROB_OPT::ROB_REQ_PHY_ID | ROB_OPT::ROB_REQ_ARCH_ID | ROB_OPT::ROB_REQ_UPT_ARCH,
                                                                           REG_OPT::REG_REQ_PHY_ALLOC | REG_OPT::REG_REQ_ARCH_RENAME}; ///// update phyReg to archReg
            ALLOC_INFO desTemp_AllocInfo {nullptr, caroPhyRegFile, ROB_OPT::ROB_REQ_PHY_ID, REG_OPT::REG_REQ_PHY_ALLOC};
            /**
             * ISA
             */
            /*
             * build opr Type
             * */

            auto* r1  = new OprTypeLoadRegFile  (srcArch_AllocInfo);
            auto* r2  = new OprTypeLoadRegFile  (srcArch_AllocInfo);
            auto* rd  = new OprTypeStoreRegFile (desU2A_AllocInfo);
            auto* ri  = new OprTypeLoadImm      (srcImm_AllocInfo, RI_POS.getSize());

            ////// for storer (a) address / (d) data
            auto* rta = new OprTypeStoreRegFile(desTemp_AllocInfo);
            auto* rtd = new OprTypeStoreRegFile(desTemp_AllocInfo);

            addOprTypes({r1,r2,rd,ri, rta});
            /*
             * build uop
             * */
            auto* a_uop = new A_UOP(r1, r2, rd);
            auto* l_uop = new L_UOP(r1, rta, rd);
            auto* s_uop = new S_UOP(r1, r2, rta, rtd);
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
            addRsvType(rsvUnitType);

            /**
             * robUnit
             */
            auto* robUnitType   = new RobUTM();
            robUnitType->addAllocType(desU2A_AllocInfo);
            robUnitType->addAllocType(desTemp_AllocInfo);
            robUnitType->addAllocType(desTemp_AllocInfo);
            addRobType(robUnitType);
        }


    };

}

#endif //src_carolyne_arch_caro_CARO_REPO_H
