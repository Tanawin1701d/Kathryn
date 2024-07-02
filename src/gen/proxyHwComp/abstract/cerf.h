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
        GEN_GLOB_INPUT_GRP,
        GEN_GLOB_OUTPUT_GRP,
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
        bool cmpLocal(const logicLocalCef& rhs) const{
            //////// this is assume that it is in the analogy module
            bool result = true;
            result &= (comptype == rhs.comptype);
            result &= (md_gen_grp == rhs.md_gen_grp);
            result &= (varMeta.isUser == rhs.varMeta.isUser);
            result &= (varMeta.varName == rhs.varMeta.varName);
            result &= (grpIdx     == rhs.grpIdx);
            result &= (idx        == rhs.idx);
            result &= (curSl      == rhs.curSl);
            return result;
        }
    };

	struct moduleLocalCef{
		VarMeta           varMeta;
		int               idx; //////// idx of sub  module to master

		///// lhs and rhs is sub module of SOME module make compare it is the same
	    bool cmpAsSubModule(const moduleLocalCef& rhs) const{
			return (varMeta == rhs.varMeta) & (idx == rhs.idx);
	    }

	};



}

#endif //LOGICCERF_H
