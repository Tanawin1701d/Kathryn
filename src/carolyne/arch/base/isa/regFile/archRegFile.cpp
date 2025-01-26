//
// Created by tanawin on 20/1/2025.
//

#include "archRegFile.h"
#include "carolyne/arch/base/march/prfUnit/prfMetaBase.h"

namespace kathryn::carolyne{

    RowMeta ArchRegFileUTM_Base::genRowMeta(CRL_GEN_MODE genMode, int subMode){

        /////// |lastInfer_idx|commitedInfer_idx|
        crlAss(genMode == CGM_INDEPENDENT, "arch regFile must be generate in independent Mode");
        RowMeta row;
        row.addField(getInferFieldName(false), _linkedPhyRegFile->getIndexWidth());
        row.addField(getInferFieldName(true ), _linkedPhyRegFile->getIndexWidth());
        return row;
    }

}