//
// Created by tanawin on 31/12/2024.
//

#ifndef src_carolyne_arch_base_march_alloc_ALLOCUNITMETABASE_H
#define src_carolyne_arch_base_march_alloc_ALLOCUNITMETABASE_H

#include "carolyne/arch/base/isa/regFile/archRegFile.h"
#include "carolyne/arch/base/march/robUnit/robMetaBase.h"
#include "carolyne/arch/base/march/rsvUnit/rsvMetaBase.h"
#include "carolyne/arch/base/isa/uop/uop_base.h"

namespace kathryn::carolyne{




        ///// UTM  = unit type meta
        struct AllocUTM_Base: VizCsvGenable{

            ///////// new depend On Uop
            std::vector<UopTypeBase*> spUopTypes; ////// supported uop type

            //////// task of allocation unit
            ///////// -> the register file allocation
            ///////// -> rob allocation
            ///////// -> rsv allocation
            ///////// all task depending on uop specification


            void addSpUopType(UopTypeBase* uopType){
                crlAss(uopType != nullptr, "cannot add support uop in allc UTM");
                spUopTypes.push_back(uopType);
            }

            void addSpUopTypes(const std::vector<UopTypeBase*>& uopTypes){
                for (auto* uopType: uopTypes){
                    addSpUopType(uopType);
                }
            }

            [[nodiscard]]
            std::vector<UopTypeBase*> getSpUopTypes() const{return spUopTypes;}

            void visual(CsvGenFile& genFile) override{

                ////// for now nothing to do with this

//                constexpr char RN_ARCH[] = "archRegFileName";
//                constexpr char RN_PHY[]  = "phyRegFileName";
//                crlAss(!_regMatchers.empty(), "in visualization regMatcher cannot empty()");
//                CsvTable table(static_cast<int>(_regMatchers.size()), 2);
//                table.setsHeadNameIterative(true, "archId");
//                table.setHeadNames(false, {RN_ARCH, RN_PHY});
//                int idx = 0;
//                for (auto& matcher: _regMatchers){
//                    table.setData(idx, 0, matcher._archRegGrpName);
//                    table.setData(idx, 1, matcher._phyRegGrpName);
//                    idx++;
//                }
//                table.setTableName("AllocUnitMeta");
//                genFile.addData(table);

            }

        };



    }


#endif //src_carolyne_arch_base_march_alloc_ALLOCUNITMETABASE_H
