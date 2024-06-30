//
// Created by tanawin on 20/6/2024.
//

#ifndef LOGICCERF_H
#define LOGICCERF_H
#include "model/hwComponent/abstract/Slice.h"
#include "model/hwComponent/abstract/identifiable.h"

namespace kathryn{


    enum MODULE_GEN_GRP{
        GEN_REG_GRP,
        GEN_WIRE_GRP,
        GEN_EXPRE_GRP,
        GEN_NEST_GRP,
        GEN_VAL_GRP,
        GEN_MEMBLK_GRP,
        GEN_MEMBLK_ELE_GRP,
        GEN_INTER_WIRE_GRP,
        GEN_AUTO_INPUT_WIRE_GRP,
        GEN_AUTO_OUTPUT_WIRE_GRP,
        GEN_USER_DEC_INPUT_GRP,
        GEN_USER_DEC_OUTPUT_GRP,
        GEN_MODULE_GRP
    };


    struct logicLocalCef{ ////// to check localization
        HW_COMPONENT_TYPE comptype;
        MODULE_GEN_GRP    md_gen_grp;
        VarMeta           varMeta; /// the varMeta type/varName/isUser
        int               grpIdx  = -1;
        int               idx     = -1;
        Slice             curSl;
        /** it requrire further check for it dependency**/
    };

	struct moduleLocalCef{
		MODULE_GEN_GRP    md_gen_grp;
		VarMeta           varMeta;
		int               idx; //////// idx of sub  module to master

	};



}

#endif //LOGICCERF_H
