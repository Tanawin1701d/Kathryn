//
// Created by tanawin on 11/10/25.
//

#include "slotProber.h"
#include "model/hwCollection/dataStructure/slot/slot.h"

namespace kathryn{

    void SlotSimProbe::init(Slot* inputSlot){

        this->slot = inputSlot;
        mfAssert(slot != nullptr, "slot is null");
        prevValues.resize(slot->getNumField(),0);
    }


    FieldSimInfo64 SlotSimProbe::getCurSimInfo(int col){
        mfAssert(slot->isValidIdx(col), "get cur sim info at " +
            std::to_string(col) + " out of range");

        FieldMeta   field       = slot->getMeta().getCopyField(col);
        HwFieldMeta hwMeta      = slot->hwFieldAt(col);
        ull         prevValue = prevValues[col];
        ull         curValue    = (ull)(*hwMeta._opr);
        return {field._name, prevValue, curValue};

    }

    /**
     *    get cur simulation value
     ***/
    SlotSimInfo64 SlotSimProbe::getCurValues(){

        SlotSimInfo64 result;
        ///////// loop to retrive all meta
        for (int idx = 0; idx < slot->getNumField(); idx++){
            result.append(getCurSimInfo(idx));
        }
        return result;

    }

    FieldSimInfo64 SlotSimProbe::getCurValue(std::string varName){

        int idx = slot->getMeta().getIdx(varName);
        return getCurSimInfo(idx);

    }

    /**
     * change value detection
     */

    bool SlotSimProbe::isThereChange(){
        SlotSimInfo64 curVals = getCurValues();

        for (int idx = 0; idx < slot->getNumField(); idx++){
            if (prevValues[idx] != curVals[idx].curValue){
                return true;
            }
        }
        return false;

    }


    std::vector<FieldSimInfo64> SlotSimProbe::detectChange(){

        SlotSimInfo64 curVals = getCurValues();
        std::vector<FieldSimInfo64> result;

        for (int idx = 0; idx < slot->getNumField(); idx++){
            FieldSimInfo64 curVal = curVals[idx];
            if (curVal.prevValue != curVal.curValue){
                result.push_back(curVal);
            }
        }
        return result;
    }

    void SlotSimProbe::applyChange(){
        SlotSimInfo64 curVals = getCurValues();

        for (int idx = 0; idx < slot->getNumField(); idx++){
            prevValues[idx] = curVals[idx].curValue;
        }

    }


}