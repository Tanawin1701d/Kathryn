//
// Created by tanawin on 6/1/2025.
//

#ifndef KATHRYN_SRC_CAROLYNE_ARCH_CARO_ISA_REPO_CARO_REPO_H
#define KATHRYN_SRC_CAROLYNE_ARCH_CARO_ISA_REPO_CARO_REPO_H

#include "carolyne/arch/base/isa/repo/repo.h"
#include "carolyne/arch/base/isa/uop/opr_lrf.h"
#include "carolyne/arch/caro/isa/regFile/caro_archRegFile.h"
#include "carolyne/arch/base/isa/uop/opr_srf.h"
#include "carolyne/arch/base/isa/uop/opr_li.h"
#include "carolyne/arch/caro/isa/uop/caro_uop.h"
#include "carolyne/arch/caro/isa/mop/caro_mop.h"

namespace kathryn::carolyne::caro{

    class Caro_IsaRepo: public IsaRepo{

        explicit Caro_IsaRepo(
                const RegTypeMeta& phyRegType,
                const std::string& phyRegGrpName){

            /////// build arch RegFile
            auto* caroArchRegfile = new ArchRegFileBase();
            setArchRegFiles(caroArchRegfile);
            /*
             * build opr Type
             * */
            auto* r1 = new OprTypeLoadRegFile(caroArchRegfile->getRegTypeMetaGroup(ARCH_REGFILE_NAME),
                                              ARCH_REGFILE_NAME,
                                              phyRegType,
                                              phyRegGrpName
                                              );

            auto* r2 = new OprTypeLoadRegFile(caroArchRegfile->getRegTypeMetaGroup(ARCH_REGFILE_NAME),
                                              ARCH_REGFILE_NAME,
                                              phyRegType,
                                              phyRegGrpName
            );
            auto* rd = new OprTypeStoreRegFile(caroArchRegfile->getRegTypeMetaGroup(ARCH_REGFILE_NAME),
                                              ARCH_REGFILE_NAME,
                                              phyRegType,
                                              phyRegGrpName
            );
            auto* ri = new OprTypeLoadImm(caroArchRegfile->getRegTypeMetaGroup(ARCH_REGFILE_NAME));
            addOprType(r1);
            addOprType(r2);
            addOprType(rd);
            addOprType(ri);
            /*
             * build uop
             * */
            auto* a_uop = new A_UOP(r1, r2, rd);
            auto* l_uop = new L_UOP(r1, rd);
            auto* s_uop = new S_UOP(r1, r2);
            auto* i_uop = new I_UOP(ri, rd);
            addUopType(a_uop);
            addUopType(l_uop);
            addUopType(s_uop);
            addUopType(i_uop);
            /**
             * build mop
             * */
             auto* a_mop = new A_MOP(a_uop);
             auto* l_mop = new L_MOP(l_uop);
             auto* s_mop = new S_MOP(s_uop);
             auto* i_mop = new I_MOP(i_uop);
             auto* b_mop = new B_MOP(a_uop);

             addMopType(a_mop);
             addMopType(l_mop);
             addMopType(s_mop);
             addMopType(i_mop);
             addMopType(b_mop);



        }

    };

}

#endif //KATHRYN_SRC_CAROLYNE_ARCH_CARO_ISA_REPO_CARO_REPO_H
