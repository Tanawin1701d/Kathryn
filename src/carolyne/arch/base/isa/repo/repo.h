//
// Created by tanawin on 30/12/2024.
//

#ifndef src_carolyne_arch_base_isa_repo_REPO_H
#define src_carolyne_arch_base_isa_repo_REPO_H


#include<vector>

#include "carolyne/arch/base/isa/mop/mop_base.h"
#include "carolyne/arch/base/isa/regFile/archRegFile.h"


namespace kathryn{
    namespace carolyne{


        /***
         * This class is the main storage for isa that designer input the instruction
         */

        class IsaRepo{
            ///// INSTRUCTION SECTION
            /// a single instuction (macro-op) can be decoded and it can produce
            /// multiple micro-operation (uop)
            /// each uop require one or more operand to do an operation
            std::vector<MopTypeBase*   > _mopTypes; //// macro operation
            std::vector<UopMatcherBase*> _uopTypes; //// micro operation
            std::vector<OprTypeBase*   > _oprTypes; //// operand meta data
            ///// REGISTER SECTION
            /// archRegfile stands for architecture register file
            std::vector<ArchRegFileBase*> _archRegfiles;
            //// DECODER SECTION
            /// it contain how each instruction should be decoded
            std::vector<MopMatcherBase*> _mopMatchers;

        public:
            [[nodiscard]] std::vector<MopTypeBase*>     mops()         const {return _mopTypes;       }
            [[nodiscard]] std::vector<UopMatcherBase*>  uops()         const {return _uopTypes;       }
            [[nodiscard]] std::vector<OprTypeBase*>     oprs()         const {return _oprTypes;       }
            [[nodiscard]] std::vector<ArchRegFileBase*> arch_regfiles() const {return _archRegfiles;}
            [[nodiscard]] std::vector<MopMatcherBase*>  mop_matchers() const {return _mopMatchers;}

            void addMop         (MopTypeBase*     mop )          {_mopTypes    .push_back(mop);         }
            void addUop         (UopMatcherBase*  uop )          {_uopTypes    .push_back(uop);         }
            void addOpr         (OprTypeBase*     opr )          {_oprTypes    .push_back(opr);         }
            void addArchRegFile (ArchRegFileBase* arch_regfile)  {_archRegfiles.push_back(arch_regfile);}
            void addMopMatcher  (MopMatcherBase*   mop_matcher ) {_mopMatchers .push_back(mop_matcher); }
        };


    }
}

#endif //src_carolyne_arch_base_isa_repo_REPO_H
