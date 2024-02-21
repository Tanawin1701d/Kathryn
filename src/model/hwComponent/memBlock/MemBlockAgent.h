//
// Created by tanawin on 19/2/2567.
//

#ifndef KATHRYN_MEMBLOCKAGENT_H
#define KATHRYN_MEMBLOCKAGENT_H


#include "model/hwComponent/abstract/operable.h"
#include "model/hwComponent/abstract/assignable.h"
#include "model/hwComponent/abstract/slicable.h"
#include "model/simIntf/memSimInterface.h"

namespace kathryn{


    class MemBlock;

    /**this class is used to hold only one element in each memblock*/
    class MemBlockEleHolder: public Assignable<MemBlockEleHolder>,
                             public Operable,
                             public Slicable<MemBlockEleHolder>,
                             public AssignCallbackFromAgent<MemBlockEleHolder>,
                             public Identifiable,
                             public MemAgentSimulatable
     {
    private:
        bool setModeYet = false; /**the goal of setMode is to prevent duplicate read write in the same index*/
        bool readMode = false; /**Therefore, we should know that if it did not set mode it may be read mode*/

        MemBlock* _master  = nullptr;
        Operable* _indexer = nullptr;

    protected:
        bool setReadMode (){setModeYet = true; readMode = true; }
        bool setWriteMode(){setModeYet = true; readMode = false;}
        bool isSetMode   (){return setModeYet;}

    public:
        explicit MemBlockEleHolder(MemBlock* master, const Operable* indexer);
        explicit MemBlockEleHolder(MemBlock* master, const int idx);

        ValRep& getCurMemVal();

        /** override assignable (need to call controller)*/
        MemBlockEleHolder& operator <<= (Operable& b) override;
        MemBlockEleHolder& operator   = (Operable& b) override;

        /** Operable*/
        Slice           getOperableSlice    () const override;
        Operable&       getExactOperable    () const override;

        Simulatable*    getSimItf()    override {return this;}
        RtlValItf*      getRtlValItf() override {return this;}


        Identifiable*   castToIdent         () override;
        ValRep&         sv                  () override;

        /** Slicable*/
        SliceAgent<MemBlockEleHolder>& operator() (int start, int stop) override;
        SliceAgent<MemBlockEleHolder>& operator() (int idx) override;

        /**Assign call back From Agent */
        MemBlockEleHolder& callBackBlockAssignFromAgent   (Operable& b, Slice absSliceOfHost);
        MemBlockEleHolder& callBackNonBlockAssignFromAgent(Operable& b, Slice absSliceOfHost);

        /**Rtl simulatable we cut many feature to make it be proxy*/
        void prepareSim         () override;
        void simStartCurCycle   () override;
        void simStartNextCycle  () override;
        bool isReadMode         () override{return readMode;};


        Operable* checkShortCircuit   () override{return nullptr;}


    };




}

#endif //KATHRYN_MEMBLOCKAGENT_H
