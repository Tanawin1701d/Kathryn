//
// Created by tanawin on 30/11/2566.
//

#include "module.h"

#include "util/termColor/termColor.h"


#include "model/controller/controller.h"


namespace kathryn{


    Module::Module(bool initComp):
            Identifiable(TYPE_MODULE),
            _moduleSimEngine(new ModuleSimEngine(this)){
        if (initComp)
            comInit();
    }

    Module::~Module() {
        ///deleteSubElement(_stateRegs);
        for(auto& _spReg: _spRegs){
            deleteSubElement(_spReg);
        }
        deleteSubElement(_flowBlockBases);
        /** delete user element*/
        deleteSubElement(_userRegs);
        deleteSubElement(_userWires);
        deleteSubElement(_userExpressions);
        deleteSubElement(_userVals);
        deleteSubElement(_userPmVals);
        deleteSubElement(_userMemBlks);
        deleteSubElement(_userNests);
        deleteSubElement(_userSubModules);
        deleteSubElement(_userBoxs);

        delete _moduleSimEngine;
        delete _moduleGenEngine;

    }

    void Module::comInit() {
        ctrl->onModuleInitComponents(this);
        /**post finalize component must be handle when object is buit finish*/
    }

    void Module::comFinal() {
        /** invoke controller to design end component init*/
        ctrl->onModuleEndInitComponents(this);
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
        _spRegs[spRegType].pushBack(reg);
    }

    void Module::addFlowBlock(FlowBlockBase* fb) {
        assert(fb != nullptr);
        _flowBlockBases.pushBack(fb);
    }

    void Module::addUserReg(Reg* reg) {
        assert(reg != nullptr);
        _userRegs.pushBack(reg);
    }

    void Module::addUserWires(Wire* wire) {
        assert(wire != nullptr);
        _userWires.pushBack(wire);
    }

    void Module::addUserExpression(expression* expr) {
        assert(expr != nullptr);
        _userExpressions.pushBack(expr);
    }

    void Module::addUserVal(Val* val) {
        assert(val != nullptr);
        _userVals.pushBack(val);
    }

    void Module::addUserPmVal(PmVal* pmVal){
        assert(pmVal != nullptr);
        _userPmVals.pushBack(pmVal);
    }


    void Module::addUserMemBlk(MemBlock* memBlock) {
        assert(memBlock != nullptr);
        _userMemBlks.pushBack(memBlock);
        /** manual*/
    }

    void Module::addUserNest(nest* nst){
        assert(nst != nullptr);
        _userNests.pushBack(nst);
    }


    void Module::addUserSubModule(Module* smd){
        assert(smd != nullptr);
        _userSubModules.pushBack(smd);
    }

    void Module::addUserBox(Box* box){
        assert(box != nullptr);
        _userBoxs.pushBack(box);
    }

    void Module::addUserItf(ModelInterface* itf){
        assert(itf != nullptr);
        _userItfs.pushBack(itf);
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

    std::vector<Wire*>
    Module::getUserWiresByMarker(WIRE_MARKER_TYPE wmt){
        std::vector<Wire*> result;
        for (Wire* wire: getUserWires()){
            assert(wire != nullptr);
            if (wire->getMarker() == wmt){
                result.pushBack(wire);
            }
        }
        return result;
    }


    void Module::buildAll(){
        /**declare to model controller that this module is initialize*/
        flow();
        for (ModelInterface* itf: getItfs()){
            itf->buildLogicBase();
        }
        /** clear everything left in flowblock stack*/
        ctrl->tryPurifyFlowStack();
        assert(ctrl->isAllFlowStackEmpty());

        /**this ensure that submodule in init component and all is ready to build flow*/
        /** build sub module first*/
        for (auto subMd: _userSubModules){
            ctrl->on_module_init_designFlow(subMd);
            ctrl->onModuleFinal(subMd);
        }
        buildFlow();

        /**
         *
         * please note that you cant change order of flow and sub model init design flow
         * */

    }

    void Module::buildFlow(){

        std::vector<NodeWrap*> frontNodeWrap;

        if (getGlobalId() == 8){
            std::cout << "start build flow of a" << std::endl;
        }

        for (auto fb: _flowBlockBases){
            assert(fb != nullptr);
            switch (fb->getJoinFbPol()) {

                case FLOW_JO_SUB_FLOW:
                    /**in case it is normal flow block*/
                    fb->buildHwMaster();
                    frontNodeWrap.pushBack(fb->sumarizeBlock());
                    break;
                case FLOW_JO_CON_FLOW:
                    mfAssert(false, "detect con bare block iteration");
                    break;
            case FLOW_JO_EXT_FLOW:
                    /**in case it is extract need flow block*/
                    std::vector<AsmNode*> extractedAsmNode = fb->extract();
                    for (auto node: extractedAsmNode){
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
            //logMd(fb->getMdIdentVal(), fb->getMdDescribe());
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