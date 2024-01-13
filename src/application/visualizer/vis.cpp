////
//// Created by tanawin on 10/12/2566.
////
//
//#include "vis.h"
//#include "model/util/StateRegUtil.h"
//
//
//namespace kathryn{
//
//
//    Vis::Vis(Module *sampleModel): _sampleModule(sampleModel) {
//        assert(_sampleModule != nullptr);
//    }
//
//    void Vis::retrieveStateReg(){
//        for (auto stReg: _sampleModule->getStateRegs()){
//            StateMeta result;
//            result.stateName = stReg->getGlobalName();
//            result.metas = getStateInfo(stReg);
//            stateMetas.push_back(result);
//        }
//
//    }
//
//
//
//    void Vis::execute() {
//        /** retrieve all states register */
//
//        retrieveStateReg();
//        /** get register*/
//        retrieveSimpleAsm(_sampleModule->getUserRegs(), regMetas);
//        /** get wire*/
//        retrieveSimpleAsm(_sampleModule->getUserWires(), wireMetas);
//        /** get exprMetas*/
//        retrieveSimpleAsm(_sampleModule->getUserExpressions(), exprMetas);
//
//        retrieveYet = true;
//    }
//
//    void Vis::print() {
//
//        assert(retrieveYet);
//        std::cout << "----------------STATE PRINTING----------------\n";
//        for (auto& stMeta: stateMetas){
//            std::cout << stMeta.getDebugString();
//            std::cout << "----------\n";
//        }
//        std::cout << "----------------REG PRINTING (NORMAL REG)----------------\n";
//        printSimpleAsm(regMetas);
//        std::cout << "----------------WIRE PRINTING----------------\n";
//        printSimpleAsm(wireMetas);
//        std::cout << "----------------EXPR PRINTING----------------\n";
//        printSimpleAsm(exprMetas);
//        std::cout << "----------------finish----------------\n";
//
//
//    }
//
//
//}