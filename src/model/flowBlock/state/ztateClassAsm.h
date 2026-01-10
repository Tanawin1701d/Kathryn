//
// Created by tanawin on 10/1/26.
//

#ifndef SRC_MODEL_FLOWBLOCK_STATE_ZTATECLASSASM_H
#define SRC_MODEL_FLOWBLOCK_STATE_ZTATECLASSASM_H

#include "model/flowBlock/abstract/flowBlock_Base.h"

namespace kathryn{


    struct ZStateClassAsm{

        Operable*                     _identifier = nullptr;
        std::vector<int>              _caseIdent;
        std::vector<ClassAssignMeta*> _caseAssignMetas; /// < caseId, ue
        ///////// the conclude result
        UpdateEventSwitch* stateUeEvent = nullptr;


        /////// disable the assignMetas because ztate don't allow it
        AssignMeta* getSampleAssignMeta();
        bool        isEmpty()       {return _caseAssignMetas.empty();}
        int         getSize() const {return _caseAssignMetas.size();}
        bool        isJoinable(AssignMeta* sample){ assert(false);}
        bool        isJoinable(ClassAssignMeta* sample);
        ////////////////////////////////////////////////////////////

        int getMaxSupportSize() const;
        /** constructor*/
        explicit ZStateClassAsm(Operable* identifier);
        /** create the block belong to the caseid*/
        void addCaseBlock(int caseIdent, ClassAssignMeta* classAssignMeta);
        /** create assignment node and stateUe that conclude this event*/
        AsmNode* createAsmNode();


    };

}

#endif //SRC_MODEL_FLOWBLOCK_STATE_ZTATECLASSASM_H