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


namespace kathryn{

    class Module;

    typedef std::shared_ptr<Module> ModulePtr;

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
        std::vector<Reg*>        _stateRegs;
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
        void com_final() override;
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

        void addStateReg      (Reg* reg);
        void addFlowBlock     (FlowBlockBase* fb);
        void addUserReg       (Reg* reg);
        void addUserWires     (Wire* wire);
        void addUserExpression(expression* expr);
        void addUserVal       (Val* val);
        void addUserSubModule (Module* smd);
        /** This allow user to custom module design flow*/
        virtual void flow() = 0;

    };




}

#endif //KATHRYN_MODULE_H
