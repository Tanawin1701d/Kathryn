//
// Created by tanawin on 10/12/2566.
//

#include "vis.h"


namespace kathryn{


    Vis::Vis(Module *sampleModel): _sampleModule(sampleModel) {
        assert(_sampleModule != nullptr);
    }



    void Vis::execute() {
        /** retrieve all states register */
        retrieveStateReg();
        /** get register*/
        retrieveSimpleAsm(_sampleModule->getUserRegs(), regMetas);
        /** get wire*/
        retrieveSimpleAsm(_sampleModule->getUserWires(), wireMetas);
        /** get exprMetas*/
        retrieveSimpleAsm(_sampleModule->getUserExpressions(), exprMetas);
    }

    void Vis::print() {

    }


}