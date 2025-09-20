//
// Created by tanawin on 18/9/25.
//

#include "table.h"

#include <utility>
#include "model/controller/controller.h"

namespace kathryn{

    Table::Table(const SlotMeta&  meta, const std::vector<RegSlot*>& rows):
        _meta(std::move(meta)), _rows(rows){
            mfAssert(!rows.empty(), "rows cannot be empty");
            for (RegSlot* row: rows){
                mfAssert(row != nullptr, "row cannot be nullptr");
            }
        }

    Table::Table(const SlotMeta&  slotMeta, int amtRow, const std::string& prefixName):
        _meta(std::move(slotMeta)){
            mfAssert(amtRow > 0, "amtRow must be greater than 0");
            buildRows(_meta, amtRow, prefixName);
        }

    Table::Table(const std::vector<std::string>& fieldNames,
              const std::vector<int>&         fieldSizes,
              int amtRow,
              const std::string& prefixName):
        _meta(fieldNames, fieldSizes){
            mfAssert(amtRow > 0, "amtRow must be greater than 0");
            buildRows(_meta, amtRow, prefixName);
        }

    Table::~Table(){
            for (RegSlot* row: _rows){
                delete row;
            }
        }



    SlotMeta Table::getMeta() const{
        return _meta;
    }

    RegSlot& Table::getRefRow(int idx){
        assert(isValidIdx(idx));
        return *_rows[idx];
    }

    RegSlot Table::getClonedRow(int idx) const{
        assert(isValidIdx(idx));
        return *_rows[idx];
    }

    void Table::buildRows(SlotMeta& slotMeta, int amtRow, std::string prefixName){
        for (int rowIdx = 0; rowIdx < amtRow; rowIdx++){
            _rows.push_back(new RegSlot(slotMeta, prefixName + "_" + std::to_string(rowIdx)));
        }
    }

    bool Table::isSufficientBinIdx(Operable& requiredIdx) const{
        int inputSize = requiredIdx.getOperableSlice().getSize();
        return (1 << inputSize) >= getNumRow();
    }

    bool Table::isSufficientOHIdx(Operable& requiredIdx) const{
        return requiredIdx.getOperableSlice().getSize() == getNumRow();
    }

    bool Table::isSufficientIdx(Operable& requiredIdx, bool isOH) const{
        if (isOH){
            return isSufficientOHIdx(requiredIdx);
        }
        return isSufficientBinIdx(requiredIdx);
    }

    Operable& Table::createIdxMatchCond(Operable& requiredIdx, int rowIdx, bool isOH){
        if (isOH){
            return *requiredIdx.doSlice({rowIdx, rowIdx+1});
        }
        return (requiredIdx == rowIdx);


    }



    bool Table::isValidIdx(int idx) const{
        return idx >= 0 && idx < _rows.size();
    }
    bool Table::checkValidRange(int start, int stop) const{
        return ( (start >= 0    ) && (start <  _rows.size()) ) &&
               ( (stop   >  start) && (stop   <= _rows.size()) );
    }

    int Table::getNumRow() const{
        return static_cast<int>(_rows.size());
    }

    int Table::getMaxCellWidth() const{
        assert(!_rows.empty());
        return _rows[0]->getMaxBitWidth();
    }

    /**
     * gen assign meta
     *
     */

