//
// Created by tanawin on 30/11/2566.
//

#ifndef KATHRYN_MODULE_H
#define KATHRYN_MODULE_H

#include <functional>
#include <vector>
#include <cassert>


#include "gen/proxyHwComp/module/moduleGen.h"
#include "model/hwComponent/globalComponent/globalComponent.h"


#include "model/hwComponent/abstract/identifiable.h"
#include "model/hwComponent/register/register.h"
#include "model/hwComponent/wire/wire.h"
#include "model/hwComponent/expression/expression.h"
#include "model/hwComponent/value/value.h"
#include "model/hwComponent/memBlock/MemBlock.h"
#include "model/hwComponent/expression/nest.h"
#include "model/hwComponent/box/box.h"

#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/spReg/waitReg.h"
#include "model/flowBlock/abstract/nodes/startNode.h"

#include "model/debugger/modelDebugger.h"
#include "model/simIntf/hwComponent/moduleSimEngine.h"
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
                   public ModelDebuggable,
                   public ModuleSimEngineInterface,
                   public ModuleGenInterface
                   {

    private:
       MODEL_STAGE              _mdStage = MODEL_UNINIT;

        /**all slave object that belong to this elements*/
        /** register that user to represent state*/
        std::vector<Reg*>           _spRegs[SP_CNT_REG]; ////// state/ cond/cycle wait use same ctrlflowRegbase class
        std::vector<FlowBlockBase*> _flowBlockBases; //// it contain only head of flowblock in module

        std::vector<Node*>       _bareNodes; ////// for the node that did not assign to flowblock and it has already dry assign
        /** user component*/
        std::vector<Reg*>        _userRegs;
        std::vector<Wire*>       _userWires;
        std::vector<expression*> _userExpressions;
        std::vector<Val*>        _userVals;
        std::vector<MemBlock*>   _userMemBlks;
        std::vector<nest*>       _userNests;
        std::vector<Module*>     _userSubModules;
        std::vector<Box*>        _userBoxs; //// it contain only head of box in module

        ModuleSimEngine*         _moduleSimEngine = nullptr;
        ModuleGen*               _moduleGenEngine = nullptr;


        /** when hardware components require data from outside class
         * the system must handle wire routing while synthesis
         * */
    protected:
        /** communicate to controller*/
        void com_init() override;

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
        void addUserBox        (Box* box);


        /**implicit element that is built from design flow*/
        auto& getSpRegs(SP_REG_TYPE spRegType){
            assert(spRegType < SP_CNT_REG);
            return _spRegs[spRegType];
        }
        auto& getFlowBlocks(){return _flowBlockBases;}
        auto& getAsmNodes(){return _bareNodes;} /**the return contain only dry assign node*/
       /**explicit element that is buillt from user declaration*/
        auto& getUserRegs(){return _userRegs; } /** the return contain only master flowblock*/
        auto& getUserWires(){return _userWires; }
        auto& getUserExpressions(){return _userExpressions; }
        auto& getUserVals(){return _userVals; }
        auto& getUserMemBlks(){return _userMemBlks;}
        auto& getUserNests(){return _userNests;}
        auto& getUserSubModules(){return _userSubModules;}
        auto& getBoxs(){return _userBoxs;}


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

        ModuleSimEngine* getSimEngine() override{
            return _moduleSimEngine;
        }

        void createModuleGen() override;

        ModuleGen* getModuleGen() override{
            return _moduleGenEngine;
        }



    };



}

#endif //KATHRYN_MODULE_H
