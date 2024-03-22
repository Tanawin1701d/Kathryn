//
// Created by tanawin on 2/12/2566.
//

#ifndef KATHRYN_SLICABLE_H
#define KATHRYN_SLICABLE_H

#include "model/hwComponent/abstract/assignable.h"
#include "model/hwComponent/abstract/operable.h"
#include "model/debugger/modelDebugger.h"

namespace kathryn {

    template<typename T>
    class SliceAgent;

    template<typename T>
    class Slicable{

    private:
        std::vector<SliceAgent<T>*> agentHolders;/// it is used to hold sliceAgent to prevent deletation
        Slice _absSlice{}; /// it is absolute slice
    public:

        explicit Slicable(Slice absSlice) : _absSlice(absSlice){}
        virtual ~Slicable(){
            for (auto agent: agentHolders){
                delete agent;
            }
        }

        virtual SliceAgent<T>& operator() (int start, int stop) = 0;
        virtual SliceAgent<T>& operator() (int idx) = 0;

        /**this is used when Slicable of normal hwtype*/
        void addAgentHolder(SliceAgent<T>* agent){
            agentHolders.push_back(agent);
        }
        /** start and stop is */
        Slice getAbsSubSlice(int start, int stop, Slice oldSlice){
            return oldSlice.getSubSlice({start, stop});
        }
        Slice getSlice() const { return _absSlice; }
        void  setSlice(Slice slc) {_absSlice =  slc;}

    };

    /**key point is make the agent that transparent while routing
     * assignable for updateMeta in assignable in agent is not used*/
    template<typename T>
    class SliceAgent : public AssignOpr<SliceAgent<T>>,public Assignable,
                       public Operable, public Slicable<T> {

    private:
        T* _master;
    public:
        SliceAgent(T* master, Slice slc) : _master(master), Slicable<T>(slc) {
            _master->Slicable<T>::addAgentHolder(this);
            AssignOpr<SliceAgent<T>>::setMaster(this);
        };

        Operable* castToOperable(){
            return static_cast<Operable*>(this);
        }

        /** slicable overload*/

        SliceAgent<T>& operator() (int start, int stop) override{
            auto ret =  new SliceAgent<T>(_master,
                                          Slicable<T>::getAbsSubSlice(start, stop, getOperableSlice())
                    );
            return *ret;
        }

        SliceAgent<T>& operator() (int idx) override{
            auto ret =  new SliceAgent<T>(_master,
                                          Slicable<T>::getAbsSubSlice(idx, idx + 1, getOperableSlice())
                    );
            return *ret;
        }

        /** override assignable*/
        void doBlockAsm(Operable& srcOpr, Slice desSlice) override {
            mfAssert(getAssignMode() == AM_MOD, "agent can use operator <<= only in MD mode");
            assert(desSlice.getSize() <= Slicable<T>::getSlice().getSize());
            assert(desSlice.stop      <= Slicable<T>::getSlice().stop);
            _master->callBackBlockAssignFromAgent(srcOpr,desSlice);
        }

        void doNonBlockAsm(Operable& srcOpr, Slice desSlice) override{
            mfAssert(getAssignMode() == AM_MOD, "agent can use operator <<= only in MD mode");
            assert(desSlice.getSize() <= Slicable<T>::getSlice().getSize());
            assert(desSlice.stop      <= Slicable<T>::getSlice().stop);
            _master->callBackNonBlockAssignFromAgent(srcOpr,desSlice);
        }

        void doBlockAsm(Operable& srcOpr,
                        std::vector<AssignMeta*>& resultMetaCollector,
                        Slice  absSrcSlice,
                        Slice  absDesSlice) override{
            _master->callBackBlockAssignFromAgent(
                    srcOpr,resultMetaCollector,
                    absSrcSlice,absDesSlice
            );
        }

        void doNonBlockAsm(Operable& srcOpr,
                           std::vector<AssignMeta*>& resultMetaCollector,
                           Slice  absSrcSlice,
                           Slice  absDesSlice) override{
            _master->callBackNonBlockAssignFromAgent(
                    srcOpr,resultMetaCollector,
                    absSrcSlice,absDesSlice
            );
        }

        SliceAgent<T>& operator = (Operable& b)     { AssignOpr<SliceAgent<T>>::operatorEq(b);      return *this;}
        SliceAgent<T>& operator = (ull b)           { AssignOpr<SliceAgent<T>>operatorEq(b);        return *this;}
        SliceAgent<T>& operator = (SliceAgent<T>& b){ if (this == &b){return *this;} operatorEq(b); return *this;}

        [[nodiscard]] Slice getAssignSlice()  override { return  Slicable<T>::getSlice(); }

        /** override assign opr*/
        Assignable* getAssignableFromAssignOpr() override{
            return this;
        }

        /** operable override*/
        [[nodiscard]] Operable& getExactOperable() const override { return *(Operable*)_master; }
        [[nodiscard]] Slice getOperableSlice() const override { return  Slicable<T>::getSlice(); }

        Operable* doSlice(Slice sl) override{
            auto x = operator() (sl.start, sl.stop);
            return x.castToOperable();
        }



        /** override simulation*/

        Simulatable*    getSimItf() override{
            return static_cast<Simulatable*>(_master->getSimEngine());
        }
        RtlValItf*      getRtlValItf() override{
            return static_cast<RtlValItf*>(_master->getSimEngine());
        }

        Identifiable* castToIdent() override{
            return static_cast<Identifiable*>(_master);
        }

        ValRep& sv() override{
            assert(false);
        }

        Operable* checkShortCircuit() override{
            return _master->checkShortCircuit();

        }
    };
}

#endif //KATHRYN_SLICABLE_H
