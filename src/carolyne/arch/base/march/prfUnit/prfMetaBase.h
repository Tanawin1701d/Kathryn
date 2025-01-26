//
// Created by tanawin on 30/12/2024.
//

#ifndef src_carolyne_arch_base_march_pRegFile_PHYSICALREGFILE_H
#define src_carolyne_arch_base_march_pRegFile_PHYSICALREGFILE_H

#include "carolyne/arch/base/util/genRowMeta.h"
#include "carolyne/arch/base/util/regFileType.h"
#include "carolyne/arch/base/util/metaIdent.h"
#include "carolyne/arch/base/isa/regFile/archRegFile.h"


namespace kathryn::carolyne{

        constexpr char PRF_FD_VALUE  [] = "value";
        constexpr char PRF_FD_ISVALID[] = "isValid";
        constexpr char PRF_FD_ISFREE [] = "isFree";

        struct PhyRegFileUTM_Base: RegTypeMeta  , GenRowMetaable,
                                   VizCsvGenable, MetaIdentifiable{

            explicit PhyRegFileUTM_Base(int indexSize, int regWidth,
                                        const std::string &typeName) :
                    RegTypeMeta(indexSize,regWidth),
                    MetaIdentifiable(typeName){}

            ArchRegFileUTM_Base* _linkedArchRegFileUtm = nullptr;
            bool _reqIsFreeStatus  = false; //// is current system require free bit?
            bool _reqValidStatus   = false; //// is current system require valid bit?

            void setLinkArchRegFile(ArchRegFileUTM_Base* archRegFile){
                crlAss(archRegFile != nullptr, "cannot set phyRegUTM with nullptr");
                _linkedArchRegFileUtm = archRegFile;
            }
            void setReqFreeBit (bool reqFreeBit ) { _reqIsFreeStatus = reqFreeBit;}
            void setReqValidBit(bool reqValidBit) { _reqValidStatus = reqValidBit;}

            [[nodiscard]] bool isReqFreeBit  () const{return _reqIsFreeStatus;}
            [[nodiscard]] bool istReqValidBit() const{return _reqValidStatus;}


            RowMeta genRowMeta(CRL_GEN_MODE genMode, int subMode) override{
                RowMeta row;
                row.addField(PRF_FD_VALUE, getRegWidth());
                if (isReqFreeBit())  {row.addField(PRF_FD_ISFREE , 1);}
                if (istReqValidBit()){row.addField(PRF_FD_ISVALID, 1);}

                return row;
            }

            void visual(CsvGenFile& genFile) override{
                genFile.addData(RegTypeMeta::genTable());
            }

        };

    }




#endif //src_carolyne_arch_base_march_pRegFile_PHYSICALREGFILE_H
