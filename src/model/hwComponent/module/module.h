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
        std::vector<RegPtr>        _stateRegs;
        /** user component*/
        std::vector<RegPtr>        _userRegs;
        std::vector<WirePtr>       _userWires;
        std::vector<expressionPtr> _userExpressions;
        std::vector<ValPtr>        _userVals;
        std::vector<ModulePtr>     _userSubModule;

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

        void addStateReg(const RegPtr& reg);
        void addUserReg(const RegPtr& reg);
        void addUserWires(const WirePtr& wire);
        void addUserExpression(const expressionPtr& expr);
        void addUserVal(const ValPtr& val);
        void addUserSubModule(const ModulePtr& smd);
        /** This allow user to interact with system*/
        virtual void flow() = 0;

    };




}

#endif //KATHRYN_MODULE_H
