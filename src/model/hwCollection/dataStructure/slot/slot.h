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
        Slot(const std::vector<std::string>& fieldNames,
             const std::vector<int>&         fieldSizes
        ):
        _meta(fieldNames, fieldSizes){};

        explicit Slot(SlotMeta  meta):
        _meta(std::move(meta)){};

        virtual ~Slot() = default;

        FieldMeta& fieldAt(int idx){
            return _meta(idx);
        }

        HwFieldMeta& hwFieldAt(int idx){
            mfAssert(isValidIdx(idx), "get hw Field at " + std::to_string(idx) + " out of range");
            return _hwFieldMetas[idx];
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

        std::vector<AssignMeta*> genAssignMetaForAll(Slot& srcSlot, ASM_TYPE asmType){

            std::vector<AssignMeta*> resultCollector;
            for (int desIdx = 0; desIdx < srcSlot.getNumField(); desIdx++){
                auto [desOpr, desAsb] = hwFieldAt(desIdx);
                auto [srcOpr, srcAsb] = srcSlot.hwFieldAt(desIdx);

                AssignMeta* assMeta = genAssignMeta(*srcOpr, *desAsb, asmType);
                resultCollector.push_back(assMeta);
            }

            return resultCollector;
        }

        std::vector<AssignMeta*> genAssignMetaForAll(
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


                if (std::find(exceptIdxs.begin(),exceptIdxs.end(),srcIdx) != exceptIdxs.end()){
                    ///// encounter except list
                    continue;
                }

                auto [desOpr, desAsb] = hwFieldAt(desIdx);
                auto [srcOpr, srcAsb] = srcSlot.hwFieldAt(srcIdx);

                AssignMeta* assMeta = genAssignMeta(*srcOpr, *desAsb, asmType);
                resultCollector.push_back(assMeta);
            }

            return resultCollector;
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
            std::vector<AssignMeta*> resultCollector
            = genAssignMetaForAll(srcSlot, srcMatchIdxs, desMatchIdxs, exceptIdxs, asmType);

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

        AsmNode* genGrpAsmNode(
        const std::vector<AssignMeta*> & assignMetas,
        const std::vector<Operable*>   & preConditions,
        LOGIC_OP preConOp
        ){
            assert(assignMetas.size() == preConditions.size());
            auto* asmNode = new AsmNode(assignMetas);
            for (int idx = 0; idx < preConditions.size(); idx++){
                asmNode->addSpecificPreCondition(preConditions[idx], preConOp, idx);
            }
            return asmNode;
        }



        virtual void doGlobAsm(
            Slot& rhs,
            const std::vector<int>& srcMatchIdxs,
            const std::vector<int>& desMatchIdxs,
            const std::vector<int>& exceptIdxs,
            ASM_TYPE asmType){assert(false);};

        virtual void doGlobAsm(
            Operable& srcOpr,
            Operable& requiredIdx,
            ASM_TYPE  asmType
        ){assert(false);}

        virtual void doGlobAsm(
            AsmNode* asmNode
        ){assert(false);};


        /** block assignment */
        virtual void doBlockAsm(Slot& rhs, const std::vector<int>& exceptIdxs, ASM_TYPE asmType){
            auto [srcMatchIdxs, desMatchIdxs] = matchByName(rhs);
            doGlobAsm(rhs, srcMatchIdxs, desMatchIdxs, {}, asmType);
        }
        virtual void doBlockAsm(Slot& rhs, const std::vector<std::string>& exceptNames, ASM_TYPE asmType){
            std::vector<int> excpetIdxs = getIdxs(exceptNames);
            doBlockAsm(rhs, excpetIdxs, asmType);
        }
        virtual void doBlockAsm(Slot& rhs, ASM_TYPE asmType){
            doBlockAsm(rhs, std::vector<int>{}, asmType);
        }
        virtual void doBlockAsm(Operable& srcOpr, Operable& requiredIdx, ASM_TYPE asmType){
            doGlobAsm(srcOpr, requiredIdx, asmType);
        }

        virtual void doNonBlockAsm(Slot& rhs, const std::vector<int>& exceptIdxs, ASM_TYPE asmType){
            auto [srcMatchIdxs, desMatchIdxs] = matchByName(rhs);
            doGlobAsm(rhs, srcMatchIdxs, desMatchIdxs, {}, asmType);
        }
        virtual void doNonBlockAsm(Slot& rhs, const std::vector<std::string>& exceptNames, ASM_TYPE asmType){
            std::vector<int> excpetIdxs = getIdxs(exceptNames);
            doNonBlockAsm(rhs, excpetIdxs, asmType);
        }
        virtual void doNonBlockAsm(Slot& rhs, ASM_TYPE asmType){
            doNonBlockAsm(rhs, std::vector<int>{}, asmType);
        }

        virtual void doNonBlockAsm(Operable& srcOpr, Operable& requiredIdx, ASM_TYPE asmType){
            doGlobAsm(srcOpr, requiredIdx, asmType);
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

            bool isUsedAsDef = false;
            //// the target structure to update
            std::vector<UpdateEvent*>& updateEvents = resultWire->getUpdateMeta();

            for (int idx = 0; idx < _masterSlot.getNumField(); idx++){
                FieldMeta fieldMeta = _masterSlot.fieldAt(idx);
                ///// we do only the target port
                if (fieldMeta._size != targetWidth ){
                    continue;
                }
                ////// we require the first val as the default value
                Operable* activateCond = nullptr;
                int       assignPri    = DEFAULT_UE_PRI_MIN;
                if (!isUsedAsDef){
                    activateCond = &(_requiredIdx == fieldMeta._size);
                    assignPri    = DEFAULT_UE_PRI_USER;
                }
                ////// create update event
                auto resultUpEvent = new UpdateEvent({
                activateCond,
                nullptr,
                _masterSlot.hwFieldAt(idx)._opr,
                resultWire->getOperableSlice(),
                assignPri
                });
                updateEvents.push_back(resultUpEvent);
                isUsedAsDef = true;
            }

            /////// set default value

            return *resultWire;
        }


    };

}


#endif //SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_SLOT_H