    WireSlot Table::genDynWireSlotBase(Operable& requiredIdx, bool isOneHotIdx){

        ////// return node
        WireSlot resultWireSlot(getMeta());
        ////// all metadata
        std::vector<AssignMeta*> allRowCollector;
        std::vector<Operable*> allRowPreCond;
        ////// generate all assign meta to all node
        for(int rowIdx = 0; rowIdx < getNumRow(); rowIdx++){
            std::vector<AssignMeta*> eachRowCollector;
            std::vector<Operable*> eachRowPreCond;
            ////// generate the assign condition
            Operable* rowIdxCheckCond = &createIdxMatchCond(requiredIdx, rowIdx, isOneHotIdx);
            //////// generate each row's assign meta
            eachRowCollector = resultWireSlot.genAssignMetaForAll(*_rows[rowIdx], ASM_DIRECT);
            for (int colIdx = 0; colIdx < resultWireSlot.getNumField(); colIdx++){
                eachRowPreCond.push_back(rowIdxCheckCond);
            }
            /////// push it to pool system
            allRowCollector.insert(allRowCollector.end(),
            eachRowCollector.begin(), eachRowCollector.end());
            allRowPreCond.insert(allRowPreCond.end(),
            eachRowPreCond.begin(), eachRowPreCond.end());
        }
        ////// generate assignment Node
        AsmNode* asmNode = resultWireSlot.genGrpAsmNode(allRowCollector, allRowPreCond, BITWISE_AND);
        //// we have to do dry assign
        asmNode->dryAssign();

        ///resultWireSlot.doGlobAsm(asmNode);
        return resultWireSlot;

    }

    WireSlot Table::genDynWireSlotBiIdx(Operable& requiredIdx){
        mfAssert(isSufficientBinIdx(requiredIdx), "requiredIdx is not sufficient to get all system");
        return genDynWireSlotBase(requiredIdx, false);
    }

    WireSlot Table::genDynWireSlotOHIdx(Operable& requiredIdx){
        mfAssert(isSufficientOHIdx(requiredIdx), "requiredIdx is not sufficient to get all system");
        return genDynWireSlotBase(requiredIdx, true);
    }

    ////// this will asssign the slot
    void Table::doGlobAsm(Slot& srcSlot, Operable& requiredIdx, ASM_TYPE asmType, bool isOneHotIdx){
        mfAssert(isSufficientIdx(requiredIdx, isOneHotIdx), "requiredIdx is not sufficient to get all system");
        std::vector<AssignMeta*> allRowCollector;
        std::vector<Operable*>   allRowPreCond;

        for (int desIdx = 0; desIdx < getNumRow(); desIdx++){
            /////// get related meta data
            std::vector<AssignMeta*> eachRowCollector;
            std::vector<Operable*>   eachRowPreCond;
            RegSlot& desSlot = getRefRow(desIdx);
            SlotMeta srcMeta = srcSlot.getMeta();
            ////// seach for match assign column and generate assign Metadata
            auto  [srcMatchidxs, desMatchIdxs] = getMeta().matchByName(srcMeta);
            eachRowCollector = desSlot.genAssignMetaForAll(srcSlot, srcMatchidxs, desMatchIdxs, {}, asmType);
            Operable* rowIdxCheckCond = &(createIdxMatchCond(requiredIdx, desIdx, isOneHotIdx));
            for (int colIdx = 0; colIdx < eachRowCollector.size(); colIdx++){
                eachRowPreCond.push_back(rowIdxCheckCond);
            }
            /////// push it to pool system
            allRowCollector.insert(allRowCollector.end(),
            eachRowCollector.begin(), eachRowCollector.end());
            allRowPreCond.insert(allRowPreCond.end(),
            eachRowPreCond.begin(), eachRowPreCond.end());
        }

        /////// we have to create own asm node and push it directly to the system
        auto* asmNode = new AsmNode(allRowCollector);
        for (int idx = 0; idx < allRowPreCond.size(); idx++){
            asmNode->addSpecificPreCondition(allRowPreCond[idx], BITWISE_AND, idx);
        }
        /////// we have to add it to controller by ourself
        ModelController* ctrl = getControllerPtr();
        assert(ctrl != nullptr);
        ctrl->on_reg_update(
            asmNode,
            nullptr
        );

    }

