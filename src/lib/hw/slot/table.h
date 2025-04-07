//
// Created by tanawin on 17/1/2025.
//


#include <utility>

#include "rowMeta.h"
#include "slot.h"
#include "model/controller/controller.h"

#ifndef src_lib_hw_slot_TABLE_H
#define src_lib_hw_slot_TABLE_H


#define scmp [&](Operable* lhsIdx, Slot lhsSlot, Operable* rhsIdx, Slot rhsSlot)->Operable&


namespace kathryn{

    struct Candidate{
        Slot      detLogic; //// determine logic
        Operable* detIdx = nullptr; //// it is optional
        Operable* valid  = nullptr; //// it is optional
    };

    class Table{
    public:
        std::string           _tableName = "UNNAME_TABLE";
        RowMeta               _meta;
        const int             _identWidth;
        const int             _amtSize;
        std::vector<RegSlot*> _hwSlots; //// for collect hardware

    /** constructor*/
    Table(std::string tableName, RowMeta  meta, int identWidth = -1):
        _tableName (std::move(tableName)),
        _meta      (std::move(meta)),
        _identWidth(identWidth),
        _amtSize   (1 << identWidth){

        mfAssert(_identWidth >= 0, "table ident width size out of range");
        std::cout << "generate " << _amtSize << " slots" << std::endl;
        for (int i = 0; i < _amtSize; i++){
            auto* regSlot = new RegSlot(_meta, i, "_onTable_" + _tableName);
            _hwSlots.push_back(regSlot);
        }
    }

    Table(std::string tableName,
          const std::vector<std::string>& fieldNames,
          const std::vector<int>&         fieldSizes,
          int identWidth = 0):
    Table(std::move(tableName), RowMeta(fieldNames, fieldSizes), identWidth){}


    virtual ~Table(){
        for (const RegSlot* regSlot: _hwSlots){ delete regSlot;}
    }

    ///// reqIdx used that Output require the index of table or not
    ///  matchCondition must return single bit whether select lhs (true) or rhs (false)

    Candidate buildCmpSearchLogic(const RowMeta& rowMeta,        //////// the field that designer concern
                                  bool  reqSlotIdx,              //////// request build idx for each comparison
                                  const std::function<Operable&(Operable* lhsIdx,
                                                                Slot      lhsSlot,
                                                                Operable* rhsIdx,
                                                                Slot      rhsSlot)>& cmpCon);

    Candidate buildMinMaxLogic(int fieldIdx, bool reqIdx, bool isMin);
    Candidate buildMinMaxLogic(const std::string& fieldName, bool reqIdx, bool isMin);
    std::vector<Candidate>
              buildFindMultiLogic(const std::string& fieldName, bool reqIdx, bool freeIsHigh,
                                  int amt, bool startFromHead = true);

    ///// buildCirSearchLogic int will search circularily start from
    ///// [startIdx, startIdx+1] to [(startIdx-1)%size, startIdx]
    Candidate buildCirSearchLogic(const RowMeta& rowMeta,
                                  bool  reqSlotIdx,
                                  int   startIdx,
                                  bool  selectLeft,
                                  Operable* defIdx,
                                  const std::function<Operable&(Operable* lhsIdx,
                                                                Slot      lhsSlot,
                                                                Operable* rhsIdx,
                                                                Slot      rhsSlot)>& cmpCon);



    /////// designer put the `matchCon` function variable to get slot that match designers' requirement
    Slot buildGetLogic(const std::function<Operable&(int idx, Slot examSlot)>& matchCon);

    Slot buildGetLogic(Operable& searchIdx);

    RegSlot& get(int constIdx);

    /////// return single wire that used to store signal
    /// system to store

    ///Wire& buildSetLogic(Slot& slot, Operable& reqIdx);

    Table& assign(const Slot& slot, Operable& reqIdx, bool isBlockAsm);
    Table& assign(const std::vector<std::string>& fieldName,
                  const std::vector<Operable*>& oprs,
                  Operable& reqIdx, bool isBlockAsm);
    Table& assign(const std::string& fieldName,
                  Operable*   opr,
                  Operable& reqIdx, bool isBlockAsm);

    Table& assign(const Slot& slot,
                  const std::function<Operable&(int idx, Slot examSlot)>& matchCon,
                  bool isBlockAsm);
    Table& assign(const std::vector<std::string>& fieldName,
                  const std::vector<Operable*>& oprs,
                  const std::function<Operable&(int idx, Slot examSlot)>& matchCon,
                  bool isBlockAsm);
    Table& assign(const std::string& fieldName,
                  Operable* opr,
                  const std::function<Operable&(int idx, Slot examSlot)>& matchCon,
                  bool isBlockAsm);


    };
}




#endif //__src_lib_hw_slot_TABLE_H
