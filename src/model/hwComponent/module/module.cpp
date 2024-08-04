//
// Created by tanawin on 30/11/2566.
//

#include "module.h"

#include <util/termColor/termColor.h>


#include "model/controller/controller.h"


namespace kathryn{


    Module::Module(bool initComp):
            Identifiable(TYPE_MODULE),
            _moduleSimEngine(new ModuleSimEngine(this)){
        if (initComp)
            com_init();
    }

    Module::~Module() {
        ///deleteSubElement(_stateRegs);
        for(auto& _spReg: _spRegs){
            deleteSubElement(_spReg);
        }
        deleteSubElement(_flowBlockBases);
        deleteSubElement(_bareNodes);
        /** delete user element*/
        deleteSubElement(_userRegs);
        deleteSubElement(_userWires);
        deleteSubElement(_userExpressions);
        deleteSubElement(_userVals);
        deleteSubElement(_userMemBlks);
        deleteSubElement(_userNests);
        deleteSubElement(_userSubModules);
        deleteSubElement(_userBoxs);

        delete _moduleSimEngine;
        delete _moduleGenEngine;

    }

    void Module::com_init() {
        ctrl->on_module_init_components(this);
        /**post finalize component must be handle when object is buit finish*/
    }

    void Module::com_final() {
        /** invoke controller to design end component init*/
        ctrl->on_module_end_init_components(this);
    }

    /**
     *
     * metas data pusher
     *
     * */
    /** todo may be check their are reg in the system */
    void Module::addSpReg(Reg* reg, SP_REG_TYPE spRegType){
        assert(reg != nullptr); /// can't be nullptr
        assert(spRegType < SP_CNT_REG);
        _spRegs[spRegType].push_back(reg);
    }

    void Module::addFlowBlock(FlowBlockBase* fb) {
        assert(fb != nullptr);
        _flowBlockBases.push_back(fb);
    }

    void Module::addNode(Node* node) {
        assert(node != nullptr);
        _bareNodes.push_back(node);
    }

    void Module::addUserReg(Reg* reg) {
        assert(reg != nullptr);
        _userRegs.push_back(reg);
    }

    void Module::addUserWires(Wire* wire) {
        assert(wire != nullptr);
        _userWires.push_back(wire);
    }

    void Module::addUserInputWires (WireIo* wire){
        assert(wire != nullptr);
        assert(wire->getWireIoType() == WIRE_IO_USER_INPUT);
        _userInputWires.push_back(wire);
    }

    void Module::addUserOutputWires(WireIo* wire){
        assert(wire != nullptr);
        assert(wire->getWireIoType() == WIRE_IO_USER_OUPUT);
        _userOutputWires.push_back(wire);
    }

    void Module::addUserExpression(expression* expr) {
        assert(expr != nullptr);
        _userExpressions.push_back(expr);
    }

    void Module::addUserVal(Val* val) {
        assert(val != nullptr);
        _userVals.push_back(val);
    }

    void Module::addUserMemBlk(MemBlock* memBlock) {
        assert(memBlock != nullptr);
        _userMemBlks.push_back(memBlock);
        /** manual*/
    }

    void Module::addUserNest(nest* nst){
        assert(nst != nullptr);
        _userNests.push_back(nst);
    }


    void Module::addUserSubModule(Module* smd){
        assert(smd != nullptr);
        _userSubModules.push_back(smd);
    }

    void Module::addUserBox(Box* box){
        assert(box != nullptr);
        _userBoxs.push_back(box);
    }

    bool Module::isTopModule(){
        return _isTopModule;
    }

    void Module::setTopModule(){
        if (_parent != nullptr){
            std::cout << TC_YELLOW <<"warning the top module have other host module" << TC_DEF << std::endl;
        }
        _isTopModule = true;
    }

    bool Module::isConstModule(){
        return _isConstMod;
    }

    void Module::setConstModule(){
        assert(_isConstMod);
        _isConstMod = true;
    }


    void Module::buildAll(){
        /**declare to model controller that this module is initialize*/
        flow();
        buildFlow();
        /**this ensure that submodule in init component and all is ready to build flow*/
        /** build sub module first*/
        for (auto subMd: _userSubModules){
            ctrl->on_module_init_designFlow(subMd);
            ctrl->on_module_final(subMd);
        }

        /**
         *
         * please note that you cant change order of flow and sub model init design flow
         * */

    }

    void Module::buildFlow(){

        ctrl->tryPurifyFlowStack();
        assert(ctrl->isAllFlowStackEmpty());

        std::vector<NodeWrap*> frontNodeWrap;

        for (auto fb: _flowBlockBases){
            assert(fb != nullptr);
            switch (fb->getJoinFbPol()) {

                case FLOW_JO_SUB_FLOW:
                    /**in case it is normal flow block*/
                    fb->buildHwMaster();
                    frontNodeWrap.push_back(fb->sumarizeBlock());
                    break;
                case FLOW_JO_CON_FLOW:
                    mfAssert(false, "detect con bare block iteration");
                    break;
                case FLOW_JO_EXT_FLOW:
                    /**in case it is extract need flow block*/
                    for (auto node: fb->getBasicNode()){
                        node->dryAssign();
                    }
                    break;
            }

        }
        for (auto nw: frontNodeWrap){
            /** we will have start wire node to start node*/
            nw->addDependNodeToAllNode(startNode);
            nw->assignAllNode();
            /** assume that node wrap that appear to module is not used anymore. */
        }

        /**check short circuit*/
        for(auto expr: _userExpressions){
            expr->startCheckShortCircuit();
        }
        for(auto wire: _userWires){
            wire->startCheckShortCircuit();
        }

    }


    std::string
    Module::getMdDescribe(){

        for (auto fb: _flowBlockBases){
            //logMD(fb->getMdIdentVal(), fb->getMdDescribe());
        }

        return "";

    }

    void Module::createModuleGen(){
        _moduleGenEngine = new ModuleGen(this);
    }


    void Module::addMdLog(MdLogVal *mdLogVal) {
        mdLogVal->addVal("[ " + getMdIdentVal() + " ]");
        for (auto sb : _flowBlockBases){
            auto subLog = mdLogVal->makeNewSubVal();
            sb->addMdLog(subLog);
        }
    }

}