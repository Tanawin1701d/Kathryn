//
// Created by tanawin on 31/12/2024.
//

#ifndef src_carolyne_arch_base_march_robUnit_ROBMETABASE_H
#define src_carolyne_arch_base_march_robUnit_ROBMETABASE_H
#include <util/numberic/pmath.h>

#include "carolyne/arch/base/util/rowMeta.h"

namespace kathryn{
    namespace carolyne{

        constexpr char ROB_META_FD_archRegIdx   [] = "archRegIdx";
        constexpr char ROB_META_FD_phyRegIdx    [] = "phyRegIdx";
        constexpr char ROB_META_FD_updateArc    [] = "updateArch";
        constexpr char ROB_META_FD_executed     [] = "executed";
        constexpr char ROB_META_FD_transferType [] = "transferType";
        constexpr char ROB_META_FD_fetchPc      [] = "fetchPc";
        constexpr char ROB_META_FD_valid        [] = "valid";


        struct RobMeta: GenRowMetaable{

            std::vector<RegTypeMeta> _archRegTransfers; //// it may be the same RegTypeMeta
            std::vector<std::string> _archRegTransferNames;
            std::vector<RegTypeMeta> _phyRegTransfers;
            std::vector<std::string> _phyRegTransferNames;

            int _idxInPhyTrans_MemAddr   = -1; /////// which field that used to be addr coressponding to archRegTransfer vector
            int _idxInPhyTrans_MemValue  = -1;
            int _pc_width                = -1;
            int _amt_transferType        =  4;

            /// transferType //////////////////////////////////////////
            /// [0 update phyReg to archReg] link only
            /// [1 reqLoad from address(_idxInPhyTrans_MemAddr) to phyReg]
            /// [2 reqStore to address(_idxInPhyTrans_MemAddr) with value(_idxInPhyTrans_MemValue) ]
            /// [3 req branch update phy to pc]
            ///////////////////////////////////////////////////////////

            ///// archreg that mapped from archRegTransfer to phyRegTransfer index by index

            virtual ~RobMeta() = default;

            void addTransferType(const RegTypeMeta& arcRegType, const std::string& archRegGroupName,
                                 const RegTypeMeta& phyRegType, const std::string& phyRegGroupName){

                _archRegTransfers.    push_back(arcRegType);
                _archRegTransferNames.push_back(archRegGroupName);
                _phyRegTransfers.     push_back(phyRegType);
                _phyRegTransferNames. push_back(phyRegGroupName);

            }

            void setIdxInRobFor(bool isAddr, int value){
                crlAss(value < _phyRegTransferNames.size(), "setIdx From rob for mem operation is outofRange");
                if (isAddr){_idxInPhyTrans_MemAddr  = value;}
                else       {_idxInPhyTrans_MemValue = value;}
            }

            void setPcWidth(int width){_pc_width = width;}

            std::string getFieldName(int idx, const std::string& fieldName, bool isArch){
                ////// build the name for eachfield
                std::string result;
                result = fieldName + "_" + std::to_string(idx) + "_";
                result += isArch ? _archRegTransferNames[idx]: _phyRegTransferNames[idx];
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

                for (int idx = 0; idx < _archRegTransfers.size(); idx++){
                    row.addField(getFieldName(idx, ROB_META_FD_updateArc , true), 1);
                    row.addField(getFieldName(idx, ROB_META_FD_executed  , true), 1);
                    row.addField(getFieldName(idx, ROB_META_FD_archRegIdx, true), _archRegTransfers[idx].getIndexWidth());
                    row.addField(getFieldName(idx, ROB_META_FD_phyRegIdx , false), _phyRegTransfers[idx].getIndexWidth());
                }

                row.reverse(); //// because we want the idx like the comment above
                return row;

            }

        };

    }
}

#endif //src_carolyne_arch_base_march_robUnit_ROBMETABASE_H
