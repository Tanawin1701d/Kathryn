//
// Created by tanawin on 30/11/2566.
//

#include "module.h"

#include "model/controller/controller.h"


namespace kathryn{

    Module::Module(): Identifiable(TYPE_MODULE),
                      HwCompControllerItf()
    {
        /**todo we will upgrade to control interface class but reg/wire did't tend to use make*/
        assert(!ctrl->isAllocationLock());
        ctrl->lockAllocation();
        com_init();
    }

    Module::~Module() {
        deleteSubElement(_stateRegs);
        deleteSubElement(_flowBlockBases);
        deleteSubElement(_userRegs);
        deleteSubElement(_userWires);
        deleteSubElement(_userExpressions);
        deleteSubElement(_userVals);
        deleteSubElement(_userSubModule);

    }

    void Module::com_init() {
        ctrl->on_module_init_components(this);
        /**post finalize component must be handle when object is buit finish*/
    }

    void Module::com_final() {
        /** invoke controller for design flow acknowledgement*/
        ctrl->on_module_init_designFlow(this);
        /** fix slave elements to belong to this module*/
        ctrl->on_module_final(this);
    }

    template<typename T>
    void Module::localizeSlaveVector(std::vector<T> &_vec) {
        for (size_t i = 0; i < _vec.size(); i++){
            _vec[i]->setLocalId((ull)i);
            _vec[i]->setParent(this);
        }
    }

    void Module::localizeSlaveElements() {
        /** state reg and user reg used same local ID sequence*/
        localizeSlaveVector(_stateRegs);
        localizeSlaveVector(_userRegs);
        localizeSlaveVector(_userWires);
        localizeSlaveVector(_userExpressions);
        localizeSlaveVector(_userVals);
        localizeSlaveVector(_userSubModule);
    }

    /**
     *
     * meta data pusher
     *
     * */
    /** todo may be check their are reg in the system */
    void Module::addStateReg(Reg* reg) {
        assert(reg != nullptr); /// can't be nullptr
        _stateRegs.push_back(reg);

    }

    void Module::addFlowBlock(FlowBlockBase* fb) {
        assert(fb != nullptr);
        _flowBlockBases.push_back(fb);
    }

    void Module::addUserReg(Reg* reg) {
        assert(reg != nullptr);
        _userRegs.push_back(reg);
    }

    void Module::addUserWires(Wire* wire) {
        assert(wire != nullptr);
        _userWires.push_back(wire);
    }

    void Module::addUserExpression(expression* expr) {
        assert(expr != nullptr);
        _userExpressions.push_back(expr);
    }

    void Module::addUserVal(Val* val) {
        assert(val != nullptr);
        _userVals.push_back(val);
    }

    void Module::addUserSubModule(Module* smd) {
        assert(smd != nullptr);
        _userSubModule.push_back(smd);
    }

}