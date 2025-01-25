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
#include "carolyne/arch/base/march/prfUnit/prfMetaBase.h"

namespace kathryn::carolyne{

        constexpr char ROB_META_FD_archRegIdx   [] = "archRegIdx";
        constexpr char ROB_META_FD_phyRegIdx    [] = "phyRegIdx";
        constexpr char ROB_META_FD_updateArc    [] = "updateArch";
        constexpr char ROB_META_FD_executed     [] = "executed";
        constexpr char ROB_META_FD_transferType [] = "transferType";
        constexpr char ROB_META_FD_fetchPc      [] = "fetchPc";
        constexpr char ROB_META_FD_valid        [] = "valid";


        struct RobUTM_Base: GenRowMetaable, VizCsvGenable{

            std::vector<RAP_Link> _transferFields;

            int _pcWidth          = -1;
            int _amtTransferType  =  4;

            /// transferType //////////////////////////////////////////
            /// [0 update phyReg to archReg] link only
            /// [1 reqLoad from address(_idxInPhyTrans_MemAddr) to phyReg]
            /// [2 reqStore to address(_idxInPhyTrans_MemAddr) with value(_idxInPhyTrans_MemValue) ]
            /// [3 req branch update phy to pc]
            ///////////////////////////////////////////////////////////

            ///// archreg that mapped from archRegTransfer to phyRegTransfer index by index


            explicit RobUTM_Base(int pcWidth, int amtTr = 4):
            _pcWidth(pcWidth),
            _amtTransferType(amtTr){}

            ~RobUTM_Base() override = default;

            int addTransferType(RAP_Link matcher){
                RAP_Link robFieldMatch = matcher;
                robFieldMatch.fieldInRob = _transferFields.size();
                _transferFields.push_back(robFieldMatch);
                ////// return by value
                return robFieldMatch.fieldInRob;
            }

            std::string getFieldName(int idx, const std::string& fieldName, bool isArch){
                ////// build the name for eachfield
                std::string result;
                result  = fieldName + "_" + std::to_string(idx) + "_";
                ArchRegFileUTM_Base* arf = _transferFields[idx].relatedArchRegFile;
                PhyRegFileUTM_Base*  prf = _transferFields[idx].relatedArchRegFile->getLinkedPhyRegFileUTM();

                crlAss(arf != nullptr, "transferField idx: " +
                                            std::to_string(idx) + " arch cannot be nullptr");
                crlAss(prf != nullptr, "transferField idx: " +
                                       std::to_string(idx) + " phy cannot be nullptr");


                result += isArch ? arf->getUtmName(): prf->getUtmName();
                return result;
            }

            RowMeta genRowMeta(CRL_GEN_MODE genMode, int subMode) override{

                crlAss(genMode == CGM_ROB, "cannot gen row meta for this genMode");

                /////////////////////////////////////////////////////////////////////////////////////////////////////////
                ///    0   1        2                      3x
                /// |valid|Pc|transferType|*[updateArchReg| executed| archIdx| phyIdx]|
                /////////////////////////////////////////////////////////////////////////////////////////////////////////
                /// valid the system will handle it self
                RowMeta row;
                ///row.addField(ROB_META_FD_valid, 1); model must be handle these thing
                row.addField(ROB_META_FD_fetchPc, _pcWidth);
                row.addField(ROB_META_FD_transferType, log2Ceil(_amtTransferType));

                for (int idx = 0; idx < _transferFields.size(); idx++){
                    ArchRegFileUTM_Base* arf = _transferFields[idx].relatedArchRegFile;
                    PhyRegFileUTM_Base*  prf = _transferFields[idx].relatedArchRegFile->getLinkedPhyRegFileUTM();
                    row.addField(getFieldName(idx, ROB_META_FD_updateArc , true), 1);
                    row.addField(getFieldName(idx, ROB_META_FD_executed  , true), 1);
                    row.addField(getFieldName(idx, ROB_META_FD_archRegIdx, true), arf->getIndexWidth());
                    row.addField(getFieldName(idx, ROB_META_FD_phyRegIdx , false),prf->getIndexWidth());
                }
                return row;

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
