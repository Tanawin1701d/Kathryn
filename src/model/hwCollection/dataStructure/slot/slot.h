//
// Created by tanawin on 14/9/25.
//

#ifndef SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_SLOT_H
#define SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_SLOT_H

#include <utility>

#include "slotMeta.h"
#include "model/controller/controller.h"
#include "model/flowBlock/abstract/nodes/asmNode.h"
#include "model/hwComponent/abstract/logicComp.h"
#include "model/hwComponent/register/register.h"

namespace kathryn{

    struct HwFieldMeta{
        Operable* _opr   = nullptr;
        Assignable* _asb = nullptr;
    };

    class Slot{

    protected:
        SlotMeta _meta;
        std::vector<HwFieldMeta> _hwFieldMetas;

        HwFieldMeta& hwFieldAt(int idx){
            mfAssert(isValidIdx(idx), "get hw Field at " + std::to_string(idx) + " out of range");
            return _hwFieldMetas[idx];
        }

    public:
        Slot(const std::vector<std::string>& fieldNames,
             const std::vector<int>&         fieldSizes
        ):
        _meta(fieldNames, fieldSizes){};

        explicit Slot(SlotMeta  meta):
        _meta(std::move(meta)){};

        ~Slot();

        int getIdx(std::string fieldName) const{
            return _meta.getIdx(std::move(fieldName));
        }

        std::vector<int> getIdxs(std::vector<std::string> fieldNames) const{
            return _meta.getIdxs(std::move(fieldNames));
        }

        int getNumField() const{
            return _meta.getNumField();
        }

        bool isValidIdx(int idx){
            return _meta.isValidIdx(idx);
        }
        bool checkValidRange(int start, int stop){
            return _meta.isValidRange(start, stop);
        }

        std::pair<std::vector<int>, std::vector<int>> matchByName(Slot& rhs){
            return _meta.matchByName(rhs._meta);
        }

        //////// assign system
        AssignMeta* genAssignMeta(Operable& srcOpr, Assignable& desAsb,
                                 ASM_TYPE asmType){

            mfAssert(desAsb.getAssignSlice().getSize() <= srcOpr.getOperableSlice().getSize(),
            "the size of srcOpr is too small to assign to desAsb");
            Slice srcSlice = srcOpr.getOperableSlice();
            Slice desSlice = desAsb.getAssignSlice();
            auto* assMeta = new AssignMeta(
                desAsb.getUpdateMeta(),
                srcOpr,
                desSlice.getMatchSizeSubSlice(srcSlice),
                asmType
            );
            return assMeta;
        }

        std::vector<AssignMeta*> genAssignMetaForAll(Operable& srcOpr, ASM_TYPE asmType){
            std::vector<AssignMeta*> resultCollector;

            for (int desIdx = 0; desIdx < getNumField(); desIdx++){
                auto [desOpr, desAsb] = hwFieldAt(desIdx);

                AssignMeta* assMeta = genAssignMeta(srcOpr, *desAsb, asmType);
                resultCollector.push_back(assMeta);
            }
            return resultCollector;
        }


        AsmNode* genGrpAsmNode (
            Slot& srcSlot,
            const std::vector<int>& srcMatchIdxs,
            const std::vector<int>& desMatchIdxs,
            const std::vector<int>& exceptIdxs,
            ASM_TYPE asmType){
            std::vector<AssignMeta*> resultCollector;

            mfAssert(srcMatchIdxs.size() == desMatchIdxs.size(),
                "size of srcMatchIdxs and desMatchIdxs must be equal");

            for (int idx = 0; idx < srcMatchIdxs.size(); idx++){

                int srcIdx    = srcMatchIdxs[idx];
                int desIdx    = desMatchIdxs[idx];
                int exceptIdx = exceptIdxs[idx];


                if (std::find(exceptIdxs.begin(),exceptIdxs.end(),srcIdx) != exceptIdxs.end()){
                    ///// encounter except list
                    continue;
                }

                auto [desOpr, desAsb] = hwFieldAt(desIdx);
                auto [srcOpr, srcAsb] = srcSlot.hwFieldAt(srcIdx);

                AssignMeta* assMeta = genAssignMeta(*srcOpr, *desAsb, asmType);
                resultCollector.push_back(assMeta);
            }

            auto* asmNode = new AsmNode(resultCollector);

            return asmNode;

        }

        AsmNode* genGrpAsmNode(
            Operable& srcOpr,
            Operable& requiredIdx,
            ASM_TYPE  asmType
        ){

            std::vector<AssignMeta*> resultCollector = genAssignMetaForAll(srcOpr, asmType);

            auto* asmNode = new AsmNode(resultCollector);

            for (int desIdx = 0; desIdx < getNumField(); desIdx++){
                Operable* idxCheckCond = &(requiredIdx == desIdx);
                asmNode->addSpecificPreCondition(idxCheckCond, BITWISE_AND, desIdx);
            }
            return asmNode;
        }



        virtual void doGlobAsm(
            Slot& rhs,
            const std::vector<int>& srcMatchIdxs,
            const std::vector<int>& desMatchIdxs,
            const std::vector<int>& exceptIdxs,
            ASM_TYPE asmType) = 0;

        virtual void doGlobAsm(
            Operable& srcOpr,
            Operable& requiredIdx,
            ASM_TYPE  asmType
        ) = 0;


        /** block assignment */
        virtual void doBlockAsm(Slot& rhs, const std::vector<int>& exceptIdxs){
            auto [srcMatchIdxs, desMatchIdxs] = matchByName(rhs);
            doGlobAsm(rhs, srcMatchIdxs, desMatchIdxs, {}, ASM_DIRECT);
        }
        virtual void doBlockAsm(Slot& rhs, const std::vector<std::string>& exceptNames){
            std::vector<int> excpetIdxs = getIdxs(exceptNames);
            doBlockAsm(rhs, excpetIdxs);
        }
        virtual void doBlockAsm(Slot& rhs){
            doBlockAsm(rhs, std::vector<int>{});
        }
        virtual void doBlockAsm(Operable& srcOpr, Operable& requiredIdx){
            doGlobAsm(srcOpr, requiredIdx, ASM_DIRECT);
        }

        virtual void doNonBlockAsm(Slot& rhs, const std::vector<int>& exceptIdxs){
            auto [srcMatchIdxs, desMatchIdxs] = matchByName(rhs);
            doGlobAsm(rhs, srcMatchIdxs, desMatchIdxs, {}, ASM_EQ_DEPNODE);
        }
        virtual void doNonBlockAsm(Slot& rhs, const std::vector<std::string>& exceptNames){
            std::vector<int> excpetIdxs = getIdxs(exceptNames);
            doNonBlockAsm(rhs, excpetIdxs);
        }
        virtual void doNonBlockAsm(Slot& rhs){
            doNonBlockAsm(rhs, std::vector<int>{});
        }

        virtual void doNonBlockAsm(Operable& srcOpr, Operable& requiredIdx){
            doGlobAsm(srcOpr, requiredIdx, ASM_EQ_DEPNODE);
        }



    };

}


#endif //SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_SLOT_H

