//
// Created by tanawin on 27/11/25.
//

#ifndef MODEL_HWCOMPONENT_ABSTACT_ASSMETA_H
#define MODEL_HWCOMPONENT_ABSTACT_ASSMETA_H

#include "updateEvent.h"
#include "model/flowBlock/abstract/nodes/asmNode.h"

namespace kathryn{

    /** This is used to describe what and where to update that send to controller and let flow block determine*/
    enum ASM_TYPE{
        ASM_DIRECT = 0,
        ASM_EQ_DEPNODE = 1
    };

    struct AssignMeta{
        static inline ull ASSIGN_CNT = 0;
        UpdatePool&           eventPool;
        UpdateEventBasic*     inputElement      = nullptr; //// it can be null if assignMeta is complex condition
        UpdateEventBase*      preUpdateElement  = nullptr; //// the event want to beb
        ASM_TYPE              asmType;

        AssignMeta(UpdatePool& u,
                   UpdateEventBasic* v,
                   ASM_TYPE at):
                    eventPool(u),
                    inputElement(v),
                    preUpdateElement(v),
                    asmType(at){
            preUpdateElement->setSubPriority(ASSIGN_CNT);
            ASSIGN_CNT++;
        }

        AssignMeta(UpdateEventBase* v,
                   UpdatePool& u,
                   ASM_TYPE at,
                   ull curAssignCnt):
                    eventPool(u),
                    inputElement(nullptr),
                    preUpdateElement(v),
                    asmType(at){
            preUpdateElement->setSubPriority(curAssignCnt);
        }

        ///// it is used to tell that two assignment is joinable into the generated (cpp or verilog block)
        [[nodiscard]]
        bool isJoinable(const AssignMeta& rhs) const{
            assert(preUpdateElement != nullptr);
            return ((&eventPool) == (&rhs.eventPool)) &&
                   (asmType == rhs.asmType) &&
                   (preUpdateElement->isJoinable(*rhs.getCurrentEvent()));
        }

        void finalUpdate(){
            eventPool.addUpdateEvent(preUpdateElement);
        }

        UpdatePool*     getEventPool(){return &eventPool;}
        [[nodiscard]]
        UpdateEventBase* getCurrentEvent() const{return preUpdateElement;}
        ASM_TYPE         getAsmType() const{return asmType;}
        ull              getCurAssignCnt() const{assert(preUpdateElement != nullptr); return preUpdateElement->getSubPriority();}

        void             setNewEditingEvent(UpdateEventBase* event){
            assert(event != nullptr);
            preUpdateElement = event;
        }

        void addSpecificPreCondition(Operable* cond){
            assert(cond != nullptr);
            assert(preUpdateElement != nullptr);
            preUpdateElement = createUEHelper(cond, nullptr, preUpdateElement);

        }

        AssignMeta* mux(AssignMeta* right, Operable* selectLeft){
            UpdateEventCond* newEvent  = createMuxUEHelper(preUpdateElement, right->preUpdateElement, selectLeft);
            AssignMeta* newAssMeta = new AssignMeta(newEvent, eventPool, asmType, preUpdateElement->getSubPriority());
            return newAssMeta;

        }

    };

    struct ClassAssignMeta{
        //// this is the group of assign meta, the order is supposed to be inorder from user input
        /// the assign meta belongs to assignMeta
        std::vector<AssignMeta*> assignMetas;

        ClassAssignMeta(){}

        ClassAssignMeta(AssignMeta* assignMeta){
            assignMetas.push_back(assignMeta);
        }

        //////// all assignment Metas is supposed to be deleted because this class eventually creates
        //////// new one.
        virtual ~ClassAssignMeta(){
            for (auto* assignMeta: assignMetas){
                delete assignMeta;
            }
        }

        bool isEmpty(){return assignMetas.empty();}

        int getSize() const{return assignMetas.size();}

        AssignMeta* getSampleAssignMeta(){
            assert(!assignMetas.empty());
            return assignMetas[0];
        }

        bool isJoinable(AssignMeta* sample){
            return getSampleAssignMeta()->isJoinable(*sample);
        }

        bool isJoinable(ClassAssignMeta* sample){
            assert(sample != nullptr);
            AssignMeta* ourSample  = getSampleAssignMeta();
            AssignMeta* yourSample = sample->getSampleAssignMeta();

            return ourSample->isJoinable(*yourSample);
        }

        void addAssignMeta(AssignMeta* assignMeta){
            if (!isEmpty()){
                assert(isJoinable(assignMeta));
            }
            assignMetas.push_back(assignMeta);
        }

        ////// this is the goal of the system
        UpdateEventGrp* createEventGrp(){
            auto* updateGrp = new UpdateEventGrp();
            for (auto* assignMeta: assignMetas){
                updateGrp->addSubStmt(assignMeta->preUpdateElement);
            }
            return updateGrp;
        }

    };

    inline void tryAddOrCreateAsmMeta(
        AsmNode* asmNode,
        std::vector<ClassAssignMeta*>& assignMetas
    ){

        assert(asmNode != nullptr);
        for (AssignMeta* asmMeta: asmNode->getAssignMetas()){
            bool found = false;
            for (ClassAssignMeta* classAsm: assignMetas){
                if (classAsm->isJoinable(asmMeta)){
                    classAsm->addAssignMeta(asmMeta);
                    found = true;
                    break;
                }
            }
            if (!found){
                assignMetas.push_back(new ClassAssignMeta(asmMeta));
            }
        }
        asmNode->transferOutAssignMetaOwnership();

    }

    //// std::vector<ClassAssignMeta*> classifyAss(std::vector<AssignMeta*>& baseMetas);

}


#endif //KATHRYN_ASS_META_MNG_H