    void Table::doGlobAsm(Operable& srcOpr, Operable& rowIdx, Operable& colIdx, ASM_TYPE asmType, bool isOHRow){
        mfAssert(isSufficientIdx(rowIdx, isOHRow), "requiredIdx is not sufficient to get all system");
        mfAssert(_rows[0]->isSufficientIdx(colIdx.getOperableSlice().getSize()), "column is not sufficient to get all column");

        std::vector<AssignMeta*> allRowCollector;
        std::vector<Operable*>   allRowPreCond;

        for (int desRowIdx = 0; desRowIdx < getNumRow(); desRowIdx++){
            /////// get related meta data
            std::vector<AssignMeta*> eachRowCollector;
            std::vector<Operable*>   eachRowPreCond;
            RegSlot& rowSlot = getRefRow(desRowIdx);

            ///// gen row assign
            eachRowCollector = rowSlot.genAssignMetaForAll(srcOpr, asmType);
            ///// gen col assign
            Operable* desRowIdxCheckCond = &(createIdxMatchCond(colIdx, desRowIdx, isOHRow));
            for (int desColIdx = 0; desColIdx < eachRowCollector.size(); desColIdx++){
                eachRowPreCond.push_back(
                    &( (*desRowIdxCheckCond) && (colIdx == desColIdx)));
            }
            ///// add to main pool
            allRowCollector.insert(allRowCollector.end(),
            eachRowCollector.begin(), eachRowCollector.end());
            allRowPreCond.insert(allRowPreCond.end(),
            eachRowPreCond.begin(), eachRowPreCond.end());
        }

        ///// gen assign Node
        auto* asmNode = new AsmNode(allRowCollector);
        for (int idx = 0; idx < allRowPreCond.size(); idx++){
            asmNode->addSpecificPreCondition(allRowPreCond[idx], BITWISE_AND, idx);
        }
        ///// add it to the whole main controller
        ModelController* ctrl = getControllerPtr();
        assert(ctrl != nullptr);
        ctrl->on_reg_update(asmNode, nullptr);

    }

    void Table::doGlobAsm(ull rhsVal, Operable& rowIdx, Operable& colIdx, ASM_TYPE asmType, bool isOHRow){
        Operable& mySrcOpr = getMatchAssignOperable(rhsVal, getMaxCellWidth());
        doGlobAsm(mySrcOpr, rowIdx, colIdx, asmType, isOHRow);
    }

    void Table::doCusLogic(std::function<void(RegSlot&, int rowIdx)>  cusLogic){
        for (int rowIdx = 0; rowIdx < getNumRow(); rowIdx++){
            cusLogic(*_rows[rowIdx], rowIdx);
        }
    }

    WireSlot Table::doReduce(std::function<Operable&(RegSlot& lhs, Operable& lidx,
                                                  RegSlot& rhs, Operable& ridx)> cusLogic){

        struct ReducNode{
            RegSlot* slot = nullptr; Operable* idx;
        };

        std::vector<AssignMeta*> allSelectAssMeta;
        std::vector<Operable*>   allSelectAssCond;

        std::vector<AssignMeta*> allIdxSelectAssMeta;
        std::vector<Operable*>   allIdxSelectAssCond;

        std::queue<ReducNode> reducQueueA;
        std::queue<ReducNode> reducQueueB;

        std::queue<ReducNode>* srcReducQueue = &reducQueueA;
        std::queue<ReducNode>* desReducQueue = &reducQueueB;

        while (srcReducQueue->size() != 1){

            while(!srcReducQueue->empty()){
                ////  if there is only one element by pass it
                if (srcReducQueue->size() == 1){
                    desReducQueue->push(srcReducQueue->front());
                    srcReducQueue->pop();
                }

                //////get two node
                ReducNode srcNodeLeft  = srcReducQueue->front();
                srcReducQueue->pop();
                ReducNode srcNodeRight = srcReducQueue->front();
                srcReducQueue->pop();

                //// get condition node
                Operable& selectLeft = cusLogic(*srcNodeLeft.slot, *srcNodeLeft.idx,
                                                *srcNodeRight.slot, *srcNodeRight.idx);
                Operable& selectRight = ~selectRight;

                WireSlot* desSlot = new WireSlot(getMeta());
                std::vector<AssignMeta*> leftSelectAssMetas  = desSlot->genAssignMetaForAll(*srcNodeLeft.slot, ASM_DIRECT);
                std::vector<AssignMeta*> rightSelectAssMetas = desSlot->genAssignMetaForAll(*srcNodeRight.slot, ASM_DIRECT);
                std::vector<Operable*>





            }
            std::swap(srcReducQueue,desReducQueue);
        }




    }

