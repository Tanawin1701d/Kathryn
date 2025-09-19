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
        Table(const SlotMeta&  meta, const std::vector<RegSlot*>& rows);

        Table(const SlotMeta&  slotMeta, int amtRow, const std::string& prefixName = "table");

        Table(const std::vector<std::string>& fieldNames,
              const std::vector<int>&         fieldSizes,
              int amtRow,
              const std::string& prefixName);

        ~Table();



        SlotMeta getMeta() const;

        RegSlot& getRefRow(int idx);

        RegSlot getClonedRow(int idx) const;

        void buildRows(SlotMeta& slotMeta, int amtRow, std::string prefixName);

        bool isSufficientIdx(Operable& requiredIdx) const;

        bool isValidIdx(int idx) const;
        bool checkValidRange(int start, int stop) const;

        int getNumRow() const;

        /**
         * gen assign meta
         *
         */

        WireSlot genDynWireSlot(Operable& requiredIdx);

        ////// this will asssign the slot
        void doGlobAsm(Slot& srcSlot, Operable& requiredIdx, ASM_TYPE asmType);

        void doGlobAsm(Operable& srcOpr, Operable& rowIdx, Operable& colIdx, ASM_TYPE asmType);

        void doCusLogic(std::function<void(RegSlot&, int rowIdx)>  cusLogic);

        /**
         * static slicing
         */
        RegSlot& operator () (int idx);

        Table operator() (int start, int end);

        Table sliceByCol(int start, int end);  //// treated as base col slice function
        Table sliceByCol(const std::string& startField, const std::string& endField);
        Table sliceByCol(const std::vector<int>& fieldIdxs); //// treated as base col slice function
        Table sliceByCol(const std::vector<std::string>& fieldNames);

        /**
         * dynamic
         */

        TableSliceAgent operator[] (Operable& requiredIdx);

        /**
         *  table join
         *
         */

        Table joinTableByRow(const Table& rhs);

        Table joinTableByCol(const Table& rhs);

        Table join(const Table& rhs,  int axis);

    };

}

#endif //KATHRYN_SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_TABLE_TABLE_H
