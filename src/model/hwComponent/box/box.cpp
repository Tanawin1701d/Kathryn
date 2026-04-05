//
// Created by tanawin on 10/4/2567.
//

#include "box.h"
#include "model/controller/controller.h"

namespace kathryn{


    Box::Box(): HwCompControllerItf(true),
                Identifiable(TYPE_BOX)
    {
        comInit();
    }

    void Box::comInit() {
        ctrl->onBoxInit(this);
    }

    void Box::comFinal() {
        ctrl->onBoxEndInit(this);
    }

    void Box::addNestMeta(NestMeta nestMeta) {
        i++;
        if (i > 5){
            assert(false);
        }
        _nestMetas.pushBack(nestMeta);
    }

    void Box::addSubBox(Box* subBox) {
       ///// add recurrent
       assert(subBox != nullptr);
       assert(subBox != this);
       _recurBoxs.pushBack(subBox);
    }

    std::vector<NestMeta>& Box::getNestMetas(){
        return _nestMetas;
    }
    std::vector<Box*>& Box::getSubBox(){
        return _recurBoxs;
    }

    void Box::collectAssignMeta(Box& rhsBox,
                                 bool isBlockAsm,
                                 std::vector<AssignMeta*>& resultCollector) {

        mfAssert(_nestMetas.size() == rhsBox.getNestMetas().size(),
                 "box mismatch");

        mfAssert(_recurBoxs.size() == rhsBox.getSubBox().size(),
                 "subblock mismatch"
                 );

        /** do element by element assignment*/

        for (sizeT idx = 0; idx < _nestMetas.size(); idx++){
            Assignable* lhsAsb = _nestMetas[idx].asb;
            Operable*   rhsOpr = rhsBox.getNestMetas()[idx].opr1;
            if (isBlockAsm) {
                _nestMetas[idx].asb->doBlockAsm(
                        *rhsOpr,
                        resultCollector,
                        rhsOpr->getOperableSlice(),
                        lhsAsb->getAssignSlice()
                );
            }else{
                _nestMetas[idx].asb->doNonBlockAsm(
                        *rhsOpr,
                        resultCollector,
                        rhsOpr->getOperableSlice(),
                        lhsAsb->getAssignSlice()
                );
            }
        }

        /** do recursive element assigning*/
        for(sizeT idx = 0; idx < _recurBoxs.size(); idx++){
            Box* subLhs    = _recurBoxs[idx];
            Box* subRhsBox = rhsBox.getSubBox()[idx];
            subLhs->collectAssignMeta(*subRhsBox, isBlockAsm, resultCollector);
        }

    }

    void Box::buildAsmNode(Box& rhsBox, bool isBlockAsm){
        /**build vector to collect system*/
        std::vector<AssignMeta*> resultCollector;
        /**collect assignmeta*/
        collectAssignMeta(rhsBox, isBlockAsm, resultCollector);
        /**build assignment node*/
        auto* asmNode = new AsmNode(resultCollector);
        /**communication controller*/
        ctrl->onBoxUpdate(asmNode, this);
    }

    Box& Box::operator=(Box& rhs) {
        buildAsmNode(rhs, false);
        return *this;
    }

    Box& Box::operator<<=(Box& rhs) {
        buildAsmNode(rhs, true);
        return *this;
    }

    std::string Box::getMdIdentVal(){
        return getIdentDebugValue();
    }

    void Box::addMdLog(MdLogVal* mdLogVal){
        assert(mdLogVal != nullptr);
        /** add sub element into log*/
        for (auto nestMeta: _nestMetas){
            std::string subIdent = nestMeta.opr1->castToIdent()->getIdentDebugValue();
            mdLogVal->addVal(subIdent);
        }
        /**add recur box*/
        for (auto recurBox: _recurBoxs){
            MdLogVal* subLog = mdLogVal->makeNewSubVal();
            recurBox->addMdLog(subLog);
        }
    }

}