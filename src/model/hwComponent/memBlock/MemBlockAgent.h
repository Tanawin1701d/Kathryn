//
// Created by tanawin on 19/2/2567.
//

#ifndef KATHRYN_MEMBLOCKAGENT_H
#define KATHRYN_MEMBLOCKAGENT_H


#include "model/hwComponent/abstract/operable.h"
#include "model/hwComponent/abstract/assignable.h"
#include "model/hwComponent/abstract/slicable.h"
#include "model/simIntf/memSimInterface.h"
#include "model/debugger/modelDebugger.h"
#include "model/controller/conInterf/controllerItf.h"

namespace kathryn{


    class MemBlock;

    /**this class is used to hold only one element in each memblock*/
    class MemBlockEleHolder: public Assignable<MemBlockEleHolder>,
                             public Operable,
                             public Slicable<MemBlockEleHolder>,
                             public AssignCallbackFromAgent<MemBlockEleHolder>,
                             public Identifiable,
                             public HwCompControllerItf,
                             public MemAgentSimulatable,
                             public ModelDebuggable{
    private:
        ///bool setModeYet = false; /**the goal of setMode is to prevent duplicate read write in the same index*/
        bool readMode = false; /**Therefore, we should know that if it did not set mode it may be read mode*/

        MemBlock* _master  = nullptr;
        Operable* _indexer = nullptr;

    protected:
        void setReadMode (){readMode = true; }
        void setWriteMode(){readMode = false;}
        ////[[nodiscard]] bool isSetMode   () const{return setModeYet;}

    public:
        explicit MemBlockEleHolder(MemBlock* master, const Operable* indexer);
        explicit MemBlockEleHolder(MemBlock* master, int idx);

        void com_init() override{};
        void com_final() override{};

        ValRep& getCurMemVal();
        int     getExactIndexSize();

        /** override assignable (need to call controller)*/
        MemBlockEleHolder& operator <<= (Operable& b) override;
        MemBlockEleHolder& operator <<= (ull       b) override;
        MemBlockEleHolder& operator   = (Operable& b) override;
        MemBlockEleHolder& operator   = (ull       b) override;
        Slice getAssignSlice() override{ return Slicable<MemBlockEleHolder>::getSlice();}

        /** Operable*/
        [[nodiscard]] Slice           getOperableSlice    () const override;
        [[nodiscard]] Operable&       getExactOperable    () const override;

        Simulatable*    getSimItf()    override {return this;}
        RtlValItf*      getRtlValItf() override {return this;}

        Identifiable*   castToIdent         () override;
        ValRep&         sv                  () override;

        /** Slicable*/
        SliceAgent<MemBlockEleHolder>& operator() (int start, int stop) override;
        SliceAgent<MemBlockEleHolder>& operator() (int idx) override;

        /**Assign call back From Agent */
        MemBlockEleHolder& callBackBlockAssignFromAgent   (Operable& b, Slice absSliceOfHost) override;
        MemBlockEleHolder& callBackNonBlockAssignFromAgent(Operable& b, Slice absSliceOfHost) override;

        /**Rtl simulatable we cut many feature to make it be proxy*/
        void simStartCurCycle   () override;
        void simStartNextCycle  () override;
        bool isReadMode         () override{return readMode;};


        Operable* checkShortCircuit   () override{return nullptr;}

        /** debug method to do will will make debug string more delightful*/
        std::string getMdDescribe() override {return Identifiable::getIdentDebugValue();}
        std::string getMdIdentVal() override {return Identifiable::getIdentDebugValue();}

    };




}

#endif //KATHRYN_MEMBLOCKAGENT_H
