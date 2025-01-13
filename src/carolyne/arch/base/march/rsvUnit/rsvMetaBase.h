//
// Created by tanawin on 2/1/2025.
//

#ifndef src_carolyne_arch_base_march_rsvUnit_RSVMETABASE_H
#define src_carolyne_arch_base_march_rsvUnit_RSVMETABASE_H

#include "carolyne/arch/base/march/execUnit/execMetaBase.h"

namespace kathryn::carolyne{

    struct RsvUTM_Base: VizCsvGenable{

        std::vector<ExecRsvUnitMatch> execUnitMatchs;

        void addExecUnit(ExecUTM* execUnitType, int amt){
            crlAss(execUnitType != nullptr, "add add execunitType as nullptr too execUnits");
            execUnitMatchs.emplace_back(execUnitType, amt);
        }

        [[nodiscard]]
        std::vector<ExecRsvUnitMatch> getExecUnitMeta()const{return execUnitMatchs;}

        void visual(CsvGenFile& genFile) override{
            constexpr char RN_EXECTYPE[] = "ExecTypeName";
            constexpr char RN_AMT     [] = "amt";
            crlAss(!execUnitMatchs.empty(), "in rsv rsv visual should not have empty execunit");

            CsvTable execTable(static_cast<int>(execUnitMatchs.size()), 2);
            execTable.setsHeadNameIterative(true, "execId");
            execTable.setHeadNames(false, {RN_EXECTYPE, RN_AMT});
            int idx = 0;
            for(ExecRsvUnitMatch execUnit: execUnitMatchs){
                execTable.setData(idx, 0, execUnit._execType->_execType);
                execTable.setData(idx, 1, std::to_string(execUnit._amt));
                idx++;
            }
            execTable.setTableName("robUnit exec MetaData");
            genFile.addData(execTable);

        }


    };

}

#endif //src_carolyne_arch_base_march_rsvUnit_RSVMETABASE_H
