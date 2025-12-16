//
// Created by tanawin on 14/9/25.
//

#ifndef SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_SLOT_H
#define SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_SLOT_H

#include <utility>

#include "slotMeta.h"
#include "model/flowBlock/abstract/nodes/asmNode.h"
#include "model/hwComponent/abstract/logicComp.h"
#include "model/hwComponent/register/register.h"
#include "model/hwCollection/dataStructure/indexing/index.h"

namespace kathryn{

    struct HwFieldMeta{
        Operable* _opr   = nullptr;
        Assignable* _asb = nullptr;
    };

    class Slot{

    protected:
        SlotMeta _meta;
        std::vector<HwFieldMeta> _hwFieldMetas;

    public:

        Slot()= default;

        Slot(const std::vector<std::string>& fieldNames,
             const std::vector<int>&         fieldSizes
        ):
        _meta(fieldNames, fieldSizes){}

        explicit Slot(SlotMeta  meta):
        _meta(std::move(meta)){}

        virtual ~Slot() = default;

        virtual void initHwStructure(const std::string& prefixName){ assert(false);}

        ////////// get static data


        [[nodiscard]] bool isThereField(const std::string& fieldName) const{
            return getIdx(fieldName) != -1;
        }

        FieldMeta& fieldRefAt(int idx){
            return _meta(idx);
        }

        [[nodiscard]]
        FieldMeta fieldAt(int idx) const{
            return _meta.getCopyField(idx);
        }

        HwFieldMeta& hwFieldRefAt(int idx){
            mfAssert(isValidIdx(idx), "get hw Field at " + std::to_string(idx) + " out of range");
            return _hwFieldMetas[idx];
        }

        HwFieldMeta& hwFieldRefAt(const std::string& fieldName){
            int fieldId = getIdx(fieldName);
            mfAssert(isValidIdx(fieldId), "field name " + fieldName + " not found");
            return hwFieldRefAt(fieldId);
        }

        HwFieldMeta hwFieldAt(int idx) const{
            mfAssert(isValidIdx(idx), "get hw Field at " + std::to_string(idx) + " out of range");
            return _hwFieldMetas[idx];
        }

        HwFieldMeta hwFieldAt(const std::string& fieldName) const{
            int fieldId = getIdx(fieldName);
            mfAssert(isValidIdx(fieldId), "field name " + fieldName + " not found");
            return hwFieldAt(fieldId);
        }

        void addHwFieldMeta(HwFieldMeta hwFieldMeta){
            _hwFieldMetas.push_back(std::move(hwFieldMeta));
        }

        SlotMeta getMeta() const{
            return _meta;
        }

        int getMaxBitWidth(){
            return _meta.getMaxBitWidth();
        }

        int getIdx(std::string fieldName) const{
            return _meta.getIdx(std::move(fieldName));
        }

        std::vector<int> getIdxs(std::vector<std::string> fieldNames) const{
            return _meta.getIdxs(std::move(fieldNames));
        }

        int getNumField() const{
            return _meta.getNumField();
        }

        bool isSufficientIdx(int idxSize){
            return _meta.isSufficientIdx(idxSize);
        }

        bool isValidIdx(int idx) const{
            return _meta.isValidIdx(idx);
        }
        bool checkValidRange(int start, int stop){
            return _meta.isValidRange(start, stop);
        }

        std::pair<std::vector<int>, std::vector<int>> matchByName(const Slot& rhs){
            return _meta.matchByName(rhs._meta);
        }


        //////// assign system
        AssignMeta* genAssignMeta(Operable& srcOpr, Assignable& desAsb,
                                 ASM_TYPE asmType) const{

            mfAssert(desAsb.getAssignSlice().getSize() <= srcOpr.getOperableSlice().getSize(),
            "the size of srcOpr is too small to assign to desAsb");
            Slice srcSlice = srcOpr.getOperableSlice();
            Slice desSlice = desAsb.getAssignSlice();

            UpdateEventBasic* ueb = createUEHelper(
                &srcOpr,
                desSlice.getMatchSizeSubSlice(srcSlice),
                -1,
                desAsb.getCurAssignClkMode(),
                true
            );

            AssignMeta* assMeta = new AssignMeta(desAsb.getUpdateMeta(),
                                                 ueb,
                                                 asmType);
            return assMeta;
        }

        AssignMeta* genAssignMeta(Operable& srcOpr, int fieldIdx,
                                  ASM_TYPE asmType) const{
            auto [desOpr, desAsb] = hwFieldAt(fieldIdx);
            return genAssignMeta(srcOpr, *desAsb, asmType);
        }

