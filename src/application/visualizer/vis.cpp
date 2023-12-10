//
// Created by tanawin on 10/12/2566.
//

#include "vis.h"


namespace kathryn{


    Vis::Vis(Module *sampleModel): _sampleModel(sampleModel) {
        assert(_sampleModel != nullptr);
    }



    void Vis::execute() {
        /** retrieve all states register */
        retrieveStateReg();
        /** get register*/
        retrieveReg();
        /** get wire*/
        retrieveWire();
        /** get expression*/
        retrieveExpr();
    }

    void Vis::print() {

    }

    void Vis::retrieveReg() {

        for ()

    }

    void Vis::retrieveWire() {

    }

    void Vis::retrieveExpr() {

    }

}