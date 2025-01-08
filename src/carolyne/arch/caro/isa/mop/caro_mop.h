//
// Created by tanawin on 6/1/2025.
//

#ifndef KATHRYN_SRC_CAROLYNE_ARCH_CARO_ISA_MOP_CARO_MOP_H
#define KATHRYN_SRC_CAROLYNE_ARCH_CARO_ISA_MOP_CARO_MOP_H

#include "carolyne/arch/base/isa/mop/mop_base.h"
#include "carolyne/arch/caro/isa/uop/caro_uop.h"
#include "caro_mop_param.h"

namespace kathryn::carolyne::caro{

    struct A_MOP: MopTypeBase{
        explicit A_MOP(A_UOP* a_uop):
            MopTypeBase(A_MOP_NAME, MOP_WIDTH){
            addUop(a_uop);
        }

        bool isEqualTypeDeep(const MopTypeBase& rhs) override{return true;}

        void generateMatchers() override{
            addMatchedSlice(FUNC7_POS);

            /** build uop matcher*/
            auto uopMatcher = new UopMatcherBase(getMopBitWidth(), _uopTypes[0]);
            /** build src opr matcher**/
            std::vector<OprTypeBase*> srcOprs = _uopTypes[0]->getOprTypes(true);
            std::vector<OprTypeBase*> desOprs = _uopTypes[0]->getOprTypes(false);
            crlAss(srcOprs.size() == 2, "a_mop opr must equal to 2");
            crlAss(srcOprs.size() == 1, "a_mop des opr must equal to 1");

            auto r1Matcher  = new OprMatcherBase(getMopBitWidth(), srcOprs[0], R1_POS);
            auto r2Matcher  = new OprMatcherBase(getMopBitWidth(), srcOprs[0], R2_POS);
            auto rdMatcher  = new OprMatcherBase(getMopBitWidth(), desOprs[0], R1_POS);
            /** put opr matcher to uop matcher */
            uopMatcher->setOprMatcher({r1Matcher, r2Matcher}, true);
            uopMatcher->setOprMatcher({rdMatcher}           , false);
            /** put uop matcher to mop matcher*/
            addUopMatcher(uopMatcher);
        }
    };

    struct L_MOP: MopTypeBase{
        explicit L_MOP(L_UOP* l_uop):
            MopTypeBase(L_MOP_NAME, MOP_WIDTH){
            addUop(l_uop);
        }
        bool isEqualTypeDeep(const MopTypeBase& rhs) override{return true;}

        void generateMatchers() override{
            addMatchedSlice(FUNC7_POS);
            /** build uop matcher*/
            auto uopMatcher = new UopMatcherBase(getMopBitWidth(), _uopTypes[0]);
            /** build src opr matcher**/
            std::vector<OprTypeBase*> srcOprs = _uopTypes[0]->getOprTypes(true);
            std::vector<OprTypeBase*> desOprs = _uopTypes[0]->getOprTypes(false);
            crlAss(srcOprs.size() == 1, "a_mop opr must equal to 1");
            crlAss(srcOprs.size() == 1, "a_mop des opr must equal to 1");

            auto r1Matcher  = new OprMatcherBase(getMopBitWidth(), srcOprs[0], R1_POS);
            auto rdMatcher  = new OprMatcherBase(getMopBitWidth(), desOprs[0], R1_POS);
            /** put opr matcher to uop matcher */
            uopMatcher->setOprMatcher({r1Matcher}, true);
            uopMatcher->setOprMatcher({rdMatcher}, false);
            /** put uop matcher to mop matcher*/
            addUopMatcher(uopMatcher);
        }
    };

    struct S_MOP: MopTypeBase{
        explicit S_MOP(S_UOP* s_uop):
            MopTypeBase(S_MOP_NAME, MOP_WIDTH){
            addUop(s_uop);
        }
        bool isEqualTypeDeep(const MopTypeBase& rhs) override{return true;}

        void generateMatchers() override{
            addMatchedSlice(FUNC7_POS);

            /** build uop matcher*/
            auto uopMatcher = new UopMatcherBase(getMopBitWidth(), _uopTypes[0]);
            /** build src opr matcher**/
            std::vector<OprTypeBase*> srcOprs = _uopTypes[0]->getOprTypes(true);
            crlAss(srcOprs.size() == 2, "a_mop opr must equal to 2");
            auto r1Matcher  = new OprMatcherBase(getMopBitWidth(), srcOprs[0], R1_POS_SOP);
            auto r2Matcher  = new OprMatcherBase(getMopBitWidth(), srcOprs[0], R2_POS_SOP);
            /** put opr matcher to uop matcher */
            uopMatcher->setOprMatcher({r1Matcher, r2Matcher}, true);
            /** put uop matcher to mop matcher*/
            addUopMatcher(uopMatcher);
        }
    };

    struct I_MOP: MopTypeBase{
        explicit I_MOP(I_UOP* i_uop):
            MopTypeBase(I_MOP_NAME, MOP_WIDTH){
            addUop(i_uop);
        }
        bool isEqualTypeDeep(const MopTypeBase& rhs) override{return true;}

        void generateMatchers() override{
            addMatchedSlice(FUNC7_POS);

            /** build uop matcher*/
            auto uopMatcher = new UopMatcherBase(getMopBitWidth(), _uopTypes[0]);
            /** build src opr matcher**/
            std::vector<OprTypeBase*> srcOprs = _uopTypes[0]->getOprTypes(true);
            std::vector<OprTypeBase*> desOprs = _uopTypes[0]->getOprTypes(false);
            crlAss(srcOprs.size() == 1, "a_mop opr must equal to 2");
            crlAss(srcOprs.size() == 1, "a_mop des opr must equal to 1");

            auto rImmMatcher  = new OprMatcherBase(getMopBitWidth(), srcOprs[0], RI_POS);
            auto rdMatcher   = new OprMatcherBase(getMopBitWidth(), desOprs[0], R1_POS);
            /** put opr matcher to uop matcher */
            uopMatcher->setOprMatcher({rImmMatcher}, true);
            uopMatcher->setOprMatcher({rdMatcher}  , false);
            /** put uop matcher to mop matcher*/
            addUopMatcher(uopMatcher);
        }
    };

    struct B_MOP: MopTypeBase{
        explicit B_MOP(A_UOP* a_uop):
        MopTypeBase(B_MOP_NAME, MOP_WIDTH){
            addUop(a_uop);
        }
        bool isEqualTypeDeep(const MopTypeBase& rhs) override{return true;}

        void generateMatchers() override{
            addMatchedSlice(FUNC7_POS);

            /** build uop matcher*/
            auto uopMatcher = new UopMatcherBase(getMopBitWidth(), _uopTypes[0]);
            /** build src opr matcher**/
            std::vector<OprTypeBase*> srcOprs = _uopTypes[0]->getOprTypes(true);
            crlAss(srcOprs.size() == 2, "a_mop opr must equal to 2");
            auto r1Matcher  = new OprMatcherBase(getMopBitWidth(), srcOprs[0], R1_POS_SOP);
            auto r2Matcher  = new OprMatcherBase(getMopBitWidth(), srcOprs[0], R2_POS_SOP);
            /** put opr matcher to uop matcher */
            uopMatcher->setOprMatcher({r1Matcher, r2Matcher}, true);
            /** put uop matcher to mop matcher*/
            addUopMatcher(uopMatcher);
        }
    };

}

#endif //KATHRYN_SRC_CAROLYNE_ARCH_CARO_ISA_MOP_CARO_MOP_H
