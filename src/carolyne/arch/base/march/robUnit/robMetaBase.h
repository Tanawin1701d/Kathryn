//
// Created by tanawin on 31/12/2024.
//

#ifndef src_carolyne_arch_base_march_robUnit_ROBMETABASE_H
#define src_carolyne_arch_base_march_robUnit_ROBMETABASE_H

#include "carolyne/arch/base/march/alloc/archPhyRegMatcher.h"
#include "util/numberic/pmath.h"
#include "carolyne/arch/base/util/genRowMeta.h"
#include "carolyne/arch/base/util/regFileType.h"
#include "carolyne/arch/base/isa/regFile/archRegFile.h"
#include "carolyne/arch/base/march/pRegFile/prfMetaBase.h"

namespace kathryn::carolyne{

        constexpr char ROB_META_FD_archRegIdx   [] = "archRegIdx";
        constexpr char ROB_META_FD_phyRegIdx    [] = "phyRegIdx";
        constexpr char ROB_META_FD_updateArc    [] = "updateArch";
        constexpr char ROB_META_FD_executed     [] = "executed";
        constexpr char ROB_META_FD_transferType [] = "transferType";
        constexpr char ROB_META_FD_fetchPc      [] = "fetchPc";
        constexpr char ROB_META_FD_valid        [] = "valid";


        struct RobUTM_Base: GenRowMetaable, VizCsvGenable{

            std::vector<APRegRobFieldMatch> _transferMeta;
            ArchRegFileBase*                _archRegFileBase = nullptr;
            PhysicalRegFileBase*            _phyRegFileBase  = nullptr;

            int _pc_width                = -1;
            int _amt_transferType        =  4;

            /// transferType //////////////////////////////////////////
            /// [0 update phyReg to archReg] link only
            /// [1 reqLoad from address(_idxInPhyTrans_MemAddr) to phyReg]
            /// [2 reqStore to address(_idxInPhyTrans_MemAddr) with value(_idxInPhyTrans_MemValue) ]
            /// [3 req branch update phy to pc]
            ///////////////////////////////////////////////////////////

            ///// archreg that mapped from archRegTransfer to phyRegTransfer index by index


            explicit RobUTM_Base(ArchRegFileBase*     archRegFileBase,
                                 PhysicalRegFileBase* phyRegFileBase):
            _archRegFileBase(archRegFileBase),
            _phyRegFileBase (phyRegFileBase){
                crlAss(_archRegFileBase != nullptr, "archRegister cannot be nullptr");
                crlAss(_phyRegFileBase != nullptr, "phyRegister cannot nullptr");
            }

            ~RobUTM_Base() override = default;

            int addTransferType(APRegRobFieldMatch matcher){
                APRegRobFieldMatch robFieldMatch = matcher;
                robFieldMatch.idxInRob = _transferMeta.size();
                _transferMeta.push_back(robFieldMatch);
                ////// return by value
                return robFieldMatch.idxInRob;
            }

            std::string getFieldName(int idx, const std::string& fieldName, bool isArch){
                ////// build the name for eachfield
                std::string result;
                result  = fieldName + "_" + std::to_string(idx) + "_";
                result += isArch ? _transferMeta[idx]._archRegGrpName: _transferMeta[idx]._phyRegGrpName;
                return result;
            }

            RowMeta genRowMeta(CRL_GEN_MODE genMode, int subMode) override{

                crlAss(genMode == CGM_ROB, "cannot gen row meta for this genMode");

                /////////////////////////////////////////////////////////////////////////////////////////////////////////
                /// |valid|Pc|transferType|*[updateArchReg| executed| archIdx| phyIdx]|
                /////////////////////////////////////////////////////////////////////////////////////////////////////////
                RowMeta row;

                ///row.addField(ROB_META_FD_valid, 1); model must be handle these thing
                row.addField(ROB_META_FD_fetchPc, _pc_width);
                row.addField(ROB_META_FD_transferType, log2Ceil(_amt_transferType));

                for (int idx = 0; idx < _transferMeta.size(); idx++){
                    RegTypeMeta archRegTypeMeta = _archRegFileBase->getRegTypeMetaGroup(_transferMeta[idx]._archRegGrpName);
                    RegTypeMeta phyRegTypeMeta  = _archRegFileBase->getRegTypeMetaGroup(_transferMeta[idx]._phyRegGrpName);
                    row.addField(getFieldName(idx, ROB_META_FD_updateArc , true), 1);
                    row.addField(getFieldName(idx, ROB_META_FD_executed  , true), 1);
                    row.addField(getFieldName(idx, ROB_META_FD_archRegIdx, true), archRegTypeMeta.getIndexWidth());
                    row.addField(getFieldName(idx, ROB_META_FD_phyRegIdx , false), phyRegTypeMeta.getIndexWidth());
                }

                row.reverse(); //// because we want the idx like the comment above
                return row;

            }

            RowMeta genRowMeta(const std::string& genMode) override{
                crlAss(false, "cannot gen row meta for string of genrow");
                return {};
            }

            void visual(CsvGenFile& genFile) override{
                RowMeta row    = genRowMeta(CGM_ROB, 0);
                CsvTable table = row.genTable();
                table.setTableName("reorderBufferMeta");
                genFile.addData(table);

            }

        };

    }


#endif //src_carolyne_arch_base_march_robUnit_ROBMETABASE_H
