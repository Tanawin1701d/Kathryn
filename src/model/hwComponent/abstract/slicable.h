//
// Created by tanawin on 2/12/2566.
//

#ifndef KATHRYN_SLICABLE_H
#define KATHRYN_SLICABLE_H

#include "model/hwComponent/abstract/assignable.h"
#include "model/hwComponent/abstract/operable.h"

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
        ~Slicable(){
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

        /** utility*/
        static bool checkValidSlice(Slice x){
            return x.checkValidSlice();
        }
        Slice getNextSlice(int start, int stop, Slice oldSlice){
            /** todo next check integrity*/
            if (oldSlice.isEntireSection()){
                return Slice{start, stop};
            }
                return Slice{oldSlice.start + start, oldSlice.start + stop};
        }

        Slice getSlice(){ return _absSlice; }
        void  setSlice(Slice slc) {_absSlice =  slc;}

    };

    /**key point is make the agent that transparent while routing
     * assignable for updateMeta in assignable in agent is not used*/
    template<typename T>
    class SliceAgent : public Assignable<SliceAgent<T>>,public Operable, public Slicable<T> {

    private:
        T* _master;
    public:
        SliceAgent(T* master, Slice slc) : _master(master), Slicable<T>(slc) {
            _master->Slicable<T>::addAgentHolder(this);
        };

        /** operable override*/

        Operable& getExactOperable() override { return *std::shared_ptr<Operable>(_master); }

        Slice getOperableSlice() override { return  Slicable<T>::getSlice(); }

        /** slicable overload*/

        SliceAgent<T>& operator() (int start, int stop) override{
            auto ret =  new SliceAgent<T>(_master,
                        Slicable<T>::getNextSlice(start, stop, getOperableSlice())
                    );
            return *ret;
        }

        SliceAgent<T>& operator() (int idx) override{
            auto ret =  new SliceAgent<T>(_master,
                        Slicable<T>::getNextSlice(idx, idx+1, getOperableSlice())
                    );
            return *ret;
        }

        /** override assignable*/
        SliceAgent<T>& operator <<= (Operable& b) override {
             _master->callBackBlockAssignFromAgent(
                b,
                Slicable<T>::getSlice());

             return *this;
        }

        SliceAgent<T>& operator = (Operable& b) override {
            _master->callBackNonBlockAssignFromAgent(
                    b,
                    Slicable<T>::getSlice()
            );
            return *this;
        }


    };
}

#endif //KATHRYN_SLICABLE_H
