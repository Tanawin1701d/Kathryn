//
// Created by tanawin on 31/12/2024.
//

#ifndef src_carolyne_arch_base_march_robUnit_ROBMETABASE_H
#define src_carolyne_arch_base_march_robUnit_ROBMETABASE_H

#include "carolyne/arch/base/march/alloc/allocInfo.h"
#include "util/numberic/pmath.h"
#include "carolyne/arch/base/util/genRowMeta.h"
#include "carolyne/arch/base/util/regFileType.h"
#include "carolyne/arch/base/isa/regFile/archRegFile.h"
#include "carolyne/arch/base/march/prfUnit/prfMetaBase.h"

namespace kathryn::carolyne{

        constexpr char ROB_META_FD_archRegIdx   [] = "archRegIdx";
        constexpr char ROB_META_FD_phyRegIdx    [] = "phyRegIdx";
        constexpr char ROB_META_FD_updateArc    [] = "updateArch";
        constexpr char ROB_META_FD_executed     [] = "executed";
        constexpr char ROB_META_FD_transferType [] = "transferType";
        constexpr char ROB_META_FD_fetchPc      [] = "fetchPc";
        constexpr char ROB_META_FD_valid        [] = "valid";


        struct RobUTM_Base{

            std::vector<ALLOC_INFO> _transferFields;

            int _pcWidth          = -1;


            explicit RobUTM_Base(int pcWidth, int amtTr = 4):
            _pcWidth(pcWidth){}

            ~RobUTM_Base() = default;

            void addAllocType(ALLOC_INFO matcher){
                ALLOC_INFO robFieldMatch = matcher;
                //////// we have redundant copy because we migh modify it

                _transferFields.push_back(robFieldMatch);
            }

            int getIndexWidth() const;
            int getRobCommitModeWidth() const;
        };

    }


#endif //src_carolyne_arch_base_march_robUnit_ROBMETABASE_H
