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
#include "model/hwComponent/abstract/logicComp.h"

namespace kathryn{


    class MemBlock;

    /**this class is used to hold only one element in each memblock*/
    class MemBlockEleHolder: public LogicComp<MemBlockEleHolder>{
        friend class MemEleHolderLogicSim;
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

        bool isReadMode() const{return readMode;}

        ValRep& getCurMemVal();
        int     getExactIndexSize();

        /** override assignable (need to call controller)*/
        MemBlockEleHolder& operator <<= (Operable& b) override;
        MemBlockEleHolder& operator <<= (ull       b) override;
        void generateAssMetaForBlocking(Operable& srcOpr,
                                std::vector<AssignMeta*>& resultMetaCollector,
                                Slice  absSrcSlice,
                                Slice  absDesSlice) override;
        MemBlockEleHolder& operator   = (Operable& b) override;
        MemBlockEleHolder& operator   = (ull       b) override;
        void generateAssMetaForNonBlocking(Operable& srcOpr,
                                   std::vector<AssignMeta*>& resultMetaCollector,
                                   Slice  absSrcSlice,
                                   Slice  absDesSlice) override;

        /** Operable*/
//        [[nodiscard]] Slice           getOperableSlice    () const override;
//        [[nodiscard]] Operable&       getExactOperable    () const override;

        Identifiable*   castToIdent         () override;
        ValRep&         sv                  () override;

        /** Slicable*/
        SliceAgent<MemBlockEleHolder>& operator() (int start, int stop) override;
        SliceAgent<MemBlockEleHolder>& operator() (int idx) override;
        Operable* doSlice(Slice sl) override;

        /**Assign call back From Agent */
        MemBlockEleHolder& callBackBlockAssignFromAgent   (Operable& b, Slice absSliceOfHost) override;
        MemBlockEleHolder& callBackNonBlockAssignFromAgent(Operable& b, Slice absSliceOfHost) override;
        void               callBackBlockAssignFromAgent(Operable& srcOpr,
                                                        std::vector<AssignMeta*>& resultMetaCollector,
                                                        Slice  absSrcSlice,
                                                        Slice  absDesSlice) override;
        void               callBackNonBlockAssignFromAgent(Operable& srcOpr,
                                                        std::vector<AssignMeta*>& resultMetaCollector,
                                                        Slice  absSrcSlice,
                                                        Slice  absDesSlice) override;


        Operable* checkShortCircuit   () override{return nullptr;}

        /** debug method to do will will make debug string more delightful*/
        std::string getMdDescribe() override {return Identifiable::getIdentDebugValue();}
        std::string getMdIdentVal() override {return Identifiable::getIdentDebugValue();}

    };

    class MemEleHolderLogicSim: public MemAgentSimEngine{
        MemBlockEleHolder* _master = nullptr;
    public:
        MemEleHolderLogicSim(MemBlockEleHolder* master, int bitWidth);
        /** override simulation engine */
        void simStartCurCycle () override;
        void simStartNextCycle() override;
        bool isReadMode       () override{return _master->readMode;};

    };




}

#endif //KATHRYN_MEMBLOCKAGENT_H
