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
    	GEN_WIRE_AUTO_GRP,
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
        	if(varMeta.isUser){
        		result &= (varMeta.varName == rhs.varMeta.varName); ////// if it is user we check the value name but internal we don't
        	}
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

	struct moduleGlobalCef{
		VarMeta varMeta;

		bool operator < (const moduleGlobalCef& rhs) const{
			if (varMeta.isUser < rhs.varMeta.isUser){
				return true;
			}else if (varMeta.isUser == rhs.varMeta.isUser){
				return varMeta.varType < rhs.varMeta.varType;
			}
			return false;
		}

        bool operator == (const moduleGlobalCef& rhs) const{
            return (varMeta.isUser == rhs.varMeta.isUser  ) &
                   (varMeta.varType == rhs.varMeta.varType);
        }

        bool operator != (const moduleGlobalCef& rhs) const{
            return ! ((*this) == rhs);
        }

	};



}

#endif //LOGICCERF_H
