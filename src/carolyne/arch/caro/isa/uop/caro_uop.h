//
// Created by tanawin on 6/1/2025.
//

#ifndef KATHRYN_SRC_CAROLYNE_ARCH_CARO_ISA_UOP_CARO_UOP_H
#define KATHRYN_SRC_CAROLYNE_ARCH_CARO_ISA_UOP_CARO_UOP_H

#include "carolyne/arch/base/isa/uop/uop_base.h"
#include "carolyne/arch/base/isa/uop/opr_lrf.h"
#include "carolyne/arch/base/isa/uop/opr_srf.h"
#include "carolyne/arch/base/isa/uop/opr_li.h"
#include "carolyne/arch/caro/isa/param/param.h"

namespace kathryn::carolyne::caro{

    struct A_UOP: UopTypeBase{
        explicit A_UOP(OprTypeLoadRegFile*  r1,
                       OprTypeLoadRegFile*  r2,
                       OprTypeStoreRegFile* rd){
            _uopName = AOP_NAME;
            setFopIdentWidth(XOP_IDENT_WIDTH);
            addOprType(r1, true);
            addOprType(r2, true);
            addOprType(rd, false);
        }
        bool isEqualTypeDeep(const UopTypeBase& rhs) override{
            return true;
        }

    };

    struct L_UOP: UopTypeBase{
        explicit L_UOP(OprTypeLoadRegFile*  r1,
                       OprTypeStoreRegFile* rta,
                       OprTypeStoreRegFile* rd){
            _uopName = LOP_NAME;
            setFopIdentWidth(XOP_IDENT_WIDTH);
            addOprType(r1, true);

            addOprType(rta, false);
            addOprType(rd , false);
        }
        bool isEqualTypeDeep(const UopTypeBase& rhs) override{
            return true;
        }
    };

    struct S_UOP: UopTypeBase{
        explicit S_UOP(OprTypeLoadRegFile*  r1,
                       OprTypeLoadRegFile*  r2,
                       OprTypeStoreRegFile* rta,
                       OprTypeStoreRegFile* rd  ){
            _uopName = SOP_NAME;
            setFopIdentWidth(XOP_IDENT_WIDTH);
            addOprType(r1, true);
            addOprType(r2, true);

            addOprType(rta, false);
            addOprType(rd , false);
        }
        bool isEqualTypeDeep(const UopTypeBase& rhs) override{
            return true;
        }
    };

    struct I_UOP: UopTypeBase{
        explicit I_UOP(OprTypeLoadImm* ri,
                       OprTypeStoreRegFile* rd){
            _uopName = IOP_NAME;
            setFopIdentWidth(XOP_IDENT_WIDTH);
            addOprType(ri, true);
            addOprType(rd, false);
        }
        bool isEqualTypeDeep(const UopTypeBase& rhs) override{
            return true;
        }
    };

}

#endif //KATHRYN_SRC_CAROLYNE_ARCH_CARO_ISA_UOP_CARO_UOP_H
