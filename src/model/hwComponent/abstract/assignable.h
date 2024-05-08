//
// Created by tanawin on 28/11/2566.
//

#ifndef KATHRYN_ASSIGNABLE_H
#define KATHRYN_ASSIGNABLE_H

#include <vector>
#include <algorithm>

#include "operable.h"
#include "model/hwComponent/abstract/operable.h"
#include "model/hwComponent/abstract/Slice.h"
#include "model/simIntf/modelSimEngineLegacy.h"
#include "updateEvent.h"

namespace kathryn{




    /** This is used to describe what and where to update that send to controller and let flow block determine*/
    enum ASM_TYPE{
        ASM_DIRECT = 0,
        ASM_EQ_DEPNODE = 1
    };
    struct AssignMeta{
        std::vector<UpdateEvent*>& updateEventsPool;
        Operable&                  valueToAssign;
        Slice                      desSlice;
        ASM_TYPE                   asmType;
        bool                       desSliceEqToEvent;
        AssignMeta(std::vector<UpdateEvent*>& u, Operable& v, Slice s, ASM_TYPE at, bool desSrc):
                                                                        updateEventsPool(u),
                                                                        valueToAssign(v),
                                                                        desSlice(s),
                                                                        asmType(at),
                                                                        desSliceEqToEvent(desSrc){}
    };
    /**
    * Assignable represent hardware component that can memorize logic value or
    *
    * */
    /** to make value when constant input is used*/
    Operable& getMatchAssignOperable(ull value, int size);

    struct AsmNode;
    class Assignable{
    protected:
        std::vector<UpdateEvent*> _updateMeta;
    public:

        explicit Assignable() = default;
        virtual ~Assignable(){
            for (auto eventPtr: _updateMeta){
                delete eventPtr;
            }
        }

        /** base function assign other operable to this operable*/
        virtual void doBlockAsm   (Operable& srcOpr, Slice desSlice) = 0;
        virtual void doNonBlockAsm(Operable& srcOpr, Slice desSlice) = 0;
        virtual void doGlobalAsm  (Operable& srcOpr, Slice desSlice, ASM_TYPE asmType){assert(false);} ///// typically, it is used in nest

        /** base function assign other operable to this operable
         * but do not communicate to controller, just give result assign meta to system*/
        virtual void doBlockAsm   (Operable& srcOpr,
                                   std::vector<AssignMeta*>& resultMetaCollector, ////// result to assign assign meta
                                   Slice  absSrcSlice,
                                   Slice  absDesSlice) = 0;

        virtual void doNonBlockAsm(Operable& srcOpr,
                                   std::vector<AssignMeta*>& resultMetaCollector,
                                   Slice  absSrcSlice,
                                   Slice  absDesSlice) = 0;

        /** global fucntion asign  other operable to this operable
         * but do not communicate to controller, just give result assign meta to system*/
        virtual void doGlobalAsm  (Operable& srcOpr,
                                   std::vector<AssignMeta*>& resultMetaCollector,
                                   Slice  absSrcSlice,
                                   Slice  absDesSlice,
                                   ASM_TYPE asmType);


        virtual void  assignSimValue(ull    b){assert(false);}
        virtual void  assignSimValue(ValRep b){assert(false);}


        virtual Slice getAssignSlice() = 0;

        /** update event management*/
        std::vector<UpdateEvent*>& getUpdateMeta(){ return _updateMeta; }

        void addUpdateMeta(UpdateEvent* event){_updateMeta.push_back(event);}

        /** generate update metas*/
        virtual AssignMeta* generateAssignMeta(Operable& srcValue, Slice desSlice, ASM_TYPE asmType){
            return new AssignMeta(_updateMeta, srcValue, desSlice, asmType, false);
        }

        /** generate the atomic node that is used to represent  state in the system*/
        AsmNode* generateBasicNode(Operable& srcOpr, Slice desSlice, ASM_TYPE asmType);

        /***
         *
         * simulation task
         *
         * */

