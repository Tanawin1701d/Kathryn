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

    static constexpr char ORDERED_USER_VALID_KW[] = "userValidCompare";
    static constexpr char ORDERED_SYSTEM_SEQ_OLD_KW[] = "systemInOldestSec";


    protected:
        SlotMeta _meta;
        std::vector<RegSlot*> _rows;

        struct ReducNode{
            WireSlot* slot = nullptr; Operable* idx{};
            void destroy() const{
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


        /**
         * get the static data
         */
        [[nodiscard]] SlotMeta getMeta() const;
                      RegSlot& getRefRow(int idx);
        [[nodiscard]] RegSlot  getClonedRow(int idx) const;
        [[nodiscard]] int      getNumRow() const;
                      int      getMaxCellWidth() const;

        /**
         *  build the reg slot for each row
         */
        void buildRows(SlotMeta& slotMeta, int amtRow, std::string prefixName);

        /**
         * check indexing size
         */
        bool isSufficientBinIdx(Operable& requiredIdx) const;
        bool isSufficientOHIdx(Operable& requiredIdx) const;
        bool isSufficientIdx(Operable& requiredIdx, bool isOH) const;
        int  getSufficientIdxSize(bool isOH) const;

        bool isValidIdx(int idx) const;
        bool checkValidRange(int start, int stop) const;

        Operable& createIdxMatchCond(Operable& requiredIdx, int rowIdx,bool isOH);



        /**
         * gen assign meta
         *
         */
        WireSlot genDynWireSlotBase(Operable& requiredIdx, bool isOneHotIdx); //// oneHotIdx will determine the the rowId by bit Idx in the requiredIdx
        WireSlot genDynWireSlotBiIdx(Operable& binIdx);
        WireSlot genDynWireSlotOHIdx(Operable& ohIdx);

        ////// this will asssign the slot
        void   doGlobAsm   (Slot& srcSlot   , Operable& requiredIdx, ASM_TYPE asmType, bool isOneHotIdx);
        void   doGlobAsm   (Operable& srcOpr, Operable& rowIdx, Operable& colIdx, ASM_TYPE asmType, bool isOneHotIdx);
        void   doGlobAsm   (ull       srcVal, Operable& rowIdx, Operable& colIdx, ASM_TYPE asmType, bool isOneHotIdx);
        ////// asmType is lock to
        Table& doGlobColAsm(int   colIdx              , ull assignVal = 0);
        Table& doGlobColAsm(const std::string& colName, ull assignVal = 0);


        void doCusLogic(std::function<void(RegSlot&, int rowIdx)>  cusLogic);

        ////// make resetEvent will do when glo
        Table& makeResetEvent    (ull   resetVal);
        Table& makeColResetEvent (int   colIdx              , ull resetVal = 0);
        Table& makeColResetEvent (const std::string& colName, ull resetVal = 0);


        /////// reduction operation

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

        ReducNode findMatchedOrdered(bool isNewest,
                                    const std::vector<ReducNode>& initReducNodes,
                                    bool requiredIdx);

        WireSlot* augmentForOrderedSearch(int rowIdx,
                                         Operable& OldestStartIndex,
                                         const std::function<Operable&(RegSlot& src)>& userValidFunc);

        std::pair<WireSlot, Operable&>
        findMBO_BIDX(
            bool isNewest, ///// otherwise it is oldest
            Operable& oldestStartIndex,
            const std::function<Operable&(RegSlot& src)>& userValidFunc);

        std::pair<WireSlot, OH>
        findMBO_OHIDX(
            bool isNewest,
            Operable& oldestStartIndex,
            const std::function<Operable&(RegSlot& src)>& userValidFunc);

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
        Table joinTableByRowInterleave(const Table& rhs);

        Table joinTableByCol(const Table& rhs);

        Table join(const Table& rhs,  int axis);

    };

}

#endif //KATHRYN_SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_TABLE_TABLE_H
