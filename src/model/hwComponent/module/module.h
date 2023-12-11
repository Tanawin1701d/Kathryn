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
#include "model/FlowBlock/abstract/stateReg.h"


namespace kathryn{


    extern Wire* startWire;

    class Module : public Identifiable, public HwCompControllerItf{
        /** todo we must make this class com_init with stack
         *  for now we assume that every register is constructed when
         *  systems are in initiating process
         *  but In the future we will make it initiat in design flow
         *
         * */
    private:
        /**all slave object that belong to this elements*/
        /** register that user to represent state*/
        std::vector<StateReg*>        _stateRegs;
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
        explicit Module();
        ~Module();
        template<typename T>
        void deleteSubElement(std::vector<T*> subEleVec){
            for (auto ele: subEleVec){
                delete ele;
            }
        }

        void com_final() override;

        void addStateReg      (StateReg* reg);
        void addFlowBlock     (FlowBlockBase* fb);
        void addUserReg       (Reg* reg);
        void addUserWires     (Wire* wire);
        void addUserExpression(expression* expr);
        void addUserVal       (Val* val);
        void addUserSubModule (Module* smd);


        auto& getStateRegs(){return _stateRegs;}
        auto& getFlowBlocks(){return _flowBlockBases;}
        auto& getUserRegs(){return _userRegs; }
        auto& getUserWires(){return _userWires; }
        auto& getUserExpressions(){return _userExpressions; }
        auto& getUserVals(){return _userVals; }
        auto& getUserSubModules(){return _userSubModule; }
        /** This allow user to custom module design flow*/
        virtual void flow(){};
        virtual void buildHardware();

    };

}

#endif //KATHRYN_MODULE_H
