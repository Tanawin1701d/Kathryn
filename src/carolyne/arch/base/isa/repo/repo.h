//
// Created by tanawin on 30/12/2024.
//

#ifndef src_carolyne_arch_base_isa_repo_REPO_H
#define src_carolyne_arch_base_isa_repo_REPO_H


#include<vector>

#include "carolyne/arch/base/isa/mop/mop_base.h"
#include "carolyne/arch/base/isa/regFile/archRegFile.h"



    namespace kathryn::carolyne{


        /***
         * This class is the main storage for isa that designer input the instruction
         */

        class IsaBaseRepo: VizCsvGenable{
        protected:
            ///// INSTRUCTION SECTION
            /// a single instuction (macro-op) can be decoded and it can produce
            /// multiple micro-operation (uop)
            /// each uop require one or more operand to do an operation
            std::vector<MopTypeBase*> _mopTypes; //// macro operation
            std::vector<UopTypeBase*> _uopTypes; //// micro operation
            std::vector<OprTypeBase*> _oprTypes; //// operand meta data

            std::vector<ArchRegFileUTM_Base*> _archRegTypes;
            ///// REGISTER SECTION
            /// archRegfile stands for architecture register file

        public:
            [[nodiscard]] std::vector<MopTypeBase*>     getMopTypes()     const {return _mopTypes;    }
            [[nodiscard]] std::vector<UopTypeBase*>     getUopTypes()     const {return _uopTypes;    }
            [[nodiscard]] std::vector<OprTypeBase*>     getOprTypes()     const {return _oprTypes;    }
            [[nodiscard]] std::vector<ArchRegFileUTM_Base*>  getArchRegFiles() const {return _archRegTypes;}

            void addMopType     (MopTypeBase*     mopType    ) {_mopTypes    .push_back(mopType);    }
            void addUopType     (UopTypeBase*     uopType    ) {_uopTypes    .push_back(uopType);    }
            void addOprType     (OprTypeBase*     oprType    ) {_oprTypes    .push_back(oprType);    }
            void addArchRegFiles(ArchRegFileUTM_Base*  archRegfile) {_archRegTypes.push_back(archRegfile);}

            void addMopTypes         (const std::vector<MopTypeBase*>& mopTypes) {
                for (auto mopType: mopTypes){
                    _mopTypes.push_back(mopType);
                }
            }
            void addUopTypes         (const std::vector<UopTypeBase*>& uopTypes) {
                for (auto uopType: uopTypes){
                    _uopTypes.push_back(uopType);
                }
            }
            void addOprTypes         (const std::vector<OprTypeBase*>& oprTypes) {
                for (auto oprType: oprTypes){
                    _oprTypes.push_back(oprType);
                }
            }

            void visual(CsvGenFile& genFile) override{
                /**  */
            }

        };


    }


#endif //src_carolyne_arch_base_isa_repo_REPO_H
