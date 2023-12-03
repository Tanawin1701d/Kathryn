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
        Slice _absSlice{}; /// it is absolute slice
    public:

        explicit Slicable(Slice absSlice) : _absSlice(absSlice){}

        virtual SliceAgent<T>& operator() (int start, int stop) = 0;
        virtual SliceAgent<T>& operator() (int idx) = 0;

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

    };

    /**key point is make the agent that transparent while routing*/
    template<typename T>
    class SliceAgent : public Assignable<SliceAgent<T>>,public Operable, public Slicable<T> {

    private:
        std::shared_ptr<T> _master;
    public:
        SliceAgent(std::shared_ptr<T> master, Slice slc) : _master(master), Slicable<T>(slc) {};

        /** operable override*/

        Operable& getExactOperable() override { return *std::shared_ptr<Operable>(_master); }

        Slice getOperableSlice() override { return  Slicable<T>::getSlice(); }

        /** slicable overload*/

        SliceAgent<T>& operator() (int start, int stop) override{
            auto ret =  std::make_shared<SliceAgent<T>>(_master,
                        Slicable<T>::getNextSlice(start, stop, getOperableSlice())
                    );
            return *ret;
        }

        SliceAgent<T>& operator() (int idx) override{
            auto ret =  std::make_shared<SliceAgent<T>>(_master,
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
