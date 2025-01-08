//
// Created by tanawin on 30/12/2024.
//

#ifndef src_carolyne_arch_base_march_pRegFile_PHYSICALREGFILE_H
#define src_carolyne_arch_base_march_pRegFile_PHYSICALREGFILE_H

#include "carolyne/arch/base/util/regFileBase.h"
#include "carolyne/arch/base/util/rowMeta.h"
#include "carolyne/arch/base/util/tableMeta.h"


    namespace kathryn::carolyne{

        constexpr char PRF_FD_VALUE  [] = "value";
        constexpr char PRF_FD_ISVALID[] = "isValid";
        constexpr char PRF_FD_ISFREE [] = "isFree";

        struct PhysicalRegFileBase: RegFileBase{

            bool reqIsFreeStatus  = false; //// is current system require free bit?
            bool reqValidStatus   = false; //// is current system require valid bit?

            void setReqFreeBit (bool reqFreeBit ) {reqIsFreeStatus = reqFreeBit;}
            void setReqValidBit(bool reqValidBit) {reqValidStatus = reqValidBit;}

            bool isReqFreeBit  () const{return reqIsFreeStatus;}
            bool istReqValidBit() const{return reqValidStatus;}


            virtual RowMeta genRowMeta(const std::string& regGrpName){
                crlAss(isThereGroup(regGrpName),
                    "can't find phyRegfile group name: " + regGrpName + "<=" );

                RegTypeMeta& regTypeMeta = _regMetas[regGrpName];

                RowMeta row;

                row.addField(PRF_FD_VALUE, regTypeMeta.getRegWidth());
                if (isReqFreeBit())  {row.addField(PRF_FD_ISFREE , 1);}
                if (istReqValidBit()){row.addField(PRF_FD_ISVALID, 1);}

                return row;
            }

        };

    }




#endif //src_carolyne_arch_base_march_pRegFile_PHYSICALREGFILE_H