    /**
     * static slicing
     */
    RegSlot& Table::operator () (int idx){
        mfAssert(isValidIdx(idx), "index out of range to get " + std::to_string(idx));
        return *_rows[idx];
    }

    Table Table::operator() (int start, int end){

        mfAssert(checkValidRange(start, end), "invalid range to get");
        std::vector<RegSlot*> newRows;
        for (int idx = start; idx < end; idx++){
            newRows.push_back(_rows[idx]);
        }
        return Table(_meta, newRows);

    }

    Table Table::sliceByCol(int start, int end){
        SlotMeta newSlotMeta = _meta(start, end);
        std::vector<RegSlot*> newRows;
        for (int rowIdx = 0; rowIdx < getNumRow(); rowIdx++){
            RegSlot newRegSlot = ((*_rows[rowIdx])(start, end));
            newRows.push_back(new RegSlot(newRegSlot));
        }
        return {newSlotMeta, newRows};
    }
        //// treated as base col slice function
    Table Table::sliceByCol(const std::string& startField, const std::string& endField){
        int startIdx = getMeta().getIdx(startField);
        int endIdx   = getMeta().getIdx(endField) + 1;
        mfAssert(getMeta().isValidIdx(startIdx), "field name " + startField + " not found");
        mfAssert(getMeta().isValidIdx(endIdx), "field name " + endField + " not found");
        return sliceByCol(startIdx, endIdx);
    }
    Table Table::sliceByCol(const std::vector<int>& fieldIdxs){
        SlotMeta newSlotMeta = _meta(fieldIdxs);
        std::vector<RegSlot*> newRows;
        for (int rowIdx = 0; rowIdx < getNumRow(); rowIdx++){
            RegSlot newRegSlot = (*_rows[rowIdx])(fieldIdxs);
            newRows.push_back(new RegSlot(newRegSlot));
        }
        return {newSlotMeta, newRows};
    }
    Table Table::sliceByCol(const std::vector<std::string>& fieldNames){
        std::vector<int> fieldIdxs = getMeta().getIdxs(fieldNames);
        return sliceByCol(fieldIdxs);
    }

    /**
     * dynamic
     */

    TableSliceAgent Table::operator[] (Operable& requiredIdx){
        isSufficientBinIdx(requiredIdx);
        return TableSliceAgent(this, requiredIdx, false);
    }

    TableSliceAgent Table::operator[] (OH ohIdx){
        isSufficientOHIdx(ohIdx.getIdx());
        return TableSliceAgent(this, ohIdx.getIdx(), true);
    }

    /**
     *  table join
     *
     */

    Table Table::joinTableByRow(const Table& rhs){
        SlotMeta rhsMeta = rhs.getMeta();
        SlotMeta newMeta = getMeta();
        mfAssert(newMeta == rhsMeta, "slot meta is not match");

        ////// new row
        std::vector<RegSlot*> newRows = _rows;
        newRows.insert(newRows.end(), rhs._rows.begin(), rhs._rows.end());

        return Table(newMeta, newRows);

    }

    Table Table::joinTableByCol(const Table& rhs){
        mfAssert(getNumRow() == rhs.getNumRow(), "row size is not match");

        std::vector<RegSlot*> newRows;

        for (int rowIdx = 0; rowIdx < getNumRow(); rowIdx++){
            RegSlot  newRegSlot = getRefRow(rowIdx) + rhs.getClonedRow(rowIdx);
            RegSlot* clonedSlot = new RegSlot(newRegSlot);
            newRows.push_back(clonedSlot);
        }

        return Table(getMeta(), newRows);

    }


    Table Table::join(const Table& rhs,  int axis){
        mfAssert(axis >= 0 && axis <= 1, "axis must be 0 or 1");
        mfAssert(false, "not implemented yet");

        switch (axis){
            case 0:
                return joinTableByRow(rhs);
            case 1:
                return joinTableByCol(rhs);
            default:
                assert(false);
        }

    }

}