        std::vector<AssignMeta*> genAssignMetaForAll(const Slot& srcSlot, ASM_TYPE asmType) const{
            assert(getNumField() == srcSlot.getNumField());
            std::vector<AssignMeta*> resultCollector;
            for (int desIdx = 0; desIdx < srcSlot.getNumField(); desIdx++){
                auto [desOpr, desAsb] = hwFieldAt(desIdx);
                auto [srcOpr, srcAsb] = srcSlot.hwFieldAt(desIdx);

                AssignMeta* assMeta = genAssignMeta(*srcOpr, *desAsb, asmType);
                resultCollector.push_back(assMeta);
            }

            return resultCollector;
        }

        std::vector<AssignMeta*> genAssignMetaForAll(std::vector<Operable*> srcOperables, ASM_TYPE asmType) const{
            assert(getNumField() == srcOperables.size());
            std::vector<AssignMeta*> resultCollector;
            for (int desIdx = 0; desIdx < getNumField(); desIdx++){
                auto [desOpr, desAsb] = hwFieldAt(desIdx);
                Operable* srcOpr = srcOperables[desIdx];
                assert(srcOpr != nullptr);
                AssignMeta* assMeta = genAssignMeta(*srcOpr, *desAsb, asmType);
                resultCollector.push_back(assMeta);
            }

            return resultCollector;
        }

        std::vector<AssignMeta*> genAssignMetaForAll(
            const Slot& srcSlot,
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


                if (std::find(exceptIdxs.begin(),exceptIdxs.end(),srcIdx) != exceptIdxs.end()){
                    ///// encounter except list
                    continue;
                }

                auto [desOpr, desAsb] = hwFieldRefAt(desIdx);
                auto [srcOpr, srcAsb] = srcSlot.hwFieldAt(srcIdx);

                AssignMeta* assMeta = genAssignMeta(*srcOpr, *desAsb, asmType);
                resultCollector.push_back(assMeta);
            }

            return resultCollector;
        }

        std::vector<AssignMeta*> genAssignMetaForAll(Operable& srcOpr, ASM_TYPE asmType){
            std::vector<AssignMeta*> resultCollector;

            for (int desIdx = 0; desIdx < getNumField(); desIdx++){
                auto [desOpr, desAsb] = hwFieldRefAt(desIdx);

                AssignMeta* assMeta = genAssignMeta(srcOpr, *desAsb, asmType);
                resultCollector.push_back(assMeta);
            }
            return resultCollector;
        }


        AsmNode* genGrpAsmNode (
            const Slot& srcSlot,
            const std::vector<int>& srcMatchIdxs,
            const std::vector<int>& desMatchIdxs,
            const std::vector<int>& exceptIdxs,
            ASM_TYPE asmType){
            std::vector<AssignMeta*> resultCollector
            = genAssignMetaForAll(srcSlot, srcMatchIdxs, desMatchIdxs, exceptIdxs, asmType);
            auto* asmNode = new AsmNode(resultCollector);
            return asmNode;

        }

