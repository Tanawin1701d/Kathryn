//
// Created by tanawin on 18/1/2567.
//


#include "assignable.h"
#include "makeComponent.h"
#include "model/hwComponent/value/value.h"
#include "model/flowBlock/abstract/nodes/asmNode.h"

namespace kathryn{

    /**assign mode control*/


    Operable& getMatchAssignOperable(ull value, const int size){
        makeVal(assUserAutoVal, size, value);
        return assUserAutoVal;
    }

    void Assignable::doGlobalAsm(
            Operable& srcOpr,
            std::vector<AssignMeta*>& resultMetaCollector,
            Slice  absSrcSlice,
            Slice  absDesSlice,
            ASM_TYPE asmType
            ){
        /** check slice integrity*/
        assert(srcOpr.getOperableSlice().isContain(absSrcSlice));
        assert(getAssignSlice().isContain(absDesSlice));
        ///assert(absSrcSlice == absDesSlice);

        Slice desireSrcSlice   = absSrcSlice.getMatchSizeSubSlice(absDesSlice);
        Slice desireDesSlice   = absDesSlice.getMatchSizeSubSlice(absSrcSlice);

        Operable* exactSrcOpr  = &srcOpr.getExactOperable();
        Operable* slicedSrcOpr = exactSrcOpr->doSlice(desireSrcSlice);

        resultMetaCollector.push_back(
                generateAssignMeta( *slicedSrcOpr, desireDesSlice, asmType, getCurAssignClkMode())
        );
    }


    AsmNode* Assignable::generateBasicNode(Operable& srcOpr, Slice desSlice, ASM_TYPE asmType){

        assert(desSlice.getSize() <= srcOpr.getOperableSlice().getSize());

        auto* assMeta = generateAssignMeta(srcOpr,desSlice,asmType, getCurAssignClkMode());
        auto* asmNode = new AsmNode(assMeta);
        return asmNode;

    }


    bool Assignable::checkDesIsFullyAssignAndEqual(){

        if (_updateMeta.empty()){ return true; }
        Slice desIs = getAssignSlice();
        for (UpdateEvent* udPtr: _updateMeta){
            if (udPtr->desUpdateSlice == desIs){
                continue;
            }
            return false;
        }
        return true;
    }




}