//
// Created by tanawin on 30/11/2566.
//

#include "module.h"

#include "model/controller/controller.h"
#include "model/hwComponent/abstract/makeComponent.h"


namespace kathryn{

    Wire* startWire = &_make<Wire>( "startWire", 1);

    Module::Module(bool initComp): Identifiable(TYPE_MODULE),
                      HwCompControllerItf()
    {
        if (initComp)
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
     * metas data pusher
     *
     * */
    /** todo may be check their are reg in the system */
    void Module::addStateReg(StateReg* reg) {
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

    void Module::buildFlow() {
        /** build all hardware to flowBlock*/
        /** every flowblock will auto build when block is detach*/
        /** create nodewrap of all flowblock*/
        /** may be if block is top flow we must clear the stack*/
        ctrl->purifyFlowStack();
        std::vector<NodeWrap*> frontNodeWrap;

        for (auto fb: _flowBlockBases){
            frontNodeWrap.push_back(fb->sumarizeBlock());
        }
        for (auto nw: frontNodeWrap){
            nw->addDependStateToAllNode(startWire, BITWISE_AND);
            nw->assignAllNode();
            /** assume that node wrap that appear to module is not used anymore. */
            nw->deleteNodesInWrap();
            delete nw;
        }

    }


}