        /** gen the group the asm node for all matched*/
        AsmNode* genGrpAsmNode(const Slot& srcSlot, ASM_TYPE asmType){

            std::vector<AssignMeta*> resultCollector = genAssignMetaForAll(srcSlot, asmType);
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
                asmNode->addSpecificPreCondition(idxCheckCond, desIdx);
            }
            return asmNode;
        }

        static AsmNode* genGrpAsmNode(
        const std::vector<AssignMeta*> & assignMetas,
        const std::vector<Operable*>   & preConditions
        ){
            assert(assignMetas.size() == preConditions.size());
            auto* asmNode = new AsmNode(assignMetas);
            for (int idx = 0; idx < preConditions.size(); idx++){
                asmNode->addSpecificPreCondition(preConditions[idx], idx);
            }
            return asmNode;
        }

        static AsmNode* genGrpAsmNode(
        const std::vector<AssignMeta*> & assignMetas){
            auto* asmNode = new AsmNode(assignMetas);
            return asmNode;
        }





        virtual void doGlobAsm(
            const Slot& rhs,
            const std::vector<int>& srcMatchIdxs,
            const std::vector<int>& desMatchIdxs,
            const std::vector<int>& exceptIdxs,
            ASM_TYPE asmType){assert(false);}

        virtual void doGlobAsm(
            Operable& srcOpr,
            Operable& requiredIdx,
            ASM_TYPE  asmType
        ){assert(false);}

        virtual void doGlobAsm(
            AsmNode* asmNode
        ){assert(false);}


        /** block assignment */

        //////// static assign
        virtual void doBlockAsm(const Slot& rhs, const std::vector<int>& exceptIdxs, ASM_TYPE asmType){
            auto [srcMatchIdxs, desMatchIdxs] = matchByName(rhs);
            doGlobAsm(rhs, srcMatchIdxs, desMatchIdxs, {}, asmType);
        }
        virtual void doBlockAsm(const Slot& rhs, const std::vector<std::string>& exceptNames, ASM_TYPE asmType){
            std::vector<int> excpetIdxs = getIdxs(exceptNames);
            doBlockAsm(rhs, excpetIdxs, asmType);
        }
        virtual void doBlockAsm(const Slot& rhs, ASM_TYPE asmType){
            doBlockAsm(rhs, std::vector<int>{}, asmType);
        }
        //////// dynamic assign
        virtual void doBlockAsm(Operable& srcOpr, Operable& requiredIdx, ASM_TYPE asmType){
            doGlobAsm(srcOpr, requiredIdx, asmType);
        }

        virtual void doBlockAsm(ull srcVal, Operable& requiredIdx, ASM_TYPE asmType){
            Operable& mySrcOpr = getMatchAssignOperable(srcVal, getMaxBitWidth());
            doBlockAsm(mySrcOpr, requiredIdx, asmType);
        }
        /** non block assignment */

        //////// static assign
        virtual void doNonBlockAsm(const Slot& rhs, const std::vector<int>& exceptIdxs, ASM_TYPE asmType){
            auto [srcMatchIdxs, desMatchIdxs] = matchByName(rhs);
            doGlobAsm(rhs, srcMatchIdxs, desMatchIdxs, {}, asmType);
        }
        virtual void doNonBlockAsm(const Slot& rhs, const std::vector<std::string>& exceptNames, ASM_TYPE asmType){
            std::vector<int> excpetIdxs = getIdxs(exceptNames);
            doNonBlockAsm(rhs, excpetIdxs, asmType);
        }
        virtual void doNonBlockAsm(const Slot& rhs, ASM_TYPE asmType){
            doNonBlockAsm(rhs, std::vector<int>{}, asmType);
        }

        //////// dynamic assign

        virtual void doNonBlockAsm(Operable& srcOpr, Operable& requiredIdx, ASM_TYPE asmType){
            doGlobAsm(srcOpr, requiredIdx, asmType);
        }

        virtual void doNonBlockAsm(ull srcVal, Operable& requiredIdx, ASM_TYPE asmType){
            Operable& mySrcOpr = getMatchAssignOperable(srcVal, getMaxBitWidth());
            doNonBlockAsm(mySrcOpr, requiredIdx, asmType);
        }

    };

    class SlotDynSliceAgent{
    protected:
        Slot&     _masterSlot;
        Operable& _requiredIdx;


    public:

        explicit SlotDynSliceAgent(Slot& master, Operable& requiredIdx):
        _masterSlot(master),
        _requiredIdx(requiredIdx)
        {}

        Operable& v(){

            int targetWidth = _masterSlot.getMaxBitWidth();

            Wire* resultWire = &makeOprWire("slotSlice", targetWidth);

            bool isUsedAsDef = true;
            //// the target structure to update
            UpdatePool& updatePool = resultWire->getUpdateMeta();

            for (int idx = 0; idx < _masterSlot.getNumField(); idx++){
                FieldMeta fieldMeta = _masterSlot.fieldRefAt(idx);
                ///// we do only the target port
                if (fieldMeta._size != targetWidth ){
                    continue;
                }
                ////// we require the first val as the default value
                Operable* activateCond = nullptr;
                int       assignPri    = DEFAULT_UE_PRI_MIN;
                if (!isUsedAsDef){
                    activateCond = &(_requiredIdx == idx);
                    assignPri    = DEFAULT_UE_PRI_USER;
                }
                ////// create update event
                UpdateEventBase* resultUpEvent = createUEHelper(
                activateCond,
                nullptr,
                _masterSlot.hwFieldRefAt(idx)._opr,
                resultWire->getOperableSlice(),
                assignPri,
                CM_CLK_FREE,
                false
                );

                // UpdateEventBasic* ueb = new
                // UpdateEventBasic(_masterSlot.hwFieldRefAt(idx)._opr,
                //     resultWire->getOperableSlice(), assignPri,CM_CLK_FREE);
                //
                // UpdateEventCond* uec = new UpdateEventCond();
                // uec->addSubStmt(activateCond, ueb);

                updatePool.addUpdateEvent(resultUpEvent);
                isUsedAsDef = false;
            }

            /////// set default value

            return *resultWire;
        }


    };

}


#endif //SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_SLOT_H

