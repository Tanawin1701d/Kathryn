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


    std::vector<std::vector<UpdateEvent*>>
    grpEventByPriorityValueAndPriority(std::vector<UpdateEvent*>& updateEvents){

        /**init variable*/
        std::vector<std::vector<UpdateEvent*>> ret;
        int idx = 0;

        /**iterate all element*/
        while (idx < updateEvents.size()){
            /** do initialize*/
            std::vector<UpdateEvent*> eventGroup;
            UpdateEvent* masterEvent = updateEvents[idx];
            assert(masterEvent != nullptr);
            eventGroup.push_back(updateEvents[idx]);
            /** getSlave event */
            for (idx = idx + 1; idx < updateEvents.size(); idx++){
                UpdateEvent* slaveEvent = updateEvents[idx];
                assert(slaveEvent != nullptr);
                if (masterEvent->priority == slaveEvent->priority){
                    if (masterEvent->srcUpdateValue->isConstOpr() &&
                        slaveEvent ->srcUpdateValue->isConstOpr() &&
                        (masterEvent->srcUpdateValue->getConstOpr() ==
                         slaveEvent ->srcUpdateValue->getConstOpr())
                    ){
                        eventGroup.push_back(slaveEvent);
                        continue;
                    }
                    if (masterEvent->srcUpdateValue == slaveEvent->srcUpdateValue){
                        eventGroup.push_back(slaveEvent);
                        continue;
                    }
                }
                ///////////// mismatch occur
                break;
            }
            assert(!eventGroup.empty());
            ret.push_back(eventGroup);
        }
        return ret;
    }


    //////////

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
                generateAssignMeta( *slicedSrcOpr, desireDesSlice, asmType)
        );
    }


    AsmNode* Assignable::generateBasicNode(Operable& srcOpr, Slice desSlice, ASM_TYPE asmType){

        assert(desSlice.getSize() <= srcOpr.getOperableSlice().getSize());

        auto* assMeta = new AssignMeta(_updateMeta,
                                       srcOpr,
                                       desSlice,
                                       asmType);
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