        /////// assign value to val representation
        ////////////////// usually it is call from sim Interface
        /////// getFromCur means get value from current cycle or from back cycle
        void assignValRepCurCycle(ValRep& desValRep){

            for (auto curUpEvent : _updateMeta){
                assert(curUpEvent != nullptr);
                curUpEvent->trySimAll();
                curUpEvent->tryAssignValRep(desValRep);
            }

        }

        static bool upEventCmp(const UpdateEvent* lhs, const UpdateEvent* rhs){
            assert(lhs != nullptr);
            assert(rhs != nullptr);
            return (*lhs) < (*rhs);
        }

        void sortUpEventByPriority(){
            std::sort(_updateMeta.begin(), _updateMeta.end(), upEventCmp);
        }





    };

    template<typename RET_TYPE>
    class AssignOpr{

    private:
        RET_TYPE* _master = nullptr;
    protected:
        void setMaster(RET_TYPE* master){_master = master;}
        virtual Assignable* getAssignableFromAssignOpr() = 0;
    public:
        virtual RET_TYPE& operator <<= (Operable& b){
            Assignable* asb = getAssignableFromAssignOpr();
            Slice slc = asb->getAssignSlice();
            asb->doBlockAsm(b, slc);
            assert(_master != nullptr);
            return *_master;
        }
        virtual RET_TYPE& operator <<= (ull b){
            Assignable* asb = getAssignableFromAssignOpr();
            Slice       slc = asb->getAssignSlice();
            Operable&   rhs = getMatchAssignOperable(b, slc.getSize());
            asb->doBlockAsm(rhs, slc);
            assert(_master != nullptr);
            return *_master;
        }
        virtual void operatorEq (Operable& b){
            Assignable* asb = getAssignableFromAssignOpr();
            Slice slc = asb->getAssignSlice();
            asb->doNonBlockAsm(b, slc);
            assert(_master != nullptr);
        }
        virtual void operatorEq (ull b){
            Assignable *asb = getAssignableFromAssignOpr();
            if (getAssignMode() == AM_MOD) {
                Slice slc = asb->getAssignSlice();
                Operable &rhs = getMatchAssignOperable(b, slc.getSize());
                asb->doNonBlockAsm(rhs, slc);
            }else if(getAssignMode() == AM_SIM){
                asb->assignSimValue(b);
            }
            assert(_master != nullptr);
        }


    };

    template<typename RET_TYPE>
    class AssignCallbackFromAgent{

    private:
        RET_TYPE* _master = nullptr;
    protected:
        void setMaster(RET_TYPE* master){_master = master;}
        virtual Assignable* getAssignableFromAssignCallbacker() = 0;
    public:
        virtual void callBackBlockAssignFromAgent(Operable& b, Slice absSliceOfHost){
            getAssignableFromAssignCallbacker()->doBlockAsm(b, absSliceOfHost);
            assert(_master != nullptr);
        }
        virtual void callBackNonBlockAssignFromAgent(Operable& b, Slice absSliceOfHost){
            getAssignableFromAssignCallbacker()->doNonBlockAsm(b, absSliceOfHost);
            assert(_master != nullptr);
        }
        virtual void      callBackBlockAssignFromAgent(Operable& srcOpr,
                                                       std::vector<AssignMeta*>& resultMetaCollector,
                                                       Slice  absSrcSlice,
                                                       Slice  absDesSlice){
            getAssignableFromAssignCallbacker()->doBlockAsm(srcOpr, resultMetaCollector, absSrcSlice, absDesSlice);
            assert(_master != nullptr);
        }
        virtual void      callBackNonBlockAssignFromAgent(Operable& srcOpr,
                                                          std::vector<AssignMeta*>& resultMetaCollector,
                                                          Slice  absSrcSlice,
                                                          Slice  absDesSlice){
            getAssignableFromAssignCallbacker()->doNonBlockAsm(srcOpr, resultMetaCollector, absSrcSlice, absDesSlice);
            assert(_master != nullptr);
        }
    };




}

#endif //KATHRYN_ASSIGNABLE_H
