//
// Created by tanawin on 19/2/2567.
//

#ifndef KATHRYN_MEMBLOCKAGENT_H
#define KATHRYN_MEMBLOCKAGENT_H


#include "model/hwComponent/abstract/operable.h"
#include "model/hwComponent/abstract/assignable.h"
#include "model/hwComponent/abstract/slicable.h"
#include "sim/modelSimEngine/hwComponent/abstract/logicSimEngine.h"
#include "model/hwComponent/abstract/logicComp.h"
#include "gen/proxyHwComp/memBlock/memAgentGen.h"

namespace kathryn{


    class MemBlock;

    /**this class is used to hold only one element in each memblock*/
    class MemBlockEleHolder: public LogicComp<MemBlockEleHolder>{
        friend class MemEleHolderSimEngine;
        friend class MemEleholderGen;
    private:
        ///bool setModeYet = false; /**the goal of setMode is to prevent duplicate read write in the same index*/
        bool readMode = true; /**Therefore, we should know that if it did not set mode it may be read mode*/

        MemBlock* _master  = nullptr;
        Operable* _indexer = nullptr;

    protected:
        void setReadMode (){readMode = true; }
        void setWriteMode(){readMode = false;}

        ////[[nodiscard]] bool isSetMode   () const{return setModeYet;}

    public:
        explicit MemBlockEleHolder(MemBlock* master, const Operable* indexer);
        explicit MemBlockEleHolder(MemBlock* master, int idx);

        void com_init () override{};
        void com_final() override{};

        bool isReadMode () const{return readMode;}
        bool isWriteMode() const{return !readMode;}

        int     getExactIndexSize();

        /** override assignable (need to call controller)*/
        void doBlockAsm (Operable& srcOpr, Slice desSlice) override;
        void doNonBlockAsm(Operable& srcOpr, Slice desSlice) override;
        void doGlobalAsm(Operable& srcOpr, Slice desSlice, ASM_TYPE asmType) override;

        void doBlockAsm(Operable& srcOpr,
                        std::vector<AssignMeta*>& resultMetaCollector,
                        Slice  absSrcSlice,
                        Slice  absDesSlice) override;

        void doNonBlockAsm(Operable& srcOpr,
                           std::vector<AssignMeta*>& resultMetaCollector,
                           Slice  absSrcSlice,
                           Slice  absDesSlice) override;

        MemBlockEleHolder& operator = (Operable& b)         { operatorEq(b);                                return *this;}
        MemBlockEleHolder& operator = (ull b)               { operatorEq(b);                                   return *this;}
        MemBlockEleHolder& operator = (MemBlockEleHolder& b){ if (this == &b){return *this;} operatorEq(b); return *this;}

        /** Operable*/
        Identifiable*   castToIdent         () override;

        /** Slicable*/
        SliceAgent<MemBlockEleHolder>& operator() (int start, int stop) override;
        SliceAgent<MemBlockEleHolder>& operator() (int idx) override;
        SliceAgent<MemBlockEleHolder>& operator() (Slice sl) override;
        Operable* doSlice(Slice sl) override;
        Operable* checkShortCircuit   () override{return nullptr;}

        /** debug method to do will will make debug string more delightful*/
        std::string getMdDescribe() override {return Identifiable::getIdentDebugValue();}
        std::string getMdIdentVal() override {return Identifiable::getIdentDebugValue();}

        Operable*   getIndexer(){ assert(_indexer != nullptr); return _indexer;}
        MemBlock*   getMasterMemBlk(){assert(_master != nullptr); return _master;}

        /**override logicc gen base*/
        void createLogicGen() override;

    };






}

#endif //KATHRYN_MEMBLOCKAGENT_H
