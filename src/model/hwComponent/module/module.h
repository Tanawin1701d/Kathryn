//
// Created by tanawin on 30/11/2566.
//

#ifndef KATHRYN_MODULE_H
#define KATHRYN_MODULE_H

#include <vector>
#include <cassert>


#include "model/hwComponent/abstract/identifiable.h"
#include "model/hwComponent/register/register.h"
#include "model/hwComponent/wire/wire.h"
#include "model/hwComponent/expression/expression.h"
#include "model/hwComponent/value/value.h"

#include "model/FlowBlock/abstract/flowBlock_Base.h"
#include "model/FlowBlock/abstract/spReg/stateReg.h"
#include "model/FlowBlock/abstract/spReg/syncReg.h"
#include "model/FlowBlock/abstract/spReg/waitReg.h"
#include "model/FlowBlock/abstract/nodes/startNode.h"

#include "model/debugger/modelDebugger.h"
#include "util/logger/logger.h"


namespace kathryn{


    extern Wire* rstWire;
    extern StartNode* startNode;

    class Module : public Identifiable,
                   public HwCompControllerItf,
                   public FlowSimulatable,
                   public ModelDebuggable{

    private:
        /**all slave object that belong to this elements*/
        /** register that user to represent state*/
        std::vector<Reg*>           _spRegs[SP_CNT_REG]; ////// state/ cond/cycle wait use same ctrlflowRegbase class
        std::vector<FlowBlockBase*> _flowBlockBases;
        /** user component*/
        std::vector<Reg*>        _userRegs;
        std::vector<Wire*>       _userWires;
        std::vector<expression*> _userExpressions;
        std::vector<Val*>        _userVals;
        std::vector<Module*>     _userSubModule;

        /** when hardware components require data from outside class
         * the system must handle wire routing while synthesis
         * */
    protected:
        /** communicate to controller*/
        void com_init() override;
        /** localize slave element to belong to this node*/
        template<typename T>
        void localizeSlaveVector(std::vector<T>& _vec);
        void localizeSlaveElements();


    public:
        explicit Module(bool initComp = true);
        ~Module();
        template<typename T>
        void deleteSubElement(std::vector<T*>& subEleVec){
            for (auto ele: subEleVec){
                delete ele;
            }
        }

        template<typename T>
        void simStartSubElement(std::vector<T*>& subEleVec){
            for (auto ele: subEleVec){
                ele->simStartCurCycle();
            }
        }

        template<typename T>
        void simExitSubElement(std::vector<T*>& subEleVec){
            for (auto ele: subEleVec){
                ele->simExitCurCycle();
            }
        }

        void com_final() override;

        /**implicit element that is built from design flow*/
        void addSpReg          (Reg* reg, SP_REG_TYPE spRegType);
        void addFlowBlock      (FlowBlockBase* fb);

        /**explicit element that is buillt from user declaration*/
        void addUserReg        (Reg* reg);
        void addUserWires      (Wire* wire);
        void addUserExpression (expression* expr);
        void addUserVal        (Val* val);
        void addUserSubModule  (Module* smd);

        /**implicit element that is built from design flow*/
        auto& getSpRegs(SP_REG_TYPE spRegType){
            assert(spRegType < SP_CNT_REG);
            return _spRegs[spRegType];
        }
        auto& getFlowBlocks(){return _flowBlockBases;}

        /**explicit element that is buillt from user declaration*/
        auto& getUserRegs(){return _userRegs; }
        auto& getUserWires(){return _userWires; }
        auto& getUserExpressions(){return _userExpressions; }
        auto& getUserVals(){return _userVals; }
        auto& getUserSubModules(){return _userSubModule; }

        /** Functions which allow user to custom  their module design flow*/
        virtual void flow(){}; //// user must inherit this function to build thier flow
        virtual void buildFlow();

        [[maybe_unused]]
        std::string getMdDescribe() override;
        void addMdLog(MdLogVal* mdLogVal) override;
        std::string getMdIdentVal() override{return getIdentDebugValue();};

        /** override simulation */
        void simStartCurCycle() override;
        void simExitCurCycle() override;

    };

}

#endif //KATHRYN_MODULE_H
