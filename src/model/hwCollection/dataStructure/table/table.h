//
// Created by tanawin on 15/9/25.
//

#ifndef KATHRYN_SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_TABLE_TABLE_H
#define KATHRYN_SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_TABLE_TABLE_H

#include "model/hwCollection/dataStructure/slot/regSlot.h"
#include "model/hwCollection/dataStructure/slot/slotMeta.h"
#include "model/hwCollection/dataStructure/slot/wireSlot.h"
#include "model/hwCollection/dataStructure/indexing/index.h"
#include "tableSliceAgent.h"

namespace kathryn{

    class Table{

    protected:
        SlotMeta _meta;
        std::vector<RegSlot*> _rows;

        struct ReducNode{
            WireSlot* slot = nullptr; Operable* idx;

            void destroy(){
                delete slot;
                //// operable is managed by module
            }
        };

        ReducNode createMux(ReducNode& lhs, ReducNode& rhs, Operable& selectLeft, int debugIdx, bool requiredIdx);


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

        bool isSufficientBinIdx(Operable& requiredIdx) const;
        bool isSufficientOHIdx(Operable& requiredIdx) const;
        bool isSufficientIdx(Operable& requiredIdx, bool isOH) const;
        int  getSufficientIdxSize(bool isOH) const;

        Operable& createIdxMatchCond(Operable& requiredIdx, int rowIdx,bool isOH);

        bool isValidIdx(int idx) const;

        bool checkValidRange(int start, int stop) const;

        int getNumRow() const;

        int getMaxCellWidth() const;

        /**
         * gen assign meta
         *
         */
        WireSlot genDynWireSlotBase(Operable& requiredIdx, bool isOneHotIdx); //// oneHotIdx will determine the the rowId by bit Idx in the requiredIdx
        WireSlot genDynWireSlotBiIdx(Operable& binIdx);
        WireSlot genDynWireSlotOHIdx(Operable& ohIdx);

        ////// this will asssign the slot
        void doGlobAsm(Slot& srcSlot, Operable& requiredIdx, ASM_TYPE asmType, bool isOneHotIdx);
        void doGlobAsm(Operable& srcOpr, Operable& rowIdx, Operable& colIdx, ASM_TYPE asmType, bool isOneHotIdx);
        void doGlobAsm(ull       srcVal, Operable& rowIdx, Operable& colIdx, ASM_TYPE asmType, bool isOneHotIdx);

        void doCusLogic(std::function<void(RegSlot&, int rowIdx)>  cusLogic);

        Table& doReset(ull resetVal = 0);

        ReducNode doReduceBase(const std::vector<ReducNode>& initReducNodes,
                               const std::function<Operable&(WireSlot& lhs, Operable* lidx,
                                                       WireSlot& rhs, Operable* ridx)>& cusLogic,
                                                       bool requiredIdx);
        WireSlot doReducNoIdx(const std::function<Operable&(WireSlot& lhs, Operable* lidx,
                                                      WireSlot& rhs, Operable* ridx)>& cusLogic);
        std::pair<WireSlot, Operable&> doReducBinIdx(const std::function<Operable&(WireSlot& lhs, Operable* lidx,
                                                                             WireSlot& rhs, Operable* ridx)>& cusLogic);
        std::pair<WireSlot, OH> doReducOHIdx(const std::function<Operable&(WireSlot& lhs, Operable* lidx,
                                                                     WireSlot& rhs, Operable* ridx)>& cusLogic);

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
        TableSliceAgent operator[] (OH ohIdx);

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
