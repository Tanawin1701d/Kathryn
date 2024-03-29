//
// Created by tanawin on 30/11/2566.
//

#ifndef KATHRYN_MODULE_H
#define KATHRYN_MODULE_H

#include <functional>
#include <vector>
#include <cassert>


#include "model/hwComponent/abstract/identifiable.h"
#include "model/hwComponent/register/register.h"
#include "model/hwComponent/wire/wire.h"
#include "model/hwComponent/expression/expression.h"
#include "model/hwComponent/value/value.h"
#include "model/hwComponent/globalComponent/globalComponent.h"
#include "model/hwComponent/memBlock/MemBlock.h"
#include "model/hwComponent/expression/nest.h"

#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/spReg/stateReg.h"
#include "model/flowBlock/abstract/spReg/syncReg.h"
#include "model/flowBlock/abstract/spReg/waitReg.h"
#include "model/flowBlock/abstract/nodes/startNode.h"

#include "moduleSimEngine.h"

#include "model/debugger/modelDebugger.h"
#include "util/logger/logger.h"



namespace kathryn{

    enum MODEL_STAGE{
        MODEL_UNINIT,
        MODEL_GLOB_INITED,
        MODEL_FLOW_INITED
    };

    class ModuleSimEngine;

    class Module : public Identifiable,
                   public HwCompControllerItf,
                   public ModuleSimEngine,
                   public ModelDebuggable,
                   public SimEngineInterface
                   {

    private:
       MODEL_STAGE              _mdStage = MODEL_UNINIT;
        /**all slave object that belong to this elements*/
        /** register that user to represent state*/
        std::vector<Reg*>           _spRegs[SP_CNT_REG]; ////// state/ cond/cycle wait use same ctrlflowRegbase class
        std::vector<FlowBlockBase*> _flowBlockBases;
        std::vector<Node*>       _bareNodes; ////// for the node that did not assign to flowblock and it has already dry assign
        /** user component*/
        std::vector<Reg*>        _userRegs;
        std::vector<Wire*>       _userWires;
        std::vector<expression*> _userExpressions;
        std::vector<Val*>        _userVals;
        std::vector<MemBlock*>   _userMemBlks;
        std::vector<nest*>       _userNests;
        std::vector<Module*>     _userSubModules;

        /** when hardware components require data from outside class
         * the system must handle wire routing while synthesis
         * */
    protected:
        /** communicate to controller*/
        void com_init() override;

        SimEngine* getSimEngine() override{
            return static_cast<SimEngine*>(this);
        }

    public:
        explicit Module(bool initComp = true);
        ~Module() override;

        void com_final() override;

        template<typename T>
        void deleteSubElement(std::vector<T*>& subEleVec){
            for (auto ele: subEleVec){
                delete ele;
            }
        }
        /** logic comp*/
        /**implicit element that is built from design flow*/
        void addSpReg          (Reg* reg, SP_REG_TYPE spRegType);
        void addFlowBlock      (FlowBlockBase* fb);
        void addNode           (Node* node);

        /**explicit element that is buillt from user declaration*/
        void addUserReg        (Reg* reg);
        void addUserWires      (Wire* wire);
        void addUserExpression (expression* expr);
        void addUserVal        (Val* val);
        void addUserMemBlk     (MemBlock* memBlock);
        void addUserNest       (nest* nst);
        void addUserSubModule  (Module* smd);


        /**implicit element that is built from design flow*/
        auto& getSpRegs(SP_REG_TYPE spRegType){
            assert(spRegType < SP_CNT_REG);
            return _spRegs[spRegType];
        }
        auto& getFlowBlocks(){return _flowBlockBases;}
        /**explicit element that is buillt from user declaration*/
        auto& getUserRegs(){return _userRegs; } /** the return contain only master flowblock*/
        auto& getAsmNodes(){return _bareNodes;} /**the return contain only dry assign node*/
        auto& getUserWires(){return _userWires; }
        auto& getUserExpressions(){return _userExpressions; }
        auto& getUserVals(){return _userVals; }
        auto& getUserMemBlks(){return _userMemBlks;}
        auto& getUserNests(){return _userNests;}
        auto& getUserSubModules(){return _userSubModules;}


        /** Functions which allow user to custom  their module design flow*/
        MODEL_STAGE  getStage(){return _mdStage;}
        void         setStage(MODEL_STAGE md_stage){_mdStage = md_stage;}
        void         buildAll();
        virtual void flow(){}; //// user must inherit this function to build thier flow
        virtual void buildFlow();
        /** model debug*/
        [[maybe_unused]]
        std::string getMdDescribe() override;
        void        addMdLog(MdLogVal* mdLogVal) override;
        std::string getMdIdentVal() override{return getIdentDebugValue();};

        /** override simulation */
        void beforePrepareSim(VcdWriter* vcdWriter, FlowColEle* flowColEle) override;
        void prepareSim() override;
        void simStartCurCycle() override;
        void simStartNextCycle() override;
        void curCycleCollectData() override;
        void simExitCurCycle() override;

    protected:
        /** must able to get sim and cast to LogicSim Engine*/
        template<typename T>
        void beforePrepareSimSubElement_RTL_only(std::vector<T*>& subEleVec, VcdWriter* writer);
        void beforePrepareSimSubElement_FB_only(std::vector<FlowBlockBase*>& subEleVec, FlowColEle* flowColEle);
        /** must able to get sim*/
        template<typename T>
        void prepareSimSubElement               (std::vector<T*>& subEleVec);
        template<typename T>
        void simStartCurSubElement              (std::vector<T*>& subEleVec);
        template<typename T>
        void simStartNextSubElement             (std::vector<T*>& subEleVec);
        template<typename T>
        void curCollectData                     (std::vector<T*>& subEleVec);
        template<typename T>
        void simExitSubElement                  (std::vector<T*>& subEleVec);


    };



}

#endif //KATHRYN_MODULE_H
