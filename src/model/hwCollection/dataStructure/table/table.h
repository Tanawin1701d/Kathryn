//
// Created by tanawin on 15/9/25.
//

#ifndef KATHRYN_SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_TABLE_TABLE_H
#define KATHRYN_SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_TABLE_TABLE_H

#include "model/hwCollection/dataStructure/slot/regSlot.h"
#include "model/hwCollection/dataStructure/slot/slotMeta.h"
#include "model/hwCollection/dataStructure/slot/wireSlot.h"
#include "tableSliceAgent.h"

namespace kathryn{

    class Table{

    protected:
        SlotMeta _meta;
        std::vector<RegSlot*> _rows;
    public:

        /**
         * this is used to initialize from slice
         */
        Table(const SlotMeta& meta, const std::vector<RegSlot*>& rows):
        _meta(meta), _rows(rows){
            mfAssert(!rows.empty(), "rows cannot be empty");
            for (RegSlot* row: rows){
                mfAssert(row != nullptr, "row cannot be nullptr");
            }
        }

        Table(SlotMeta& slotMeta, int amtRow):
        _meta(slotMeta){
            mfAssert(amtRow > 0, "amtRow must be greater than 0");
            buildRows(_meta, amtRow);
        }

        Table(const std::vector<std::string>& fieldNames,
              const std::vector<int>&         fieldSizes,
              int amtRow):
        _meta(fieldNames, fieldSizes){
            mfAssert(amtRow > 0, "amtRow must be greater than 0");
            buildRows(_meta, amtRow);
        }

        ~Table(){
            for (RegSlot* row: _rows){
                delete row;
            }
        }



        SlotMeta getMeta() const{
            return _meta;
        }

        RegSlot& getRefRow(int idx){
            assert(isValidIdx(idx));
            return *_rows[idx];
        }

        RegSlot getClonedRow(int idx) const{
            assert(isValidIdx(idx));
            return *_rows[idx];
        }

        void buildRows(SlotMeta& slotMeta, int amtRow){
            for (int rowIdx = 0; rowIdx < amtRow; rowIdx++){
                _rows.push_back(new RegSlot(slotMeta));
            }
        }

        bool isSufficientIdx(Operable& requiredIdx) const{
            int inputSize = requiredIdx.getOperableSlice().getSize();
            return (1 << inputSize) >= getNumRow();
        }

        bool isValidIdx(int idx) const{
            return idx >= 0 && idx < _rows.size();
        }
        bool checkValidRange(int start, int stop) const{
            return ( (start >= 0    ) && (start <  _rows.size()) ) &&
                   ( (stop   >  start) && (stop   <= _rows.size()) );
        }

        int getNumRow() const{
            return _rows.size();
        }

        /**
         * gen assign meta
         *
         */

        WireSlot genDynWireSlot(Operable& requiredIdx){

            mfAssert(isSufficientIdx(requiredIdx), "requiredIdx is not sufficient to get all system");
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
                Operable* rowIdxCheckCond = &(requiredIdx == rowIdx);
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
            resultWireSlot.doGlobAsm(asmNode);
            return resultWireSlot;

        }

        ////// this will asssign the slot
        void doGlobAsm(Slot& srcSlot, Operable& requiredIdx, ASM_TYPE asmType){
            mfAssert(isSufficientIdx(requiredIdx), "requiredIdx is not sufficient to get all system");
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
                Operable* rowIdxCheckCond = &(requiredIdx == desIdx);
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

        void doGlobAsm(Operable& srcOpr, Operable& rowIdx, Operable& colIdx, ASM_TYPE asmType){
            mfAssert(isSufficientIdx(rowIdx), "requiredIdx is not sufficient to get all system");
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
                for (int desColIdx = 0; desColIdx < eachRowCollector.size(); desColIdx++){
                    eachRowPreCond.push_back(
                        &( (rowIdx == desRowIdx) && (colIdx == desColIdx)));
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

        /**
         * static slicing
         */
        RegSlot& operator () (int idx){
            mfAssert(isValidIdx(idx), "index out of range to get " + std::to_string(idx));
            return *_rows[idx];
        }

        Table operator() (int start, int end){

            mfAssert(checkValidRange(start, end), "invalid range to get");
            std::vector<RegSlot*> newRows;
            for (int idx = start; idx < end; idx++){
                newRows.push_back(_rows[idx]);
            }
            return Table(_meta, newRows);

        }

        /**
         * dynamic
         */

        TableSliceAgent operator[] (Operable& requiredIdx){
            isSufficientIdx(requiredIdx);
            return TableSliceAgent(this, requiredIdx);
        }

        /**
         *  table join
         *
         */

        Table joinTableByRow(const Table& rhs){
            SlotMeta rhsMeta = rhs.getMeta();
            SlotMeta newMeta = getMeta();
            mfAssert(newMeta == rhsMeta, "slot meta is not match");

            ////// new row
            std::vector<RegSlot*> newRows = _rows;
            newRows.insert(newRows.end(), rhs._rows.begin(), rhs._rows.end());

            return Table(newMeta, newRows);

        }

        Table joinTableByCol(const Table& rhs){
            mfAssert(getNumRow() == rhs.getNumRow(), "row size is not match");

            std::vector<RegSlot*> newRows;

            for (int rowIdx = 0; rowIdx < getNumRow(); rowIdx++){
                RegSlot  newRegSlot = getRefRow(rowIdx) + rhs.getClonedRow(rowIdx);
                RegSlot* clonedSlot = new RegSlot(newRegSlot);
                newRows.push_back(clonedSlot);
            }

            return Table(getMeta(), newRows);

        }


        Table join(const Table& rhs,  int axis){
            mfAssert(axis >= 0 && axis <= 1, "axis must be 0 or 1");
            mfAssert(false, "not implemented yet");

            switch (axis){
                case 0:
                    return joinTableByRow(rhs);
                case 1:
                    return joinTableByCol(rhs);
                default:
                    return assert(false);
            }

        }



    };

}

#endif //KATHRYN_SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_TABLE_TABLE_H
